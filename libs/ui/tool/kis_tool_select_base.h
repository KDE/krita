/* This file is part of the KDE project
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2015 Michael Abrahams <miabraha@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KISTOOLSELECTBASE_H
#define KISTOOLSELECTBASE_H

#include "KoPointerEvent.h"
#include "kis_tool.h"
#include "kis_canvas2.h"
#include "kis_selection.h"
#include "kis_selection_options.h"
#include "kis_selection_tool_config_widget_helper.h"
#include "KisViewManager.h"
#include "kis_selection_manager.h"
#include "kis_selection_modifier_mapper.h"
#include "strokes/move_stroke_strategy.h"
#include "kis_image.h"
#include "kis_cursor.h"
#include "kis_action_manager.h"
#include "kis_action.h"
#include "kis_signal_auto_connection.h"

/**
 * This is a basic template to create selection tools from basic path based drawing tools.
 * The template overrides the ability to execute alternate actions correctly.
 * The default behavior for the modifier keys is as follows:
 *
 * Shift: add to selection
 * Alt: subtract from selection
 * Shift+Alt: intersect current selection
 * Ctrl: replace selection
 *
 * The mapping itself is done in KisSelectionModifierMapper.
 *
 * Certain tools also use modifier keys to alter their behavior, e.g. forcing square proportions with the rectangle tool.
 * The template enables the following rules for forwarding keys:

 * 1) Any modifier keys held *when the tool is first activated* will determine
 * the new selection method. This is recorded in m_selectionActionAlternate. A
 * value of m_selectionActionAlternate = SELECTION_DEFAULT means no modifier was
 * being pressed when the tool was activated.
 *
 * 2) If the underlying tool *does not take modifier keys*, pressing modifier
 * keys in the middle of a stroke will change the selection method. This is
 * recorded in m_selectionAction. A value of SELECTION_DEFAULT means no modifier
 * is being pressed. Applies to the lasso tool and polygon tool.
 *
 * 3) If the underlying tool *takes modifier keys,* they will always be
 * forwarded to the underlying tool, and it is not possible to change the
 * selection method in the middle of a stroke.
 */

template <class BaseClass>
class KisToolSelectBase : public BaseClass
{

public:

    KisToolSelectBase(KoCanvasBase* canvas, const QString toolName)
        :BaseClass(canvas),
         m_widgetHelper(toolName),
         m_selectionAction(SELECTION_DEFAULT),
         m_selectionActionAlternate(SELECTION_DEFAULT)
    {
        KisSelectionModifierMapper::instance();
        initShortcuts();
    }

    KisToolSelectBase(KoCanvasBase* canvas, const QCursor cursor, const QString toolName)
        :BaseClass(canvas, cursor),
         m_widgetHelper(toolName),
         m_selectionAction(SELECTION_DEFAULT),
         m_selectionActionAlternate(SELECTION_DEFAULT)
    {
        KisSelectionModifierMapper::instance();
        initShortcuts();
    }

    KisToolSelectBase(KoCanvasBase* canvas, QCursor cursor, QString toolName, KisTool *delegateTool)
        :BaseClass(canvas, cursor, delegateTool),
         m_widgetHelper(toolName),
         m_selectionAction(SELECTION_DEFAULT),
         m_selectionActionAlternate(SELECTION_DEFAULT)
    {
        KisSelectionModifierMapper::instance();
        initShortcuts();
    }

    void initShortcuts()
    {
        KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
        KisViewManager* viewManager = kiscanvas->viewManager();
        KisActionManager *manager = viewManager->actionManager();

        KisAction *action = 0;

        action = manager->createAction("selection_tool_mode_add");
        this->addAction(action->objectName(), action);

        action = manager->createAction("selection_tool_mode_replace");
        this->addAction(action->objectName(), action);

        action = manager->createAction("selection_tool_mode_subtract");
        this->addAction(action->objectName(), action);

        action = manager->createAction("selection_tool_mode_intersect");
        this->addAction(action->objectName(), action);
    }

