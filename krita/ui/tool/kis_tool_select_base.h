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

#ifndef KISTOOLSELECTBASE_H
#define KISTOOLSELECTBASE_H

#include "kis_tool.h"
#include "kis_selection.h"
#include "kis_cursor.h"

class KisSelectionOptions;
//class KAction;

class KRITAUI_EXPORT KisToolSelectBase : public KisTool
{
    Q_OBJECT
public:
    KisToolSelectBase(KoCanvasBase *canvas, const QCursor& cursor=KisCursor::load("tool_rectangular_selection_cursor.png", 6, 6));

    virtual QWidget* createOptionWidget();
    virtual QWidget* optionWidget();

public slots:
    void activate(bool temporary);
    void deactivate();

    virtual void slotSetAction(int);
    virtual void slotSetSelectionMode(int);

protected:
    virtual void keyPressEvent(QKeyEvent *event);

    KisSelectionOptions* m_optWidget;
    selectionAction m_selectAction;
    selectionMode m_selectionMode;
};

#endif // KISTOOLSELECTBASE_H
