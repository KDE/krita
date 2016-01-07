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

#ifndef DELETETABLECOLUMNCOMMAND_H
#define DELETETABLECOLUMNCOMMAND_H

#include <kundo2command.h>
#include <QList>
#include <KoTableColumnStyle.h>

class KoTextEditor;
class QTextTable;

class DeleteTableColumnCommand : public KUndo2Command
{
public:

    DeleteTableColumnCommand(KoTextEditor *te, QTextTable *t, KUndo2Command *parent = 0);

    virtual void undo();
    virtual void redo();

private:
    bool m_first;
    KoTextEditor *m_textEditor;
    QTextTable *m_table;
    int m_selectionColumn;
    int m_selectionColumnSpan;
    int m_changeId;
    QList<KoTableColumnStyle> m_deletedStyles;
};

#endif // DELETETABLEROWCOMMAND_H
