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

#include "kis_tool_polyline_base.h"
//#include "flake/kis_node_shape.h"
#include <kis_icon.h>

class KisToolPolyline : public KisToolPolylineBase
{

    Q_OBJECT

public:
    KisToolPolyline(KoCanvasBase * canvas);
    ~KisToolPolyline() override;

protected:
    QWidget* createOptionWidget() override;
    void finishPolyline(const QVector<QPointF>& points) override;

protected Q_SLOTS:
    void resetCursorStyle() override;
};


#include "KoToolFactoryBase.h"

class KisToolPolylineFactory : public KisToolPolyLineFactoryBase
{

public:
    KisToolPolylineFactory()
            : KisToolPolyLineFactoryBase("KisToolPolyline") {
        setToolTip(i18n("Polyline Tool: Shift-mouseclick ends the polyline."));
        setSection(TOOL_TYPE_SHAPE);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("polyline"));
        setPriority(5);
    }

    ~KisToolPolylineFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolPolyline(canvas);
    }

};


#endif //__KIS_TOOL_POLYLINE_H__
