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

#include "KoTextCommandBase.h"
#include "KoListStyle.h"
#include "KoTextEditor.h"
#include "KoListLevelProperties.h"

#include <QTextBlock>
#include <QList>
#include <QHash>

class KoList;

/**
 * This command is useful to alter the list-association of a single textBlock.
 */
class ChangeListCommand : public KoTextCommandBase
{
public:

    //FIXME: following comments seems to describe another function
    /**
     * Change the list property of 'block'.
     * @param block the paragraph to change the list property of
     * @param style indicates which style to use.
     * @param parent the parent undo command for macro functionality
     */
    ChangeListCommand(const QTextCursor &cursor,
                      const KoListLevelProperties &levelProperties,
                      KoTextEditor::ChangeListFlags flags,
                      KUndo2Command *parent = 0);

    /**
     * Change the list property of 'block'.
     * @param block the paragraph to change the list property of
     * @param style the style to apply
     * @param exact if true then the actual style 'style' should be set, if false we possibly  merge with another similar style that is near the block
     * @param parent the parent undo command for macro functionality
     */
    ChangeListCommand(const QTextCursor &cursor, KoListStyle *style, int level,
                      KoTextEditor::ChangeListFlags flags,
                      KUndo2Command *parent = 0);
    ~ChangeListCommand() override;

    /// redo the command
    void redo() override;
    /// revert the actions done in redo
    void undo() override;

    /// reimplemented from KUndo2Command
    int id() const override {
        return 58450687;
    }
    /// reimplemented from KUndo2Command
    bool mergeWith(const KUndo2Command *other) override;

private:
    enum CommandAction {
        CreateNew,
        ModifyExisting,
        ReparentList,
        MergeList,
        RemoveList
    };
    bool extractTextBlocks(const QTextCursor &cursor, int level, KoListStyle::Style newStyle = KoListStyle::None);
    int detectLevel(const QTextBlock &block, int givenLevel);
    void initList(KoListStyle *style);
    bool formatsEqual(const KoListLevelProperties &llp, const QTextListFormat &format);

    int m_flags;
    bool m_first;
    bool m_alignmentMode;

    QList<QTextBlock> m_blocks;
    QHash<int, KoListLevelProperties> m_formerProperties;
    QHash<int, KoListLevelProperties> m_newProperties;
    QHash<int, int> m_levels;
    QHash<int, KoList*> m_list;
    QHash<int, KoList*> m_oldList;
    QHash<int, CommandAction> m_actions;
};



#endif
