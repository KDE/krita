/* This file is part of the KDE project
 * Copyright 2010 (C) Boudewijn Rempt <boud@valdyas.org>
 * Copyright 2011 (C) Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kis_scratch_pad.h"

#include <QApplication>
#include <QScreen>
#include <QMutex>
#include <QMutexLocker>
#include <QWheelEvent>
#include <QPaintEvent>

#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>
#include <KoPointerEvent.h>
#include <resources/KoAbstractGradient.h>

#include <KisPortingUtils.h>

#include <kis_cursor.h>
#include <kis_tool_utils.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_gradient_painter.h>
#include <kis_fill_painter.h>
#include <kis_default_bounds.h>
#include <kis_canvas_resource_provider.h>
#include <KisPortingUtils.h>

#include "kis_config.h"
#include "kis_image.h"
#include "kis_undo_stores.h"
#include "kis_update_scheduler.h"
#include "kis_post_execution_undo_adapter.h"
#include "kis_scratch_pad_event_filter.h"
#include "kis_painting_information_builder.h"
#include "kis_tool_freehand_helper.h"
#include "kis_image_patch.h"
#include "kis_canvas_widget_base.h"
#include "kis_layer_projection_plane.h"
#include "kis_node_graph_listener.h"
#include "kis_transaction.h"
#include "kis_algebra_2d.h"
#include <KisAdaptedLock.h>
#include <kis_async_action_feedback.h>
#include <KisScreenMigrationTracker.h>
#include <kis_config_notifier.h>

namespace {
class KisUpdateSchedulerLockAdapter
{
public:
    KisUpdateSchedulerLockAdapter(KisUpdateScheduler *scheduler)
        : m_scheduler(scheduler)
    {
    }

    void lock() {
        m_scheduler->barrierLock();
    }

    bool try_lock() {
        return m_scheduler->tryBarrierLock();
    }

    void unlock() {
        m_scheduler->unlock();
    }

private:
    KisUpdateScheduler *m_scheduler;
};

/**
 * Define an adapted lock that has application-wide busy-wait feedback
 */
KIS_DECLARE_ADAPTED_LOCK(KisUpdateSchedulerLockWithFeedback,
                         KisAsyncActionFeedback::MutexWrapper<KisUpdateSchedulerLockAdapter>)

}


class KisScratchPadNodeListener : public KisNodeGraphListener
{
public:
    KisScratchPadNodeListener(KisScratchPad *scratchPad)
        : m_scratchPad(scratchPad)
    {
    }

    void requestProjectionUpdate(KisNode *node, const QVector<QRect> &rects, KisProjectionUpdateFlags flags) override {
        KisNodeGraphListener::requestProjectionUpdate(node, rects, flags);

        QMutexLocker locker(&m_lock);

        Q_FOREACH (const QRect &rc, rects) {
            m_scratchPad->imageUpdated(rc);
        }
    }

private:
    KisScratchPad *m_scratchPad;
    QMutex m_lock;
};


class KisScratchPadDefaultBounds : public KisDefaultBounds
{
public:

    KisScratchPadDefaultBounds(KisScratchPad *scratchPad)
        : m_scratchPad(scratchPad)
    {
    }

    ~KisScratchPadDefaultBounds() override {}

    QRect bounds() const override {
        return m_scratchPad->imageBounds();
    }

    void * sourceCookie() const override {
        return m_scratchPad;
    }

private:
    Q_DISABLE_COPY(KisScratchPadDefaultBounds)

    KisScratchPad *m_scratchPad;
};

class KisScratchPadPaintingInformationBuilder : public KisPaintingInformationBuilder
{
    Q_OBJECT

public:
    KisScratchPadPaintingInformationBuilder(KisScratchPad *scratchPad)
        : m_scratchPad(scratchPad)
    {
    }

protected:
    QPointF imageToView(const QPointF &point) override {
        return m_scratchPad->documentToWidget().map(point);
    }

private:
    const KisScratchPad *m_scratchPad;
};


