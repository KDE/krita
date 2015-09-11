/*
 * This file is part of the KDE project
 * Copyright (C) 2015 Denis Kuplaykov <dener.kup@gmail.com>
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

#ifndef SPLITSECTIONSCOMMAND_H
#define SPLITSECTIONSCOMMAND_H

#include <kundo2command.h>

class KoSection;
class QTextDocument;

//FIXME: why it is not going from KoTextCommandBase?
// If it will be changed to KoTextCommandBase,
// don't forget to add UndoRedoFinalizer.
class SplitSectionsCommand : public KUndo2Command
{
public:
    enum SplitType
    {
        Startings,
        Endings
    };

    explicit SplitSectionsCommand(QTextDocument *document, SplitType type, int splitPosition);
    virtual ~SplitSectionsCommand();

    virtual void undo();
    virtual void redo();

private:
    bool m_first; ///< Checks first call of redo
    QTextDocument *m_document; ///< Pointer to document
    SplitType m_type; ///< Split type
    int m_splitPosition; ///< Split position
};

#endif // SPLITSECTIONSCOMMAND_H
