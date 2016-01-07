/*
 This file is part of the KDE project
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 C. Boemann <cbo@boemann.dk>
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
 * Boston, MA 02110-1301, USA.*/

#include "DeleteTableRowCommand.h"

#include <KoTextEditor.h>
#include "KoTableColumnAndRowStyleManager.h"

#include <QTextTableCell>
#include <QTextTable>

#include <klocalizedstring.h>
#include "TextDebug.h"

DeleteTableRowCommand::DeleteTableRowCommand(KoTextEditor *te, QTextTable *t, KUndo2Command *parent) :
    KUndo2Command (parent)
    ,m_first(true)
    ,m_textEditor(te)
    ,m_table(t)
{
    setText(kundo2_i18n("Delete Row"));
}

void DeleteTableRowCommand::undo()
{
    KoTableColumnAndRowStyleManager carsManager = KoTableColumnAndRowStyleManager::getManager(m_table);
    for (int i = 0; i < m_selectionRowSpan; ++i) {
        carsManager.insertRows(m_selectionRow + i, 1, m_deletedStyles.at(i));
    }
    KUndo2Command::undo();
}

void DeleteTableRowCommand::redo()
{
    KoTableColumnAndRowStyleManager carsManager = KoTableColumnAndRowStyleManager::getManager(m_table);
    if (!m_first) {         carsManager.removeRows(m_selectionRow, m_selectionRowSpan);
        KUndo2Command::redo();
    } else {
        m_first = false;
        int selectionColumn;
        int selectionColumnSpan;
        if(m_textEditor->hasComplexSelection()) {
            m_textEditor->cursor()->selectedTableCells(&m_selectionRow, &m_selectionRowSpan, &selectionColumn, &selectionColumnSpan);
        } else {
            QTextTableCell cell = m_table->cellAt(*m_textEditor->cursor());
            m_selectionRow = cell.row();
            m_selectionRowSpan = 1;
        }

        for (int i = m_selectionRow; i < m_selectionRow + m_selectionRowSpan; ++i) {
            m_deletedStyles.append(carsManager.rowStyle(i));
        }
        carsManager.removeRows(m_selectionRow, m_selectionRowSpan);

        m_table->removeRows(m_selectionRow, m_selectionRowSpan);
    }
}
