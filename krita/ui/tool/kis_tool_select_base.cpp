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
      m_widgetHelper(windowTitle)
{
}

SelectionMode KisToolSelectBase::selectionMode() const
{
    return m_widgetHelper.selectionMode();
}

SelectionAction KisToolSelectBase::selectionAction() const
{
    return m_widgetHelper.selectionAction();
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
