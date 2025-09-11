/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_canvas_controller.h"

#include <QMouseEvent>
#include <QScrollBar>
#include <QTabletEvent>

#include <klocalizedstring.h>
#include <kactioncollection.h>
#include "kis_canvas_decoration.h"
#include "kis_coordinates_converter.h"
#include "kis_canvas2.h"
#include "opengl/kis_opengl_canvas2.h"
#include "KisDocument.h"
#include "kis_image.h"
#include "KisViewManager.h"
#include "KisView.h"
#include "krita_utils.h"
#include "kis_config.h"
#include "kis_signal_compressor_with_param.h"
#include "kis_config_notifier.h"
#include <KoUnit.h>
#include <KoViewTransformStillPoint.h>

#include "KisCanvasState.h"

static const int gRulersUpdateDelay = 80 /* ms */;

struct KisCanvasController::Private {
    Private(KisCanvasController *qq)
        : q(qq)
    {
        using namespace std::placeholders;

        std::function<void (QPoint)> callback(
            std::bind(&KisCanvasController::Private::emitPointerPositionChangedSignals, this, _1));

        mousePositionCompressor.reset(
            new KisSignalCompressorWithParam<QPoint>(
                gRulersUpdateDelay,
                callback,
                KisSignalCompressor::FIRST_ACTIVE));
    }

    QPointer<KisView> view;
    KisCoordinatesConverter *coordinatesConverter {0};
    KisCanvasController *q {0};
    QScopedPointer<KisSignalCompressorWithParam<QPoint> > mousePositionCompressor;
    bool usePrintResolutionMode {false};
    qreal physicalDpiX {72.0};
    qreal physicalDpiY {72.0};
    qreal devicePixelRatio {1.0};

    void emitPointerPositionChangedSignals(QPoint pointerPos);
    void showRotationValueOnCanvas();
    void showMirrorStateOnCanvas();
};

void KisCanvasController::Private::emitPointerPositionChangedSignals(QPoint pointerPos)
{
    if (!coordinatesConverter) return;

    QPointF documentPos = coordinatesConverter->widgetToDocument(pointerPos);

    q->proxyObject->emitDocumentMousePositionChanged(documentPos);
    q->proxyObject->emitCanvasMousePositionChanged(pointerPos);
}

KisCanvasController::KisCanvasController(QPointer<KisView>parent, KoCanvasSupervisor *observerProvider, KisKActionCollection * actionCollection)
    : KoCanvasControllerWidget(actionCollection, observerProvider, parent),
      m_d(new Private(this))
{
    m_d->view = parent;
}

KisCanvasController::~KisCanvasController()
{
    delete m_d;
}

void KisCanvasController::setCanvas(KoCanvasBase *canvas)
{
    if (canvas) {
        KisCanvas2 *kritaCanvas = qobject_cast<KisCanvas2*>(canvas);
        m_d->coordinatesConverter =
            const_cast<KisCoordinatesConverter*>(kritaCanvas->coordinatesConverter());
    } else {
        m_d->coordinatesConverter = 0;
    }

    KoCanvasControllerWidget::setCanvas(canvas);
}

void KisCanvasController::updateScreenResolution(QWidget *parentWidget)
{
    if (qFuzzyCompare(parentWidget->physicalDpiX(), m_d->physicalDpiX) &&
        qFuzzyCompare(parentWidget->physicalDpiY(), m_d->physicalDpiY) &&
        qFuzzyCompare(parentWidget->devicePixelRatioF(), m_d->devicePixelRatio)) {

        return;
    }

    m_d->physicalDpiX = parentWidget->physicalDpiX();
    m_d->physicalDpiY = parentWidget->physicalDpiY();
    m_d->devicePixelRatio = parentWidget->devicePixelRatioF();

    m_d->coordinatesConverter->setDevicePixelRatio(m_d->devicePixelRatio);

    setUsePrintResolutionMode(m_d->usePrintResolutionMode);
}

