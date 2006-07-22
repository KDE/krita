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

#include <math.h>

#include <qobject.h>
#include <qpainter.h>

#include "kis_types.h"
#include "kis_point.h"

class KisSubPerspectiveGrid;

struct LineEquation {
    // y = a*x + b
    double a, b;
};

class GridDrawer {
    public:
        GridDrawer() {}
        virtual ~GridDrawer() {}
    
    public:
        void drawGrid(KisImageSP image, const QRect& wr);
        void drawPerspectiveGrid(KisImageSP image, const QRect& wr, const KisSubPerspectiveGrid* grid);
    
        virtual void setPen(const QPen& pen) = 0;
        virtual void drawLine(Q_INT32 x1, Q_INT32 y1, Q_INT32 x2, Q_INT32 y2) = 0;
        inline void drawLine(const QPoint& p1, const QPoint& p2) { drawLine(p1.x(), p1.y(), p2.x(), p2.y() ); }
        inline void drawLine(const KisPoint* p1, const KisPoint* p2) { drawLine( p1->roundQPoint(), p2->roundQPoint()); }
    private:
        inline LineEquation computeLineEquation(const KisPoint* p1, const KisPoint* p2) const
        {
            LineEquation eq;
            double x1 = p1->x(); double x2 = p2->x();
            if( fabs(x1 - x2) < 0.000001 )
            {
                x1 += 0.00001; // Introduce a small perturbation
            }
            eq.a = (p2->y() - p1->y()) / (double)( x2 - x1 );
            eq.b = -eq.a * x1 + p1->y();
            return eq;
        }
        inline KisPoint computeIntersection(const LineEquation& d1, const LineEquation& d2) const
        {
            double a1 = d1.a; double a2 = d2.a;
            if( fabs(a1 - a2) < 0.000001 )
            {
                a1 += 0.00001; // Introduce a small perturbation
            }
            double x = (d1.b - d2.b) / (a2 - a1);
            return KisPoint(x, a2 * x + d2.b);
        }
        Qt::PenStyle gs2style(Q_UINT32 s);
};

class QPainterGridDrawer : public GridDrawer {
public:
    QPainterGridDrawer(QPainter *p) { m_painter = p; }

    virtual void setPen(const QPen& pen) { m_painter->setPen(pen); }
    virtual void drawLine(Q_INT32 x1, Q_INT32 y1, Q_INT32 x2, Q_INT32 y2) { m_painter->drawLine(x1, y1, x2, y2); }

private:
    QPainter *m_painter;
};

class OpenGLGridDrawer : public GridDrawer {
public:
    OpenGLGridDrawer();
    virtual ~OpenGLGridDrawer();

    virtual void setPen(const QPen& pen);
    virtual void drawLine(Q_INT32 x1, Q_INT32 y1, Q_INT32 x2, Q_INT32 y2);
};

#endif
