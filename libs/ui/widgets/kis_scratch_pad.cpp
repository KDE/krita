/* This file is part of the KDE project
 * Copyright 2010 (C) Boudewijn Rempt <boud@valdyas.org>
 * Copyright 2011 (C) Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kis_scratch_pad.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QMutex>

#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>
#include <KoPointerEvent.h>
#include <resources/KoAbstractGradient.h>

#include <kis_cursor.h>
#include <kis_tool_utils.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_gradient_painter.h>
#include <kis_default_bounds.h>
#include <kis_canvas_resource_provider.h>

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

class KisScratchPadNodeListener : public KisNodeGraphListener
{
public:
    KisScratchPadNodeListener(KisScratchPad *scratchPad)
        : m_scratchPad(scratchPad)
    {
    }

    void requestProjectionUpdate(KisNode *node, const QVector<QRect> &rects, bool resetAnimationCache) override {
        KisNodeGraphListener::requestProjectionUpdate(node, rects, resetAnimationCache);

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


KisScratchPad::KisScratchPad(QWidget *parent)
    : QWidget(parent)
    , m_toolMode(HOVERING)
    , isModeManuallySet(false)
    , isMouseDown(false)
    , linkCanvasZoomLevel(true)
    , m_paintLayer(0)
    , m_displayProfile(0)
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

    m_infoBuilder = new KisPaintingInformationBuilder();

    m_scaleBorderWidth = 1;
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
        button == Qt::MidButton ? PANNING :
        button == Qt::RightButton ? SAMPLING :
        PAINTING;
}

void KisScratchPad::pointerPress(KoPointerEvent *event)
{
    if(!isEnabled()) return;

    if (isModeManuallySet == false) {
        m_toolMode = modeFromButton(event->button());
    }

    // see if we are pressing down with a button
    if (event->button() == Qt::LeftButton ||
        event->button() == Qt::MidButton ||
        event->button() == Qt::RightButton) {
        isMouseDown = true;
    } else {
        isMouseDown = false;
    }

    // if mouse is down, we are doing one of three things
    if(isMouseDown) {
        if (m_toolMode == PAINTING) {
            beginStroke(event);
            event->accept();
        }
        else if (m_toolMode == PANNING) {
            beginPan(event);
            event->accept();
        }
        else if (m_toolMode == SAMPLING) {
            sample(event);
            event->accept();
        }
    }

}

void KisScratchPad::pointerRelease(KoPointerEvent *event)
{
    if(!isEnabled()) return;
    isMouseDown = false;

    if (isModeManuallySet == false) {
        if (modeFromButton(event->button()) != m_toolMode) return;

        if (m_toolMode == PAINTING) {
            endStroke(event);
            m_toolMode = HOVERING;
            event->accept();
        }
        else if (m_toolMode == PANNING) {
            endPan(event);
            m_toolMode = HOVERING;
            event->accept();
        }
        else if (m_toolMode == SAMPLING) {
            event->accept();
            m_toolMode = HOVERING;
        }

    } else {
        if (m_toolMode == PAINTING) {
            endStroke(event);
        }
        else if (m_toolMode == PANNING) {
            endPan(event);
        }

        event->accept();
    }


}

void KisScratchPad::pointerMove(KoPointerEvent *event)
{
    if(!isEnabled()) return;

    if(event && event->point.isNull() == false) {
        m_helper->cursorMoved(documentToWidget().map(event->point));
    }


    if (isMouseDown) {
        if (m_toolMode == PAINTING) {
            doStroke(event);
            event->accept();
        }
        else if (m_toolMode == PANNING) {
            doPan(event);
            event->accept();
        }
        else if (m_toolMode == SAMPLING) {
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
    if(isModeManuallySet) {
        setCursor(QCursor(Qt::OpenHandCursor));
    } else {
        setCursor(m_cursor);
    }

}

void KisScratchPad::sample(KoPointerEvent *event)
{
    KoColor color;
    if (KisToolUtils::sampleColor(color, m_paintLayer->projection(), event->point.toPoint())) {
        emit colorSelected(color);
    }
}

void KisScratchPad::setOnScreenResolution(qreal scaleX, qreal scaleY)
{
    m_scaleBorderWidth = BORDER_SIZE(qMax(scaleX, scaleY));

    // the scratchpad will use the canvas zoom level...or not
    if(linkCanvasZoomLevel) {
        m_scaleTransform = QTransform::fromScale(scaleX, scaleY);
    } else {
        m_scaleTransform = QTransform::fromScale(1, 1);
    }

    updateTransformations();
    update();
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
    return widgetToDocument().mapRect(rect());
}

void KisScratchPad::imageUpdated(const QRect &rect)
{
    emit sigUpdateCanvas(documentToWidget().mapRect(QRectF(rect)).toAlignedRect());
}

void KisScratchPad::slotUpdateCanvas(const QRect &rect)
{
    update(rect);
}

void KisScratchPad::paintEvent ( QPaintEvent * event ) {
    if(!m_paintLayer) return;

    QRectF imageRect = widgetToDocument().mapRect(QRectF(event->rect()));

    QRect alignedImageRect =
        imageRect.adjusted(-m_scaleBorderWidth, -m_scaleBorderWidth,
                           m_scaleBorderWidth, m_scaleBorderWidth).toAlignedRect();

    QPointF offset = alignedImageRect.topLeft();

    m_paintLayer->projectionPlane()->recalculate(alignedImageRect, m_paintLayer);
    KisPaintDeviceSP projection = m_paintLayer->projection();



    QImage image = projection->convertToQImage(m_displayProfile,
                                               alignedImageRect.x(),
                                               alignedImageRect.y(),
                                               alignedImageRect.width(),
                                               alignedImageRect.height(),
                                               KoColorConversionTransformation::internalRenderingIntent(),
                                               KoColorConversionTransformation::internalConversionFlags());


    QPainter gc(this);
    gc.fillRect(event->rect(), m_checkBrush);

    gc.setRenderHints(QPainter::SmoothPixmapTransform);
    gc.drawImage(QRectF(event->rect()), image, imageRect.translated(-offset));

    QBrush brush(Qt::lightGray);
    QPen pen(brush, 1, Qt::DotLine);
    gc.setPen(pen);
    if (m_cutoutOverlay.isValid()) {
        gc.drawRect(m_cutoutOverlay);
    }

    if(!isEnabled()) {
        QColor color(Qt::lightGray);
        color.setAlphaF(0.5);
        QBrush disabledBrush(color);
        gc.fillRect(event->rect(), disabledBrush);
    }
    gc.end();
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
    KisConfig cfg(true);
    setDisplayProfile(cfg.displayProfile(QApplication::desktop()->screenNumber(this)));
    connect(m_resourceProvider, SIGNAL(sigDisplayProfileChanged(const KoColorProfile*)),
            SLOT(setDisplayProfile(const KoColorProfile*)));

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
    }
    else if (mode.toLower() == "panning") {
        m_toolMode = PANNING;
        setCursor(Qt::OpenHandCursor);
    }
    else if (mode.toLower() == "colorsampling") {
        m_toolMode = SAMPLING;
        setCursor(m_colorSamplerCursor);
    }
}

void KisScratchPad::linkCanvavsToZoomLevel(bool value)
{
    linkCanvasZoomLevel = value;
}

QImage KisScratchPad::cutoutOverlay() const
{
    if(!m_paintLayer) return QImage();
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
    if(!m_paintLayer) return;
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();


    QRect overlayRect = widgetToDocument().mapRect(m_cutoutOverlay);
    QRect imageRect(QPoint(), overlayRect.size());

    QImage scaledImage = loadedImage.scaled(overlayRect.size(),
                                              Qt::IgnoreAspectRatio,
                                              Qt::SmoothTransformation);
    KisPaintDeviceSP device = new KisPaintDevice(paintDevice->colorSpace());
    device->convertFromQImage(scaledImage, 0);

    KisPainter painter(paintDevice);
    painter.beginTransaction();
    painter.bitBlt(overlayRect.topLeft(), device, imageRect);
    painter.deleteTransaction();
    update();
}

void KisScratchPad::loadScratchpadImage(QImage image)
{
    if(!m_paintLayer) return;

    m_translateTransform.reset(); // image will be loaded at 0,0, so reset panning location
    updateTransformations();

    fillDefault(); // wipes out whatever was there before

    QRect imageSize = image.rect();
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();

    KisPaintDeviceSP device = new KisPaintDevice(paintDevice->colorSpace());
    device->convertFromQImage(image, 0);

    KisPainter painter(paintDevice);
    painter.beginTransaction();
    painter.bitBlt(imageSize.topLeft(), device, imageSize);
    painter.deleteTransaction();
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
    if(!m_paintLayer) return;
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();


    QRect overlayRect = widgetToDocument().mapRect(m_cutoutOverlay);
    QRect imageRect(QPoint(), overlayRect.size());

    QImage scaledImage = m_presetImage.scaled(overlayRect.size(),
                                              Qt::IgnoreAspectRatio,
                                              Qt::SmoothTransformation);
    KisPaintDeviceSP device = new KisPaintDevice(paintDevice->colorSpace());
    device->convertFromQImage(scaledImage, 0);

    KisPainter painter(paintDevice);
    painter.beginTransaction();
    painter.bitBlt(overlayRect.topLeft(), device, imageRect);
    painter.deleteTransaction();
    update();
}

void KisScratchPad::setDisplayProfile(const KoColorProfile *colorProfile)
{
    if (colorProfile) {
        m_displayProfile = colorProfile;
        QWidget::update();
    }
}

void KisScratchPad::fillDefault()
{
    if(!m_paintLayer) return;
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();

    KisTransaction t(paintDevice);
    paintDevice->setDefaultPixel(m_defaultColor);
    paintDevice->clear();
    t.end();
    update();
}

void KisScratchPad::fillTransparent() {
    if(!m_paintLayer) return;
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();

    QColor transQColor(0,0,0,0);
    KoColor transparentColor(transQColor, KoColorSpaceRegistry::instance()->rgb8());
    transparentColor.setOpacity(0.0);

    KisTransaction t(paintDevice);
    paintDevice->setDefaultPixel(transparentColor);
    paintDevice->clear();
    t.end();
    update();
}

void KisScratchPad::setFillColor(QColor newColor)
{
    m_defaultColor = KoColor(newColor, KoColorSpaceRegistry::instance()->rgb8());
}

void KisScratchPad::fillGradient()
{
    if(!m_paintLayer) return;
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();

    KoAbstractGradientSP gradient = m_resourceProvider->currentGradient();
    QRect gradientRect = widgetToDocument().mapRect(rect());

    KisTransaction t(paintDevice);

    paintDevice->clear();

    KisGradientPainter painter(paintDevice);
    painter.setGradient(gradient);
    painter.setGradientShape(KisGradientPainter::GradientShapeLinear);
    painter.paintGradient(gradientRect.topLeft(),
                          gradientRect.bottomRight(),
                          KisGradientPainter::GradientRepeatNone,
                          0.2, false,
                          gradientRect.left(), gradientRect.top(),
                          gradientRect.width(), gradientRect.height());

    t.end();
    update();
}

void KisScratchPad::fillBackground()
{
    if(!m_paintLayer) return;
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();

    KisTransaction t(paintDevice);
    paintDevice->setDefaultPixel(m_resourceProvider->bgColor());
    paintDevice->clear();
    t.end();
    update();
}

void KisScratchPad::fillLayer()
{
    if(!m_paintLayer) return;
    KisPaintDeviceSP paintDevice = m_paintLayer->paintDevice();

    QRect sourceRect(0, 0, paintDevice->exactBounds().width(), paintDevice->exactBounds().height());

    KisPainter painter(paintDevice);
    painter.beginTransaction();
    painter.bitBlt(QPoint(0, 0), m_resourceProvider->currentImage()->projection(), sourceRect);
    painter.deleteTransaction();
    update();
}
