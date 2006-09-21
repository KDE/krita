/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_GRID_DRAWER_H
#define KIS_GRID_DRAWER_H

#include <QObject>
#include <QPainter>

#include "kis_types.h"

class KisView;
class KActionCollection;
class KToggleAction;
class KAction;
class KisSubPerspectiveGrid;

class GridDrawer {
    public:
        GridDrawer() {}
        virtual ~GridDrawer() {}
    
    public:
        void drawGrid(KisImageSP image, const QRect& wr);
        void drawPerspectiveGrid(KisImageSP image, const QRect& wr, const KisSubPerspectiveGrid* grid);
    
        virtual void setPen(const QPen& pen) = 0;
        virtual void drawLine(qint32 x1, qint32 y1, qint32 x2, qint32 y2) = 0;
        inline void drawLine(const QPoint& p1, const QPoint& p2) { drawLine(p1.x(), p1.y(), p2.x(), p2.y() ); }
        inline void drawLine(const QPointF* p1, const QPointF* p2) { drawLine( p1->toPoint(), p2->toPoint()); }
    private:
        Qt::PenStyle gs2style(quint32 s);
};

class QPainterGridDrawer : public GridDrawer {
    public:
        QPainterGridDrawer(QPainter *p) { m_painter = p; }
    
        virtual void setPen(const QPen& pen) { m_painter->setPen(pen); }
        virtual void drawLine(qint32 x1, qint32 y1, qint32 x2, qint32 y2) { m_painter->drawLine(x1, y1, x2, y2); }
    
    private:
        QPainter *m_painter;
};

class OpenGLGridDrawer : public GridDrawer {
    public:
        OpenGLGridDrawer();
        virtual ~OpenGLGridDrawer();
    
        virtual void setPen(const QPen& pen);
        virtual void drawLine(qint32 x1, qint32 y1, qint32 x2, qint32 y2);
};

#endif
