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

#include <KoTextBlockData.h>
#include <KoTextDocument.h>
#include <QTextCursor>
#include <KoParagraphStyle.h>

#include <KLocale>
#include <KDebug>

ChangeListCommand::ChangeListCommand(const QTextCursor &cursor, KoListStyle::Style style, int level,
                                     ChangeFlags flags, QUndoCommand *parent)
        : TextCommandBase(parent),
        m_flags(flags),
        m_first(true)
{
    extractTextBlocks(cursor, level);
    QSet<int> levels = m_levels.values().toSet();
    KoListStyle listStyle;

    foreach (int lev, levels) {
        KoListLevelProperties llp;
        llp.setLevel(lev);
        llp.setStyle(style);
        if (KoListStyle::isNumberingStyle(style)) {
            llp.setStartValue(1);
            llp.setListItemSuffix(".");
        }
        if (lev > 1)
            llp.setIndent((lev-1) * 20); // make this configurable

        listStyle.setLevelProperties(llp);
    }

    initList(&listStyle);

    setText(i18n("Change List"));
}

ChangeListCommand::ChangeListCommand(const QTextCursor &cursor, KoListStyle *style, int level,
                                     ChangeFlags flags, QUndoCommand *parent)
        : TextCommandBase(parent),
          m_flags(flags),
          m_first(true)
{
    Q_ASSERT(style);
    extractTextBlocks(cursor, level);
    initList(style);
    setText(i18n("Change List"));
}

ChangeListCommand::~ChangeListCommand()
{
}

void ChangeListCommand::extractTextBlocks(const QTextCursor &cursor, int level)
{
    int selectionStart = qMin(cursor.anchor(), cursor.position());
    int selectionEnd = qMax(cursor.anchor(), cursor.position());

    QTextBlock block = cursor.block().document()->findBlock(selectionStart);

    bool oneOf = (selectionStart == selectionEnd); //ensures the block containing the cursor is selected in that case

    while (block.isValid() && ((block.position() < selectionEnd) || oneOf)) {
        m_blocks.append(block);
        if (block.textList()) {
            m_formerProperties.insert((m_blocks.size() - 1), KoListLevelProperties::fromTextList(block.textList()));
            m_levels.insert((m_blocks.size() - 1), detectLevel(block, level));
        }
        else {
            KoListLevelProperties prop;
            prop.setStyle(KoListStyle::None);
            m_formerProperties.insert((m_blocks.size() - 1), prop);
            m_levels.insert((m_blocks.size() - 1), level);
        }
        oneOf = false;
        block = block.next();
    }
}

int ChangeListCommand::detectLevel(const QTextBlock &block, int givenLevel)
{
    if (givenLevel != 0)
        return givenLevel;
    if (block.textList()) {
        if (block.blockFormat().hasProperty(KoParagraphStyle::ListLevel))
            return block.blockFormat().intProperty(KoParagraphStyle::ListLevel);
        else
            return block.textList()->format().intProperty(KoListStyle::Level);
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
        return (int) llp.style() == (int) format.style();
    }
}

