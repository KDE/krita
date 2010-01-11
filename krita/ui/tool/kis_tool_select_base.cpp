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
#include "canvas/kis_canvas2.h"
#include "kis_selection_options.h"
#include <QVBoxLayout>
#include <QKeyEvent>
#include <KAction>
#include <KActionCollection>
#include "kis_view2.h"

KisToolSelectBase::KisToolSelectBase(KoCanvasBase *canvas, const QCursor& cursor) :
        KisTool(canvas, cursor),
        m_optWidget(0),
        m_selectAction(SELECTION_REPLACE),
        m_selectionMode(PIXEL_SELECTION)
{
}


QWidget* KisToolSelectBase::createOptionWidget()
{
    KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(this->canvas());
    Q_ASSERT(canvas);
    m_optWidget = new KisSelectionOptions(canvas);
    Q_CHECK_PTR(m_optWidget);
    m_optWidget->setObjectName(toolId() + " option widget");
    m_optWidget->setWindowTitle(i18n("Selection"));

    connect(m_optWidget, SIGNAL(actionChanged(int)), this, SLOT(slotSetAction(int)));
    connect(m_optWidget, SIGNAL(modeChanged(int)), this, SLOT(slotSetSelectionMode(int)));

    m_optWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_optWidget->adjustSize();

    return m_optWidget;
}

QWidget* KisToolSelectBase::optionWidget()
{
    return m_optWidget;
}

void KisToolSelectBase::activate(bool temporary)
{
    KisTool::activate(temporary);
}

void KisToolSelectBase::deactivate()
{
    KisTool::deactivate();
}

void KisToolSelectBase::slotSetAction(int action)
{
    if (action >= SELECTION_REPLACE && action <= SELECTION_INTERSECT) {
        m_selectAction = (selectionAction)action;
        m_optWidget->setAction(action);
    }
}

void KisToolSelectBase::slotSetSelectionMode(int mode)
{
    m_selectionMode = (selectionMode)mode;
}

void KisToolSelectBase::keyPressEvent(QKeyEvent *event)
{
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
    case Qt::Key_Escape:
        //don't forward to KisTool::keyPressEvent()
        break;
    default:
        KisTool::keyPressEvent(event);
        return;
    }
    event->accept();
}

#include "kis_tool_select_base.moc"

