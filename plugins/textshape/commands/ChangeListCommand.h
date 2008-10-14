/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef CHANGELISTCOMMAND
#define CHANGELISTCOMMAND

#include "TextCommandBase.h"

#include <KoListStyle.h>
#include <KoList.h>
#include <KoListLevelProperties.h>

#include <QTextBlock>

/**
 * This command is useful to alter the list-association of a single textBlock.
 */
class ChangeListCommand : public TextCommandBase
{
public:
    enum ChangeFlag {
        ModifyExistingList = 1,
        MergeWithAdjacentList = 2,
        Default = ModifyExistingList | MergeWithAdjacentList,
        CreateNumberedParagraph = 4
    };

    Q_DECLARE_FLAGS(ChangeFlags, ChangeFlag)

    /**
     * Change the list property of 'block'.
     * @param block the paragraph to change the list property of
     * @param style indicates which style to use.
     * @param parent the parent undo command for macro functionality
     */
    ChangeListCommand(const QTextBlock &block, KoListStyle::Style style, int level = 0, 
                      ChangeFlags flags = Default, QUndoCommand *parent = 0);

    /**
     * Change the list property of 'block'.
     * @param block the paragraph to change the list property of
     * @param style the style to apply
     * @param exact if true then the actual style 'style' should be set, if false we possibly  merge with another similar style that is near the block
     * @param parent the parent undo command for macro functionality
     */
    ChangeListCommand(const QTextBlock &block, KoListStyle *style, int level = 0,
                      ChangeFlags flags = Default, QUndoCommand *parent = 0);
    ~ChangeListCommand();

    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

    /// reimplemnted from QUndoCommand
    virtual int id() const {
        return 58450687;
    }
    /// reimplemnted from QUndoCommand
    virtual bool mergeWith(const QUndoCommand *other);

private:
    void storeOldProperties();
    int detectLevel(int givenLevel);
    void initList(KoListStyle *style, int level);

    QTextBlock m_block;
    KoList *m_list, *m_oldList;
    KoListLevelProperties m_newProperties;
    ChangeFlags m_flags;
    KoListLevelProperties m_formerProperties;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ChangeListCommand::ChangeFlags)

#endif
