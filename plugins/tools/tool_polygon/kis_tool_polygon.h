/*
 *  kis_tool_polygon.h - part of Krita
 *
 *  SPDX-FileCopyrightText: 2004 Michael Thaler <michael Thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_POLYGON_H_
#define KIS_TOOL_POLYGON_H_

#include "kis_tool_shape.h"
#include "flake/kis_node_shape.h"
#include <kis_tool_polyline_base.h>
#include <kis_icon.h>

class KoCanvasBase;

class KisToolPolygon : public KisToolPolylineBase
{
    Q_OBJECT

public:
    KisToolPolygon(KoCanvasBase *canvas);
    ~KisToolPolygon() override;
protected:
    void finishPolyline(const QVector<QPointF>& points) override;
protected Q_SLOTS:
    void resetCursorStyle() override;
};


#include "KoToolFactoryBase.h"

class KisToolPolygonFactory : public KisToolPolyLineFactoryBase
{

public:
    KisToolPolygonFactory()
            : KisToolPolyLineFactoryBase("KisToolPolygon") {
        setToolTip(i18n("Polygon Tool: Shift-mouseclick ends the polygon."));
        setSection(TOOL_TYPE_SHAPE);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("krita_tool_polygon"));
        setPriority(4);
    }

    ~KisToolPolygonFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolPolygon(canvas);
    }
};


#endif //__KIS_TOOL_POLYGON_H__
