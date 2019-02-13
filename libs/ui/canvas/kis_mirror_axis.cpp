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
#include <QAction>

#include <kis_icon.h>

#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"
#include "KisViewManager.h"
#include "KisView.h"
#include "kis_image.h"
#include "canvas/kis_canvas_controller.h"
#include "input/kis_input_manager.h"
#include "kis_algebra_2d.h"

#include <KisMirrorAxisConfig.h>
#include <kis_signals_blocker.h>
#include <kactioncollection.h>

class KisMirrorAxis::Private
{
public:
    Private(KisMirrorAxis* qq)
        : q(qq)
        , resourceProvider(0)
        , xActive(false)
        , yActive(false)
        , sideMargin(10.f)
        , minHandlePosition(10.f + 32.f)
        , horizontalContainsCursor(false)
        , verticalContainsCursor(false)
        , horizontalAxis(QLineF())
        , verticalAxis(QLineF())
        , config(KisMirrorAxisConfig())
    { }

    void setAxisPosition(float x, float y);
    void recomputeVisibleAxes(QRect viewport);

    KisMirrorAxis* q;

    KisCanvasResourceProvider* resourceProvider;
    KisImageWSP image;

    QPixmap horizontalHandleIcon;
    QPixmap verticalHandleIcon;
    QPixmap horizontalIcon;
    QPixmap verticalIcon;

    QRectF horizontalHandle;
    QRectF verticalHandle;
    bool xActive;
    bool yActive;

    float sideMargin;
    float minHandlePosition;
    bool horizontalContainsCursor;
    bool verticalContainsCursor;

    QLineF horizontalAxis;
    QLineF verticalAxis;

    KisMirrorAxisConfig config;
};

KisMirrorAxis::KisMirrorAxis(KisCanvasResourceProvider* provider, QPointer<KisView>parent)
    : KisCanvasDecoration("mirror_axis", parent)
    , d(new Private(this))
{
    d->resourceProvider = provider;
    connect(d->resourceProvider, SIGNAL(mirrorModeChanged()), SLOT(mirrorModeChanged()));
    connect(d->resourceProvider, SIGNAL(moveMirrorVerticalCenter()), SLOT(moveVerticalAxisToCenter()));
    connect(d->resourceProvider, SIGNAL(moveMirrorHorizontalCenter()), SLOT(moveHorizontalAxisToCenter()));

    d->config.setMirrorHorizontal(d->resourceProvider->mirrorHorizontal());
    d->config.setMirrorVertical(d->resourceProvider->mirrorVertical());
    d->horizontalIcon = KisIconUtils::loadIcon("mirrorAxis-HorizontalMove").pixmap(d->config.handleSize(), QIcon::Normal, QIcon::On);
    d->verticalIcon = KisIconUtils::loadIcon("mirrorAxis-VerticalMove").pixmap(d->config.handleSize(), QIcon::Normal, QIcon::On);
    d->horizontalHandleIcon = KisIconUtils::loadIcon("transform-move").pixmap(d->config.handleSize(), QIcon::Normal, QIcon::On);
    d->verticalHandleIcon = KisIconUtils::loadIcon("transform-move").pixmap(d->config.handleSize(), QIcon::Normal, QIcon::On);
    setVisible(d->config.mirrorHorizontal() || d->config.mirrorVertical());

    d->image = parent->canvasBase()->image();
}

KisMirrorAxis::~KisMirrorAxis()
{
    delete d;
}

float KisMirrorAxis::handleSize() const
{
    return d->config.handleSize();
}

void KisMirrorAxis::setHandleSize(float newSize)
{
    if(d->config.handleSize() != newSize) {
        d->config.setHandleSize(newSize);
        d->horizontalIcon = KisIconUtils::loadIcon("symmetry-horyzontal").pixmap(d->config.handleSize(), QIcon::Normal, QIcon::On);
        d->verticalIcon = KisIconUtils::loadIcon("symmetry-vertical").pixmap(d->config.handleSize(), QIcon::Normal, QIcon::On);
        d->horizontalHandleIcon = KisIconUtils::loadIcon("transform-move").pixmap(d->config.handleSize(), QIcon::Normal, QIcon::On);
        d->verticalHandleIcon = KisIconUtils::loadIcon("transform-move").pixmap(d->config.handleSize(), QIcon::Normal, QIcon::On);
        d->minHandlePosition = d->sideMargin + newSize;
        emit handleSizeChanged();
        emit sigConfigChanged();
    }
}

