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

KisZoomableScrollbar::KisZoomableScrollbar(QWidget *parent)
    : QScrollBar(parent)
    , previousPosition(0,0)
    , accelAccumulator(0,0)
    , scrollAccumulator(0.0f)
    , zoomDeadzone(0.90f)
    , catchTeleportCorrection(false)
{
}

KisZoomableScrollbar::KisZoomableScrollbar(Qt::Orientation orientation, QWidget *parent)
    : KisZoomableScrollbar(parent)
{
    setOrientation(orientation);
}

KisZoomableScrollbar::~KisZoomableScrollbar()
{

}

void KisZoomableScrollbar::mousePressEvent(QMouseEvent *event)
{
    const bool wasSliderDownBefore = isSliderDown();
    QScrollBar::mousePressEvent(event);

    if( isSliderDown() && !wasSliderDownBefore ){
        previousPosition = mapToGlobal(event->pos());
        accelAccumulator = QVector2D(0,0);
        setCursor(Qt::BlankCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }

}

void KisZoomableScrollbar::mouseMoveEvent(QMouseEvent *event)
{
    if (isSliderDown()) {
        //Catch for teleportation from one side of the screen to the other.
        if (catchTeleportCorrection) {
            catchTeleportCorrection = false;
            event->accept();
            return;
        }

        QPoint globalMouseCoord = mapToGlobal(event->pos());
        QPoint accel = globalMouseCoord - previousPosition;
        accelAccumulator += QVector2D(accel);

        if( accelAccumulator.length() > 10 ) {
            accelAccumulator = accelAccumulator.normalized() * 10;
        }

        //Window-space wrapping for mouse dragging. Allows for blender-like
        //infinite mouse scrolls.
        QRect windowRect = window()->geometry();
        windowRect = kisGrowRect(windowRect, -2);
        const bool xWrap = true;
        const bool yWrap = true;
        if (!windowRect.contains(globalMouseCoord)) {
            int x = globalMouseCoord.x();
            int y = globalMouseCoord.y();

            if (x < windowRect.x() ) {
                if (xWrap) {
                    x += windowRect.width();
                } else {
                    x = (windowRect.x() + 2);
                }
            } else if (x > (windowRect.x() + windowRect.width()) ) {
                if (xWrap) {
                    x -= windowRect.width();
                } else {
                    x = windowRect.x() + (windowRect.width() - 2);
                }
            }

            if (y < windowRect.y()) {
                if (yWrap) {
                    y += windowRect.height();
                } else {
                    y = (windowRect.y() + 2);
                }
            } else if (y > (windowRect.y() + windowRect.height())) {
                if (yWrap){
                    y -= windowRect.height();
                } else {
                    y = windowRect.y() + (windowRect.height() - 2);
                }
            }

            if (globalMouseCoord.x() != x || globalMouseCoord.y() != y) {
                QCursor::setPos(x, y);
                previousPosition = QPoint(x, y) - accel;

                //Important -- teleportation needs to caught to prevent high-acceleration
                //values from QCursor::setPos being read in this event.
                catchTeleportCorrection = true;
            }
        } else {
            previousPosition = globalMouseCoord;
        }

        const qreal sliderMovementPix = (orientation() == Qt::Horizontal) ? accel.x() : accel.y();
        const qreal zoomMovementPix = (orientation() == Qt::Horizontal) ? -accel.y() : -accel.x();
        const qreal documentLength = maximum() - minimum() + pageStep();
        const qreal widgetLength = (orientation() == Qt::Horizontal) ? width() : height();
        const qreal widgetThickness = (orientation() == Qt::Horizontal) ? height() : width();


        const QVector2D perpendicularDirection = (orientation() == Qt::Horizontal) ? QVector2D(0, 1) : QVector2D(1, 0);
        const float perpendicularity = QVector2D::dotProduct(perpendicularDirection.normalized(), accelAccumulator.normalized());

        if (abs(perpendicularity) > zoomDeadzone && zoomMovementPix != 0) {
            zoom(qreal(zoomMovementPix) / (widgetThickness * 5));
        } else if (sliderMovementPix != 0) {

            const int currentPosition = sliderPosition();
            scrollAccumulator += (documentLength) * (sliderMovementPix / widgetLength);

            setSliderPosition(currentPosition + scrollAccumulator);
            if (currentPosition + scrollAccumulator > maximum() || currentPosition + scrollAccumulator <  minimum() ) {
                overscroll(scrollAccumulator);
            }

            const int sign = (scrollAccumulator > 0) - (scrollAccumulator < 0);
            scrollAccumulator -= floor(abs(scrollAccumulator)) * sign;

        }


    } else {
        QScrollBar::mouseMoveEvent(event);
    }
}

void KisZoomableScrollbar::mouseReleaseEvent(QMouseEvent *event)
{
    const QPoint cursorTranslationNormal = (orientation() == Qt::Horizontal) ? QPoint(1,0) : QPoint(0,1);
    const qreal widgetLength = (orientation() == Qt::Horizontal) ? width() : height();
    const qreal documentLength = maximum() - minimum() + pageStep();
    const qreal widgetLengthOffsetPix = (sliderPosition() / documentLength) * widgetLength;

    setCursor(Qt::ArrowCursor);

    QScrollBar::mouseReleaseEvent(event);

}

void KisZoomableScrollbar::wheelEvent(QWheelEvent *event) {
    const int delta = (event->angleDelta().y() / 8) * singleStep() * -1;
    const int currentPosition = sliderPosition();

    if (currentPosition + delta > maximum() || currentPosition + delta < minimum()){
        overscroll(delta);
    }

    QScrollBar::wheelEvent(event);
}

void KisZoomableScrollbar::setZoomDeadzone(float value)
{
    zoomDeadzone = value;
}
