/*
 *  kis_tool_select_polygonal.h - part of Krayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2000 John Califf <jcaliff@compuzone.net>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_SELECT_POLYGONAL_H_
#define KIS_TOOL_SELECT_POLYGONAL_H_

#include "KisSelectionToolFactoryBase.h"
#include "kis_tool_polyline_base.h"
#include <kis_tool_select_base.h>
#include "kis_selection_tool_config_widget_helper.h"
#include <kis_icon.h>

class __KisToolSelectPolygonalLocal : public KisToolPolylineBase
{
    Q_OBJECT
public:
    __KisToolSelectPolygonalLocal(KoCanvasBase *canvas);
protected:
    virtual SelectionMode selectionMode() const = 0;
    virtual SelectionAction selectionAction() const = 0;
    virtual bool antiAliasSelection() const = 0;
private:
    void finishPolyline(const QVector<QPointF> &points) override;
private:
};

class KisToolSelectPolygonal : public KisToolSelectBase<__KisToolSelectPolygonalLocal>
{
    Q_OBJECT
public:
    KisToolSelectPolygonal(KoCanvasBase* canvas);
    void resetCursorStyle() override;
};



class KisToolSelectPolygonalFactory : public KisSelectionToolFactoryBase
{
public:
    KisToolSelectPolygonalFactory()
        : KisSelectionToolFactoryBase("KisToolSelectPolygonal")
    {
        setToolTip(i18n("Polygonal Selection Tool"));
        setSection(ToolBoxSection::Select);
        setIconName(koIconNameCStr("tool_polygonal_selection"));
        setPriority(2);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    ~KisToolSelectPolygonalFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolSelectPolygonal(canvas);
    }

    QList<QAction *> createActionsImpl() override
    {
        KisActionRegistry *actionRegistry = KisActionRegistry::instance();
        QList<QAction *> actions = KisSelectionToolFactoryBase::createActionsImpl();

        actions << actionRegistry->makeQAction("undo_polygon_selection");
        actions << actionRegistry->makeQAction("selection_tool_mode_add");

        return actions;
    }


};

#endif //__selecttoolpolygonal_h__

