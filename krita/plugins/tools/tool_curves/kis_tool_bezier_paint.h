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

#include "KoToolFactory.h"
#include "kis_tool_bezier.h"

class KisToolBezierPaint : public KisToolBezier {

    typedef KisToolBezier super;
    Q_OBJECT

public:
    KisToolBezierPaint();
    virtual ~KisToolBezierPaint();

    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_SHAPE; }
    virtual quint32 priority() { return 7; }

protected:

    virtual KisCurve::iterator paintPoint(KisPainter& painter, KisCurve::iterator point);

};

class KisToolBezierPaintFactory : public KoToolFactory {

public:
    KisToolBezierPaintFactory(QObject *parent, const QStringList&)
        : KoToolFactory(parent, "KisToolBezierPaint", i18n( "Paint with curves" ))
        {
            setToolTip(i18n("Draw cubic beziers. Keep Alt, Control or Shift pressed for options. Return or double-click to finish."));
            setToolType(TOOL_TYPE_SHAPE);
            setPriority(0);
            setIcon("tool_bezier_paint");
        };

    virtual ~KisToolBezierPaintFactory(){}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolBezierPaint(canvas);
    }

};

#endif //__KIS_TOOL_CURVE_PAINT_H_
