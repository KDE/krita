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

#ifndef H_CROP_WIDGET_H
#define H_CROP_WIDGET_H

#include <QWidget>
#include <QImage>
#include "SelectionRect.h"

class PictureShape;

class CropWidget : public QWidget
{
    Q_OBJECT

public:
    CropWidget();
    
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void resizeEvent(QResizeEvent *event);

    void setPictureShape(PictureShape* shape);

private:
    void calcImageRect();
    QPointF toImageCoord(const QPointF& coord) const;
    QPointF fromImageCoord(const QPointF& coord) const;
    
private:
    PictureShape *m_pictureShape;
    QRectF m_imageRect;
    SelectionRect m_selectionRect;
};

#endif // H_CROP_WIDGET_H