qreal KisCanvasController::effectiveCanvasResolutionX() const
{
    KisImageSP image = m_d->view->image();
    return m_d->usePrintResolutionMode ? POINT_TO_INCH(m_d->physicalDpiX) : image->xRes() / m_d->devicePixelRatio;
}

qreal KisCanvasController::effectiveCanvasResolutionY() const
{
    KisImageSP image = m_d->view->image();
    return m_d->usePrintResolutionMode ? POINT_TO_INCH(m_d->physicalDpiY) : image->yRes() / m_d->devicePixelRatio;
}

void KisCanvasController::activate()
{
    KoCanvasControllerWidget::activate();
}

QPointF KisCanvasController::currentCursorPosition() const
{
    KoCanvasBase *canvas = m_d->view->canvasBase();
    QWidget *canvasWidget = canvas->canvasWidget();
    const QPointF cursorPosWidget = canvasWidget->mapFromGlobal(QCursor::pos());

    return m_d->coordinatesConverter->widgetToDocument(cursorPosWidget);
}

void KisCanvasController::keyPressEvent(QKeyEvent *event)
{
    /**
     * Dirty Hack Alert:
     * Do not call the KoCanvasControllerWidget::keyPressEvent()
     * to avoid activation of Pan and Default tool activation shortcuts
     */
    Q_UNUSED(event);
}

void KisCanvasController::wheelEvent(QWheelEvent *event)
{
    /**
     * Dirty Hack Alert:
     * Do not call the KoCanvasControllerWidget::wheelEvent()
     * to disable the default behavior of KoCanvasControllerWidget and QAbstractScrollArea
     */
    Q_UNUSED(event);
}

bool KisCanvasController::eventFilter(QObject *watched, QEvent *event)
{
    KoCanvasBase *canvas = this->canvas();
    if (!canvas || !canvas->canvasWidget() || canvas->canvasWidget() != watched) return false;

    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mevent = static_cast<QMouseEvent*>(event);
        m_d->mousePositionCompressor->start(mevent->pos());
    } else if (event->type() == QEvent::TabletMove) {
        QTabletEvent *tevent = static_cast<QTabletEvent*>(event);
        m_d->mousePositionCompressor->start(tevent->pos());
    } else if (event->type() == QEvent::FocusIn) {
        m_d->view->syncLastActiveNodeToDocument();
    }

    return false;
}

void KisCanvasController::ensureVisibleDoc(const QRectF &docRect, bool smooth)
{
    const QRect currentVisible = viewport()->rect();
    const QRect viewRect = m_d->coordinatesConverter->documentToWidget(docRect).toAlignedRect();

    if (!viewRect.isValid() || currentVisible.contains(viewRect))
        return; // its visible. Nothing to do.

    // if we move, we move a little more so the amount of times we have to move is less.
    int jumpWidth = smooth ? 0 : currentVisible.width() / 5;
    int jumpHeight = smooth ? 0 : currentVisible.height() / 5;
    if (!smooth && viewRect.width() + jumpWidth > currentVisible.width())
        jumpWidth = 0;
    if (!smooth && viewRect.height() + jumpHeight > currentVisible.height())
        jumpHeight = 0;

    int horizontalMove = 0;
    if (currentVisible.width() <= viewRect.width())      // center view
        horizontalMove = viewRect.center().x() - currentVisible.center().x();
    else if (currentVisible.x() > viewRect.x())          // move left
        horizontalMove = viewRect.x() - currentVisible.x() - jumpWidth;
    else if (currentVisible.right() < viewRect.right())  // move right
        horizontalMove = viewRect.right() - qMax(0, currentVisible.right() - jumpWidth);

    int verticalMove = 0;
    if (currentVisible.height() <= viewRect.height())       // center view
        verticalMove = viewRect.center().y() - currentVisible.center().y();
    if (currentVisible.y() > viewRect.y())               // move up
        verticalMove = viewRect.y() - currentVisible.y() - jumpHeight;
    else if (currentVisible.bottom() < viewRect.bottom()) // move down
        verticalMove = viewRect.bottom() - qMax(0, currentVisible.bottom() - jumpHeight);

    pan(QPoint(horizontalMove, verticalMove));
}

