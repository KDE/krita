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
#include <KoTextDebug.h>

#include <KoParagraphStyle.h>

#include <KLocale>
#include <KDebug>

ChangeListCommand::ChangeListCommand(const QTextBlock &block, KoListStyle::Style style, int level,
                                     ChangeListCommand::ChangeFlags flags, QUndoCommand *parent)
        : TextCommandBase(parent),
        m_block(block),
        m_list(0),
        m_style(style),
        m_level(level),
        m_flags(flags)
{
    Q_ASSERT(block.isValid());
    storeOldProperties();
    initLevel();

    KoListLevelProperties llp;
    llp.setLevel(m_level);
    llp.setStyle(m_style);
    if (KoListStyle::isNumberingStyle(m_style))
        llp.setListItemSuffix(".");
    if (m_level > 1)
        llp.setIndent((m_level-1) * 20); // make this configurable
    KoListStyle listStyle;
    listStyle.setLevelProperties(llp);

    initList(&listStyle);

    setText(i18n("Change List"));
}

ChangeListCommand::ChangeListCommand(const QTextBlock &block, KoListStyle *style, int level,
                                     ChangeListCommand::ChangeFlags flags, QUndoCommand *parent)
        : TextCommandBase(parent),
          m_block(block),
          m_list(0),
          m_style(KoListStyle::None),
          m_level(level),
          m_flags(flags)
{
    Q_ASSERT(block.isValid());
    Q_ASSERT(style);
    storeOldProperties();
    m_list = new KoList(block.document(), style);
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

void ChangeListCommand::initLevel()
{
    if (m_level != 0)
        return;
    if (m_block.textList()) {
        if (m_block.blockFormat().hasProperty(KoParagraphStyle::ListLevel))
            m_level = m_block.blockFormat().intProperty(KoParagraphStyle::ListLevel);
        else
            m_level = m_block.textList()->format().intProperty(KoListStyle::Level);
    } else {
        m_level = 1;
    }
    Q_ASSERT(m_level != 0);
}

void ChangeListCommand::initList(KoListStyle *listStyle)
{
    KoTextDocument document(m_block.document());

    m_list = 0;
    if (m_style == KoListStyle::None)
        return;

    if (m_flags & ModifyExistingList) {
        m_list = document.list(m_block);
        if (m_list)
            return;
    }

    if (m_flags & MergeWithAdjacentList) {
        KoListLevelProperties llp = listStyle->levelProperties(m_level);

        // attempt to merge with previous block
        QTextBlock prev = m_block.previous();
        if (prev.isValid() && prev.textList()) {
            QTextListFormat format = prev.textList()->format();
            QTextListFormat prevFormat;
            llp.applyStyle(prevFormat);
            if (prevFormat == format) {
                m_list = document.list(prev);
                if (m_list)
                    return;
            }
        }

        // attempt to merge with next block
        QTextBlock next = m_block.next();
        if (next.isValid() && next.textList()) {
            QTextListFormat format = next.textList()->format();
            QTextListFormat nextFormat;
            llp.applyStyle(nextFormat);

            if (nextFormat == format) {
                m_list = document.list(next);
                if (m_list)
                    return;
            }
        }
    }

    KoList::Type type = m_flags & CreateNumberedParagraph ? KoList::NumberedParagraph : KoList::TextList;
    m_list = new KoList(m_block.document(), listStyle, type);
}

void ChangeListCommand::recalcList(const QTextBlock &block) const
{
    KoTextBlockData *userData = dynamic_cast<KoTextBlockData*>(block.userData());
    if (userData)
        userData->setCounterWidth(-1.0);
}

void ChangeListCommand::redo()
{
    TextCommandBase::redo();
    UndoRedoFinalizer finalizer(this, m_tool);
    if (m_list == 0) { // no list item (anymore)
        KoList::remove(m_block);
    } else if ((m_flags & ModifyExistingList) && m_block.textList()) {
        KoListStyle *listStyle = m_list->style();
        KoListLevelProperties llp = listStyle->levelProperties(m_level);
        if (llp.style() != m_style) {
            llp.setStyle(m_style);
            llp.setListItemSuffix(KoListStyle::isNumberingStyle(m_style) ? "." : "");
            listStyle->setLevelProperties(llp);
        }
    } else {
        m_list->add(m_block, m_level);
        QTextCursor cursor(m_block);
        QTextBlockFormat format = m_block.blockFormat();
        format.clearProperty(KoParagraphStyle::UnnumberedListItem);
        cursor.setBlockFormat(format);
    }
}

void ChangeListCommand::undo()
{
    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this, m_tool);

    if (m_formerProperties.style() == KoListStyle::None)
        return;
    Q_ASSERT(m_block.textList());
    QTextListFormat format;
    m_formerProperties.applyStyle(format);
    m_block.textList()->setFormat(format);
    recalcList(m_block);
}

bool ChangeListCommand::mergeWith(const QUndoCommand *other)
{
    const ChangeListCommand *clc = dynamic_cast<const ChangeListCommand*>(other);
    if (clc == 0)
        return false;
    if (clc->m_block != m_block)
        return false;

    m_formerProperties = clc->m_formerProperties;
    return true;
}
