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
#include <QMouseEvent>

KisZoomableScrollbar::KisZoomableScrollbar(QWidget *parent)
    : QScrollBar(parent)
    , scrollAccumulator(0.0f)
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

        //Window-space wrapping for mouse dragging. Allows for blender-like
        //infinite mouse scrolls.
        QRect windowRect = window()->geometry();
        windowRect = kisGrowRect(windowRect, -2);
        if (!windowRect.contains(globalMouseCoord)) {
            int x = globalMouseCoord.x();
            int y = globalMouseCoord.y();

            if (x < windowRect.x() ) {
                x += windowRect.width();
            } else if (x > (windowRect.x() + windowRect.width()) ) {
                x -= windowRect.width();
            }

            if (y < windowRect.y()) {
                y += windowRect.height();
            } else if (y > (windowRect.y() + windowRect.height())) {
                y -= windowRect.height();
            }

            if (globalMouseCoord.x() != x || globalMouseCoord.y() != y) {
                QCursor::setPos(x, y);
                previousPosition = QPoint(x, y) - accel;

                //Important -- teleportation needs to caught to prevent high-acceleration
                //values from being read in this event.
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

        if (sliderMovementPix != 0) {
            const int currentPosition = sliderPosition();
            scrollAccumulator += (documentLength) * (sliderMovementPix / widgetLength);

            setSliderPosition(currentPosition + scrollAccumulator);
            if (currentPosition + scrollAccumulator > maximum() || currentPosition + scrollAccumulator <  minimum() ) {
                overscroll(scrollAccumulator);
            }

            const int sign = (scrollAccumulator > 0) - (scrollAccumulator < 0);
            scrollAccumulator -= floor(abs(scrollAccumulator)) * sign;
        }

        if (zoomMovementPix != 0) {
            zoom(qreal(zoomMovementPix) / (widgetThickness * 10));
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

    if (isSliderDown()) {
        QCursor::setPos(mapToGlobal(pos()) + cursorTranslationNormal * widgetLengthOffsetPix);
    }

    QScrollBar::mouseReleaseEvent(event);

}