void KisCanvasController::Private::showMirrorStateOnCanvas()
{
    bool isXMirrored = coordinatesConverter->xAxisMirrored();

    view->viewManager()->
        showFloatingMessage(
            i18nc("floating message about mirroring",
                  "Horizontal mirroring: %1 ", isXMirrored ? i18n("ON") : i18n("OFF")),
            QIcon(), 500, KisFloatingMessage::Low);
}

void KisCanvasController::mirrorCanvasImpl(const std::optional<KoViewTransformStillPoint> &stillPoint, bool enable)
{
    const KisCanvasState oldCanvasState = canvasState();
    m_d->coordinatesConverter->mirror(stillPoint, enable, false);
    const KisCanvasState newCanvasState = canvasState();
    emitSignals(oldCanvasState, newCanvasState);

    m_d->showMirrorStateOnCanvas();
}

void KisCanvasController::mirrorCanvas(bool enable)
{
    mirrorCanvasImpl(std::nullopt, enable);
}

void KisCanvasController::mirrorCanvasAroundCursor(bool enable)
{
    QVariant customPos = sender()->property("customPosition");
    QPoint pos = customPos.isValid()
        ? customPos.value<QPoint>()
        : QCursor::pos();
    KoCanvasBase* canvas = m_d->view->canvasBase();
    QWidget *canvasWidget = canvas->canvasWidget();
    const QPointF cursorPosWidget = canvasWidget->mapFromGlobal(pos);

    std::optional<KoViewTransformStillPoint> stillPoint;

    if (canvasWidget->rect().contains(cursorPosWidget.toPoint())) {
        stillPoint = m_d->coordinatesConverter->makeWidgetStillPoint(cursorPosWidget);
    }

    mirrorCanvasImpl(stillPoint, enable);
}

void KisCanvasController::mirrorCanvasAroundCanvas(bool enable)
{
    auto stillPoint =
        m_d->coordinatesConverter->makeDocStillPoint(
            m_d->coordinatesConverter->imageRectInDocumentPixels().center());
    mirrorCanvasImpl(stillPoint, enable);
}

void KisCanvasController::Private::showRotationValueOnCanvas()
{
    qreal rotationAngle = coordinatesConverter->rotationAngle();
    view->viewManager()->
        showFloatingMessage(
            i18nc("floating message about rotation", "Rotation: %1° ",
                  KritaUtils::prettyFormatReal(rotationAngle)),
            QIcon(), 500, KisFloatingMessage::Low, Qt::AlignCenter);
}

void KisCanvasController::beginCanvasRotation()
{
    m_d->coordinatesConverter->beginRotation();
}

void KisCanvasController::endCanvasRotation()
{
    m_d->coordinatesConverter->endRotation();
}

void KisCanvasController::rotateCanvas(qreal angle, const std::optional<KoViewTransformStillPoint> &stillPoint, bool isNativeGesture)
{
    if(isNativeGesture) {
        m_d->coordinatesConverter->enableNatureGestureFlag();
    }
    const KisCanvasState oldCanvasState = canvasState();
    m_d->coordinatesConverter->rotate(stillPoint, angle);
    const KisCanvasState newCanvasState = canvasState();
    emitSignals(oldCanvasState, newCanvasState);

    m_d->showRotationValueOnCanvas();
}

void KisCanvasController::rotateCanvas(qreal angle)
{
    rotateCanvas(angle, std::nullopt);
}

void KisCanvasController::rotateCanvasRight15()
{
    rotateCanvas(15.0);
}

void KisCanvasController::rotateCanvasLeft15()
{
    rotateCanvas(-15.0);
}

qreal KisCanvasController::rotation() const
{
    return m_d->coordinatesConverter->rotationAngle();
}

void KisCanvasController::resetCanvasRotation()
{
    const KisCanvasState oldCanvasState = canvasState();
    m_d->coordinatesConverter->resetRotation(std::nullopt);
    const KisCanvasState newCanvasState = canvasState();
    emitSignals(oldCanvasState, newCanvasState);

    m_d->showRotationValueOnCanvas();
}

