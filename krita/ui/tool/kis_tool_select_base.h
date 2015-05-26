/* This file is part of the KDE project
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2015 Michael Abrahams <Michael Abrahams>
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

#ifndef KIS_SELECTION_ACTION_TEMPLATE_H
#define KIS_SELECTION_ACTION_TEMPLATE_H

#include "KoPointerEvent.h"
#include "kis_tool.h"
#include "kis_canvas2.h"
#include "kis_selection.h"
#include "kis_selection_options.h"
#include "kis_selection_tool_config_widget_helper.h"
#include "KisViewManager.h"
#include "kis_selection_manager.h"
#include <bitset>


#define QMOD_BINARY() QString(std::bitset<sizeof(int) * 8>(event->modifiers()).to_string().c_str())


static SelectionAction selectionModifierMap(Qt::KeyboardModifiers m) {
    SelectionAction newAction = SELECTION_DEFAULT;
    if (m & Qt::ControlModifier) {
        newAction = SELECTION_REPLACE;
    } else if ((m & Qt::ShiftModifier) && (m & Qt::AltModifier)) {
        newAction = SELECTION_INTERSECT;
    } else if (m & Qt::ShiftModifier) {
        newAction = SELECTION_ADD;
    } else if (m & Qt::AltModifier) {
        newAction = SELECTION_SUBTRACT;
    }
    return newAction;
}

template <class BaseClass>
    class SelectionActionHandler : public BaseClass
{

public:

    SelectionActionHandler(KoCanvasBase* canvas, QString toolName)
        :BaseClass(canvas),
         m_widgetHelper(toolName),
         m_selectionAction(SELECTION_DEFAULT),
         m_selectionActionAlternate(SELECTION_DEFAULT)
    {
    }

    SelectionActionHandler(KoCanvasBase* canvas, QCursor cursor, QString toolName)
        :BaseClass(canvas, cursor),
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
        m_widgetHelper.optionWidget()->disableAntiAliasSelectionOption();
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

    //hmm, when is this used?
    KisSelectionOptions* selectionOptionWidget()
    {
        return m_widgetHelper.optionWidget();
    }


    void setAlternateSelectionAction(SelectionAction action)
    {
        m_selectionActionAlternate = action;
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
        kDebug() << "endAlternateAction " << "; selectionAction=" << selectionAction() << "; modifiers=" << QMOD_BINARY();;
        endPrimaryAction(event);
    }

    virtual void beginPrimaryAction(KoPointerEvent *event)
    {
        keysAtStart = event->modifiers();

        setAlternateSelectionAction(selectionModifierMap(keysAtStart));
        if (alternateSelectionAction() != SELECTION_DEFAULT) {
            kDebug() << "beginPrimaryAction" << alternateSelectionAction()
                     << "; modifiers =" << QMOD_BINARY();
        } else {
            kDebug() << "Starting selection action" << SELECTION_DEFAULT << "(default)";
        }

        //Start in greedy mode & tell base class not to listen to modifiers
        //Ctrl key (SELECTION_REPLACE) forwards all further keyboard input to the underlying tool
        if (alternateSelectionAction() != SELECTION_REPLACE) {
            BaseClass::listenToModifiers(false);
        }

        BaseClass::beginPrimaryAction(event);
    }

    virtual void continuePrimaryAction(KoPointerEvent *event)
    {
        //Tell the base tool it can start capturing modifiers if it pleases
        if ((keysAtStart != event->modifiers()) && !BaseClass::listeningToModifiers()) {
                BaseClass::listenToModifiers(true);
        }

        //Either we are performing the action in greedy mode or the base tool won't take modifiers
        if ((keysAtStart == Qt::NoModifier) || !BaseClass::listeningToModifiers()) {
            setAlternateSelectionAction(selectionModifierMap(event->modifiers()));
        }

        BaseClass::continuePrimaryAction(event);
    }

    void endPrimaryAction(KoPointerEvent *event)
    {
        kDebug() << "Ending selection action " << alternateSelectionAction();
        keysAtStart = Qt::NoModifier; //reset this with each action
        BaseClass::endPrimaryAction(event);
    }


protected:
    using BaseClass::canvas;
    KisSelectionToolConfigWidgetHelper m_widgetHelper;
    SelectionAction m_selectionAction;
    SelectionAction m_selectionActionAlternate;

private:
    Qt::KeyboardModifiers keysAtStart;
};


typedef SelectionActionHandler<KisTool> KisToolSelectBase;


#endif // KIS_SELECTION_ACTION_TEMPLATE_H
