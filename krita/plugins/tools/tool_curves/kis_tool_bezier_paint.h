/*
 *  kis_tool_curve_paint.h -- part of Krita
 *
 *  Copyright (c) 2006 Emanuele Tamponi <emanuele@valinor.it>
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

#ifndef KIS_TOOL_BEZIER_PAINT_H_
#define KIS_TOOL_BEZIER_PAINT_H_

#include "kis_tool_factory.h"
#include "kis_tool_bezier.h"
#include "kis_point.h"

class KisToolBezierPaint : public KisToolBezier {

    typedef KisToolBezier super;
    Q_OBJECT

public:
    KisToolBezierPaint();
    virtual ~KisToolBezierPaint();

    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_SHAPE; }

protected:

    virtual KisCurve::iterator paintPoint(KisPainter& painter, KisCurve::iterator point);

};

class KisToolBezierPaintFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolBezierPaintFactory() : super() {};
    virtual ~KisToolBezierPaintFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolBezierPaint();
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t;
    }
    virtual KisID id() { return KisID("beziershape", i18n("Bezier painting Tool")); }
};

#endif //__KIS_TOOL_CURVE_PAINT_H_