KisScratchPad::KisScratchPad(QWidget *parent)
    : QWidget(parent)
    , m_toolMode(HOVERING)
    , isModeManuallySet(false)
    , isMouseDown(false)
    , m_linkCanvasZoomLevel(true)
    , m_canvasScaleX(1.0)
    , m_canvasScaleY(1.0)
    , m_scratchpadScaleX(1.0)
    , m_scratchpadScaleY(1.0)
    , m_accumulatedMouseDelta(0)
    , m_paintLayer(0)
    , m_resourceProvider(0)
{

    setAutoFillBackground(false);
    setMouseTracking(true);

    m_cursor = KisCursor::load("tool_freehand_cursor.xpm", 2, 2);
    m_colorSamplerCursor = KisCursor::load("tool_color_sampler_cursor.xpm", 2, 2);
    setCursor(m_cursor);


    KisConfig cfg(true);
    QImage checkImage = KisCanvasWidgetBase::createCheckersImage(cfg.checkSize());
    m_checkBrush = QBrush(checkImage);


    // We are not supposed to use updates here,
    // so just set the listener to null
    m_updateScheduler = new KisUpdateScheduler(0);
    m_undoStore = new KisSurrogateUndoStore();
    m_undoAdapter = new KisPostExecutionUndoAdapter(m_undoStore, m_updateScheduler);
    m_nodeListener = new KisScratchPadNodeListener(this);

    connect(this, SIGNAL(sigUpdateCanvas(QRect)), SLOT(slotUpdateCanvas(QRect)), Qt::QueuedConnection);

    // filter will be deleted by the QObject hierarchy
    m_eventFilter = new KisScratchPadEventFilter(this);

    m_infoBuilder = new KisScratchPadPaintingInformationBuilder(this);

    m_scaleBorderWidth = 1;

    m_screenMigrationTracker.reset(new KisScreenMigrationTracker(this));

    connect(m_screenMigrationTracker.data(), &KisScreenMigrationTracker::sigScreenChanged,
            this, &KisScratchPad::slotScreenChanged);
    slotScreenChanged(m_screenMigrationTracker->currentScreenSafe());

    connect(KisConfigNotifier::instance(), &KisConfigNotifier::configChanged,
            this, &KisScratchPad::slotConfigChanged);
}

KisScratchPad::~KisScratchPad()
{
    delete m_infoBuilder;

    delete m_undoAdapter;
    delete m_undoStore;
    delete m_updateScheduler;
    delete m_nodeListener;
}

KisScratchPad::Mode KisScratchPad::modeFromButton(Qt::MouseButton button) const
{
    return
        button == Qt::NoButton ? HOVERING :
        button == Qt::MiddleButton ? PANNING :
        button == Qt::RightButton ? SAMPLING :
        PAINTING;
}

void KisScratchPad::wheelDelta(QWheelEvent *event)
{
    // if linked to canvas zoom level, ignore wheel event scale
    if (!isEnabled() || m_linkCanvasZoomLevel) return;

    const int angleDelta = event->angleDelta().y();

#ifdef Q_OS_MACOS
    // --> from KisInputManager::eventFilterImpl()

    // Some QT wheel events are actually touch pad pan events. From the QT docs:
    // "Wheel events are generated for both mouse wheels and trackpad scroll gestures."
    // We differentiate between touchpad events and real mouse wheels by inspecting the
    // event source.
    if (event->source() == Qt::MouseEventSource::MouseEventSynthesizedBySystem) {
        return;
    }

    /**
     * Ignore delta 0 events on OSX, since they are triggered by tablet
     * proximity when using Wacom devices.
     */
    if (angleDelta == 0) {
        return;
    }
#endif

    m_accumulatedMouseDelta += angleDelta;

    qreal scaleX = m_scratchpadScaleX;
    qreal scaleY = m_scratchpadScaleY;

    // cursor position on widget
    QPointF position = event->position();
    // matching cursor position in document (taking in account scale + translate)
    QPointF docPosition = widgetToDocument().map(position);

    // loop manage case where angleDelta is greated than one mouse wheel tick  and allows to refresh zoom smoothly
    // in this case
    //      if angleDelta = 360, 360/120 = 3, then scale will be refreshed 3 times
    // m_accumulatedMouseDelta allow to manage the case where angleDelta in not equal QWheelEvent::DefaultDeltasPerStep
    while (qAbs(m_accumulatedMouseDelta) >= QWheelEvent::DefaultDeltasPerStep) {
        if (m_accumulatedMouseDelta > 0) {
            qreal scaleFactor = 0.5;
            if (scaleX < 1) scaleFactor = 0.05;
            // zoom In
            scaleX += scaleFactor;
            scaleY += scaleFactor;
        } else {
            qreal scaleFactor = 0.5;
            if (scaleX <= 1) scaleFactor = 0.05;
            // zoom out
            scaleX -= scaleFactor;
            scaleY -= scaleFactor;
        }

        // update zoom level
        if (!setScaleImpl(scaleX, scaleY)) {
            // if can't apply zoom level, reset accumulated delta and exit, no need to try
            // to continue
            m_accumulatedMouseDelta = 0;
            break;
        }

        Q_EMIT scaleChanged(m_scratchpadScaleX);

        // cursor position after scale
        QPointF offsetPosition = QPointF(position.x() / m_scratchpadScaleX, position.y() / m_scratchpadScaleY);

        // new position after scale is applied
        //
        QPoint panPosition = QPointF( docPosition - offsetPosition).toPoint();

        // pan to ensure zoomed point is always under cursor
        panTo(panPosition.x(), panPosition.y());

        m_accumulatedMouseDelta -= KisAlgebra2D::signPZ(m_accumulatedMouseDelta) * QWheelEvent::DefaultDeltasPerStep;
    }
}

