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

    ~KCanvasPreview() override;

    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void setImageSize(qint32 w, qint32 h);
    void setCanvasSize(qint32 w, qint32 h);
    void setImageOffset(qint32 x, qint32 y);
Q_SIGNALS:
    void sigModifiedXOffset(int);
    void sigModifiedYOffset(int);

protected:
    bool isInRegion(QPoint point);
    double scalingFactor();

protected:
    qint32 m_width {0}, m_height {0};
    qint32 m_imageWidth {0}, m_imageHeight {0};
    qint32 m_xOffset {0}, m_yOffset {0};

    qint16 m_xCanvasOffset {0}, m_yCanvasOffset {0};
    bool m_dragging {false};

    QImage m_image;
    QPoint m_prevDragPoint;
};


#endif /* KCANVASPREVIEW_H */
