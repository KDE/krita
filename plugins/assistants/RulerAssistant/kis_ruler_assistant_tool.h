/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_RULER_ASSISTANT_TOOL_H_
#define _KIS_RULER_ASSISTANT_TOOL_H_

#include <QPointer>

#include <KoToolFactoryBase.h>
#include <KoIcon.h>

#include <kis_tool.h>
#include "kis_painting_assistant.h"
#include <kis_icon.h>
#include <kis_canvas2.h>

#include "ui_AssistantsToolOptions.h"

class KisRulerAssistantTool : public KisTool
{
    Q_OBJECT
    enum PerspectiveAssistantEditionMode {
        MODE_CREATION, // This is the mode when there is not yet a perspective grid
        MODE_EDITING, // This is the mode when the grid has been created, and we are waiting for the user to click on a control box
        MODE_DRAGGING_NODE, // In this mode one node is translated
        MODE_DRAGGING_TRANSLATING_TWONODES // This mode is used when creating a new sub perspective grid
    };
public:
    KisRulerAssistantTool(KoCanvasBase * canvas);
    ~KisRulerAssistantTool() override;

    virtual quint32 priority() {
        return 3;
    }
    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;
    void mouseMoveEvent(KoPointerEvent *event) override;

    QWidget *createOptionWidget() override;

private:
    void addAssistant();
    void removeAssistant(KisPaintingAssistantSP assistant);
    void snappingOn(KisPaintingAssistantSP assistant);
    void snappingOff(KisPaintingAssistantSP assistant);
    void outlineOn(KisPaintingAssistantSP assistant);
    void outlineOff(KisPaintingAssistantSP assistant);
    bool mouseNear(const QPointF& mousep, const QPointF& point);
    KisPaintingAssistantHandleSP nodeNearPoint(KisPaintingAssistantSP grid, QPointF point);
    QPointF snapToGuide(KoPointerEvent *e, const QPointF &offset, bool useModifiers);
    QPointF snapToGuide(const QPointF& pt, const QPointF &offset);

public Q_SLOTS:
    void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes) override;
    void deactivate() override;

private Q_SLOTS:
    void removeAllAssistants();
    void saveAssistants();
    void loadAssistants();

protected:

    void paint(QPainter& gc, const KoViewConverter &converter) override;

protected:
    QPointer<KisCanvas2> m_canvas;
    QList<KisPaintingAssistantHandleSP> m_handles;
    QList<KisPaintingAssistantHandleSP> m_sideHandles;
    KisPaintingAssistantHandleSP m_handleDrag;
    KisPaintingAssistantHandleSP m_handleCombine;
    KisPaintingAssistantSP m_assistantDrag;
    KisPaintingAssistantSP m_newAssistant;
    QPointF m_cursorStart;
    QPointF m_currentAdjustment;
    Ui::AssistantsToolOptions m_options;
    QWidget* m_optionsWidget;
    QPointF m_dragStart;
    QLineF m_radius;
    bool m_snapIsRadial;
    QPointF m_dragEnd;

private:
    PerspectiveAssistantEditionMode m_internalMode;
    qint32 m_handleSize, m_handleHalfSize;
    KisPaintingAssistantHandleSP m_selectedNode1, m_selectedNode2, m_higlightedNode;
    int m_assistantHelperYOffset;
};


class KisRulerAssistantToolFactory : public KoToolFactoryBase
{
public:
    KisRulerAssistantToolFactory()
            : KoToolFactoryBase("KisRulerAssistantTool") {
        setToolTip(i18n("Assistant Tool"));
        setSection(TOOL_TYPE_VIEW);
        setIconName(koIconNameCStr("krita_tool_ruler_assistant"));
        setPriority(0);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    };


    ~KisRulerAssistantToolFactory() override {}

    KoToolBase * createTool(KoCanvasBase * canvas) override {
        return new KisRulerAssistantTool(canvas);
    }

};


#endif

