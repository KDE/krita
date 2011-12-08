/* This file is part of the KDE project
   Copyright 2011 Silvio Heinrich <plassy@web.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "CropWidget.h"
#include "PictureShape.h"
#include "KoImageData.h"

#include <QPainter>
#include <QResizeEvent>

qreal calcScale(const QSizeF& imgSize, const QSizeF viewSize, bool fitView)
{
    qreal viewAspect = viewSize.width() / viewSize.height();
    qreal imgAspect  = imgSize.width() / imgSize.height();
    
    if (fitView) {
        if(viewAspect > imgAspect) { return viewSize.height() / imgSize.height(); }
        else                       { return viewSize.width()  / imgSize.width();  }
    }
    else {
        if(viewAspect > imgAspect) { return viewSize.width()  / imgSize.width();  }
        else                       { return viewSize.height() / imgSize.height(); }
    }
}

QRectF centerRect(const QRectF& rect, const QSizeF viewSize)
{
    QSizeF diff = viewSize - rect.size();
    return QRectF(diff.width() / 2.0, diff.height() / 2.0, rect.width(), rect.height());
}

// ---------------------------------------------------------------- //

CropWidget::CropWidget()
{
}

void CropWidget::paintEvent(QPaintEvent *event)
{
    if(!m_pictureShape)
        return;
    
    QPainter painter(this);
    QImage   image = m_pictureShape->imageData()->image();

    painter.translate(m_imageRect.topLeft());
    painter.scale(m_imageRect.width(), m_imageRect.height());
    
    painter.drawImage(QRectF(0, 0, 1, 1), image);
    painter.drawRect(m_selectionRect.getRect());
    
    for (int i=0; i<m_selectionRect.getNumHandles(); ++i)
        painter.fillRect(m_selectionRect.getHandleRect(m_selectionRect.getHandleFlags(i)), Qt::blue);
}

void CropWidget::mousePressEvent(QMouseEvent *event)
{
    m_selectionRect.beginDragging(toImageCoord(event->posF()));
}

void CropWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_selectionRect.doDragging(toImageCoord(event->posF()));
    update();
}

void CropWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_selectionRect.finishDragging();
    
    if (m_pictureShape) {
        m_pictureShape->setCropRect(m_selectionRect.getRect());
    }
    
    update();
}

void CropWidget::setPictureShape(PictureShape *shape)
{
    m_pictureShape = shape;
    
    calcImageRect();
    m_selectionRect.setRect(shape->cropRect());
    m_selectionRect.setConstrainingRect(QRectF(0, 0, 1, 1));
    m_selectionRect.setHandleSize(0.04);
}

QPointF CropWidget::toImageCoord(const QPointF& coord) const
{
    QPointF result = coord - m_imageRect.topLeft();
    return QPointF(result.x() / m_imageRect.width(), result.y() / m_imageRect.height());
}

QPointF CropWidget::fromImageCoord(const QPointF& coord) const
{
    return m_imageRect.topLeft() + QPointF(coord.x()*m_imageRect.width(), coord.y()*m_imageRect.height());
}

void CropWidget::calcImageRect()
{
    if (m_pictureShape) {
        QSizeF imageSize = m_pictureShape->imageData()->imageSize();
        imageSize = imageSize * calcScale(imageSize, size(), true);
        m_imageRect = centerRect(QRect(0, 0, imageSize.width(), imageSize.height()), size());
    }
    else m_imageRect = QRectF();
}

void CropWidget::resizeEvent(QResizeEvent* event)
{
    calcImageRect();
}

