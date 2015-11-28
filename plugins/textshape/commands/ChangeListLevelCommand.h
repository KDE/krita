/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#ifndef CHANGELISTLEVELCOMMAND
#define CHANGELISTLEVELCOMMAND

#include "KoTextCommandBase.h"

#include <QTextBlock>
#include <QList>
#include <QHash>

class KoList;

/**
 * This command is used the change level of a list-item.
 */
class ChangeListLevelCommand : public KoTextCommandBase
{
public:
    enum CommandType {
        IncreaseLevel,
        DecreaseLevel,
        SetLevel
    };

    /**
     * Change the list property of 'block'.
     * @param block the paragraph to change the list property of
     * @param coef indicates by how many levels the list item should be displaced
     * @param parent the parent undo command for macro functionality
     */
    ChangeListLevelCommand(const QTextCursor &cursor, CommandType type, int coef, KUndo2Command *parent = 0);

    ~ChangeListLevelCommand();

    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

    /// reimplemnted from KUndo2Command
    virtual int id() const
    {
        return 58450689;
    }
    /// reimplemnted from KUndo2Command
    virtual bool mergeWith(const KUndo2Command *other);

private:
    int effectiveLevel(int level);

    CommandType m_type;
    int m_coefficient;

    QList<QTextBlock> m_blocks;
    QHash<int, KoList *> m_lists;
    QHash<int, int> m_levels;

    bool m_first;
};

#endif
