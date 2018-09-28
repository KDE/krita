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

#include <kis_icon.h>

#include "kis_canvas2.h"
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
        , lockHorizontal(false)
        , lockVertical(false)
        , hideVerticalDecoration(false)
        , hideHorizontalDecoration(false)
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
    KisImageWSP image;
    bool mirrorHorizontal;
    bool mirrorVertical;

    bool lockHorizontal;
    bool lockVertical;
    bool hideVerticalDecoration;
    bool hideHorizontalDecoration;



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
    connect(d->resourceProvider, SIGNAL(moveMirrorVerticalCenter()), SLOT(moveVerticalAxisToCenter()));
    connect(d->resourceProvider, SIGNAL(moveMirrorHorizontalCenter()), SLOT(moveHorizontalAxisToCenter()));

    d->mirrorHorizontal = d->resourceProvider->mirrorHorizontal();
    d->mirrorVertical = d->resourceProvider->mirrorVertical();
    d->horizontalIcon = KisIconUtils::loadIcon("mirrorAxis-HorizontalMove").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
    d->verticalIcon = KisIconUtils::loadIcon("mirrorAxis-VerticalMove").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
    d->horizontalHandleIcon = KisIconUtils::loadIcon("transform-move").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
    d->verticalHandleIcon = KisIconUtils::loadIcon("transform-move").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
    setVisible(d->mirrorHorizontal || d->mirrorVertical);


    d->image = parent->canvasBase()->image();
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
        d->horizontalIcon = KisIconUtils::loadIcon("symmetry-horyzontal").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
        d->verticalIcon = KisIconUtils::loadIcon("symmetry-vertical").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
        d->horizontalHandleIcon = KisIconUtils::loadIcon("transform-move").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
        d->verticalHandleIcon = KisIconUtils::loadIcon("transform-move").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
        d->minHandlePosition = d->sideMargin + newSize;
        emit handleSizeChanged();
    }
}

void KisMirrorAxis::drawDecoration(QPainter& gc, const QRectF& updateArea, const KisCoordinatesConverter* converter, KisCanvas2* canvas)
{
    Q_UNUSED(updateArea);
    Q_UNUSED(converter);
    Q_UNUSED(canvas);

    gc.save();
    gc.setPen(QPen(QColor(0, 0, 0, 128), 1));
    gc.setBrush(Qt::white);
    gc.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

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

    float halfHandleSize = d->handleSize / 2;

    d->recomputeVisibleAxes(gc.viewport());

    if(d->mirrorHorizontal && !d->hideHorizontalDecoration) {
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

            // don't draw the handles if we are locking the axis for movement
            if (!d->lockHorizontal) {
                gc.setPen(QPen(QColor(0, 0, 0, 128), 2));
                gc.drawEllipse(d->horizontalHandle);
                gc.drawPixmap(d->horizontalHandle.adjusted(5, 5, -5, -5).toRect(), d->horizontalIcon);
            }

        } else {
            d->horizontalHandle = QRectF();
        }
    }

    if(d->mirrorVertical && !d->hideVerticalDecoration) {
        if (!d->verticalAxis.isNull()) {

            gc.setPen(QPen(QColor(0, 0, 0, 64), 2, Qt::DashDotDotLine, Qt::RoundCap, Qt::RoundJoin));
            gc.drawLine(d->verticalAxis);


           // QPointF verticalIndicatorCenter = d->verticalAxis.unitVector().pointAt(15);
           // QRectF verticalIndicator = QRectF(verticalIndicatorCenter.x() - halfHandleSize, verticalIndicatorCenter.y() - halfHandleSize, d->handleSize, d->handleSize);

            float verticalHandlePosition = qBound<float>(d->minHandlePosition, d->verticalHandlePosition, d->verticalAxis.length() - d->minHandlePosition);
            QPointF verticalHandleCenter = d->verticalAxis.unitVector().pointAt(verticalHandlePosition);
            d->verticalHandle = QRectF(verticalHandleCenter.x() - halfHandleSize, verticalHandleCenter.y() - halfHandleSize, d->handleSize, d->handleSize);

            // don't draw the handles if we are locking the axis for movement
            if (!d->lockVertical) {
                gc.setPen(QPen(QColor(0, 0, 0, 128), 2));
                gc.drawEllipse(d->verticalHandle);
                gc.drawPixmap(d->verticalHandle.adjusted(5, 5, -5, -5).toRect(), d->verticalIcon);
            }

        } else {
            d->verticalHandle = QRectF();
        }
    }

    if (hasMultisample) {
        gc.beginNativePainting();
        ctx->functions()->glDisable(GL_MULTISAMPLE);
        gc.endNativePainting();
    }

    gc.restore();

}