void KisScratchPad::pointerPress(KoPointerEvent *event)
{
    if (!isEnabled()) return;

    if (isModeManuallySet == false) {
        m_toolMode = modeFromButton(event->button());
    }

    // see if we are pressing down with a button
    if (event->button() == Qt::LeftButton ||
        event->button() == Qt::MiddleButton ||
        event->button() == Qt::RightButton) {
        isMouseDown = true;
    } else {
        isMouseDown = false;
    }

    // if mouse is down, we are doing one of three things
    if (isMouseDown) {
        if (m_toolMode == PAINTING) {
            beginStroke(event);
            event->accept();
        } else if (m_toolMode == PANNING) {
            beginPan(event);
            event->accept();
        } else if (m_toolMode == SAMPLING) {
            sample(event);
            event->accept();
        }
    }
}

void KisScratchPad::pointerRelease(KoPointerEvent *event)
{
    if (!isEnabled()) return;
    isMouseDown = false;

    if (isModeManuallySet == false) {
        if (modeFromButton(event->button()) != m_toolMode) return;

        if (m_toolMode == PAINTING) {
            endStroke(event);
            m_toolMode = HOVERING;
            event->accept();
        } else if (m_toolMode == PANNING) {
            endPan(event);
            m_toolMode = HOVERING;
            event->accept();
        } else if (m_toolMode == SAMPLING) {
            event->accept();
            m_toolMode = HOVERING;
        }

    } else {
        if (m_toolMode == PAINTING) {
            endStroke(event);
        } else if (m_toolMode == PANNING) {
            endPan(event);
        }

        event->accept();
    }
}

void KisScratchPad::pointerMove(KoPointerEvent *event)
{
    if (!isEnabled()) return;
    KIS_SAFE_ASSERT_RECOVER_RETURN(event);

    if (event->point.isNull() == false) {
        m_helper->cursorMoved(documentToWidget().map(event->point));
    }

    if (isMouseDown) {
        if (m_toolMode == PAINTING) {
            doStroke(event);
            event->accept();
        } else if (m_toolMode == PANNING) {
            doPan(event);
            event->accept();
        } else if (m_toolMode == SAMPLING) {
            sample(event);
            event->accept();
        }
    }
}

void KisScratchPad::beginStroke(KoPointerEvent *event)
{

    m_helper->initPaint(event,
                        documentToWidget().map(event->point),
                        0,
                        0,
                        m_updateScheduler,
                        m_paintLayer,
                        m_paintLayer->paintDevice()->defaultBounds());


}

void KisScratchPad::doStroke(KoPointerEvent *event)
{
    m_helper->paintEvent(event);
}

void KisScratchPad::endStroke(KoPointerEvent *event)
{
    Q_UNUSED(event);
    m_helper->endPaint();
    Q_EMIT contentChanged();
}

void KisScratchPad::beginPan(KoPointerEvent *event)
{
    setCursor(QCursor(Qt::ClosedHandCursor));
    m_panDocPoint = event->point;
}

void KisScratchPad::doPan(KoPointerEvent *event)
{
    QPointF docOffset = event->point - m_panDocPoint;

    m_translateTransform.translate(-docOffset.x(), -docOffset.y());
    updateTransformations();
    update();
}

void KisScratchPad::endPan(KoPointerEvent *event)
{
    Q_UNUSED(event);

    // the normal brush editor scratchpad reverts back to paint mode when done
    if (isModeManuallySet) {
        setCursor(QCursor(Qt::OpenHandCursor));
    } else {
        setCursor(m_cursor);
    }
    updateViewport();
}

void KisScratchPad::sample(KoPointerEvent *event)
{
    KoColor color;
    if (KisToolUtils::sampleColor(color, m_paintLayer->projection(), event->point.toPoint())) {
        Q_EMIT colorSelected(color);
    }
}

