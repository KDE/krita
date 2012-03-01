/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_selection_tool_config_widget_helper.h"

#include <QKeyEvent>
#include "kis_selection_options.h"


KisSelectionToolConfigWidgetHelper::KisSelectionToolConfigWidgetHelper(const QString &windowTitle)
    : m_optionWidget(0),
      m_selectionAction(SELECTION_REPLACE),
      m_selectionMode(PIXEL_SELECTION),
      m_windowTitle(windowTitle)
{
}

void KisSelectionToolConfigWidgetHelper::createOptionWidget(KisCanvas2 *canvas, const QString &toolId)
{
    m_optionWidget = new KisSelectionOptions(canvas);
    Q_CHECK_PTR(m_optionWidget);
    m_optionWidget->setObjectName(toolId + "option widget");
    m_optionWidget->setWindowTitle(m_windowTitle);

    connect(m_optionWidget, SIGNAL(actionChanged(int)), this, SLOT(slotSetAction(int)));
    connect(m_optionWidget, SIGNAL(modeChanged(int)), this, SLOT(slotSetSelectionMode(int)));

    m_optionWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_optionWidget->adjustSize();
}

KisSelectionOptions* KisSelectionToolConfigWidgetHelper::optionWidget() const
{
    return m_optionWidget;
}

SelectionMode KisSelectionToolConfigWidgetHelper::selectionMode() const
{
    return m_selectionMode;
}

SelectionAction KisSelectionToolConfigWidgetHelper::selectionAction() const
{
    return m_selectionAction;
}

void KisSelectionToolConfigWidgetHelper::slotSetAction(int action)
{
    if (action >= SELECTION_REPLACE && action <= SELECTION_INTERSECT) {
        m_selectionAction = (SelectionAction)action;
        m_optionWidget->setAction(action);
    }
}

void KisSelectionToolConfigWidgetHelper::slotSetSelectionMode(int mode)
{
    m_selectionMode = (SelectionMode)mode;
}

bool KisSelectionToolConfigWidgetHelper::processKeyPressEvent(QKeyEvent *event)
{
    event->accept();

    switch(event->key()) {
    case Qt::Key_A:
        slotSetAction(SELECTION_ADD);
        break;
    case Qt::Key_S:
        slotSetAction(SELECTION_SUBTRACT);
        break;
    case Qt::Key_R:
        slotSetAction(SELECTION_REPLACE);
        break;
    case Qt::Key_T:
        slotSetAction(SELECTION_INTERSECT);
        break;
    default:
        event->ignore();
    }

    return event->isAccepted();
}
