/*
 This file is part of the KDE project
 * Copyright (C) 2012 C. Boemann <cbo@boemann.dk>
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
#include "ResizeTableCommand.h"

#include "KoTableColumnAndRowStyleManager.h"
#include "KoTableColumnStyle.h"
#include "KoTableRowStyle.h"

#include <QTextTable>
#include <QTextCursor>
#include <QTextDocument>

#include <klocalizedstring.h>
#include "TextDebug.h"

ResizeTableCommand::ResizeTableCommand(QTextTable *t, bool horizontal, int band, qreal size, KUndo2Command *parent) :
    KUndo2Command (parent)
    , m_first(true)
    , m_tablePosition(t->firstPosition())
    , m_document(t->document())
    , m_horizontal(horizontal)
    , m_band(band)
    , m_size(size)
    , m_oldColumnStyle(0)
    , m_oldRowStyle(0)
{
    if (horizontal) {
        setText(kundo2_i18n("Adjust Column Width"));
    } else {
        setText(kundo2_i18n("Adjust Row Height"));
    }
}

ResizeTableCommand::~ResizeTableCommand()
{
    delete m_oldColumnStyle;
    delete m_oldRowStyle;
}

void ResizeTableCommand::undo()
{
    QTextCursor c(m_document);
    c.setPosition(m_tablePosition);
    QTextTable *table = c.currentTable();

    KoTableColumnAndRowStyleManager carsManager = KoTableColumnAndRowStyleManager::getManager(table);

    if (m_oldColumnStyle) {
        KoTableColumnStyle style = carsManager.columnStyle(m_band);
        style.copyProperties(m_oldColumnStyle);
        carsManager.setColumnStyle(m_band, style);
    }
    if (m_oldRowStyle) {
        KoTableRowStyle style = carsManager.rowStyle(m_band);
        style.copyProperties(m_oldRowStyle);
        carsManager.setRowStyle(m_band, style);
    }
    KUndo2Command::undo();
    m_document->markContentsDirty(m_tablePosition, table->lastPosition()-table->firstPosition());
}

void ResizeTableCommand::redo()
{
    QTextCursor c(m_document);
    c.setPosition(m_tablePosition);
    QTextTable *table = c.currentTable();

    KoTableColumnAndRowStyleManager carsManager = KoTableColumnAndRowStyleManager::getManager(table);

    if (!m_first) {
        if (m_horizontal) {
            KoTableColumnStyle style = carsManager.columnStyle(m_band);
            style.copyProperties(m_newColumnStyle);
            carsManager.setColumnStyle(m_band, style);
        } else {
            KoTableRowStyle style = carsManager.rowStyle(m_band);
            style.copyProperties(m_newRowStyle);
            carsManager.setRowStyle(m_band, style);
        }
        KUndo2Command::redo();
    } else {
        m_first = false;
        if (m_horizontal) {
            m_oldColumnStyle = carsManager.columnStyle(m_band).clone();
            // make sure the style is set (could have been a default style)
            carsManager.setColumnStyle(m_band, carsManager.columnStyle(m_band));

            KoTableColumnStyle style = carsManager.columnStyle(m_band);
            style.setColumnWidth(m_size);
            carsManager.setColumnStyle(m_band, style);

            m_newColumnStyle = carsManager.columnStyle(m_band).clone();
        } else {
            m_oldRowStyle = carsManager.rowStyle(m_band).clone();

            // make sure the style is set (could have been a default style)
            carsManager.setRowStyle(m_band, carsManager.rowStyle(m_band));

            KoTableRowStyle style = carsManager.rowStyle(m_band);
            style.setMinimumRowHeight(m_size);
            carsManager.setRowStyle(m_band, style);

            m_newRowStyle = carsManager.rowStyle(m_band).clone();
        }
    }
    m_document->markContentsDirty(m_tablePosition, table->lastPosition()-table->firstPosition());
}
