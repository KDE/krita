/*
 *  Copyright (c) 2012 C. Boemann <cbo@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CHANGESTYLESCOMMAND_H
#define CHANGESTYLESCOMMAND_H

#include <kundo2command.h>

#include <QList>
#include <QSet>

class ChangeFollower;
class KoCharacterStyle;
class KoParagraphStyle;

class ChangeStylesCommand : public KUndo2Command
{
public:
    ChangeStylesCommand(ChangeFollower *changeFollower
        , const QList<KoCharacterStyle *> &origCharacterStyles
        , const QList<KoParagraphStyle *> &origParagraphStyles
        , const QSet<int> &changedStyles
        , KUndo2Command *parent);
    virtual ~ChangeStylesCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    ChangeFollower *m_changeFollower;
    QList<KoCharacterStyle *> m_origCharacterStyles;
    QList<KoParagraphStyle *> m_origParagraphStyles;
    QSet<int> m_changedStyles;
    bool m_first;
};

#endif // CHANGESTYLESCOMMAND_H