    void updateActionShortcutToolTips() {
        KisSelectionOptions *widget = m_widgetHelper.optionWidget();
        if (widget) {
            widget->updateActionButtonToolTip(
                SELECTION_REPLACE,
                this->action("selection_tool_mode_replace")->shortcut());
            widget->updateActionButtonToolTip(
                SELECTION_ADD,
                this->action("selection_tool_mode_add")->shortcut());
            widget->updateActionButtonToolTip(
                SELECTION_SUBTRACT,
                this->action("selection_tool_mode_subtract")->shortcut());
            widget->updateActionButtonToolTip(
                SELECTION_INTERSECT,
                this->action("selection_tool_mode_intersect")->shortcut());
        }
    }

    void activate(KoToolBase::ToolActivation activation, const QSet<KoShape*> &shapes)
    {
        BaseClass::activate(activation, shapes);

        m_modeConnections.addUniqueConnection(
            this->action("selection_tool_mode_replace"), SIGNAL(triggered()),
            &m_widgetHelper, SLOT(slotReplaceModeRequested()));

        m_modeConnections.addUniqueConnection(
            this->action("selection_tool_mode_add"), SIGNAL(triggered()),
            &m_widgetHelper, SLOT(slotAddModeRequested()));

        m_modeConnections.addUniqueConnection(
            this->action("selection_tool_mode_subtract"), SIGNAL(triggered()),
            &m_widgetHelper, SLOT(slotSubtractModeRequested()));

        m_modeConnections.addUniqueConnection(
            this->action("selection_tool_mode_intersect"), SIGNAL(triggered()),
            &m_widgetHelper, SLOT(slotIntersectModeRequested()));

        updateActionShortcutToolTips();
    }

    void deactivate()
    {
        BaseClass::deactivate();
        m_modeConnections.clear();
    }

    QWidget* createOptionWidget()
    {
        KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(this->canvas());
        Q_ASSERT(canvas);

        m_widgetHelper.createOptionWidget(canvas, this->toolId());
        this->connect(this, SIGNAL(isActiveChanged(bool)), &m_widgetHelper, SLOT(slotToolActivatedChanged(bool)));

        updateActionShortcutToolTips();

        return m_widgetHelper.optionWidget();
    }

    SelectionMode selectionMode() const
    {
        return m_widgetHelper.selectionMode();
    }

    SelectionAction selectionAction() const
    {
        if (alternateSelectionAction() == SELECTION_DEFAULT) {
            return m_widgetHelper.selectionAction();
        }
        return alternateSelectionAction();
    }

    bool antiAliasSelection() const
    {
        return m_widgetHelper.optionWidget()->antiAliasSelection();
    }

    SelectionAction alternateSelectionAction() const
    {
        return m_selectionActionAlternate;
    }

    KisSelectionOptions* selectionOptionWidget()
    {
        return m_widgetHelper.optionWidget();
    }

    virtual void setAlternateSelectionAction(SelectionAction action)
    {
        m_selectionActionAlternate = action;
        dbgKrita << "Changing to selection action" << m_selectionActionAlternate;
    }

    void activateAlternateAction(KisTool::AlternateAction action)
    {
        Q_UNUSED(action);
        BaseClass::activatePrimaryAction();
    }

    void deactivateAlternateAction(KisTool::AlternateAction action)
    {
        Q_UNUSED(action);
        BaseClass::deactivatePrimaryAction();
    }

