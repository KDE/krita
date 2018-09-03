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
    }

    KisToolSelectBase(KoCanvasBase* canvas, const QCursor cursor, const QString toolName)
        :BaseClass(canvas, cursor),
         m_widgetHelper(toolName),
         m_selectionAction(SELECTION_DEFAULT),
         m_selectionActionAlternate(SELECTION_DEFAULT)
    {
    }

    KisToolSelectBase(KoCanvasBase* canvas, QCursor cursor, QString toolName, KisTool *delegateTool)
        :BaseClass(canvas, cursor, delegateTool),
         m_widgetHelper(toolName),
         m_selectionAction(SELECTION_DEFAULT),
         m_selectionActionAlternate(SELECTION_DEFAULT)
    {
    }

    QWidget* createOptionWidget()
    {
        KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(this->canvas());
        Q_ASSERT(canvas);

        m_widgetHelper.createOptionWidget(canvas, this->toolId());
        this->connect(this, SIGNAL(isActiveChanged(bool)), &m_widgetHelper, SLOT(slotToolActivatedChanged(bool)));
        return m_widgetHelper.optionWidget();
    }

    void keyPressEvent(QKeyEvent *event)
    {
        if (!m_widgetHelper.processKeyPressEvent(event)) {
            BaseClass::keyPressEvent(event);
        }
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

    virtual void beginPrimaryAction(KoPointerEvent *event)
    {
        keysAtStart = event->modifiers();

        setAlternateSelectionAction(KisSelectionModifierMapper::map(keysAtStart));
        if (alternateSelectionAction() != SELECTION_DEFAULT) {
            BaseClass::listenToModifiers(false);
        }
        BaseClass::beginPrimaryAction(event);
    }

    virtual void continuePrimaryAction(KoPointerEvent *event)
    {
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

protected:
    using BaseClass::canvas;
    KisSelectionToolConfigWidgetHelper m_widgetHelper;
    SelectionAction m_selectionAction;
    SelectionAction m_selectionActionAlternate;

private:
    Qt::KeyboardModifiers keysAtStart;

};


typedef KisToolSelectBase<KisTool> KisToolSelect;


#endif // KISTOOLSELECTBASE_H
