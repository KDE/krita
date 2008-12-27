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

#include "ChangeListCommand.h"
#include "KoList_p.h"

#include <KoTextBlockData.h>
#include <KoTextDocument.h>
#include <QTextCursor>
#include <KoTextDebug.h>

#include <KoParagraphStyle.h>

#include <KLocale>
#include <KDebug>

ChangeListCommand::ChangeListCommand(const QTextBlock &block, KoListStyle::Style style, int level,
                                     int flags, QUndoCommand *parent)
        : TextCommandBase(parent),
        m_block(block),
        m_list(0),
        m_oldList(0),
        m_flags(flags),
        m_first(true)
{
    Q_ASSERT(block.isValid());
    storeOldProperties();
    level = detectLevel(level);

    KoListLevelProperties llp;
    llp.setLevel(level);
    llp.setStyle(style);
    if (KoListStyle::isNumberingStyle(style)) {
        llp.setStartValue(1);
        llp.setListItemSuffix(".");
    }
    if (level > 1)
        llp.setIndent((level-1) * 20); // make this configurable
    KoListStyle listStyle;
    listStyle.setLevelProperties(llp);

    initList(&listStyle, level);

    setText(i18n("Change List"));
}

ChangeListCommand::ChangeListCommand(const QTextBlock &block, KoListStyle *style, int level,
                                     int flags, QUndoCommand *parent)
        : TextCommandBase(parent),
          m_block(block),
          m_list(0),
          m_oldList(0),
          m_flags(flags),
          m_first(true)
{
    Q_ASSERT(block.isValid());
    Q_ASSERT(style);
    storeOldProperties();
    level = detectLevel(level);
    initList(style, level);
    setText(i18n("Change List"));
}

ChangeListCommand::~ChangeListCommand()
{
}

void ChangeListCommand::storeOldProperties()
{
    if (m_block.textList())
        m_formerProperties = KoListLevelProperties::fromTextList(m_block.textList());
    else
        m_formerProperties.setStyle(KoListStyle::None);
}

int ChangeListCommand::detectLevel(int givenLevel)
{
    if (givenLevel != 0)
        return givenLevel;
    if (m_block.textList()) {
        if (m_block.blockFormat().hasProperty(KoParagraphStyle::ListLevel))
            return m_block.blockFormat().intProperty(KoParagraphStyle::ListLevel);
        else
            return m_block.textList()->format().intProperty(KoListStyle::Level);
    }
    return 1;
}

bool ChangeListCommand::formatsEqual(const KoListLevelProperties &llp, const QTextListFormat &format)
{
    if (m_flags & MergeExactly) {
        QTextListFormat listFormat;
        llp.applyStyle(listFormat);
        return listFormat == format;
    } else {
        return llp.style() == format.style();
    }
}

void ChangeListCommand::initList(KoListStyle *listStyle, int level)
{
    KoTextDocument document(m_block.document());

    m_list = 0;
    m_oldList = document.list(m_block);
    m_newProperties = listStyle->levelProperties(level);
    if (m_newProperties.style() == KoListStyle::None) {
        m_currentAction = ChangeListCommand::removeList;
        return;
    }
    if (m_flags & ModifyExistingList) {
        m_list = document.list(m_block);
        if (m_list) {
            m_currentAction = ChangeListCommand::modifyExisting;
            return;
        }
    }

    if (m_flags & MergeWithAdjacentList) {
        KoListLevelProperties llp = listStyle->levelProperties(level);

        // attempt to merge with previous block
        QTextBlock prev = m_block.previous();
        if (prev.isValid() && prev.textList()) {
            if (formatsEqual(llp, prev.textList()->format())) {
                m_list = document.list(prev);
                if (m_list) {
                    m_currentAction = ChangeListCommand::mergeList;
                    return;
                }
            }
        }

        // attempt to merge with next block
        QTextBlock next = m_block.next();
        if (next.isValid() && next.textList()) {
            if (formatsEqual(llp, next.textList()->format())) {
                m_list = document.list(next);
                if (m_list) {
                    m_currentAction = ChangeListCommand::mergeList;
                    return;
                }
            }
        }
    }

    KoList::Type type = m_flags & CreateNumberedParagraph ? KoList::NumberedParagraph : KoList::TextList;
    m_list = new KoList(m_block.document(), listStyle, type);
    m_currentAction = ChangeListCommand::createNew;
}

