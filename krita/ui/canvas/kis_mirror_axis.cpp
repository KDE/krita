/*
 * Copyright (c) 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "kis_mirror_axis.h"

#include "KoConfig.h"

#include <QPainter>
#include <QToolButton>
#include <QApplication>
#include <QPaintEngine>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <KoIcon.h>

#include "kis_canvas_resource_provider.h"
#include "KisViewManager.h"
#include "KisView.h"
#include "kis_image.h"
#include "canvas/kis_canvas_controller.h"
#include "input/kis_input_manager.h"
#include "kis_algebra_2d.h"

class KisMirrorAxis::Private
{
public:
    Private(KisMirrorAxis* qq)
        : q(qq)
        , resourceProvider(0)
        , mirrorHorizontal(false)
        , mirrorVertical(false)
        , handleSize(32.f)
        , xActive(false)
        , yActive(false)
        , horizontalHandlePosition(64.f)
        , verticalHandlePosition(64.f)
        , sideMargin(10.f)
        , minHandlePosition(10.f + 32.f)
        , horizontalContainsCursor(false)
        , verticalContainsCursor(false)
        , horizontalAxis(QLineF())
        , verticalAxis(QLineF())
    { }

    void setAxisPosition(float x, float y);
    void recomputeVisibleAxes(QRect viewport);

    KisMirrorAxis* q;

    KisCanvasResourceProvider* resourceProvider;
    bool mirrorHorizontal;
    bool mirrorVertical;

    float handleSize;
    QPixmap horizontalHandleIcon;
    QPixmap verticalHandleIcon;
    QPixmap horizontalIcon;
    QPixmap verticalIcon;
    QPointF axisPosition;
    QRectF horizontalHandle;
    QRectF verticalHandle;
    bool xActive;
    bool yActive;
    float horizontalHandlePosition;
    float verticalHandlePosition;
    float sideMargin;
    float minHandlePosition;
    bool horizontalContainsCursor;
    bool verticalContainsCursor;

    QLineF horizontalAxis;
    QLineF verticalAxis;
};

KisMirrorAxis::KisMirrorAxis(KisCanvasResourceProvider* provider, QPointer<KisView>parent)
    : KisCanvasDecoration("mirror_axis", parent)
    , d(new Private(this))
{
    d->resourceProvider = provider;
    connect(d->resourceProvider, SIGNAL(mirrorModeChanged()), SLOT(mirrorModeChanged()));
    d->mirrorHorizontal = d->resourceProvider->mirrorHorizontal();
    d->mirrorVertical = d->resourceProvider->mirrorVertical();
    d->horizontalIcon = koIcon("mirrorAxis-HorizontalMove").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
    d->verticalIcon = koIcon("mirrorAxis-VerticalMove").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
    d->horizontalHandleIcon = koIcon("transform-move").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
    d->verticalHandleIcon = koIcon("transform-move").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
    setVisible(d->mirrorHorizontal || d->mirrorVertical);

    int imageWidth = parent->canvasBase()->image()->width();
    int imageHeight = parent->canvasBase()->image()->height();
    QPointF point(imageWidth / 2, imageHeight / 2);
    d->resourceProvider->resourceManager()->setResource(KisCanvasResourceProvider::MirrorAxesCenter, point);

    parent->installEventFilter(this);

    KisInputManager *inputManager = parent->canvasBase()->globalInputManager();
    if (inputManager) {
        inputManager->attachPriorityEventFilter(this);
    }
}

KisMirrorAxis::~KisMirrorAxis()
{
    delete d;
}

float KisMirrorAxis::handleSize() const
{
    return d->handleSize;
}

void KisMirrorAxis::setHandleSize(float newSize)
{
    if(d->handleSize != newSize) {
        d->handleSize = newSize;
        d->horizontalIcon = koIcon("symmetry-horyzontal").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
        d->verticalIcon = koIcon("symmetry-vertical").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
        d->horizontalHandleIcon = koIcon("transform-move").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
        d->verticalHandleIcon = koIcon("transform-move").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
        d->minHandlePosition = d->sideMargin + newSize;
        emit handleSizeChanged();
    }
}

void KisMirrorAxis::drawDecoration(QPainter& gc, const QRectF& updateArea, const KisCoordinatesConverter* converter, KisCanvas2* canvas)
{
    Q_UNUSED(updateArea);
    Q_UNUSED(converter);
    Q_UNUSED(canvas);

    gc.setPen(QPen(QColor(0, 0, 0, 128), 1));
    gc.setBrush(Qt::white);
    gc.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

#if defined(HAVE_OPENGL)
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    bool hasMultisample = ((gc.paintEngine()->type() == QPaintEngine::OpenGL2) &&
                           (ctx->hasExtension("GL_ARB_multisample")));

    // QPainter cannot anti-alias the edges of circles etc. when using OpenGL
    // So instead, use native OpenGL anti-aliasing when available.
    if (hasMultisample) {
        gc.beginNativePainting();
        ctx->functions()->glEnable(GL_MULTISAMPLE);
        gc.endNativePainting();
    }
#endif

    float halfHandleSize = d->handleSize / 2;

    d->recomputeVisibleAxes(gc.viewport());

    if(d->mirrorHorizontal) {
        if (!d->horizontalAxis.isNull()) {
           // QPointF horizontalIndicatorCenter = d->horizontalAxis.unitVector().pointAt(15);
           // QRectF horizontalIndicator = QRectF(horizontalIndicatorCenter.x() - halfHandleSize, horizontalIndicatorCenter.y() - halfHandleSize, d->handleSize, d->handleSize);

            float horizontalHandlePosition = qBound<float>(d->minHandlePosition, d->horizontalHandlePosition, d->horizontalAxis.length() - d->minHandlePosition);
            QPointF horizontalHandleCenter = d->horizontalAxis.unitVector().pointAt(horizontalHandlePosition);
            d->horizontalHandle = QRectF(horizontalHandleCenter.x() - halfHandleSize, horizontalHandleCenter.y() - halfHandleSize, d->handleSize, d->handleSize);

            gc.setPen(QPen(QColor(0, 0, 0, 64), 2, Qt::DashDotDotLine, Qt::RoundCap, Qt::RoundJoin));
            gc.drawLine(d->horizontalAxis);

           // gc.drawEllipse(horizontalIndicator);
          //  gc.drawPixmap(horizontalIndicator.adjusted(5, 5, -5, -5).toRect(), d->horizontalIcon);

            gc.setPen(QPen(QColor(0, 0, 0, 128), 2));
            gc.drawEllipse(d->horizontalHandle);
            gc.drawPixmap(d->horizontalHandle.adjusted(5, 5, -5, -5).toRect(), d->horizontalIcon);

        } else {
            d->horizontalHandle = QRectF();
        }
    }

    if(d->mirrorVertical) {
        if (!d->verticalAxis.isNull()) {

            gc.setPen(QPen(QColor(0, 0, 0, 64), 2, Qt::DashDotDotLine, Qt::RoundCap, Qt::RoundJoin));
            gc.drawLine(d->verticalAxis);


           // QPointF verticalIndicatorCenter = d->verticalAxis.unitVector().pointAt(15);
           // QRectF verticalIndicator = QRectF(verticalIndicatorCenter.x() - halfHandleSize, verticalIndicatorCenter.y() - halfHandleSize, d->handleSize, d->handleSize);

            float verticalHandlePosition = qBound<float>(d->minHandlePosition, d->verticalHandlePosition, d->verticalAxis.length() - d->minHandlePosition);
            QPointF verticalHandleCenter = d->verticalAxis.unitVector().pointAt(verticalHandlePosition);
            d->verticalHandle = QRectF(verticalHandleCenter.x() - halfHandleSize, verticalHandleCenter.y() - halfHandleSize, d->handleSize, d->handleSize);

           // gc.drawEllipse(verticalIndicator);
          //  gc.drawPixmap(verticalIndicator.adjusted(5, 5, -5, -5).toRect(), d->verticalIcon);
            gc.setPen(QPen(QColor(0, 0, 0, 128), 2));
            gc.drawEllipse(d->verticalHandle);
            gc.drawPixmap(d->verticalHandle.adjusted(5, 5, -5, -5).toRect(), d->verticalIcon);
        } else {
            d->verticalHandle = QRectF();
        }
    }

#if defined(HAVE_OPENGL)
    if (hasMultisample) {
        gc.beginNativePainting();
        ctx->functions()->glDisable(GL_MULTISAMPLE);
        gc.endNativePainting();
    }
#endif
}

bool KisMirrorAxis::eventFilter(QObject* target, QEvent* event)
{
    if(event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if(d->mirrorHorizontal && d->horizontalHandle.contains(me->posF())) {
            d->xActive = true;
            QApplication::setOverrideCursor(Qt::ClosedHandCursor);
            event->accept();
            return true;
        }

        if(d->mirrorVertical && d->verticalHandle.contains(me->posF())) {
            d->yActive = true;
            QApplication::setOverrideCursor(Qt::ClosedHandCursor);
            event->accept();
            return true;
        }
    }
    if(event->type() == QEvent::MouseMove) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if(d->xActive) {
            float axisX = view()->viewConverter()->widgetToImage<QPoint>(me->pos()).x();

            d->setAxisPosition(axisX, d->axisPosition.y());
            d->horizontalHandlePosition = KisAlgebra2D::dotProduct<QPointF>(me->pos() - d->horizontalAxis.p1(), d->horizontalAxis.unitVector().p2() - d->horizontalAxis.p1());

            event->accept();
            return true;
        }
        if(d->yActive) {
            float axisY = view()->viewConverter()->widgetToImage<QPoint>(me->pos()).y();

            d->setAxisPosition(d->axisPosition.x(), axisY);
            d->verticalHandlePosition = KisAlgebra2D::dotProduct<QPointF>(me->pos() - d->verticalAxis.p1(), d->verticalAxis.unitVector().p2() - d->verticalAxis.p1());

            event->accept();
            return true;
        }
        if(d->mirrorHorizontal) {
            if(d->horizontalHandle.contains(me->posF())) {
                if(!d->horizontalContainsCursor) {
                    QApplication::setOverrideCursor(Qt::OpenHandCursor);
                    d->horizontalContainsCursor = true;
                }
            } else if(d->horizontalContainsCursor) {
                QApplication::restoreOverrideCursor();
                d->horizontalContainsCursor = false;
            }
        }
        if(d->mirrorVertical) {
            if(d->verticalHandle.contains(me->posF())) {
                if(!d->verticalContainsCursor) {
                    QApplication::setOverrideCursor(Qt::OpenHandCursor);
                    d->verticalContainsCursor = true;
                }
            } else if(d->verticalContainsCursor) {
                QApplication::restoreOverrideCursor();
                d->verticalContainsCursor = false;
            }
        }
    }
    if(event->type() == QEvent::MouseButtonRelease) {
        if(d->xActive) {
            d->xActive = false;
            QApplication::restoreOverrideCursor();
            event->accept();
            return true;
        }
        if(d->yActive) {
            d->yActive = false;
            QApplication::restoreOverrideCursor();
            event->accept();
            return true;
        }
    }
    return QObject::eventFilter(target, event);
}

void KisMirrorAxis::mirrorModeChanged()
{
    d->mirrorHorizontal = d->resourceProvider->mirrorHorizontal();
    d->mirrorVertical = d->resourceProvider->mirrorVertical();
    setVisible(d->mirrorHorizontal || d->mirrorVertical);
}

void KisMirrorAxis::Private::setAxisPosition(float x, float y)
{
    QPointF newPosition = QPointF(x, y);

    resourceProvider->resourceManager()->setResource(KisCanvasResourceProvider::MirrorAxesCenter, newPosition);

    q->view()->canvasBase()->updateCanvas();
}

void KisMirrorAxis::Private::recomputeVisibleAxes(QRect viewport)
{
    KisCoordinatesConverter *converter = q->view()->viewConverter();

    axisPosition = resourceProvider->resourceManager()->resource(KisCanvasResourceProvider::MirrorAxesCenter).toPointF();

    QPointF samplePt1 = converter->imageToWidget<QPointF>(axisPosition);
    QPointF samplePt2 = converter->imageToWidget<QPointF>(QPointF(axisPosition.x(), axisPosition.y() - 100));

    horizontalAxis = QLineF(samplePt1, samplePt2);
    if (!KisAlgebra2D::intersectLineRect(horizontalAxis, viewport)) horizontalAxis = QLineF();

    samplePt2 = converter->imageToWidget<QPointF>(QPointF(axisPosition.x() - 100, axisPosition.y()));
    verticalAxis = QLineF(samplePt1, samplePt2);
    if (!KisAlgebra2D::intersectLineRect(verticalAxis, viewport)) verticalAxis = QLineF();
}