void KisCanvasController::slotToggleWrapAroundMode(bool value)
{
    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kritaCanvas);

    if (!canvas()->canvasIsOpenGL() && value) {
        m_d->view->viewManager()->showFloatingMessage(i18n("You are activating wrap-around mode, but have not enabled OpenGL.\n"
                                                          "To visualize wrap-around mode, enable OpenGL."), QIcon());
    }
    kritaCanvas->setWrapAroundViewingMode(value);
    kritaCanvas->image()->setWrapAroundModePermitted(value);
}

bool KisCanvasController::wrapAroundMode() const
{
    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kritaCanvas);

    return kritaCanvas->wrapAroundViewingMode();
}

void KisCanvasController::slotSetWrapAroundModeAxis(WrapAroundAxis value)
{
    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kritaCanvas);

    kritaCanvas->setWrapAroundViewingModeAxis(value);
    kritaCanvas->image()->setWrapAroundModeAxis(value);
}

void KisCanvasController::slotSetWrapAroundModeAxisHV()
{
    slotSetWrapAroundModeAxis(WRAPAROUND_BOTH);
}

void KisCanvasController::slotSetWrapAroundModeAxisH()
{
    slotSetWrapAroundModeAxis(WRAPAROUND_HORIZONTAL);
}

void KisCanvasController::slotSetWrapAroundModeAxisV()
{
    slotSetWrapAroundModeAxis(WRAPAROUND_VERTICAL);
}

WrapAroundAxis KisCanvasController::wrapAroundModeAxis() const
{
    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kritaCanvas);

    return kritaCanvas->wrapAroundViewingModeAxis();
}

void KisCanvasController::slotTogglePixelGrid(bool value)
{
    KisConfig cfg(false);
    cfg.enablePixelGrid(value);

    KisConfigNotifier::instance()->notifyPixelGridModeChanged();
}

void KisCanvasController::slotToggleLevelOfDetailMode(bool value)
{
    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kritaCanvas);

    kritaCanvas->setLodPreferredInCanvas(value);

    bool result = levelOfDetailMode();

    if (!value || result) {
        m_d->view->viewManager()->showFloatingMessage(
            i18n("Instant Preview Mode: %1", result ?
                 i18n("ON") : i18n("OFF")),
            QIcon(), 500, KisFloatingMessage::Low);
    } else {
        QString reason;

        if (!kritaCanvas->canvasIsOpenGL()) {
            reason = i18n("Instant Preview is only supported with OpenGL activated");
        }
        else if (kritaCanvas->openGLFilterMode() == KisOpenGL::BilinearFilterMode ||
                   kritaCanvas->openGLFilterMode() == KisOpenGL::NearestFilterMode) {
            QString filteringMode =
                kritaCanvas->openGLFilterMode() == KisOpenGL::BilinearFilterMode ?
                i18n("Bilinear") : i18n("Nearest Neighbour");
            reason = i18n("Instant Preview is supported\n in Trilinear or High Quality filtering modes.\nCurrent mode is %1", filteringMode);
        }

        m_d->view->viewManager()->showFloatingMessage(
            i18n("Failed activating Instant Preview mode!\n\n%1", reason),
            QIcon(), 5000, KisFloatingMessage::Low);
    }


}

bool KisCanvasController::levelOfDetailMode() const
{
    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kritaCanvas);

    return kritaCanvas->lodPreferredInCanvas();
}

void KisCanvasController::saveCanvasState(KisPropertiesConfiguration &config) const
{
    const QPointF &center = preferredCenter();
    config.setProperty("panX", center.x());
    config.setProperty("panY", center.y());

    config.setProperty("rotation", rotation());
    config.setProperty("mirror", m_d->coordinatesConverter->xAxisMirrored());
    config.setProperty("wrapAround", wrapAroundMode());
    config.setProperty("wrapAroundAxis", wrapAroundModeAxis());
    config.setProperty("enableInstantPreview", levelOfDetailMode());
}

