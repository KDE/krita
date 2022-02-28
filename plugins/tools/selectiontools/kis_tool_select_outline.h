/*
 *  kis_tool_select_freehand.h - part of Krayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2000 John Califf <jcaliff@compuzone.net>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_SELECT_OUTLINE_H_
#define KIS_TOOL_SELECT_OUTLINE_H_

#include <QPoint>
#include <KisSelectionToolFactoryBase.h>
#include <KisToolOutlineBase.h>
#include <kis_tool_select_base.h>
#include <kis_icon.h>

class __KisToolSelectOutlineLocal : public KisToolOutlineBase
{
    Q_OBJECT

public:
    __KisToolSelectOutlineLocal(KoCanvasBase * canvas);
};

class KisToolSelectOutline : public KisToolSelectBase<__KisToolSelectOutlineLocal>
{
    Q_OBJECT

public:
    KisToolSelectOutline(KoCanvasBase *canvas);

    bool primaryActionSupportsHiResEvents() const override;
    bool alternateActionSupportsHiResEvents(KisTool::AlternateAction action) const override;
    void resetCursorStyle() override;

private:
    void finishOutline(const QVector<QPointF>& points) override;
    void beginShape() override;
    void endShape() override;
};

class KisToolSelectOutlineFactory : public KisSelectionToolFactoryBase
{
public:
    KisToolSelectOutlineFactory()
        : KisSelectionToolFactoryBase("KisToolSelectOutline")
    {
        setToolTip(i18n("Freehand Selection Tool"));
        setSection(ToolBoxSection::Select);
        setIconName(koIconNameCStr("tool_outline_selection"));
        setPriority(3);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    ~KisToolSelectOutlineFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolSelectOutline(canvas);
    }
};


#endif //__selecttoolfreehand_h__

