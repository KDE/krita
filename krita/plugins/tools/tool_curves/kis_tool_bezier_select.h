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

#ifndef KIS_TOOL_BEZIER_SELECT_H_
#define KIS_TOOL_BEZIER_SELECT_H_



#include "KoToolFactoryBase.h"
#include "kis_tool_bezier.h"

class KisToolBezierSelect : public KisToolBezier
{

    Q_OBJECT

public:
    KisToolBezierSelect();
    virtual ~KisToolBezierSelect();

    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() {
        return TOOL_SELECT;
    }
    virtual quint32 priority() {
        return 10;
    }

protected:

    virtual QVector<QPointF> convertCurve();


};

class KisToolBezierSelectFactory : public KoToolFactoryBase
{

public:
    KisToolBezierSelectFactory(QObject *parent, const QStringList&)
            : KoToolFactoryBase(parent, "KisToolBezierSelect") {
        setToolTip(i18n("Select an area of the image with curves"));
        setToolType(TOOL_TYPE_SELECTED);
        setPriority(0);
        setIcon("tool_bezier_select");
    };

    virtual ~KisToolBezierSelectFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolBezierSelect(canvas);
    }

};

#endif //__KIS_TOOL_CURVE_PAINT_H_
