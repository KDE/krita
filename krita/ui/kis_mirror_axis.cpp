/*
 * Copyright (c) 2014 <copyright holder> <email>
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

#include <QtGui/QPainter>
#include <QtGui/QToolButton>
#include <QApplication>

#include <kaction.h>
#include <kactioncollection.h>

#include <KoIcon.h>

#include "kis_canvas_resource_provider.h"
#include "kis_view2.h"
#include "kis_image.h"
#include "canvas/kis_canvas_controller.h"
#include "input/kis_input_manager.h"

class KisMirrorAxis::Private
{
public:
    Private(KisMirrorAxis* qq)
        : q(qq)
        , resourceProvider(0)
        , mirrorX(false)
        , mirrorY(false)
        , handleSize(50)
        , xActive(false)
        , yActive(false)
        , xHandlePosition(30.f)
        , yHandlePosition(30.f)
    { }

    void setAxisPosition(float x, float y);

    KisMirrorAxis* q;

    KisCanvasResourceProvider* resourceProvider;
    bool mirrorX;
    bool mirrorY;

    float handleSize;
    QPixmap xHandleIcon;
    QPixmap yHandleIcon;
    QPixmap xAxisIcon;
    QPixmap yAxisIcon;
    QPointF axisPosition;
    QRectF xHandle;
    QRectF yHandle;
    bool xActive;
    bool yActive;
    float xHandlePosition;
    float yHandlePosition;
};

KisMirrorAxis::KisMirrorAxis(KisCanvasResourceProvider* provider, KisView2* parent)
    : KisCanvasDecoration("mirror_axis", i18n("Mirror Axis"), parent), d(new Private(this))
{
    d->resourceProvider = provider;
    connect(d->resourceProvider, SIGNAL(mirrorModeChanged()), SLOT(mirrorModeChanged()));
    d->mirrorX = d->resourceProvider->mirrorHorizontal();
    d->mirrorY = d->resourceProvider->mirrorVertical();
    d->xAxisIcon = koIcon("symmetry-horyzontal").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
    d->yAxisIcon = koIcon("symmetry-vertical").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
    d->xHandleIcon = koIcon("transform-move").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
    d->yHandleIcon = koIcon("transform-move").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
    setVisible(d->mirrorX || d->mirrorY);

    int imageWidth = parent->canvasBase()->image()->width();
    int imageHeight = parent->canvasBase()->image()->height();
    QPointF point(imageWidth / 2, imageHeight / 2);
    d->resourceProvider->resourceManager()->setResource(KisCanvasResourceProvider::MirrorAxesCenter, point);
    QPointF handlePos = parent->canvasBase()->coordinatesConverter()->imageToViewport(QPointF(imageWidth / 3, imageHeight / 3));
    d->xHandlePosition = handlePos.x();
    d->yHandlePosition = handlePos.y();

    parent->installEventFilter(this);
    parent->canvasBase()->inputManager()->attachPriorityEventFilter(this);
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
        d->xAxisIcon = koIcon("symmetry-horyzontal").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
        d->yAxisIcon = koIcon("symmetry-vertical").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
        d->xHandleIcon = koIcon("transform-move").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
        d->yHandleIcon = koIcon("transform-move").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
        emit handleSizeChanged();
    }
}

void KisMirrorAxis::drawDecoration(QPainter& gc, const QRectF& updateArea, const KisCoordinatesConverter* converter, KisCanvas2* canvas)
{
    Q_UNUSED(updateArea);

    gc.setPen(QPen(Qt::black, 2));
    gc.setBrush(Qt::white);
    float halfHandleSize = d->handleSize / 2;

    d->axisPosition = converter->imageToWidget<QPointF>(canvas->resourceManager()->resource(KisCanvasResourceProvider::MirrorAxesCenter).toPointF());

    QRectF xIndicator = QRectF(d->axisPosition.x() - halfHandleSize, 10, d->handleSize, d->handleSize);
    QRectF yIndicator = QRectF(10, d->axisPosition.y() - halfHandleSize, d->handleSize, d->handleSize);

    d->xHandle = QRectF(d->axisPosition.x() - halfHandleSize, d->xHandlePosition, d->handleSize, d->handleSize);
    d->yHandle = QRectF(d->yHandlePosition, d->axisPosition.y() - halfHandleSize, d->handleSize, d->handleSize);

    if(d->mirrorX) {
        gc.drawLine(d->axisPosition.x(), 0, d->axisPosition.x(), view()->height());
        gc.drawEllipse(xIndicator);
        gc.drawPixmap(xIndicator.adjusted(10, 10, -10, -10).toRect(), d->xAxisIcon);
        gc.drawEllipse(d->xHandle);
        gc.drawPixmap(d->xHandle.adjusted(5, 5, -5, -5).toRect(), d->xHandleIcon);
    }

    if(d->mirrorY) {
        gc.drawLine(0, d->axisPosition.y(), view()->width(), d->axisPosition.y());
        gc.drawEllipse(yIndicator);
        gc.drawPixmap(yIndicator.adjusted(10, 10, -10, -10).toRect(), d->yAxisIcon);
        gc.drawEllipse(d->yHandle);
        gc.drawPixmap(d->yHandle.adjusted(5, 5, -5, -5).toRect(), d->yHandleIcon);
    }
}

bool KisMirrorAxis::eventFilter(QObject* target, QEvent* event)
{
    if(event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if(d->mirrorX && d->xHandle.contains(me->posF())) {
            d->xActive = true;
            QApplication::setOverrideCursor(Qt::ClosedHandCursor);
            event->accept();
            return true;
        }

        if(d->mirrorY && d->yHandle.contains(me->posF())) {
            d->yActive = true;
            QApplication::setOverrideCursor(Qt::ClosedHandCursor);
            event->accept();
            return true;
        }
    }
    if(event->type() == QEvent::MouseMove) {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if(d->xActive) {
            d->setAxisPosition(me->posF().x(), d->axisPosition.y());
            d->xHandlePosition = me->posF().y() - d->handleSize / 2;
            event->accept();
            return true;
        }
        if(d->yActive) {
            d->setAxisPosition(d->axisPosition.x(), me->posF().y());
            d->yHandlePosition = me->posF().x() - d->handleSize / 2;
            event->accept();
            return true;
        }
        if(d->mirrorX && d->xHandle.contains(me->posF())) {
            QApplication::setOverrideCursor(Qt::OpenHandCursor);
        } else if(d->mirrorY && d->yHandle.contains(me->posF())) {
            QApplication::setOverrideCursor(Qt::OpenHandCursor);
        } else {
            QApplication::restoreOverrideCursor();
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
    if(target == view() && event->type() == QEvent::Resize) {
        QResizeEvent* re = static_cast<QResizeEvent*>(event);
        if(re->oldSize().width() > 0 && re->oldSize().height() > 0) {
            d->xHandlePosition = (d->xHandlePosition / re->oldSize().width()) * re->size().width();
            d->yHandlePosition = (d->yHandlePosition / re->oldSize().height()) * re->size().height();
        }
    }
    return QObject::eventFilter(target, event);
}

void KisMirrorAxis::mirrorModeChanged()
{
    d->mirrorX = d->resourceProvider->mirrorHorizontal();
    d->mirrorY = d->resourceProvider->mirrorVertical();
    setVisible(d->mirrorX || d->mirrorY);
}

void KisMirrorAxis::Private::setAxisPosition(float x, float y)
{
    QPointF newPosition = QPointF(x, y);

    newPosition = q->view()->canvasBase()->coordinatesConverter()->widgetToDocument(newPosition);
    newPosition = q->view()->canvasBase()->image()->documentToPixel(newPosition);

    resourceProvider->resourceManager()->setResource(KisCanvasResourceProvider::MirrorAxesCenter, newPosition);

    q->view()->canvasBase()->updateCanvas();
}

#include "kis_mirror_axis.moc"