void KisCanvasController::restoreCanvasState(const KisPropertiesConfiguration &config)
{
    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kritaCanvas);

    mirrorCanvas(config.getBool("mirror", false));
    rotateCanvas(config.getFloat("rotation", 0.0f));

    const QPointF &center = preferredCenter();
    float panX = config.getFloat("panX", center.x());
    float panY = config.getFloat("panY", center.y());
    setPreferredCenter(QPointF(panX, panY));

    slotToggleWrapAroundMode(config.getBool("wrapAround", false));
    slotSetWrapAroundModeAxis((WrapAroundAxis)config.getInt("wrapAroundAxis", 0));
    kritaCanvas->setLodPreferredInCanvas(config.getBool("enableInstantPreview", false));
}

void KisCanvasController::syncOnReferencesChange(const QRectF &referencesRect)
{
    const KisCanvasState oldCanvasState = canvasState();
    m_d->coordinatesConverter->setExtraReferencesBounds(referencesRect.toAlignedRect());
    const KisCanvasState newCanvasState = canvasState();

    emitSignals(oldCanvasState, newCanvasState);
}

void KisCanvasController::syncOnImageResolutionChange()
{
    const KisCanvasState oldCanvasState = canvasState();

    KisImageSP image = m_d->view->image();
    m_d->coordinatesConverter->setImageResolution(image->xRes(), image->yRes());

    if (!m_d->usePrintResolutionMode) {
        m_d->coordinatesConverter->setZoom(m_d->coordinatesConverter->zoomMode(),
                                            m_d->coordinatesConverter->zoom(),
                                            effectiveCanvasResolutionX(),
                                            effectiveCanvasResolutionY(),
                                            m_d->coordinatesConverter->makeWidgetStillPoint(
                                                m_d->coordinatesConverter->imageCenterInWidgetPixel()));
    }

    const KisCanvasState newCanvasState = canvasState();

    emitSignals(oldCanvasState, newCanvasState);
}

void KisCanvasController::syncOnImageSizeChange(const QPointF &oldStillPoint, const QPointF &newStillPoint)
{
    const KisCanvasState oldCanvasState = canvasState();

    KisImageSP image = m_d->view->image();
    m_d->coordinatesConverter->setImageBounds(image->bounds(), oldStillPoint, newStillPoint);

    const KisCanvasState newCanvasState = canvasState();

    emitSignals(oldCanvasState, newCanvasState);
}

void KisCanvasController::updateCanvasOffsetInternal(const QPointF &newOffset)
{
    // not need to snap to device pixels, it is done by the converter automatically
    m_d->coordinatesConverter->setDocumentOffset(newOffset);
}

void KisCanvasController::updateCanvasWidgetSizeInternal(const QSize &newSize)
{
    m_d->coordinatesConverter->setCanvasWidgetSizeKeepZoom(newSize);
}

void KisCanvasController::updateCanvasZoomInternal(KoZoomMode::Mode mode, qreal zoom, qreal resolutionX, qreal resolutionY, const std::optional<KoViewTransformStillPoint> &docStillPoint)
{
    m_d->coordinatesConverter->setZoom(mode, zoom, resolutionX, resolutionY, docStillPoint);
}

KisCanvasState KisCanvasController::canvasState() const
{
    return KisCanvasState::fromConverter(*m_d->coordinatesConverter);
}

KoZoomState KisCanvasController::zoomState() const
{
    return canvasState().zoomState();
}

QPointF KisCanvasController::preferredCenter() const
{
    const QPointF transformedImageBoundsTopleft = m_d->coordinatesConverter->imageRectInWidgetPixels().topLeft();
    return m_d->coordinatesConverter->widgetCenterPoint() - transformedImageBoundsTopleft;
}

void KisCanvasController::setPreferredCenter(const QPointF &viewPoint)
{
    const KisCanvasState oldState = canvasState();

    const QPointF expectedTransformedImageBoundsTopleft = m_d->coordinatesConverter->widgetCenterPoint() - viewPoint;
    updateCanvasOffsetInternal(-expectedTransformedImageBoundsTopleft);

    emitSignals(oldState, canvasState());
}

