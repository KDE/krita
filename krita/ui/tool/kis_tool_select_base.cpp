/* This file is part of the KDE project
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_tool_select_base.h"

#include "kis_cursor.h"
#include "kis_canvas2.h"
#include "kis_selection_options.h"


KisToolSelectBase::KisToolSelectBase(KoCanvasBase *canvas,
                                     const QCursor& cursor,
                                     const QString &windowTitle)
    : KisTool(canvas, cursor),
      m_widgetHelper(windowTitle),
      m_selectionAction(SELECTION_REPLACE)
{
    connect(&m_widgetHelper, SIGNAL(selectionActionChanged(int)), this, SLOT(setSelectionAction(int)));
}

SelectionMode KisToolSelectBase::selectionMode() const
{
    return m_widgetHelper.selectionMode();
}

SelectionAction KisToolSelectBase::selectionAction() const
{
    if (m_selectionActionAlternate == SELECTION_DEFAULT) {
        return m_selectionAction;
    }
    return m_selectionActionAlternate;
}

void KisToolSelectBase::setSelectionAction(int newSelectionAction)
{
    if(newSelectionAction >= SELECTION_REPLACE && newSelectionAction <= SELECTION_INTERSECT && m_selectionAction != newSelectionAction)
    {
        if(m_widgetHelper.optionWidget())
        {
            m_widgetHelper.slotSetAction(newSelectionAction);
        }
        m_selectionAction = (SelectionAction)newSelectionAction;
        emit selectionActionChanged();
    }
}

QWidget* KisToolSelectBase::createOptionWidget()
{
    KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(this->canvas());
    Q_ASSERT(canvas);

    m_widgetHelper.createOptionWidget(canvas, this->toolId());
    return m_widgetHelper.optionWidget();
}

KisSelectionOptions* KisToolSelectBase::selectionOptionWidget()
{
    return m_widgetHelper.optionWidget();
}

void KisToolSelectBase::keyPressEvent(QKeyEvent *event)
{
    if (!m_widgetHelper.processKeyPressEvent(event)) {
        KisTool::keyPressEvent(event);
    }
}

void KisToolSelectBase::activateAlternateAction(AlternateAction action)
{
    kDebug() << "Switching to alternate action " << action;
    switch (action) {
    case AlternateSecondary:
        //useCursor(KisCursor::pickerPlusCursor());
        setAlternateSelectionAction(SELECTION_ADD);
        emit selectionActionChanged();
        break;
    case AlternateThird:
        //useCursor(KisCursor::pickerMinusCursor());
        setAlternateSelectionAction(SELECTION_SUBTRACT);
        emit selectionActionChanged();
        break;
    case AlternateFourth:
        //useCursor(KisCursor::pickerMinusCursor());
        setAlternateSelectionAction(SELECTION_INTERSECT);
        emit selectionActionChanged();
        break;
    default:
        KisTool::activateAlternateAction(action);
    };
}


void KisToolSelectBase::setAlternateSelectionAction(SelectionAction action)
{
    m_selectionActionAlternate = action;
}

void KisToolSelectBase::deactivateAlternateAction(AlternateAction action)
{
    if (action != AlternateSecondary &&
        action != AlternateThird &&
        action != AlternateFourth) {
        KisTool::deactivateAlternateAction(action);
        return;
    }

    resetCursorStyle();
    setAlternateSelectionAction(SELECTION_DEFAULT);
    emit selectionActionChanged();
}


void KisToolSelectBase::beginAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (m_selectionActionAlternate == SELECTION_DEFAULT) {
        KisTool::beginAlternateAction(event, action);
    } else {
        beginPrimaryAction(event);
    }

}

void KisToolSelectBase::continueAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (m_selectionActionAlternate == SELECTION_DEFAULT) {
        KisTool::continueAlternateAction(event, action);
    } else {
        continuePrimaryAction(event);
    }

}

void KisToolSelectBase::endAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (m_selectionActionAlternate == SELECTION_DEFAULT) {
        KisTool::endAlternateAction(event, action);
    } else {
        endPrimaryAction(event);
    }
}