void ChangeListCommand::initList(KoListStyle *listStyle)
{
    KoTextDocument document(m_blocks.first().document());

    KoList *mergeableList = 0;
    KoList *newList = 0;
    //First check if we could merge with previous or next list
    if (m_flags & MergeWithAdjacentList) {
        QSet<int> levels = m_levels.values().toSet();
        // attempt to merge with previous block
        QTextBlock prev = m_blocks.value(0).previous();
        bool isMergeable = true;
        foreach (int lev, levels) {
            KoListLevelProperties llp = listStyle->levelProperties(lev);
            // checks format compatibility
            isMergeable = (isMergeable && prev.isValid() && prev.textList() && (formatsEqual(llp, prev.textList()->format())));
        }
        if (isMergeable)
            mergeableList = document.list(prev);

        if (!mergeableList) {
            // attempt to merge with next block if previous failed
            isMergeable = true;
            QTextBlock next = m_blocks.value(m_blocks.size()-1).next();
            foreach (int lev, levels) {
                KoListLevelProperties llp = listStyle->levelProperties(lev);
                isMergeable = (isMergeable && next.isValid() && next.textList() && (formatsEqual(llp, next.textList()->format())));
            }
            if (isMergeable)
                mergeableList = document.list(next);
        }
    }
    // Now iterates over the blocks and set-up the various lists
    for (int i = 0; i < m_blocks.size(); ++i) {
        m_list.insert(i, 0);
        m_oldList.insert(i, document.list(m_blocks.at(i)));
        m_newProperties.insert(i, listStyle->levelProperties(m_levels.value(i)));
        // First check if we want to remove a list
        if (m_newProperties.value(i).style() == KoListStyle::None) {
            m_actions.insert(i, ChangeListCommand::RemoveList);
            continue;
        }
        // Then check if we want to modify an existing list.
        // The behaviour chosen for modifying a list is the following. If the selection contains more than one block, a new list is always created. If the selection only contains one block, the behaviour depends on the flag.
        if ((m_flags & ModifyExistingList) && (m_blocks.size() == 1)) {
            m_list.insert(i, document.list(m_blocks.at(i)));
            if (m_list.value(i)) {
                m_actions.insert(i, ChangeListCommand::ModifyExisting);
                continue;
            }
        }
        // Then check if we can merge with an existing list. The actual check was done before, here we just check the result.
        if (mergeableList) {
            m_list.insert(i, mergeableList);
            m_actions.insert(i, ChangeListCommand::MergeList);
            continue;
        }
        // All else failing, we need to create a new list.
        KoList::Type type = m_flags & CreateNumberedParagraph ? KoList::NumberedParagraph : KoList::TextList;
        if (!newList)
            newList = new KoList(m_blocks.at(i).document(), listStyle, type);
        m_list.insert(i, newList);
        m_actions.insert(i, ChangeListCommand::CreateNew);
    }
}

void ChangeListCommand::redo()
{
    if (!m_first) {
        for (int i = 0; i < m_blocks.size(); ++i) { // We invalidate the lists before calling redo on the QTextDocument
            if (m_actions.value(i) == ChangeListCommand::RemoveList)
                for (int j = 0; j < m_blocks.at(i).textList()->count(); j++) {
                    if (m_blocks.at(i).textList()->item(j) != m_blocks.at(i)) {
                        if (KoTextBlockData *userData = dynamic_cast<KoTextBlockData*>(m_blocks.at(i).textList()->item(j).userData()))
                            userData->setCounterWidth(-1.0);
                        break;
                    }
                }
        }
        TextCommandBase::redo();
        UndoRedoFinalizer finalizer(this);
        for (int i = 0; i < m_blocks.size(); ++i) {
            if ((m_actions.value(i) == ChangeListCommand::ModifyExisting) || (m_actions.value(i) == ChangeListCommand::CreateNew)
                    || (m_actions.value(i) == ChangeListCommand::MergeList)) {
                m_list.value(i)->updateStoredList(m_blocks.at(i));
                KoListStyle *listStyle = m_list.value(i)->style();
                listStyle->refreshLevelProperties(m_newProperties.value(i));
                for (int j = 0; j < m_blocks.at(i).textList()->count(); j++) {
                    if (m_blocks.at(i).textList()->item(j) != m_blocks.at(i)) {
                        if (KoTextBlockData *userData = dynamic_cast<KoTextBlockData*>(m_blocks.at(i).textList()->item(j).userData()))
                            userData->setCounterWidth(-1.0);
                        break;
                    }
                }
            }
            if (KoTextBlockData *userData = dynamic_cast<KoTextBlockData*>(m_blocks.at(i).userData()))
                userData->setCounterWidth(-1.0);
        }
    }
    else {
        for (int i = 0; i < m_blocks.size(); ++i) {
            if (m_actions.value(i) == ChangeListCommand::RemoveList) {
                KoList::remove(m_blocks.at(i));
            }
            else if (m_actions.value(i) == ChangeListCommand::ModifyExisting) {
                KoListStyle *listStyle = m_list.value(i)->style();
                listStyle->setLevelProperties(m_newProperties.value(i));
                QTextCursor cursor(m_blocks.at(i));
                QTextBlockFormat format = m_blocks.at(i).blockFormat();
                format.clearProperty(KoParagraphStyle::UnnumberedListItem);
                cursor.setBlockFormat(format);
            }
            else {
                //(ChangeListCommand::CreateNew)
                //(ChangeListCommand::MergeList)
                m_list.value(i)->add(m_blocks.at(i), m_newProperties.value(i).level());
                QTextCursor cursor(m_blocks.at(i));
                QTextBlockFormat format = m_blocks.at(i).blockFormat();
                format.clearProperty(KoParagraphStyle::UnnumberedListItem);
                cursor.setBlockFormat(format);
            }
        }
    }
    m_first = false;
}

