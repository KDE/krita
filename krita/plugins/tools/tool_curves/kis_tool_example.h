/*
 *  kis_tool_example.h -- part of Krita
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

#ifndef KIS_TOOL_EXAMPLE_H_
#define KIS_TOOL_EXAMPLE_H_

#include "kis_tool_factory.h"
#include "kis_tool_curve.h"
#include "kis_point.h"

class CurvePoint;
class KisPoint;
class KisCanvas;
class KisCurve;
class KisPainter;
class KisPoint;
class WdgToolExample;

class KisToolExample : public KisToolCurve {

    typedef KisToolCurve super;
    Q_OBJECT

public:
    KisToolExample();
    virtual ~KisToolExample();

    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_SHAPE; }

};

class KisToolExampleFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolExampleFactory() : super() {};
    virtual ~KisToolExampleFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolExample();
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t;
    }
    virtual KisID id() { return KisID("exampleshape", i18n("Example Tool")); }
};


#endif //__KIS_TOOL_EXAMPLE_H__