bool KisMirrorAxis::eventFilter(QObject* target, QEvent* event)
{
    if (!visible()) return false;

    QObject *expectedCanvasWidget = view() ?
        view()->canvasBase()->canvasWidget() : 0;

    if (!expectedCanvasWidget || target != expectedCanvasWidget) return false;

    if(event->type() == QEvent::MouseButtonPress || event->type() == QEvent::TabletPress) {
        QMouseEvent *me = dynamic_cast<QMouseEvent*>(event);
        QTabletEvent *te = dynamic_cast<QTabletEvent*>(event);
        QPoint pos = me ? me->pos() : (te ? te->pos() : QPoint(77,77));




        if(d->mirrorHorizontal && d->horizontalHandle.contains(pos) && !d->lockHorizontal && !d->hideHorizontalDecoration ) {
            d->xActive = true;
            QApplication::setOverrideCursor(Qt::ClosedHandCursor);
            event->accept();
            return true;
        }

        if(d->mirrorVertical && d->verticalHandle.contains(pos) && !d->lockVertical && !d->hideVerticalDecoration) {
            d->yActive = true;
            QApplication::setOverrideCursor(Qt::ClosedHandCursor);
            event->accept();
            return true;
        }
    }
    if(event->type() == QEvent::MouseMove || event->type() == QEvent::TabletMove) {
        QMouseEvent *me = dynamic_cast<QMouseEvent*>(event);
        QTabletEvent *te = dynamic_cast<QTabletEvent*>(event);

        QPoint pos = me ? me->pos() : (te ? te->pos() : QPoint(77,77));

        if(d->xActive) {
            float axisX = view()->viewConverter()->widgetToImage<QPoint>(pos).x();

            d->setAxisPosition(axisX, d->axisPosition.y());
            d->horizontalHandlePosition = KisAlgebra2D::dotProduct<QPointF>(pos - d->horizontalAxis.p1(), d->horizontalAxis.unitVector().p2() - d->horizontalAxis.p1());

            event->accept();
            return true;
        }
        if(d->yActive) {
            float axisY = view()->viewConverter()->widgetToImage<QPoint>(pos).y();

            d->setAxisPosition(d->axisPosition.x(), axisY);
            d->verticalHandlePosition = KisAlgebra2D::dotProduct<QPointF>(pos - d->verticalAxis.p1(), d->verticalAxis.unitVector().p2() - d->verticalAxis.p1());

            event->accept();
            return true;
        }
        if(d->mirrorHorizontal && !d->hideHorizontalDecoration) {
            if(d->horizontalHandle.contains(pos) && !d->lockHorizontal) {
                if(!d->horizontalContainsCursor) {
                    QApplication::setOverrideCursor(Qt::OpenHandCursor);
                    d->horizontalContainsCursor = true;
                }
            } else if(d->horizontalContainsCursor) {
                QApplication::restoreOverrideCursor();
                d->horizontalContainsCursor = false;
            }
        }
        if(d->mirrorVertical && !d->hideVerticalDecoration) {
            if(d->verticalHandle.contains(pos) && !d->lockVertical) {
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
    if(event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::TabletRelease) {

        if(d->xActive) {
            QApplication::restoreOverrideCursor();
            d->xActive = false;
            event->accept();
            return true;
        }
        if(d->yActive) {
            QApplication::restoreOverrideCursor();
            d->yActive = false;
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

    d->lockHorizontal = d->resourceProvider->mirrorHorizontalLock();
    d->lockVertical = d->resourceProvider->mirrorVerticalLock();

    d->hideHorizontalDecoration = d->resourceProvider->mirrorHorizontalHideDecorations();
    d->hideVerticalDecoration = d->resourceProvider->mirrorVerticalHideDecorations();

    setVisible(d->mirrorHorizontal || d->mirrorVertical);

}

void KisMirrorAxis::setVisible(bool v)
{
    KisCanvasDecoration::setVisible(v);


    KisInputManager *inputManager = view() ? view()->canvasBase()->globalInputManager() : 0;
    if (!inputManager) return;

    if (v) {
        inputManager->attachPriorityEventFilter(this);
    } else {
        inputManager->detachPriorityEventFilter(this);
    }
}

void KisMirrorAxis::moveHorizontalAxisToCenter()
{
    d->setAxisPosition(d->image->width()/2, d->axisPosition.y());
}

void KisMirrorAxis::moveVerticalAxisToCenter()
{
    d->setAxisPosition(d->axisPosition.x(), d->image->height()/2 );
}


void KisMirrorAxis::Private::setAxisPosition(float x, float y)
{
    QPointF newPosition = QPointF(x, y);

    const QPointF relativePosition = KisAlgebra2D::absoluteToRelative(newPosition, image->bounds());
    image->setMirrorAxesCenter(relativePosition);

    q->view()->canvasBase()->updateCanvas();
}


void KisMirrorAxis::Private::recomputeVisibleAxes(QRect viewport)
{
    KisCoordinatesConverter *converter = q->view()->viewConverter();

    axisPosition = KisAlgebra2D::relativeToAbsolute(image->mirrorAxesCenter(), image->bounds());

    QPointF samplePt1 = converter->imageToWidget<QPointF>(axisPosition);
    QPointF samplePt2 = converter->imageToWidget<QPointF>(QPointF(axisPosition.x(), axisPosition.y() - 100));

    horizontalAxis = QLineF(samplePt1, samplePt2);
    if (!KisAlgebra2D::intersectLineRect(horizontalAxis, viewport)) horizontalAxis = QLineF();

    samplePt2 = converter->imageToWidget<QPointF>(QPointF(axisPosition.x() - 100, axisPosition.y()));
    verticalAxis = QLineF(samplePt1, samplePt2);
    if (!KisAlgebra2D::intersectLineRect(verticalAxis, viewport)) verticalAxis = QLineF();
}

