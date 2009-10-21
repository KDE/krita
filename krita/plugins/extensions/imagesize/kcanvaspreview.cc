/*
 *
 *  Copyright (c) 2009 Edward Apap <schumifer@hotmail.com>
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

#include "kcanvaspreview.h"

#include <QMouseEvent>
#include <QPainter>
#include <QCursor>

KCanvasPreview::KCanvasPreview(QWidget * parent) : QWidget(parent), m_dragging(false)
{
    setMouseTracking(true);
}

KCanvasPreview::~KCanvasPreview()
{
}

void KCanvasPreview::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    double scale = scalingFactor();

    m_xCanvasOffset = (width() - scale * m_width) / 2.0;
    m_yCanvasOffset = (height() - scale * m_height) / 2.0;

    p.fillRect(m_xCanvasOffset, m_yCanvasOffset, m_width * scale, m_height * scale, QBrush(Qt::white));
    p.setPen(QPen(Qt::red));
    p.drawRect(m_xCanvasOffset + m_xOffset * scale, m_yCanvasOffset + m_yOffset * scale, m_imageWidth * scale, m_imageHeight * scale);
}

void KCanvasPreview::mousePressEvent(QMouseEvent *event)
{
    if (isInRegion(event->pos())) {
        m_dragging = true;
        m_prevDragPoint = event->pos();
    }
}

void KCanvasPreview::mouseReleaseEvent(QMouseEvent *)
{
    m_dragging = false;
}

void KCanvasPreview::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        // calculate new image offset
        double scale = scalingFactor();

        int newXOffset = ((event->pos().x() - m_prevDragPoint.x()) / scale) + m_xOffset;
        int newYOffset = ((event->pos().y() - m_prevDragPoint.y()) / scale) + m_yOffset;

        m_prevDragPoint = event->pos();

        emit sigModifiedXOffset(newXOffset);
        emit sigModifiedYOffset(newYOffset);
    } else {
        QCursor cursor;

        if (isInRegion(event->pos())) {
            cursor.setShape(Qt::SizeAllCursor);
        } else {
            cursor.setShape(Qt::ArrowCursor);
        }

        setCursor(cursor);
    }
}

bool KCanvasPreview::isInRegion(QPoint point)
{
    double scale = scalingFactor();

    if ((point.x() >= (m_xOffset * scale) + m_xCanvasOffset) && (point.x() < ((m_xOffset + m_imageWidth) * scale) + m_xCanvasOffset) &&
            (point.y() >= (m_yOffset * scale) + m_yCanvasOffset) && (point.y() < ((m_yOffset + m_imageHeight) * scale) + m_yCanvasOffset)) {
        return true;
    }

    return false;
}

double KCanvasPreview::scalingFactor()
{
    double xScale = (double)(height() - 1) / m_height;
    double yScale = (double)(width() - 1) / m_width;

    return (xScale < yScale) ? xScale : yScale;
}

void KCanvasPreview::setImageSize(qint32 w, qint32 h)
{
    m_imageWidth = w;
    m_imageHeight = h;

    update();
}

void KCanvasPreview::setCanvasSize(qint32 w, qint32 h)
{
    m_width = w;
    m_height = h;

    update();
}

void KCanvasPreview::setImageOffset(qint32 x, qint32 y)
{
    m_xOffset = x;
    m_yOffset = y;

    update();
}
