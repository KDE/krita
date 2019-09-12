/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    KisCoordinatesConverter *coordinatesConverter;
    KisCanvasController *q;
    QScopedPointer<KisSignalCompressorWithParam<QPoint> > mousePositionCompressor;

    void emitPointerPositionChangedSignals(QPoint pointerPos);
    void updateDocumentSizeAfterTransform();
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

void KisCanvasController::Private::updateDocumentSizeAfterTransform()
{
    // round the size of the area to the nearest integer instead of getting aligned rect
    QSize widgetSize = coordinatesConverter->imageRectInWidgetPixels().toRect().size();
    q->updateDocumentSize(widgetSize, true);

    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(q->canvas());
    Q_ASSERT(kritaCanvas);

    kritaCanvas->notifyZoomChanged();
}


KisCanvasController::KisCanvasController(QPointer<KisView>parent, KActionCollection * actionCollection)
    : KoCanvasControllerWidget(actionCollection, parent),
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

void KisCanvasController::updateDocumentSize(const QSizeF &sz, bool recalculateCenter)
{
    KoCanvasControllerWidget::updateDocumentSize(sz, recalculateCenter);

    emit documentSizeChanged();
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

void KisCanvasController::mirrorCanvas(bool enable)
{
    QPoint newOffset = m_d->coordinatesConverter->mirror(m_d->coordinatesConverter->widgetCenterPoint(), enable, false);
    m_d->updateDocumentSizeAfterTransform();
    setScrollBarValue(newOffset);
    m_d->showMirrorStateOnCanvas();
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

void KisCanvasController::rotateCanvas(qreal angle, const QPointF &center)
{
    QPoint newOffset = m_d->coordinatesConverter->rotate(center, angle);
    m_d->updateDocumentSizeAfterTransform();
    setScrollBarValue(newOffset);
    m_d->showRotationValueOnCanvas();
}

void KisCanvasController::rotateCanvas(qreal angle)
{
    rotateCanvas(angle, m_d->coordinatesConverter->widgetCenterPoint());
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
    QPoint newOffset = m_d->coordinatesConverter->resetRotation(m_d->coordinatesConverter->widgetCenterPoint());
    m_d->updateDocumentSizeAfterTransform();
    setScrollBarValue(newOffset);
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

    kritaCanvas->setLodAllowedInCanvas(value);

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

    return kritaCanvas->lodAllowedInCanvas();
}

void KisCanvasController::saveCanvasState(KisPropertiesConfiguration &config) const
{
    const QPointF &center = preferredCenter();
    config.setProperty("panX", center.x());
    config.setProperty("panY", center.y());

    config.setProperty("rotation", rotation());
    config.setProperty("mirror", m_d->coordinatesConverter->xAxisMirrored());
    config.setProperty("wrapAround", wrapAroundMode());
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
    kritaCanvas->setLodAllowedInCanvas(config.getBool("enableInstantPreview", false));
}

void KisCanvasController::resetScrollBars()
{
    // The scrollbar value always points at the top-left corner of the
    // bit of image we paint.

    KisDocument *doc = m_d->view->document();
    if (!doc) return;

    QRectF documentBounds = doc->documentBounds();
    QRectF viewRect = m_d->coordinatesConverter->imageToWidget(documentBounds);

    // Cancel out any existing pan
    const QRectF imageBounds = m_d->view->image()->bounds();
    const QRectF imageBB = m_d->coordinatesConverter->imageToWidget(imageBounds);
    QPointF pan = imageBB.topLeft();
    viewRect.translate(-pan);

    int drawH = viewport()->height();
    int drawW = viewport()->width();

    qreal horizontalReserve = vastScrollingFactor() * drawW;
    qreal verticalReserve = vastScrollingFactor() * drawH;

    qreal xMin = viewRect.left() - horizontalReserve;
    qreal yMin = viewRect.top() - verticalReserve;

    qreal xMax = viewRect.right() - drawW + horizontalReserve;
    qreal yMax = viewRect.bottom() - drawH + verticalReserve;

    QScrollBar *hScroll = horizontalScrollBar();
    QScrollBar *vScroll = verticalScrollBar();

    hScroll->setRange(static_cast<int>(xMin), static_cast<int>(xMax));
    vScroll->setRange(static_cast<int>(yMin), static_cast<int>(yMax));

    int fontHeight = QFontMetrics(font()).height();

    vScroll->setPageStep(drawH);
    vScroll->setSingleStep(fontHeight);
    hScroll->setPageStep(drawW);
    hScroll->setSingleStep(fontHeight);
}
