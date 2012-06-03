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

/**
 * This widget is a graphical user interface to the functionality
 * the SelectionRect class provides. It shows an image and the user can
 * move a rectangle freely to select a certain region of that image.
 */
class CropWidget : public QWidget
{
    Q_OBJECT

public:
    CropWidget(QWidget *parent=0);

    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void resizeEvent(QResizeEvent *event);

    void setPictureShape(PictureShape* shape);
    void setCropRect(const QRectF &rect);
    void setKeepPictureProportion(bool keepProportion);
    void maximizeCroppedArea();

signals:
    void sigCropRegionChanged(const QRectF&, bool undoLast);

private:
    void calcImageRect();
    void emitCropRegionChanged();
    QPointF toUniformCoord(const QPointF& coord) const;
    QPointF fromUniformCoord(const QPointF& coord) const;

private:
    PictureShape *m_pictureShape;
    QRectF m_imageRect;
    QRectF m_oldSelectionRect;
    SelectionRect m_selectionRect;
    bool m_isMousePressed;
    bool m_undoLast;
};

#endif // H_CROP_WIDGET_H
