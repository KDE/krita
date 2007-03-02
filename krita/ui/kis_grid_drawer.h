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

class KoViewConverter;
class KisView;
class KActionCollection;
class KToggleAction;
class KAction;
class KisSubPerspectiveGrid;
class KisDoc2;

class GridDrawer {
    public:
        GridDrawer(KisDoc2* doc, KoViewConverter * viewConverter) { m_doc = doc; m_viewConverter = viewConverter; }
        virtual ~GridDrawer() {}
    
    public:
        void drawGrid(const QRectF& area);
        void drawPerspectiveGrid(KisImageSP image, const QRect& wr, const KisSubPerspectiveGrid* grid);
    
        virtual void setPen(const QPen& pen) = 0;
        virtual void drawLine(qint32 x1, qint32 y1, qint32 x2, qint32 y2) = 0;
        virtual void drawLine(const QPointF& p1, const QPointF& p2) { drawLine( p1.toPoint(), p2.toPoint()); }
        inline void drawLine(const QPoint& p1, const QPoint& p2) { drawLine(p1.x(), p1.y(), p2.x(), p2.y() ); }
    private:

    protected:
        Qt::PenStyle gs2style(quint32 s);
        KisDoc2* m_doc;
        KoViewConverter * m_viewConverter;
};

class QPainterGridDrawer : public GridDrawer {
    public:
        QPainterGridDrawer(KisDoc2* doc, KoViewConverter * viewConverter);

        virtual void draw(QPainter* p, const QRectF& area);
        virtual void setPen(const QPen& pen) { m_painter->setPen(pen); }
        virtual void drawLine(qint32 x1, qint32 y1, qint32 x2, qint32 y2) { m_painter->drawLine(x1, y1, x2, y2); }
        virtual void drawLine(const QPointF& p1, const QPointF& p2) { m_painter->drawLine( p1, p2); }
    private:
        QPainter* m_painter;
};

class OpenGLGridDrawer : public GridDrawer {
    public:
        OpenGLGridDrawer(KisDoc2* doc, KoViewConverter * viewConverter);
        virtual ~OpenGLGridDrawer();
    
        virtual void setPen(const QPen& pen);
        virtual void drawLine(qint32 x1, qint32 y1, qint32 x2, qint32 y2);
};


#endif
