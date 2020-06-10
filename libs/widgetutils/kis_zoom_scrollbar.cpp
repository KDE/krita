/*
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
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
#include "kis_zoom_scrollbar.h"

#include "kis_global.h"
#include "kis_debug.h"
#include <QMouseEvent>
#include <QTabletEvent>

KisZoomableScrollBar::KisZoomableScrollBar(QWidget *parent)
    : QScrollBar(parent)
    , lastKnownPosition(0,0)
    , accelerationAccumulator(0,0)
    , scrollSubPixelAccumulator(0.0f)
    , zoomPerpendicularityThreshold(0.75f)
    , catchTeleportCorrection(false)
{
}

KisZoomableScrollBar::KisZoomableScrollBar(Qt::Orientation orientation, QWidget *parent)
    : KisZoomableScrollBar(parent)
{
    setOrientation(orientation);
}

KisZoomableScrollBar::~KisZoomableScrollBar()
{

}

bool KisZoomableScrollBar::catchTeleports(QMouseEvent *event) {
    if (catchTeleportCorrection) {
        catchTeleportCorrection = false;
        event->accept();
        return true;
    }

    return false;
}

void KisZoomableScrollBar::handleWrap( const QPoint &accel, const QPoint &mouseCoord)
{
    QRect windowRect = window()->geometry();
    windowRect = kisGrowRect(windowRect, -2);
    const int windowWidth = windowRect.width();
    const int windowHeight = windowRect.height();
    const int windowX = windowRect.x();
    const int windowY = windowRect.y();
    const bool xWrap = true;
    const bool yWrap = true;

    if (!windowRect.contains(mouseCoord)) {
        int x = mouseCoord.x();
        int y = mouseCoord.y();

        if (x < windowX && xWrap ) {
            x += (windowWidth - 2);
        } else if (x > (windowX + windowWidth) && xWrap ) {
            x -= (windowWidth - 2);
        }

        if (y < windowY && yWrap) {
            y += (windowHeight - 2);
        } else if (y > (windowY + windowHeight) && yWrap) {
            y -= (windowHeight - 2);
        }

        QCursor::setPos(x, y);
        lastKnownPosition = QPoint(x, y) - accel;

        //Important -- teleportation needs to caught to prevent high-acceleration
        //values from QCursor::setPos being read in this event.
        catchTeleportCorrection = true;
    }
}

void KisZoomableScrollBar::handleScroll(const QPoint &accel)
{
    const qreal sliderMovementPix = (orientation() == Qt::Horizontal) ? accel.x() * devicePixelRatio() : accel.y() * devicePixelRatio();
    const qreal zoomMovementPix = (orientation() == Qt::Horizontal) ? -accel.y() : -accel.x();
    const qreal documentLength = maximum() - minimum() + pageStep();
    const qreal widgetLength = (orientation() == Qt::Horizontal) ? width() * devicePixelRatio() : height() * devicePixelRatio();
    const qreal widgetThickness = (orientation() == Qt::Horizontal) ? height() * devicePixelRatio() : width() * devicePixelRatio();

    const QVector2D perpendicularDirection = (orientation() == Qt::Horizontal) ? QVector2D(0, 1) : QVector2D(1, 0);
    const float perpendicularity = QVector2D::dotProduct(perpendicularDirection.normalized(), accelerationAccumulator.normalized());

    if (abs(perpendicularity) > zoomPerpendicularityThreshold && zoomMovementPix != 0) {
        zoom(qreal(zoomMovementPix) / (widgetThickness * 2));
    } else if (sliderMovementPix != 0) {
        const int currentPosition = sliderPosition();
        scrollSubPixelAccumulator += (documentLength) * (sliderMovementPix / widgetLength);

        setSliderPosition(currentPosition + scrollSubPixelAccumulator);
        if (currentPosition + scrollSubPixelAccumulator > maximum() ||
            currentPosition + scrollSubPixelAccumulator <  minimum()) {
            overscroll(scrollSubPixelAccumulator);
        }

        const int sign = (scrollSubPixelAccumulator > 0) - (scrollSubPixelAccumulator < 0);
        scrollSubPixelAccumulator -= floor(abs(scrollSubPixelAccumulator)) * sign;
    }
}

void KisZoomableScrollBar::tabletEvent(QTabletEvent *event) {
    if ( event->type() == QTabletEvent::TabletMove && isSliderDown() ) {
        QPoint globalMouseCoord = mapToGlobal(event->pos());
        QPoint accel = globalMouseCoord - lastKnownPosition;
        accelerationAccumulator += QVector2D(accel);

        if( accelerationAccumulator.length() > 5) {
            accelerationAccumulator = accelerationAccumulator.normalized();
        }

        handleScroll(accel);
        lastKnownPosition = globalMouseCoord;
        event->accept();
    } else {

        if (event->type() == QTabletEvent::TabletPress) {
            QPoint globalMouseCoord = mapToGlobal(event->pos());
            lastKnownPosition = globalMouseCoord;
        }

        QScrollBar::tabletEvent(event);
    }
}

void KisZoomableScrollBar::mousePressEvent(QMouseEvent *event)
{
    const bool wasSliderDownBefore = isSliderDown();
    QScrollBar::mousePressEvent(event);

    if( isSliderDown() && !wasSliderDownBefore ){
        lastKnownPosition = mapToGlobal(event->pos());
        accelerationAccumulator = QVector2D(0,0);
    }

}


void KisZoomableScrollBar::mouseMoveEvent(QMouseEvent *event)
{
    if (isSliderDown()) {
        QPoint globalMouseCoord = mapToGlobal(event->pos());

        QPoint accel = globalMouseCoord - lastKnownPosition;
        accelerationAccumulator += QVector2D(accel);

        if (catchTeleports(event)){
            return;
        }

        if( accelerationAccumulator.length() > 5 ) {
            accelerationAccumulator = accelerationAccumulator.normalized();
        }

        handleScroll(accel);
        lastKnownPosition = globalMouseCoord;
        handleWrap(accel, mapToGlobal(event->pos()));
        event->accept();
    } else {
        QScrollBar::mouseMoveEvent(event);
    }
}

void KisZoomableScrollBar::mouseReleaseEvent(QMouseEvent *event)
{
    QScrollBar::mouseReleaseEvent(event);
}

void KisZoomableScrollBar::wheelEvent(QWheelEvent *event) {
    const int delta = (event->angleDelta().y() / 8) * singleStep() * -1;
    const int currentPosition = sliderPosition();

    if (currentPosition + delta > maximum() || currentPosition + delta < minimum()){
        overscroll(delta);
    }

    QScrollBar::wheelEvent(event);
}

void KisZoomableScrollBar::setZoomDeadzone(float value)
{
    zoomPerpendicularityThreshold = value;
}
