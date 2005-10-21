/***************************************************************************
 *   Copyright (C) 2005 Eyal Lotem <eyal.lotem@gmail.com>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02111-1307, USA.          *
 ***************************************************************************/
#ifndef PIXMAPVIEWER_H
#define PIXMAPVIEWER_H

#include <qscrollview.h>
#include <qimage.h>

class ImageViewer : public QScrollView {
    Q_OBJECT

public:
    ImageViewer(QWidget *widget, const char * name = 0);

    void setImage(QImage & image);
    
    // The size of this widget that requires no scrollbars
    QSize maximalSize();

    void drawContents( QPainter * p, int clipx, int clipy, int clipw, int cliph );

    void contentsMousePressEvent(QMouseEvent *event);
    void contentsMouseReleaseEvent(QMouseEvent *event);
    void contentsMouseMoveEvent(QMouseEvent *event);

private:
    bool m_isDragging;
    QPoint m_currentPos;
    QImage m_image;
};

#endif
