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

#ifndef KIS_GRID_PAINTER_CONFIGURATION_H
#define KIS_GRID_PAINTER_CONFIGURATION_H

class QPen;

class KisGridPainterConfiguration
{
public:
    /**
     * @return the pen use for drawing the main line of the grid
     */
    static QPen mainPen();
    /**
     * @return the pen use for drawing the subdivision line of the grid
     */
    static QPen subdivisionPen();
};

#if 0

#include <QObject>
#include <QPainter>

#include "kis_types.h"

class KoViewConverter;
class KisSubPerspectiveGrid;
class KisDoc2;


class KisGridDrawer
{

public:

    KisGridDrawer(KisDoc2* doc, const KoViewConverter * viewConverter);
    virtual ~KisGridDrawer() {}

public:

    void drawGrid(const QRectF& area);
    void drawPerspectiveGrid(KisImageWSP image, const QRect& wr, const KisSubPerspectiveGrid* grid);

    virtual void setPen(const QPen& pen) = 0;
    virtual void drawLine(qint32 x1, qint32 y1, qint32 x2, qint32 y2) = 0;
    virtual void drawLine(const QPointF& p1, const QPointF& p2) {
        drawLine(p1.toPoint(), p2.toPoint());
    }
    inline void drawLine(const QPoint& p1, const QPoint& p2) {
        drawLine(p1.x(), p1.y(), p2.x(), p2.y());
    }

protected:

    Qt::PenStyle gs2style(quint32 s);
    KisDoc2* m_doc;
    const KoViewConverter * m_viewConverter;
};


class QPainterGridDrawer : public KisGridDrawer
{

public:

    QPainterGridDrawer(KisDoc2* doc, const KoViewConverter * viewConverter);

    void setPainter(QPainter* p) {
        m_painter = p;
    }
    void setPen(const QPen& pen) {
        m_painter->setPen(pen);
    }
    void drawLine(qint32 x1, qint32 y1, qint32 x2, qint32 y2) {
        m_painter->drawLine(x1, y1, x2, y2);
    }
    void drawLine(const QPointF& p1, const QPointF& p2) {
        m_painter->drawLine(p1, p2);
    }

private:

    QPainter* m_painter;
};

#endif

#endif
