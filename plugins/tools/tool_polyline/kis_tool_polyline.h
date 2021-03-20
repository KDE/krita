/*
 *  kis_tool_polyline.h - part of Krita
 *
 *  SPDX-FileCopyrightText: 2004 Michael Thaler <michael Thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
