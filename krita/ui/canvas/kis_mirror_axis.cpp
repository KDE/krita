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
#include "config-glew.h"

#if defined(HAVE_OPENGL) && defined(HAVE_GLEW)
    #include <GL/glew.h>
#endif

#include <QtGui/QPainter>
#include <QtGui/QToolButton>
#include <QApplication>
#include <QPaintEngine>

#include <KoIcon.h>

#include "kis_canvas_resource_provider.h"
#include "KisViewManager.h"
#include "KisView.h"
#include "kis_image.h"
#include "canvas/kis_canvas_controller.h"
#include "input/kis_input_manager.h"

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
        , horizontalHandlePosition(0.f)
        , verticalHandlePosition(0.f)
        , sideMargin(10.f)
        , minHandlePosition(10.f + 32.f)
        , horizontalContainsCursor(false)
        , verticalContainsCursor(false)
    { }

    void setAxisPosition(float x, float y);

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
};

KisMirrorAxis::KisMirrorAxis(KisCanvasResourceProvider* provider, QPointer<KisView>parent)
    : KisCanvasDecoration("mirror_axis", parent)
    , d(new Private(this))
{
    d->resourceProvider = provider;
    connect(d->resourceProvider, SIGNAL(mirrorModeChanged()), SLOT(mirrorModeChanged()));
    d->mirrorHorizontal = d->resourceProvider->mirrorHorizontal();
    d->mirrorVertical = d->resourceProvider->mirrorVertical();
    d->horizontalIcon = koIcon("dark_symmetry-horizontal").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
    d->verticalIcon = koIcon("dark_symmetry-vertical").pixmap(d->handleSize, QIcon::Normal, QIcon::On);
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

    gc.setPen(QPen(QColor(0, 0, 0, 128), 1));
    gc.setBrush(Qt::white);
    gc.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    float halfHandleSize = d->handleSize / 2;

    d->axisPosition = converter->imageToWidget<QPointF>(canvas->resourceManager()->resource(KisCanvasResourceProvider::MirrorAxesCenter).toPointF());

    QRectF horizontalIndicator = QRectF(d->axisPosition.x() - halfHandleSize, 10, d->handleSize, d->handleSize);
    QRectF verticalIndicator = QRectF(10, d->axisPosition.y() - halfHandleSize, d->handleSize, d->handleSize);

    d->horizontalHandle = QRectF(d->axisPosition.x() - halfHandleSize, d->horizontalHandlePosition, d->handleSize, d->handleSize);
    d->verticalHandle = QRectF(d->verticalHandlePosition, d->axisPosition.y() - halfHandleSize, d->handleSize, d->handleSize);

#if defined(HAVE_OPENGL) && defined(HAVE_GLEW)
    // QPainter cannot anti-alias the edges of circles etc. when using OpenGL
    // So instead, use native OpenGL anti-aliasing when available.
    if(gc.paintEngine()->type() == QPaintEngine::OpenGL2 && GLEW_ARB_multisample) {
        gc.beginNativePainting();
        glEnable(GL_MULTISAMPLE);
        gc.endNativePainting();
    }
#endif

    if(d->mirrorHorizontal) {
        gc.drawLine(d->axisPosition.x(), 0, d->axisPosition.x(), view()->height());
        gc.drawEllipse(horizontalIndicator);
        gc.drawPixmap(horizontalIndicator.adjusted(5, 5, -5, -5).toRect(), d->horizontalIcon);
        gc.drawEllipse(d->horizontalHandle);
        gc.drawPixmap(d->horizontalHandle.adjusted(5, 5, -5, -5).toRect(), d->horizontalHandleIcon);
    }

    if(d->mirrorVertical) {
        gc.drawLine(0, d->axisPosition.y(), view()->width(), d->axisPosition.y());
        gc.drawEllipse(verticalIndicator);
        gc.drawPixmap(verticalIndicator.adjusted(5, 5, -5, -5).toRect(), d->verticalIcon);
        gc.drawEllipse(d->verticalHandle);
        gc.drawPixmap(d->verticalHandle.adjusted(5, 5, -5, -5).toRect(), d->verticalHandleIcon);
    }

#if defined(HAVE_OPENGL) && defined(HAVE_GLEW)
    if(gc.paintEngine()->type() == QPaintEngine::OpenGL2 && GLEW_ARB_multisample) {
        gc.beginNativePainting();
        glDisable(GL_MULTISAMPLE);
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
            d->setAxisPosition(me->posF().x(), d->axisPosition.y());
            d->horizontalHandlePosition = qBound<float>(d->minHandlePosition, me->posF().y() - d->handleSize / 2, view()->height() - d->handleSize);
            event->accept();
            return true;
        }
        if(d->yActive) {
            d->setAxisPosition(d->axisPosition.x(), me->posF().y());
            d->verticalHandlePosition = qBound<float>(d->minHandlePosition, me->posF().x() - d->handleSize / 2, view()->width() - d->handleSize);
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
    if(target == view() && event->type() == QEvent::Resize) {
        QResizeEvent* re = static_cast<QResizeEvent*>(event);
        if(re->oldSize().width() > 0 && re->oldSize().height() > 0) {
            d->horizontalHandlePosition = qBound<float>(d->minHandlePosition, (d->horizontalHandlePosition / re->oldSize().height()) * re->size().height(), re->size().height());
            d->verticalHandlePosition = qBound<float>(d->minHandlePosition, (d->verticalHandlePosition / re->oldSize().width()) * re->size().width(), re->size().width());
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

    newPosition = q->view()->canvasBase()->coordinatesConverter()->widgetToDocument(newPosition);
    newPosition = q->view()->canvasBase()->image()->documentToPixel(newPosition);

    resourceProvider->resourceManager()->setResource(KisCanvasResourceProvider::MirrorAxesCenter, newPosition);

    q->view()->canvasBase()->updateCanvas();
}

#include "kis_mirror_axis.moc"
