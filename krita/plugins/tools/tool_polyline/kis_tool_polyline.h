/*
 *  kis_tool_polyline.h - part of Krita
 *
 *  Copyright (c) 2004 Michael Thaler <michael Thaler@physik.tu-muenchen.de>
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

#ifndef KIS_TOOL_POLYLINE_H_
#define KIS_TOOL_POLYLINE_H_

#include <QString>

#include "kis_tool_polyline_base.h"
//#include "flake/kis_node_shape.h"




class KisToolPolyline : public KisToolPolylineBase
{

    Q_OBJECT

public:
    KisToolPolyline(KoCanvasBase * canvas);
    virtual ~KisToolPolyline();

protected:
    virtual void finishPolyline(const QVector<QPointF>& points);
};


#include "KoToolFactory.h"

class KisToolPolylineFactory : public KoToolFactory
{

public:
    KisToolPolylineFactory(QObject *parent, const QStringList&)
            : KoToolFactory(parent, "KisToolPolyline") {
        setToolTip(i18n("Draw a polyline. Shift-mouseclick ends the polyline."));
        setToolType(TOOL_TYPE_SHAPE);
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
        setIcon("polyline");
        setPriority(5);
        setInputDeviceAgnostic(false);
    }

    virtual ~KisToolPolylineFactory() {}

    virtual KoTool * createTool(KoCanvasBase *canvas) {
        return new KisToolPolyline(canvas);
    }

};


#endif //__KIS_TOOL_POLYLINE_H__
