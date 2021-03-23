/*
 *
 *  SPDX-FileCopyrightText: 2009 Edward Apap <schumifer@hotmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
        int newXOffset , newYOffset;
        if(!xOffsetLocked()) {
          newXOffset = ((event->pos().x() - m_prevDragPoint.x()) / scale) + m_xOffset;
        }
        if(!yOffsetLocked()) {
          newYOffset = ((event->pos().y() - m_prevDragPoint.y()) / scale) + m_yOffset;
        }
        m_prevDragPoint = event->pos();
        if(!xOffsetLocked()) {
          emit sigModifiedXOffset(newXOffset);
        }
        if(!yOffsetLocked()) {
          emit sigModifiedYOffset(newYOffset);
        }
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
    // take into account offset frame size to show when the canvas has been shrinked
    const int maxHeight = (m_height > m_imageHeight) ? m_height : m_imageHeight;
    const int maxWidth = (m_width > m_imageWidth) ? m_width : m_imageWidth;

    const double xScale = (double)(height() - 1) / maxHeight;
    const double yScale = (double)(width() - 1) / maxWidth;

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
bool KCanvasPreview::xOffsetLocked() const
{
    return m_xOffsetLocked;
}
void KCanvasPreview::setxOffsetLock(bool value)
{
    m_xOffsetLocked = value;
}
bool KCanvasPreview::yOffsetLocked() const
{
    return m_yOffsetLocked;
}
void KCanvasPreview::setyOffsetLock(bool value)
{
    m_yOffsetLocked = value;
}