void KisScratchPad::setOnScreenResolution(qreal scaleX, qreal scaleY)
{
    // This method is called only when canvas zoom is changed (sigOnScreenResolutionChanged) AND scratchPad is
    // linked to it
    //
    // Otherwise do nothing
    // If zoom on scratchPad have to be updated, then it have to be udpated manually through setScale() method

    // value not changed? do nothing
    if (qFuzzyCompare(scaleX, m_canvasScaleX) && qFuzzyCompare(scaleY, m_canvasScaleY)) return;

    // always keep in memory canvas zoom, even if link is not active
    m_canvasScaleX = scaleX;
    m_canvasScaleY = scaleY;

    // the scratchpad will use the canvas zoom level...or not
    if (m_linkCanvasZoomLevel) {
        m_scaleBorderWidth = BORDER_SIZE(qMax(scaleX, scaleY));

        // memorize current scratchPad scale
        m_scratchpadScaleX = scaleX;
        m_scratchpadScaleY = scaleY;

        m_scaleTransform = QTransform::fromScale(scaleX, scaleY);
        updateTransformations();
        update();

        Q_EMIT scaleChanged(m_scratchpadScaleX);
        updateViewport();
    }
}

bool KisScratchPad::setScale(qreal scaleX, qreal scaleY)
{
    const bool retval = setScaleImpl(scaleX, scaleY);

    if (retval) {
        Q_EMIT scaleChanged(m_scratchpadScaleX);
        updateViewport();
    }

    return retval;
}

bool KisScratchPad::setScaleImpl(qreal scaleX, qreal scaleY)
{
    // developer must ensure zoom level is not linked with canvas zoom level before
    // calling this method
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!m_linkCanvasZoomLevel, false);

    scaleX = qBound(0.05, scaleX, 16.0);
    scaleY = qBound(0.05, scaleY, 16.0);

    // value not changed? do nothing and return false
    if (scaleX == m_scratchpadScaleX && scaleY == m_scratchpadScaleY) return false;

    m_scaleBorderWidth = BORDER_SIZE(qMax(scaleX, scaleY));

    // memorize current scratchPad scale
    m_scratchpadScaleX = scaleX;
    m_scratchpadScaleY = scaleY;

    m_scaleTransform = QTransform::fromScale(m_scratchpadScaleX, m_scratchpadScaleY);
    updateTransformations();
    update();

    return true;
}

void KisScratchPad::resetWheelDelta()
{
    m_accumulatedMouseDelta = 0;
}

qreal KisScratchPad::scaleX()
{
    return m_scratchpadScaleX;
}

qreal KisScratchPad::scaleY()
{
    return m_scratchpadScaleY;
}

void KisScratchPad::scaleToFit() {
    // developer must ensure zoom level is not linked with canvas zoom level before
    // calling this method
    KIS_SAFE_ASSERT_RECOVER_RETURN(!m_linkCanvasZoomLevel);

    QRectF viewportF = QRectF(imageBounds());
    QRectF contentsF = QRectF(contentBounds());
    qreal scale;

    if (contentsF.isEmpty() || viewportF.isEmpty()) return;

    qreal contentRatio = contentsF.width() / contentsF.height();
    qreal viewportRatio = viewportF.width() / viewportF.height();

    if (viewportRatio > contentRatio) {
        scale = viewportF.height() / contentsF.height();
    } else {
        scale = viewportF.width() / contentsF.width();
    }

    if (setScaleImpl(scale, scale)) {
        Q_EMIT scaleChanged(m_scratchpadScaleX);
    }
    panCenter();
}

void KisScratchPad::scaleReset() {
    // developer must ensure zoom level is not linked with canvas zoom level before
    // calling this method
    KIS_SAFE_ASSERT_RECOVER_RETURN(!m_linkCanvasZoomLevel);

    if (setScaleImpl(1.0, 1.0)) {
        Q_EMIT scaleChanged(m_scratchpadScaleX);
    }
    panTo(0, 0);
}

void KisScratchPad::panTo(qint32 x, qint32 y) {
    m_translateTransform.reset();
    m_translateTransform.translate(x, y);
    updateTransformations();
    update();
    updateViewport();
}

void KisScratchPad::panCenter() {
    updateViewportImpl();

    QPointF viewportF = QPointF(viewportBounds().width(), viewportBounds().height());
    QRectF contentsF = QRectF(contentBounds());

    QPoint panPosition = QPointF( contentsF.center() - viewportF/2 ).toPoint() ;

    panTo(panPosition.x(), panPosition.y());
}