void ChangeListCommand::redo()
{
    if (!m_first) {
        if (m_currentAction == ChangeListCommand::removeList) {
            // Here we just need to invalidate the list numbering, removal of the block from the QTextList is handled by QTextDocument undo
            KoListPrivate::invalidateList(m_block);
            TextCommandBase::redo();
            UndoRedoFinalizer finalizer(this, m_tool);
        }
        else {
            //(ChangeListCommand::modifyExisting)
            //(ChangeListCommand::createNew)
            //(ChangeListCommand::mergeList)
            TextCommandBase::redo();
            UndoRedoFinalizer finalizer(this, m_tool);
            // Here we need to update the pointer to the QTextList in KoList. The pointer initially stored there is not valid anymore since undo might have destroyed the QTextList when removing the block. We also need to restore the list style for that level in the KoList. Updating the QTextList style is done by QTextDocument.
            m_list->listPrivate()->textLists[m_block.textList()->format().property(KoListStyle::Level).toInt() - 1] = m_block.textList();
            KoListStyle *listStyle = m_list->style();
            listStyle->refreshLevelProperties(m_newProperties);
            KoListPrivate::invalidateList(m_block);
        }
        KoListPrivate::invalidate(m_block);
    }
    else {
        if (m_currentAction == ChangeListCommand::removeList) {
                KoList::remove(m_block);
        }
        else if (m_currentAction == ChangeListCommand::modifyExisting) {
                KoListStyle *listStyle = m_list->style();
                listStyle->setLevelProperties(m_newProperties);
                QTextCursor cursor(m_block);
                QTextBlockFormat format = m_block.blockFormat();
                format.clearProperty(KoParagraphStyle::UnnumberedListItem);
                cursor.setBlockFormat(format);
        }
        else {
            //(ChangeListCommand::createNew)
            //(ChangeListCommand::mergeList)
            m_list->add(m_block, m_newProperties.level());
            QTextCursor cursor(m_block);
            QTextBlockFormat format = m_block.blockFormat();
            format.clearProperty(KoParagraphStyle::UnnumberedListItem);
            cursor.setBlockFormat(format);
        }
    }
    m_first = false;
}

void ChangeListCommand::undo()
{
    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this, m_tool);
    
    // command to undo:
    if (m_currentAction == ChangeListCommand::removeList) {
        m_oldList->listPrivate()->textLists[m_block.textList()->format().property(KoListStyle::Level).toInt() - 1] = m_block.textList();
        if ((m_flags & ModifyExistingList) && (m_formerProperties.style() != KoListStyle::None)) {
            KoListStyle *listStyle = m_oldList->style();
            listStyle->refreshLevelProperties(m_formerProperties);
        }
        KoListPrivate::invalidateList(m_block);
    }
    else if (m_currentAction == ChangeListCommand::modifyExisting) {
         m_list->listPrivate()->textLists[m_block.textList()->format().property(KoListStyle::Level).toInt() - 1] = m_block.textList();
        if ((m_flags & ModifyExistingList) && (m_formerProperties.style() != KoListStyle::None)) {
            KoListStyle *listStyle = m_oldList->style();
            listStyle->refreshLevelProperties(m_formerProperties);
        }
        KoListPrivate::invalidateList(m_block);
    }
    // for createNew and mergeList, nothing has to be done since the block will be removed by QTextDocument, eventually deleting the QTextList which will automatically zero the QPointer in KoList registry.

    KoListPrivate::invalidate(m_block);
}

bool ChangeListCommand::mergeWith(const QUndoCommand *other)
{
    return false;
}
