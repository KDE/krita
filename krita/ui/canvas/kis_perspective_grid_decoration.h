/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006,2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2014 Sven Langkamp <sven.langkamp@gmail.com>
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


#ifndef KIS_PERSPECTIVE_GRID_DECORATION_H
#define KIS_PERSPECTIVE_GRID_DECORATION_H

#include "kis_canvas_decoration.h"
#include <krita_export.h>

class KisView;

class KRITAUI_EXPORT KisPerspectiveGridDecoration : public KisCanvasDecoration
{
    Q_OBJECT

public:
    KisPerspectiveGridDecoration(QPointer<KisView> parent);
    virtual ~KisPerspectiveGridDecoration();

protected:
    virtual void drawDecoration(QPainter& gc, const QRectF& updateArea, const KisCoordinatesConverter* converter, KisCanvas2* canvas);

private:
    class LineWrapper;
    struct SubdivisionLinesInfo;

    void drawSubdivisions(QPainter& gc, const SubdivisionLinesInfo &info);
    
    SubdivisionLinesInfo getSubdivisionsInfo(const LineWrapper &l0,
                                             const LineWrapper &l1,
                                             const QPointF &focusPoint,
                                             int numSubdivisions);
};

#endif // KIS_PERSPECTIVE_GRID_DECORATION_H