void ChangeListCommand::undo()
{
    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this);

    for (int i = 0; i < m_blocks.size(); ++i) {
        // command to undo:
        if (m_actions.value(i) == ChangeListCommand::RemoveList) {
            m_oldList.value(i)->updateStoredList(m_blocks.at(i));
            if ((m_flags & ModifyExistingList) && (m_formerProperties.value(i).style() != KoListStyle::None)) {
                KoListStyle *listStyle = m_oldList.value(i)->style();
                listStyle->refreshLevelProperties(m_formerProperties.value(i));
            }
            for (int j = 0; j < m_blocks.at(i).textList()->count(); j++) {
                if (m_blocks.at(i).textList()->item(j) != m_blocks.at(i)) {
                    if (KoTextBlockData *userData = dynamic_cast<KoTextBlockData*>(m_blocks.at(i).textList()->item(j).userData()))
                        userData->setCounterWidth(-1.0);
                    break;
                }
            }
        }
        else if (m_actions.value(i) == ChangeListCommand::ModifyExisting) {
            m_list.value(i)->updateStoredList(m_blocks.at(i));
            if ((m_flags & ModifyExistingList) && (m_formerProperties.value(i).style() != KoListStyle::None)) {
                KoListStyle *listStyle = m_oldList.value(i)->style();
                listStyle->refreshLevelProperties(m_formerProperties.value(i));
            }
            for (int j = 0; j < m_blocks.at(i).textList()->count(); j++) {
                if (m_blocks.at(i).textList()->item(j) != m_blocks.at(i)) {
                    if (KoTextBlockData *userData = dynamic_cast<KoTextBlockData*>(m_blocks.at(i).textList()->item(j).userData()))
                        userData->setCounterWidth(-1.0);
                    break;
                }
            }
        }
        else {
            //(ChangeListCommand::CreateNew)
            //(ChangeListCommand::MergeList)

            //if the new/merged list replaced an existing list, the pointer to QTextList in oldList needs updating.
            if ((m_oldList.value(i))) {
                m_oldList.value(i)->updateStoredList(m_blocks.at(i));
                for (int j = 0; j < m_blocks.at(i).textList()->count(); j++) {
                    if (m_blocks.at(i).textList()->item(j) != m_blocks.at(i)) {
                        if (KoTextBlockData *userData = dynamic_cast<KoTextBlockData*>(m_blocks.at(i).textList()->item(j).userData()))
                            userData->setCounterWidth(-1.0);
                        break;
                    }
                }
            }
        }

        if (KoTextBlockData *userData = dynamic_cast<KoTextBlockData*>(m_blocks.at(i).userData()))
            userData->setCounterWidth(-1.0);
    }
}

bool ChangeListCommand::mergeWith(const QUndoCommand *)
{
    return false;
}