QTransform KisScratchPad::documentToWidget() const
{
    return m_translateTransform.inverted() * m_scaleTransform;
}

QTransform KisScratchPad::widgetToDocument() const
{
    return m_scaleTransform.inverted() * m_translateTransform;
}

void KisScratchPad::updateTransformations()
{
    m_eventFilter->setWidgetToDocumentTransform(widgetToDocument());
}

QRect KisScratchPad::imageBounds() const
{
    return rect();
}

QRect KisScratchPad::viewportBounds() const
{
    return m_viewport;
}

void KisScratchPad::updateViewport()
{
    if (updateViewportImpl()) {
        Q_EMIT viewportChanged(m_viewport);
    }
}

bool KisScratchPad::updateViewportImpl()
{
    const QRect viewport = widgetToDocument().mapRect(rect());

    if (viewport != m_viewport) {
        m_viewport = viewport;
        return true;
    }
    return false;
}


QRect KisScratchPad::contentBounds() const
{
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();

    // -- DIRTY CODE HERE --
    // Call of nonDefaultPixelArea() should return expected result but no, not in all case
    //
    // Doing
    // - fillLayer()
    // - contentBounds() --> return expected QRect() bounds
    //
    // Doing
    // - fillDefault()
    // - do a stroke
    // - contentBounds() --> return empty QRect()
    // - do another stroke
    // - call contentBounds() --> return expected QRect() bounds of 1st stroke, ignoring 2nd stroke
    // - do another stroke
    // - call contentBounds() --> return expected QRect() bounds of 2nd stroke, ignoring 3rd stroke...
    //
    // Not sure what happen here and what's the best and cleanest solution to apply
    // Calling calculateExactBounds/nonDefaultPixelArea multiple times fix the problem, except for 1st stroke
    // for which an empty QRect() is still returned :-/
    paintDevice->calculateExactBounds(true);
    paintDevice->nonDefaultPixelArea();
    paintDevice->calculateExactBounds(true);
    paintDevice->nonDefaultPixelArea();
    paintDevice->calculateExactBounds(true);

    return paintDevice->nonDefaultPixelArea();
}

void KisScratchPad::imageUpdated(const QRect &rect)
{
    Q_EMIT sigUpdateCanvas(documentToWidget().mapRect(QRectF(rect)).toAlignedRect());
}

void KisScratchPad::slotUpdateCanvas(const QRect &rect)
{
    update(rect);
}

void KisScratchPad::slotConfigChanged()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_screenMigrationTracker);

    KisConfig cfg(true);
    QScreen *screen = m_screenMigrationTracker->currentScreenSafe();
    const int canvasScreenNumber = qApp->screens().indexOf(screen);
    KisDisplayConfig newConfig(canvasScreenNumber, cfg);
    if (newConfig != m_displayConfig) {
        slotScreenChanged(screen);
    }
}

void KisScratchPad::paintEvent ( QPaintEvent * event ) {
    if (!m_paintLayer) return;

    QRectF imageRect = widgetToDocument().mapRect(QRectF(event->rect()));

    QRect alignedImageRect =
        imageRect.adjusted(-m_scaleBorderWidth, -m_scaleBorderWidth,
                           m_scaleBorderWidth, m_scaleBorderWidth).toAlignedRect();

    QPointF offset = alignedImageRect.topLeft();

    m_paintLayer->projectionPlane()->recalculate(alignedImageRect, m_paintLayer, KisRenderPassFlag::None);
    KisPaintDeviceSP projection = m_paintLayer->projection();

    QImage image = projection->convertToQImage(m_displayConfig.profile,
                                               alignedImageRect.x(),
                                               alignedImageRect.y(),
                                               alignedImageRect.width(),
                                               alignedImageRect.height(),
                                               m_displayConfig.intent,
                                               m_displayConfig.conversionFlags);


    QPainter gc(this);
    gc.fillRect(event->rect(), m_checkBrush);

    // if we scale down, it should use Smooth
    // if we scale up, it should use Fast (nearest Neighbour) to show pixels
    if (event->rect().width() < image.rect().width()) {
        gc.setRenderHints(QPainter::SmoothPixmapTransform, true);
    } else {
        gc.setRenderHints(QPainter::SmoothPixmapTransform, false); // that will use NN
    }

    gc.drawImage(QRectF(event->rect()), image, imageRect.translated(-offset));

    QBrush brush(Qt::lightGray);
    QPen pen(brush, 1, Qt::DotLine);
    gc.setPen(pen);
    if (m_cutoutOverlay.isValid()) {
        gc.drawRect(m_cutoutOverlay);
    }

    if (!isEnabled()) {
        QColor color(Qt::lightGray);
        color.setAlphaF(0.5);
        QBrush disabledBrush(color);
        gc.fillRect(event->rect(), disabledBrush);
    }
    gc.end();
}