void KisMirrorAxis::drawDecoration(QPainter& gc, const QRectF& updateArea, const KisCoordinatesConverter* converter, KisCanvas2* canvas)
{
    Q_UNUSED(updateArea);
    Q_UNUSED(converter);
    Q_UNUSED(canvas);

    if (!view()->isCurrent()) {
        return;
    }

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

    float halfHandleSize = d->config.handleSize() / 2;

    d->recomputeVisibleAxes(gc.viewport());

    if(d->config.mirrorHorizontal() && !d->config.hideHorizontalDecoration()) {
        if (!d->horizontalAxis.isNull()) {
           // QPointF horizontalIndicatorCenter = d->horizontalAxis.unitVector().pointAt(15);
           // QRectF horizontalIndicator = QRectF(horizontalIndicatorCenter.x() - halfHandleSize, horizontalIndicatorCenter.y() - halfHandleSize, d->config.handleSize(), d->config.handleSize());

            float horizontalHandlePosition = qBound<float>(d->minHandlePosition, d->config.horizontalHandlePosition(), d->horizontalAxis.length() - d->minHandlePosition);
            QPointF horizontalHandleCenter = d->horizontalAxis.unitVector().pointAt(horizontalHandlePosition);
            d->horizontalHandle = QRectF(horizontalHandleCenter.x() - halfHandleSize, horizontalHandleCenter.y() - halfHandleSize, d->config.handleSize(), d->config.handleSize());

            gc.setPen(QPen(QColor(0, 0, 0, 64), 2, Qt::DashDotDotLine, Qt::RoundCap, Qt::RoundJoin));
            gc.drawLine(d->horizontalAxis);

           // gc.drawEllipse(horizontalIndicator);
          //  gc.drawPixmap(horizontalIndicator.adjusted(5, 5, -5, -5).toRect(), d->horizontalIcon);

            // don't draw the handles if we are locking the axis for movement
            if (!d->config.lockHorizontal()) {
                gc.setPen(QPen(QColor(0, 0, 0, 128), 2));
                gc.drawEllipse(d->horizontalHandle);
                gc.drawPixmap(d->horizontalHandle.adjusted(5, 5, -5, -5).toRect(), d->horizontalIcon);
            }

        } else {
            d->horizontalHandle = QRectF();
        }
    }

    if(d->config.mirrorVertical() && !d->config.hideVerticalDecoration()) {
        if (!d->verticalAxis.isNull()) {

            gc.setPen(QPen(QColor(0, 0, 0, 64), 2, Qt::DashDotDotLine, Qt::RoundCap, Qt::RoundJoin));
            gc.drawLine(d->verticalAxis);


           // QPointF verticalIndicatorCenter = d->verticalAxis.unitVector().pointAt(15);
           // QRectF verticalIndicator = QRectF(verticalIndicatorCenter.x() - halfHandleSize, verticalIndicatorCenter.y() - halfHandleSize, d->config.handleSize(), d->config.handleSize());

            float verticalHandlePosition = qBound<float>(d->minHandlePosition, d->config.verticalHandlePosition(), d->verticalAxis.length() - d->minHandlePosition);
            QPointF verticalHandleCenter = d->verticalAxis.unitVector().pointAt(verticalHandlePosition);
            d->verticalHandle = QRectF(verticalHandleCenter.x() - halfHandleSize, verticalHandleCenter.y() - halfHandleSize, d->config.handleSize(), d->config.handleSize());

            // don't draw the handles if we are locking the axis for movement
            if (!d->config.lockVertical()) {
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

        if(d->config.mirrorHorizontal() && d->horizontalHandle.contains(pos) && !d->config.lockHorizontal() && !d->config.hideHorizontalDecoration() ) {
            d->xActive = true;
            QApplication::setOverrideCursor(Qt::ClosedHandCursor);
            event->accept();
            return true;
        }

        if(d->config.mirrorVertical() && d->verticalHandle.contains(pos) && !d->config.lockVertical() && !d->config.hideVerticalDecoration()) {
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

            d->setAxisPosition(axisX, d->config.axisPosition().y());
            d->config.setHorizontalHandlePosition(KisAlgebra2D::dotProduct<QPointF>(pos - d->horizontalAxis.p1(), d->horizontalAxis.unitVector().p2() - d->horizontalAxis.p1()));
            emit sigConfigChanged();

            event->accept();
            return true;
        }
        if(d->yActive) {
            float axisY = view()->viewConverter()->widgetToImage<QPoint>(pos).y();

            d->setAxisPosition(d->config.axisPosition().x(), axisY);
            d->config.setVerticalHandlePosition(KisAlgebra2D::dotProduct<QPointF>(pos - d->verticalAxis.p1(), d->verticalAxis.unitVector().p2() - d->verticalAxis.p1()));
            emit sigConfigChanged();

            event->accept();
            return true;
        }
        if(d->config.mirrorHorizontal() && !d->config.hideHorizontalDecoration()) {
            if(d->horizontalHandle.contains(pos) && !d->config.lockHorizontal()) {
                if(!d->horizontalContainsCursor) {
                    QApplication::setOverrideCursor(Qt::OpenHandCursor);
                    d->horizontalContainsCursor = true;
                }
            } else if(d->horizontalContainsCursor) {
                QApplication::restoreOverrideCursor();
                d->horizontalContainsCursor = false;
            }
        }
        if(d->config.mirrorVertical() && !d->config.hideVerticalDecoration()) {
            if(d->verticalHandle.contains(pos) && !d->config.lockVertical()) {
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
    if (!view()->isCurrent()) {
        return;
    }

    d->config.setMirrorHorizontal(d->resourceProvider->mirrorHorizontal());
    d->config.setMirrorVertical(d->resourceProvider->mirrorVertical());

    d->config.setLockHorizontal(d->resourceProvider->mirrorHorizontalLock());
    d->config.setLockVertical(d->resourceProvider->mirrorVerticalLock());

    d->config.setHideHorizontalDecoration(d->resourceProvider->mirrorHorizontalHideDecorations());
    d->config.setHideVerticalDecoration(d->resourceProvider->mirrorVerticalHideDecorations());

    setVisible(d->config.mirrorHorizontal() || d->config.mirrorVertical());

    emit sigConfigChanged();
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

void KisMirrorAxis::setMirrorAxisConfig(const KisMirrorAxisConfig &config)
{
    if (config != d->config) {
        KisSignalsBlocker blocker(d->resourceProvider);

        d->config = config;

        d->setAxisPosition(d->config.axisPosition().x(), d->config.axisPosition().y());

        d->resourceProvider->setMirrorHorizontal(d->config.mirrorHorizontal());
        d->resourceProvider->setMirrorVertical(d->config.mirrorVertical());

        d->resourceProvider->setMirrorHorizontalLock(d->config.lockHorizontal());
        d->resourceProvider->setMirrorVerticalLock(d->config.lockVertical());

        d->resourceProvider->setMirrorHorizontal(d->config.mirrorHorizontal());
        d->resourceProvider->setMirrorVertical(d->config.mirrorVertical());

        d->resourceProvider->setMirrorHorizontalHideDecorations(d->config.hideHorizontalDecoration());
        d->resourceProvider->setMirrorVerticalHideDecorations(d->config.hideVerticalDecoration());
    }

    toggleMirrorActions();
    setVisible(d->config.mirrorHorizontal() || d->config.mirrorVertical());
}

const KisMirrorAxisConfig &KisMirrorAxis::mirrorAxisConfig() const
{
    return d->config;
}

void KisMirrorAxis::toggleMirrorActions()
{
    KActionCollection* collection = view()->viewManager()->actionCollection();
    // first uncheck the action, then set according to config;
    // otherwise the connected KisHighlightedToolButton's highlight color is not
    // properly set
    collection->action("hmirror_action")->setChecked(false);
    collection->action("vmirror_action")->setChecked(false);

    if (d->config.mirrorHorizontal()) {
        collection->action("hmirror_action")->setChecked(d->config.mirrorHorizontal());
    }

    if (d->config.mirrorVertical()) {
        collection->action("vmirror_action")->setChecked(d->config.mirrorVertical());
    }

    collection->action("mirrorX-lock")->setChecked(d->config.lockHorizontal());
    collection->action("mirrorY-lock")->setChecked(d->config.lockVertical());

    collection->action("mirrorX-hideDecorations")->setChecked(d->config.hideHorizontalDecoration());
    collection->action("mirrorY-hideDecorations")->setChecked(d->config.hideVerticalDecoration());
}

void KisMirrorAxis::moveHorizontalAxisToCenter()
{
    if (!view()->isCurrent()) {
        return;
    }

    d->setAxisPosition(d->image->width()/2, d->config.axisPosition().y());
    emit sigConfigChanged();
}

void KisMirrorAxis::moveVerticalAxisToCenter()
{
    if (!view()->isCurrent()) {
        return;
    }

    d->setAxisPosition(d->config.axisPosition().x(), d->image->height()/2 );
    emit sigConfigChanged();
}


void KisMirrorAxis::Private::setAxisPosition(float x, float y)
{
    QPointF newPosition = QPointF(x, y);

    config.setAxisPosition(newPosition);

    const QPointF relativePosition = KisAlgebra2D::absoluteToRelative(newPosition, image->bounds());
    image->setMirrorAxesCenter(relativePosition);

    q->view()->canvasBase()->updateCanvas();
}


void KisMirrorAxis::Private::recomputeVisibleAxes(QRect viewport)
{
    KisCoordinatesConverter *converter = q->view()->viewConverter();

    config.setAxisPosition(KisAlgebra2D::relativeToAbsolute(image->mirrorAxesCenter(), image->bounds()));

    QPointF samplePt1 = converter->imageToWidget<QPointF>(config.axisPosition());
    QPointF samplePt2 = converter->imageToWidget<QPointF>(QPointF(config.axisPosition().x(), config.axisPosition().y() - 100));

    horizontalAxis = QLineF(samplePt1, samplePt2);
    if (!KisAlgebra2D::intersectLineRect(horizontalAxis, viewport)) horizontalAxis = QLineF();

    samplePt2 = converter->imageToWidget<QPointF>(QPointF(config.axisPosition().x() - 100, config.axisPosition().y()));
    verticalAxis = QLineF(samplePt1, samplePt2);
    if (!KisAlgebra2D::intersectLineRect(verticalAxis, viewport)) verticalAxis = QLineF();
}

