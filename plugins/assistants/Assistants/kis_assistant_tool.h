/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2017 Scott Petrovic <scottpetrovic@gmail.com>
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

#ifndef _KIS_ASSISTANT_TOOL_H_
#define _KIS_ASSISTANT_TOOL_H_

#include <QPointer>

#include <KoToolFactoryBase.h>
#include <KoIcon.h>

#include <kis_tool.h>
#include "kis_painting_assistant.h"
#include <kis_icon.h>
#include <kis_canvas2.h>

#include "ui_AssistantsToolOptions.h"

/* The assistant tool allows artists to create special guides on the canvas
 * to help them with things like perspective and parallel lines
 * This tool has its own canvas decoration on it that only appears when the tool
 * is active. This decoration allows people to edit assistant points as well as delete assistants
 * Many of the operations here are forwarded on to another class (kis_painting_assistant_decoration)
 * that stores the assistant information as well as the decoration information with lines
 *
 * Drawing in two separate classes creates an issue where the editor controls in this class
 * are covered by the kis_painting_assistant_decoration class. In the future, we probably need to
 * do all the drawing in one class so we have better control of what is in front
 */
class KisAssistantTool : public KisTool
{
    Q_OBJECT
    enum PerspectiveAssistantEditionMode {
        MODE_CREATION, // This is the mode when there is not yet a perspective grid
        MODE_EDITING, // This is the mode when the grid has been created, and we are waiting for the user to click on a control box
        MODE_DRAGGING_NODE, // In this mode one node is translated
        MODE_DRAGGING_TRANSLATING_TWONODES // This mode is used when creating a new sub perspective grid
    };
public:
    KisAssistantTool(KoCanvasBase * canvas);
    ~KisAssistantTool() override;

    virtual quint32 priority() {
        return 3;
    }


    /* this is a very big function that has to figure out if we are adding a new assistant,
     * or editing an existing one when we click on the canvas. There is also a lot of logic
     * in here that is specific to certain assistants and how they should be handled.
     * The editor widget is not a UI file, so the move, delete, preview areas have manual
     * hitbox regions specified to know if a click is doing any of those actions.
     */
    void beginPrimaryAction(KoPointerEvent *event) override;


    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;
    void mouseMoveEvent(KoPointerEvent *event) override;

    QWidget *createOptionWidget() override;

private:
    // adds and removes assistant.
    // this is event is forwarded to the kis_painting_decoration class
    // perspective grids seem to be managed in two places with these calls
    void addAssistant();
    void removeAssistant(KisPaintingAssistantSP assistant);

    void assistantSelected(KisPaintingAssistantSP assistant);

public Q_SLOTS:
    void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes) override;
    void deactivate() override;


    void slotChangeVanishingPointAngle(double value);

private Q_SLOTS:
    void removeAllAssistants();
    void saveAssistants();
    void loadAssistants();
    void updateToolOptionsUI();

    /// send the color and opacity information from the UI to the kis_painting_decoration
    /// which manages the assistants
    void slotGlobalAssistantsColorChanged(const QColor&);
    void slotGlobalAssistantOpacityChanged();

    void slotUpdateCustomColor();
    void slotCustomOpacityChanged();

protected:
    /// Draws the editor widget controls with move, activate, and delete
    /// This also creates a lot of assistant specific stuff for vanishing points and perspective grids
    /// Whatever is painted here will be underneath the content painted in the kis_painting_assistant_decoration
    /// The kis_painting_assistant_decoration paints the final assistant, so this is more of just editor controls
    void paint(QPainter& gc, const KoViewConverter &converter) override;

protected:
    /// this class manipulates the kis_painting_assistant_decorations a lot, so this class is a helper
    ///  to get a reference to it and call "updateCanvas" which refreshes the display
    QPointer<KisCanvas2> m_canvas;

    /// the handles are retrieved from the kis_painting_decoration originally
    /// They are used here to generate and manipulate editor handles with the tool's primary action
    QList<KisPaintingAssistantHandleSP> m_handles;
    QList<KisPaintingAssistantHandleSP> m_sideHandles;
    KisPaintingAssistantHandleSP m_handleDrag;
    KisPaintingAssistantHandleSP m_handleCombine;
    KisPaintingAssistantSP m_assistantDrag;

    /// Used while a new assistant is being created. Most assistants need multiple points to exist
    /// so this helps manage the visual state while this creation process is going on
    KisPaintingAssistantSP m_newAssistant;

    QPointF m_cursorStart;
    QPointF m_currentAdjustment;
    Ui::AssistantsToolOptions m_options;
    QWidget* m_optionsWidget;
    QPointF m_dragStart;
    QLineF m_radius;
    bool m_snapIsRadial;
    QPointF m_dragEnd;
    int m_handleSize; // how large the editor handles will appear


private:
    void drawEditorWidget(KisPaintingAssistantSP assistant, QPainter& _gc);

    PerspectiveAssistantEditionMode m_internalMode;
    KisPaintingAssistantHandleSP m_selectedNode1, m_selectedNode2, m_higlightedNode;
    int m_assistantHelperYOffset; // used by the assistant editor icons for placement on the canvas.
};


class KisAssistantToolFactory : public KoToolFactoryBase
{
public:
    KisAssistantToolFactory()
            : KoToolFactoryBase("KisAssistantTool") {
        setToolTip(i18n("Assistant Tool"));
        setSection(TOOL_TYPE_VIEW);
        setIconName(koIconNameCStr("krita_tool_assistant"));
        setPriority(0);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }


    ~KisAssistantToolFactory() override {}

    KoToolBase * createTool(KoCanvasBase * canvas) override {
        return new KisAssistantTool(canvas);
    }

};


#endif