void KisScratchPad::resizeEvent( QResizeEvent *event) {
    updateViewport();
    QWidget::resizeEvent(event);
}

void KisScratchPad::resetState()
{
    if (m_helper->isRunning()) {
        m_helper->endPaint();
    }

    m_toolMode = HOVERING;
    setCursor(m_cursor);
}

void KisScratchPad::setupScratchPad(KisCanvasResourceProvider* resourceProvider,
                                    const QColor &defaultColor)
{
    m_resourceProvider = resourceProvider;

    connect(m_resourceProvider, SIGNAL(sigOnScreenResolutionChanged(qreal,qreal)),
            SLOT(setOnScreenResolution(qreal,qreal)));
    connect(this, SIGNAL(colorSelected(KoColor)),
            m_resourceProvider, SLOT(slotSetFGColor(KoColor)));

    m_helper.reset(new KisToolFreehandHelper(m_infoBuilder, m_resourceProvider->resourceManager()));

    setFillColor(defaultColor);

    KisPaintDeviceSP paintDevice =
        new KisPaintDevice(m_defaultColor.colorSpace(), "scratchpad");

    m_paintLayer = new KisPaintLayer(0, "ScratchPad", OPACITY_OPAQUE_U8, paintDevice);
    m_paintLayer->setGraphListener(m_nodeListener);
    m_paintLayer->paintDevice()->setDefaultBounds(new KisScratchPadDefaultBounds(this));

    fillDefault();
}

void KisScratchPad::setCutoutOverlayRect(const QRect& rc)
{
    m_cutoutOverlay = rc;
}

void KisScratchPad::setModeManually(bool value)
{
    isModeManuallySet = value;
}

void KisScratchPad::setModeType(QString mode)
{
    if (mode.toLower() == "painting") {
        m_toolMode = PAINTING;
        setCursor(m_cursor);
    } else if (mode.toLower() == "panning") {
        m_toolMode = PANNING;
        setCursor(Qt::OpenHandCursor);
    } else if (mode.toLower() == "colorsampling") {
        m_toolMode = SAMPLING;
        setCursor(m_colorSamplerCursor);
    }
}

void KisScratchPad::setCanvasZoomLink(bool value)
{
    if (m_linkCanvasZoomLevel != value) {
        m_accumulatedMouseDelta = 0;
        m_linkCanvasZoomLevel = value;

        if (m_linkCanvasZoomLevel) {
            // link status updated AND set to True
            // Then need to update scratchPad zoom to canvas zoom immediately
            setOnScreenResolution(m_canvasScaleX, m_canvasScaleY);
        }
    }
}

bool KisScratchPad::canvasZoomLink()
{
    return m_linkCanvasZoomLevel;
}

QImage KisScratchPad::cutoutOverlay() const
{
    if (!m_paintLayer) return QImage();
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();


    QRect rc = widgetToDocument().mapRect(m_cutoutOverlay);
    QImage rawImage = paintDevice->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height(), KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());

    QImage scaledImage = rawImage.scaled(m_cutoutOverlay.size(),
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation);

    return scaledImage;
}

void KisScratchPad::setPresetImage(const QImage& image)
{
    m_presetImage = image;
}

void KisScratchPad::paintCustomImage(const QImage& loadedImage)
{
    // this is 99% copied from the normal paintPresetImage()
    // we don't want to save over the preset image, so we don't
    // want to store it in the m_presetImage
    if (!m_paintLayer) return;
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();


    QRect overlayRect = widgetToDocument().mapRect(m_cutoutOverlay);
    QRect imageRect(QPoint(), overlayRect.size());

    QImage scaledImage = loadedImage.scaled(overlayRect.size(),
                                              Qt::IgnoreAspectRatio,
                                              Qt::SmoothTransformation);
    KisPaintDeviceSP device = new KisPaintDevice(paintDevice->colorSpace());
    device->convertFromQImage(scaledImage, 0);

    {
        KisUpdateSchedulerLockWithFeedback l(m_updateScheduler);
        KisPainter painter(paintDevice);
        painter.beginTransaction();
        painter.bitBlt(overlayRect.topLeft(), device, imageRect);
        painter.deleteTransaction();
    }


    update();
    Q_EMIT contentChanged();
}

