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
 * Boston, MA 02110-1301, USA.*/

#ifndef RESIZETABLECOMMAND_H
#define RESIZETABLECOMMAND_H

#include <kundo2command.h>

class QTextDocument;
class QTextTable;
class KoTableColumnStyle;
class KoTableRowStyle;

class ResizeTableCommand : public KUndo2Command
{
public:
    ResizeTableCommand(QTextTable *t, bool horizontal, int band, qreal size, KUndo2Command *parent = 0);
    virtual ~ResizeTableCommand();

    virtual void undo();
    virtual void redo();

private:
    bool m_first;
    int m_tablePosition;
    QTextDocument *m_document;
    bool m_horizontal;
    int m_band;
    qreal m_size;
    KoTableColumnStyle *m_oldColumnStyle;
    KoTableColumnStyle *m_newColumnStyle;
    KoTableRowStyle *m_oldRowStyle;
    KoTableRowStyle *m_newRowStyle;
};

#endif // RESIZETABLECOMMAND_H