void KisCanvasController::zoomIn()
{
    zoomInImpl(std::nullopt);
}

void KisCanvasController::zoomOut()
{
    zoomOutImpl(std::nullopt);
}

void KisCanvasController::zoomIn(const KoViewTransformStillPoint &stillPoint)
{
    zoomInImpl(stillPoint);
}

void KisCanvasController::zoomOut(const KoViewTransformStillPoint &stillPoint)
{
    zoomOutImpl(stillPoint);
}

void KisCanvasController::zoomInImpl(const std::optional<KoViewTransformStillPoint> &stillPoint)
{
    const qreal newZoom = KisCoordinatesConverter::findNextZoom(m_d->coordinatesConverter->zoom(), m_d->coordinatesConverter->standardZoomLevels());
    if (!qFuzzyCompare(newZoom, m_d->coordinatesConverter->zoom())) {
        if (stillPoint) {
            setZoom(KoZoomMode::ZOOM_CONSTANT, newZoom, *stillPoint);
        } else {
            setZoom(KoZoomMode::ZOOM_CONSTANT, newZoom);
        }
    }
}

void KisCanvasController::zoomOutImpl(const std::optional<KoViewTransformStillPoint> &stillPoint)
{
    const qreal newZoom = KisCoordinatesConverter::findPrevZoom(m_d->coordinatesConverter->zoom(), m_d->coordinatesConverter->standardZoomLevels());
    if (!qFuzzyCompare(newZoom, m_d->coordinatesConverter->zoom())) {
        if (stillPoint) {
            setZoom(KoZoomMode::ZOOM_CONSTANT, newZoom, *stillPoint);
        } else {
            setZoom(KoZoomMode::ZOOM_CONSTANT, newZoom);
        }
    }
}

void KisCanvasController::zoomToInternal(const QRect &viewRect)
{
    m_d->coordinatesConverter->zoomTo(viewRect);
}

void KisCanvasController::resetScrollBars()
{
    // The scrollbar value always points at the top-left corner of the
    // bit of image we paint.

    const QPoint minOffset = m_d->coordinatesConverter->minimumOffset();
    const QPoint maxOffset = m_d->coordinatesConverter->maximumOffset();
    const QPoint offset = m_d->coordinatesConverter->documentOffset();

    QScrollBar *hScroll = horizontalScrollBar();
    QScrollBar *vScroll = verticalScrollBar();

    QSignalBlocker b1(hScroll);
    QSignalBlocker b2(vScroll);

    hScroll->setRange(minOffset.x(), maxOffset.x());
    hScroll->setValue(offset.x());
    vScroll->setRange(minOffset.y(), maxOffset.y());
    vScroll->setValue(offset.y());

    const int drawH = viewport()->height() / 4;
    const int drawW = viewport()->width() / 4;
    const int fontHeight = QFontMetrics(font()).height();

    vScroll->setPageStep(drawH);
    vScroll->setSingleStep(fontHeight);
    hScroll->setPageStep(drawW);
    hScroll->setSingleStep(fontHeight);
}

bool KisCanvasController::usePrintResolutionMode()
{
    return m_d->usePrintResolutionMode;
}

void KisCanvasController::setUsePrintResolutionMode(bool value)
{
    const bool changed = value != m_d->usePrintResolutionMode;

    KisImageSP image = m_d->view->image();

    // changeCanvasMappingMode is called with the same canvasMappingMode when the window is
    // moved across screens. Preserve the old zoomMode if this is the case.
    const KoZoomMode::Mode newMode =
            !changed ? m_d->coordinatesConverter->zoomMode() : KoZoomMode::ZOOM_CONSTANT;
    const qreal newZoom = m_d->coordinatesConverter->zoom();

    m_d->usePrintResolutionMode = value;

    setZoom(newMode, newZoom, effectiveCanvasResolutionX(), effectiveCanvasResolutionY());

    if (changed) {
        Q_EMIT sigUsePrintResolutionModeChanged(value);
    }
}