    void beginAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action) {
        Q_UNUSED(action);
        beginPrimaryAction(event);
    }

    void continueAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action) {
        Q_UNUSED(action);
        continuePrimaryAction(event);
    }

    void endAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action) {
        Q_UNUSED(action);
        endPrimaryAction(event);
    }

    KisNodeSP locateSelectionMaskUnderCursor(const QPointF &pos, Qt::KeyboardModifiers modifiers) {
        if (modifiers != Qt::NoModifier) return 0;

        KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(this->canvas());
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(canvas, 0);

        KisSelectionSP selection = canvas->viewManager()->selection();
        if (selection &&
            selection->outlineCacheValid() &&
            selection->outlineCache().contains(pos)) {

            KisNodeSP parent = selection->parentNode();
            if (parent && parent->isEditable()) {
                return parent;
            }
        }

        return 0;
    }

    void mouseMoveEvent(KoPointerEvent *event) {
        if (!this->hasUserInteractionRunning() &&
           (m_moveStrokeId || this->mode() != KisTool::PAINT_MODE)) {

            const QPointF pos = this->convertToPixelCoord(event->point);
            KisNodeSP selectionMask = locateSelectionMaskUnderCursor(pos, event->modifiers());
            if (selectionMask) {
                this->useCursor(KisCursor::moveCursor());
            } else {
                this->resetCursorStyle();
            }
        }

        BaseClass::mouseMoveEvent(event);
    }


    virtual void beginPrimaryAction(KoPointerEvent *event)
    {
        if (!this->hasUserInteractionRunning()) {
            const QPointF pos = this->convertToPixelCoord(event->point);
            KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(this->canvas());
            KIS_SAFE_ASSERT_RECOVER_RETURN(canvas);

            KisNodeSP selectionMask = locateSelectionMaskUnderCursor(pos, event->modifiers());
            if (selectionMask) {
                KisStrokeStrategy *strategy = new MoveStrokeStrategy({selectionMask}, this->image().data(), this->image().data());
                m_moveStrokeId = this->image()->startStroke(strategy);
                m_dragStartPos = pos;

                return;
            }
        }

        keysAtStart = event->modifiers();

        setAlternateSelectionAction(KisSelectionModifierMapper::map(keysAtStart));
        if (alternateSelectionAction() != SELECTION_DEFAULT) {
            BaseClass::listenToModifiers(false);
        }
        BaseClass::beginPrimaryAction(event);
    }

    virtual void continuePrimaryAction(KoPointerEvent *event)
    {
        if (m_moveStrokeId) {
            const QPointF pos = this->convertToPixelCoord(event->point);
            const QPoint offset((pos - m_dragStartPos).toPoint());

            this->image()->addJob(m_moveStrokeId, new MoveStrokeStrategy::Data(offset));
            return;
        }


        //If modifier keys have changed, tell the base tool it can start capturing modifiers
        if ((keysAtStart != event->modifiers()) && !BaseClass::listeningToModifiers()) {
            BaseClass::listenToModifiers(true);
        }

        //Always defer to the base class if it signals it is capturing modifier keys
        if (!BaseClass::listeningToModifiers()) {
            setAlternateSelectionAction(KisSelectionModifierMapper::map(event->modifiers()));
        }

        BaseClass::continuePrimaryAction(event);
    }

    void endPrimaryAction(KoPointerEvent *event)
    {
        if (m_moveStrokeId) {
            this->image()->endStroke(m_moveStrokeId);

            m_moveStrokeId.clear();
            return;
        }


        keysAtStart = Qt::NoModifier; //reset this with each action
        BaseClass::endPrimaryAction(event);
    }

    void changeSelectionAction(int newSelectionAction)
    {
        // Simple sanity check
        if(newSelectionAction >= SELECTION_REPLACE &&
           newSelectionAction <= SELECTION_INTERSECT &&
           m_selectionAction != newSelectionAction)
        {
            m_selectionAction = (SelectionAction)newSelectionAction;
        }
    }

    bool selectionDragInProgress() const {
        return m_moveStrokeId;
    }

protected:
    using BaseClass::canvas;
    KisSelectionToolConfigWidgetHelper m_widgetHelper;
    SelectionAction m_selectionAction;
    SelectionAction m_selectionActionAlternate;

private:
    Qt::KeyboardModifiers keysAtStart;

    QPointF m_dragStartPos;
    KisStrokeId m_moveStrokeId;

    KisSignalAutoConnectionsStore m_modeConnections;
};

struct FakeBaseTool : KisTool
{
    FakeBaseTool(KoCanvasBase* canvas)
        : KisTool(canvas, QCursor())
    {
    }

    FakeBaseTool(KoCanvasBase* canvas, const QString &toolName)
        : KisTool(canvas, QCursor())
    {
        Q_UNUSED(toolName);
    }

    FakeBaseTool(KoCanvasBase* canvas, const QCursor &cursor)
        : KisTool(canvas, cursor)
    {
    }

    bool hasUserInteractionRunning() const {
        return false;
    }

};


typedef KisToolSelectBase<FakeBaseTool> KisToolSelect;


#endif // KISTOOLSELECTBASE_H
