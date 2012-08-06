/*
 *  kis_tool_select_polygonal.h - part of Krayon^WKrita
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_TOOL_SELECT_POLYGONAL_H_
#define KIS_TOOL_SELECT_POLYGONAL_H_

#include "KoToolFactoryBase.h"
#include "kis_tool_polyline_base.h"
#include "kis_selection_tool_config_widget_helper.h"
#include <KoIcon.h>


class KisToolSelectPolygonal : public KisToolPolylineBase
{
    Q_OBJECT

public:
    KisToolSelectPolygonal(KoCanvasBase *canvas);
    QWidget* createOptionWidget();

private:
    void keyPressEvent(QKeyEvent *event);
    void finishPolyline(const QVector<QPointF> &points);
private:
    KisSelectionToolConfigWidgetHelper m_widgetHelper;
};


class KisToolSelectPolygonalFactory : public KoToolFactoryBase
{

public:
    KisToolSelectPolygonalFactory(const QStringList&)
            : KoToolFactoryBase("KisToolSelectPolygonal") {
        setToolTip(i18n("Select a polygonal region"));
        setToolType(TOOL_TYPE_SELECTED);
        setIconName(koIconNameCStr("tool_polygonal_selection"));
        setPriority(54);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    virtual ~KisToolSelectPolygonalFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolSelectPolygonal(canvas);
    }
};

#endif //__selecttoolpolygonal_h__