void KisScratchPad::loadScratchpadImage(QImage image)
{
    if (!m_paintLayer) return;

    m_translateTransform.reset(); // image will be loaded at 0,0, so reset panning location
    updateTransformations();

    fillDefault(); // wipes out whatever was there before

    QRect imageSize = image.rect();
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();

    KisPaintDeviceSP device = new KisPaintDevice(paintDevice->colorSpace());
    device->convertFromQImage(image, 0);

    {
        KisUpdateSchedulerLockWithFeedback l(m_updateScheduler);
        KisPainter painter(paintDevice);
        painter.beginTransaction();
        painter.bitBlt(imageSize.topLeft(), device, imageSize);
        painter.deleteTransaction();
    }

    update();
}

QImage KisScratchPad::copyScratchpadImageData()
{
    const QRect paintingBounds = m_paintLayer.data()->exactBounds();
    QImage imageData = m_paintLayer->paintDevice()->convertToQImage(0, paintingBounds.x(), paintingBounds.y(), paintingBounds.width(), paintingBounds.height(),
                            KoColorConversionTransformation::internalRenderingIntent(),
                            KoColorConversionTransformation::internalConversionFlags());
    return imageData;
}

void KisScratchPad::paintPresetImage()
{
    if (!m_paintLayer) return;
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();


    QRect overlayRect = widgetToDocument().mapRect(m_cutoutOverlay);
    QRect imageRect(QPoint(), overlayRect.size());

    QImage scaledImage = m_presetImage.scaled(overlayRect.size(),
                                              Qt::IgnoreAspectRatio,
                                              Qt::SmoothTransformation);
    KisPaintDeviceSP device = new KisPaintDevice(paintDevice->colorSpace());
    device->convertFromQImage(scaledImage, 0);

    {
        KisUpdateSchedulerLockWithFeedback l(m_updateScheduler);
        KisPainter painter(paintDevice);
        painter.beginTransaction();
        painter.bitBlt(overlayRect.topLeft(), device, imageRect);
        painter.deleteTransaction();
    }

    update();
    Q_EMIT contentChanged();
}

void KisScratchPad::slotScreenChanged(QScreen *screen)
{
    KisConfig cfg(true);
    const int canvasScreenNumber = qApp->screens().indexOf(screen);
    m_displayConfig = KisDisplayConfig(canvasScreenNumber, cfg);
    update();
}

void KisScratchPad::fillDefault()
{
    if (!m_paintLayer) return;
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();

    {
        KisUpdateSchedulerLockWithFeedback l(m_updateScheduler);

        KisTransaction t(paintDevice);
        paintDevice->setDefaultPixel(m_defaultColor);
        paintDevice->clear();
        t.end();
    }

    update();
    Q_EMIT contentChanged();
}

void KisScratchPad::fillTransparent()
{
    if (!m_paintLayer) return;
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();

    QColor transQColor(0,0,0,0);
    KoColor transparentColor(transQColor, KoColorSpaceRegistry::instance()->rgb8());
    transparentColor.setOpacity(0.0);

    {
        KisUpdateSchedulerLockWithFeedback l(m_updateScheduler);

        KisTransaction t(paintDevice);
        paintDevice->setDefaultPixel(transparentColor);
        paintDevice->clear();
        t.end();
    }

    update();
    Q_EMIT contentChanged();
}

void KisScratchPad::setFillColor(QColor newColor)
{
    m_defaultColor = KoColor(newColor, KoColorSpaceRegistry::instance()->rgb8());
}

