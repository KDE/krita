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

#ifndef KCANVASPREVIEW_H
#define KCANVASPREVIEW_H

#include <QWidget>

class KCanvasPreview : public QWidget
{
    Q_OBJECT

public:
    KCanvasPreview(QWidget * parent = 0);

    virtual ~KCanvasPreview();

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *event);

    void setImageSize(qint32 w, qint32 h);
    void setCanvasSize(qint32 w, qint32 h);
    void setImageOffset(qint32 x, qint32 y);
signals:
    void sigModifiedXOffset(int);
    void sigModifiedYOffset(int);

protected:
    bool isInRegion(QPoint point);
    double scalingFactor();

protected:
    qint32 m_width, m_height;
    qint32 m_imageWidth, m_imageHeight;
    qint32 m_xOffset, m_yOffset;

    qint16 m_xCanvasOffset, m_yCanvasOffset;
    bool m_dragging;

    QImage m_image;
    QPoint m_prevDragPoint;
};


#endif /* KCANVASPREVIEW_H */
