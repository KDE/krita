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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/
#ifndef PIXMAPVIEWER_H
#define PIXMAPVIEWER_H

#include <q3scrollview.h>
#include <qimage.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QPixmap>
#include <QLabel>
#include <QWheelEvent>
#include <krita_export.h>

class QLabel;

/**
 * A scrollable image view.
 *
 * XXX: We should add a signal that emits newly eposed rects so the filters
 *      don't have to filter everything, but just the the new bits.
 */
class KRITAUI_EXPORT ImageViewer : public Q3ScrollView {
    Q_OBJECT

public:
    ImageViewer(QWidget *widget, const char * name = 0);

    void setImage(QImage & image);
    
    void contentsMousePressEvent(QMouseEvent *event);
    void contentsMouseReleaseEvent(QMouseEvent *event);
    void contentsMouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent * event) { event->ignore(); };
private:
    QLabel* m_label;
    bool m_isDragging;
    QPoint m_currentPos;
    QPixmap m_image;
};

#endif