void KisScratchPad::fillGradient(const QPoint &gradientVectorStart,
                                 const QPoint &gradientVectorEnd,
                                 KisGradientPainter::enumGradientShape gradientShape,
                                 KisGradientPainter::enumGradientRepeat gradientRepeat,
                                 bool reverseGradient,
                                 bool dither)
{
    if (!m_paintLayer) return;
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();

    KoAbstractGradientSP gradient = m_resourceProvider->currentGradient();
    QRect gradientRect = widgetToDocument().mapRect(rect());


    {
        KisUpdateSchedulerLockWithFeedback l(m_updateScheduler);
        KisTransaction t(paintDevice);

        paintDevice->clear();

        KisGradientPainter painter(paintDevice);
        painter.setGradient(gradient);
        painter.setGradientShape(gradientShape);
        if (gradientVectorStart == gradientVectorEnd && gradientVectorStart == QPoint()) {
            // start & end are the same and are at origin: use default rect
            painter.paintGradient(gradientRect.topLeft(),
                                  gradientRect.bottomRight(),
                                  gradientRepeat,
                                  0.2,
                                  reverseGradient,
                                  gradientRect.left(), gradientRect.top(),
                                  gradientRect.width(), gradientRect.height(),
                                  dither);
        } else {
            painter.paintGradient(gradientVectorStart,
                                  gradientVectorEnd,
                                  gradientRepeat,
                                  0.2,
                                  reverseGradient,
                                  gradientRect.left(), gradientRect.top(),
                                  gradientRect.width(), gradientRect.height(),
                                  dither);
        }
        t.end();
    }

    update();
    Q_EMIT contentChanged();
}

void KisScratchPad::fillGradient()
{
    // default legacy method
    if (!m_paintLayer) return;
    QRect gradientRect = widgetToDocument().mapRect(rect());
    fillGradient(gradientRect.topLeft(),
                 gradientRect.bottomRight(),
                 KisGradientPainter::GradientShapeLinear,
                 KisGradientPainter::GradientRepeatNone,
                 false,
                 false
                 );
}

void KisScratchPad::fillPattern(QTransform transform)
{
    if (!m_paintLayer) return;
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();

    KoPatternSP pattern = m_resourceProvider->currentPattern();

    QRect patternRect = widgetToDocument().mapRect(rect());
    {
        KisUpdateSchedulerLockWithFeedback l(m_updateScheduler);

        KisTransaction t(paintDevice);

        paintDevice->clear();

        KisFillPainter painter(paintDevice);
        painter.setPattern(pattern);
        painter.setWidth(patternRect.width());
        painter.setHeight(patternRect.height());
        painter.fillPattern(0, 0, paintDevice, transform);

        t.end();
    }

    update();
    Q_EMIT contentChanged();
}

void KisScratchPad::fillBackground()
{
    if (!m_paintLayer) return;
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();

    {
        KisUpdateSchedulerLockWithFeedback l(m_updateScheduler);
        KisTransaction t(paintDevice);
        paintDevice->setDefaultPixel(m_resourceProvider->bgColor());
        paintDevice->clear();
        t.end();
    }

    update();
    Q_EMIT contentChanged();
}

void KisScratchPad::fillForeground()
{
    if (!m_paintLayer) return;
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();

    {
        KisUpdateSchedulerLockWithFeedback l(m_updateScheduler);
        KisTransaction t(paintDevice);
        paintDevice->setDefaultPixel(m_resourceProvider->fgColor());
        paintDevice->clear();
        t.end();
    }

    update();
    Q_EMIT contentChanged();
}

void KisScratchPad::fillDocument(bool fullContent)
{
    QRect sourceRect;

    if (!m_paintLayer) return;

    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();

    if (fullContent) {
        sourceRect = QRect(0, 0, m_resourceProvider->currentImage()->width(), m_resourceProvider->currentImage()->height());
    } else {
        sourceRect = QRect(0, 0, paintDevice->exactBounds().width(), paintDevice->exactBounds().height());
    }

    {
        KisUpdateSchedulerLockWithFeedback l(m_updateScheduler);
        KisPainter painter(paintDevice);
        painter.beginTransaction();
        painter.bitBlt(QPoint(0, 0), m_resourceProvider->currentImage()->projection(), sourceRect);
        painter.deleteTransaction();
    }

    update();
    Q_EMIT contentChanged();
}

void KisScratchPad::fillDocument()
{
    if (!m_paintLayer) return;
    fillDocument(false);
}

void KisScratchPad::fillLayer(bool fullContent)
{
    QRect sourceRect;

    if (!m_paintLayer) return;

    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();

    if (fullContent) {
        sourceRect = m_resourceProvider->currentNode()->exactBounds();
    } else {
        sourceRect = QRect(0, 0, paintDevice->exactBounds().width(), paintDevice->exactBounds().height());
    }

    {
        KisUpdateSchedulerLockWithFeedback l(m_updateScheduler);
        KisPainter painter(paintDevice);
        painter.beginTransaction();
        painter.bitBlt(QPoint(0, 0), m_resourceProvider->currentNode()->projection(), sourceRect);
        painter.deleteTransaction();
    }

    update();
    Q_EMIT contentChanged();
}

#include "kis_scratch_pad.moc"
