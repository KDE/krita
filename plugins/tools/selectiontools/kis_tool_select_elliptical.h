/*
 *  kis_tool_select_elliptical.h - part of Krayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2000 John Califf <jcaliff@compuzone.net>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org> *
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TOOL_SELECT_ELLIPTICAL_H__
#define __KIS_TOOL_SELECT_ELLIPTICAL_H__

#include "KisSelectionToolFactoryBase.h"
#include "kis_tool_ellipse_base.h"
#include <kis_tool_select_base.h>
#include "kis_selection_tool_config_widget_helper.h"
#include <KoIcon.h>
#include <QKeySequence>
#include <kis_icon.h>
#include <QMenu>



class __KisToolSelectEllipticalLocal : public KisToolEllipseBase
{
    Q_OBJECT

public:
    __KisToolSelectEllipticalLocal(KoCanvasBase *canvas);
    bool hasUserInteractionRunning() const;
protected:
    virtual SelectionMode selectionMode() const = 0;
    virtual SelectionAction selectionAction() const = 0;
    virtual bool antiAliasSelection() const = 0;
private:
    void finishRect(const QRectF &rect, qreal roundCornersX, qreal roundCornersY) override;



};



typedef KisToolSelectBase<__KisToolSelectEllipticalLocal> KisToolSelectEllipticalTemplate;

class KisToolSelectElliptical : public KisToolSelectEllipticalTemplate
{
    Q_OBJECT
public:
    KisToolSelectElliptical(KoCanvasBase* canvas);
    void resetCursorStyle() override;
};

class KisToolSelectEllipticalFactory : public KisSelectionToolFactoryBase
{
public:
    KisToolSelectEllipticalFactory()
        : KisSelectionToolFactoryBase("KisToolSelectElliptical")
    {
        setToolTip(i18n("Elliptical Selection Tool"));
        setSection(ToolBoxSection::Select);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("tool_elliptical_selection"));
        setShortcut(QKeySequence(Qt::Key_J));
        setPriority(1);
    }

    ~KisToolSelectEllipticalFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolSelectElliptical(canvas);
    }

};

#endif //__KIS_TOOL_SELECT_ELLIPTICAL_H__

