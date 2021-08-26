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
#include "KisSelectionToolFactoryBase.h"
#include <kis_tool_select_base.h>
#include <kis_icon.h>

class QPainterPath;

class KisToolSelectOutline : public KisToolSelect
{
    Q_OBJECT

public:
    KisToolSelectOutline(KoCanvasBase *canvas);
    ~KisToolSelectOutline() override;
    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;
    void paint(QPainter& gc, const KoViewConverter &converter) override;

    bool primaryActionSupportsHiResEvents() const override;
    bool alternateActionSupportsHiResEvents(KisTool::AlternateAction action) const override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void mouseMoveEvent(KoPointerEvent *event) override;

    void resetCursorStyle() override;

public Q_SLOTS:
    void deactivate() override;

protected:
    using KisToolSelectBase::m_widgetHelper;

private:
    void finishSelectionAction();
    void updateFeedback();
    void updateContinuedMode();
    void updateCanvas();

    QPainterPath m_paintPath;
    vQPointF m_points;
    bool m_continuedMode;
    QPointF m_lastCursorPos;
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

