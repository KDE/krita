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

#include <KoClipPath.h>

#include <QPainter>
#include <QResizeEvent>

qreal calcScale(const QSizeF& imgSize, const QSizeF viewSize, bool fitView)
{
    if (qFuzzyCompare(imgSize.width(), qreal(0)) || qFuzzyCompare(imgSize.height(), qreal(0)) ||
        qFuzzyCompare(viewSize.width(), qreal(0)) || qFuzzyCompare(viewSize.height(), qreal(0))) {
        return 1;
    }

    qreal viewAspect = viewSize.width() / viewSize.height();
    qreal imgAspect = imgSize.width() / imgSize.height();

    if (fitView) {
        if (viewAspect > imgAspect) {
            return viewSize.height() / imgSize.height();
        }
        else {
            return viewSize.width()  / imgSize.width();
        }
    }
    else {
        if (viewAspect > imgAspect) {
            return viewSize.width()  / imgSize.width();
        }
        else {
            return viewSize.height() / imgSize.height();
        }
    }
}

QRectF centerRectHorizontally(const QRectF& rect, const QSizeF viewSize)
{
    QSizeF diff = viewSize - rect.size();
    return QRectF(diff.width() / 2.0, rect.y(), rect.width(), rect.height());
}

bool compareRects(const QRectF &a, const QRectF &b, qreal epsilon)
{
    qreal x = qAbs(a.x() - b.x());
    qreal y = qAbs(a.y() - b.y());
    qreal w = qAbs(a.width() - b.width());
    qreal h = qAbs(a.height() - b.height());

    return x <= epsilon && y <= epsilon && w <= epsilon && h <= epsilon;
}

// ---------------------------------------------------------------- //

CropWidget::CropWidget(QWidget *parent):
    QWidget(parent)
    , m_pictureShape(0)
    , m_isMousePressed(false)
    , m_undoLast(false)
{
    setMinimumSize(100, 100);
    setMouseTracking(true);
}

void CropWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if(!m_pictureShape)
        return;

    QPainter painter(this);
    QImage image = m_pictureShape->imageData()->image();

    painter.translate(m_imageRect.topLeft());
    painter.scale(m_imageRect.width(), m_imageRect.height());

    painter.drawImage(QRectF(0, 0, 1, 1), image);
    painter.drawRect(m_selectionRect.getRect());

    painter.setBrush(QBrush(Qt::yellow));
    for (int i=0; i<m_selectionRect.getNumHandles(); ++i)
        painter.drawRect(m_selectionRect.getHandleRect(m_selectionRect.getHandleFlags(i)));

    KoClipPath *clipPath = m_pictureShape->clipPath();
    if (clipPath) {
        painter.scale(0.01, 0.01); // the path is defined in 100x100 equaling shapesize
        painter.setBrush(Qt::NoBrush);
        painter.setPen(Qt::red);
        painter.drawPath(clipPath->path());
    }

}

void CropWidget::mousePressEvent(QMouseEvent *event)
{
    m_selectionRect.beginDragging(toUniformCoord(event->posF()));
    m_isMousePressed = true;
}

void CropWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPointF pos = toUniformCoord(event->posF());
    SelectionRect::HandleFlags flags = m_selectionRect.getHandleFlags(pos);

    switch (flags)
    {
    case SelectionRect::TOP_HANDLE:
    case SelectionRect::BOTTOM_HANDLE:
        QWidget::setCursor(Qt::SizeVerCursor);
        break;

    case SelectionRect::LEFT_HANDLE:
    case SelectionRect::RIGHT_HANDLE:
        QWidget::setCursor(Qt::SizeHorCursor);
        break;

    case SelectionRect::LEFT_HANDLE|SelectionRect::TOP_HANDLE:
    case SelectionRect::RIGHT_HANDLE|SelectionRect::BOTTOM_HANDLE:
        QWidget::setCursor(Qt::SizeFDiagCursor);
        break;

    case SelectionRect::LEFT_HANDLE|SelectionRect::BOTTOM_HANDLE:
    case SelectionRect::RIGHT_HANDLE|SelectionRect::TOP_HANDLE:
        QWidget::setCursor(Qt::SizeBDiagCursor);
        break;

    case SelectionRect::INSIDE_RECT:
        QWidget::setCursor(Qt::SizeAllCursor);
        break;

    default:
        QWidget::setCursor(Qt::ArrowCursor);
        break;
    }

    if (m_isMousePressed) {
        m_selectionRect.doDragging(pos);
        update();
        emitCropRegionChanged();
    }
}

void CropWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_selectionRect.finishDragging();
    m_isMousePressed = false;
    emitCropRegionChanged();
    m_undoLast = false; // we are done dragging
}

void CropWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    calcImageRect();
}

void CropWidget::setPictureShape(PictureShape *shape)
{
    m_pictureShape = shape;

    calcImageRect();
    m_oldSelectionRect = shape->cropRect();
    m_selectionRect.setRect(shape->cropRect());
    m_selectionRect.setConstrainingRect(QRectF(0, 0, 1, 1));
    m_selectionRect.setHandleSize(0.04);
    //emit sigCropRegionChanged(shape->cropRect());
    update();
}

void CropWidget::setCropRect(const QRectF &rect)
{
    m_selectionRect.setRect(rect);
    emitCropRegionChanged();
}

void CropWidget::setKeepPictureProportion(bool keepProportion)
{
    qreal aspect = keepProportion ? (m_pictureShape->size().width() / m_pictureShape->size().height()) : 0.0;
    m_selectionRect.setConstrainingAspectRatio(aspect);
    emitCropRegionChanged();
}

void CropWidget::maximizeCroppedArea()
{
    m_selectionRect.setRect(QRectF(0, 0, 1, 1));
    emitCropRegionChanged();
}

QPointF CropWidget::toUniformCoord(const QPointF& coord) const
{
    QPointF result = coord - m_imageRect.topLeft();
    return QPointF(result.x() / m_imageRect.width(), result.y() / m_imageRect.height());
}

QPointF CropWidget::fromUniformCoord(const QPointF& coord) const
{
    return m_imageRect.topLeft() + QPointF(coord.x()*m_imageRect.width(), coord.y()*m_imageRect.height());
}

void CropWidget::emitCropRegionChanged()
{
    if (!compareRects(m_oldSelectionRect, m_selectionRect.getRect(), 0.01)) {
        m_oldSelectionRect = m_selectionRect.getRect();
        emit sigCropRegionChanged(m_selectionRect.getRect(), m_undoLast);
        update();

        m_undoLast = m_isMousePressed;
    }
}

void CropWidget::calcImageRect()
{
    if (m_pictureShape) {
        QSizeF imageSize = m_pictureShape->imageData()->image().size();
        imageSize = imageSize * calcScale(imageSize, size(), true);
        m_imageRect = centerRectHorizontally (QRect(0, 0, imageSize.width(), imageSize.height()), size());
        m_selectionRect.setAspectRatio(m_imageRect.width() / m_imageRect.height());
    }
    else {
        m_imageRect = QRectF();
    }
}
