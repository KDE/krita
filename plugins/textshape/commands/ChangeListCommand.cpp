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

#include <KLocale>
#include <KDebug>

ChangeListCommand::ChangeListCommand(const QTextBlock &block, KoListStyle::Style style, QUndoCommand *parent)
        : TextCommandBase(parent),
        m_block(block),
        m_list(0)
{
// kDebug() <<"ChangeListCommand" << style;
    Q_ASSERT(block.isValid());
    storeOldProperties();
    KoTextDocument document(block.document());

    if (style != KoListStyle::NoItem) {
        QTextBlock prev = block.previous();
        if (prev.isValid() && prev.textList()) {
            QTextListFormat format = prev.textList()->format();
            if (format.intProperty(QTextListFormat::ListStyle) == static_cast<int>(style)) { //kDebug() <<" merge with prev";
                m_list = document.list(prev);
            }
        }
        QTextBlock next = block.next();
        if (m_list == 0 && next.isValid() && next.textList()) {
            QTextListFormat format = next.textList()->format();
            if (format.intProperty(QTextListFormat::ListStyle) == static_cast<int>(style)) { //kDebug() <<" merge with next";
                m_list = document.list(next);
            }
        }
        if (m_list == 0) { // create a new one
            KoListLevelProperties llp;
            if (block.textList()) { // find out current list-level / etc
                llp = KoListLevelProperties::fromTextList(block.textList());
//kDebug() <<" reuse current (level:" << llp.level() <<")";
            }
            llp.setStyle(style);
            if (style == KoListStyle::SquareItem || style == KoListStyle::DiscItem ||
                    style == KoListStyle::CircleItem || style == KoListStyle::BoxItem ||
                    style == KoListStyle::RhombusItem || style == KoListStyle::HeavyCheckMarkItem ||
                    style == KoListStyle::BallotXItem || style == KoListStyle::RightArrowItem ||
                    style == KoListStyle::RightArrowHeadItem)
                llp.setListItemSuffix(""); // for non-numbered items, remove any suffix.
            else
                llp.setListItemSuffix("."); // for numbered items, add a trailing dot.
            KoListStyle style;
            style.setLevelProperties(llp);
            m_list = new KoList(block.document(), &style);
        }
    }

    setText(i18n("Change List"));
}

ChangeListCommand::ChangeListCommand(const QTextBlock &block, KoListStyle *style, bool exact, QUndoCommand *parent)
        : TextCommandBase(parent),
        m_block(block)
{
    Q_ASSERT(block.isValid());
    Q_ASSERT(style);
    storeOldProperties();
    if (! exact) {
        // search for similar ones in the next / prev parags.
        // TODO
    }
    m_list = new KoList(block.document(), style);
    setText(i18n("Change List"));
}

void ChangeListCommand::storeOldProperties()
{
    if (m_block.textList())
        m_formerProperties = KoListLevelProperties::fromTextList(m_block.textList());
    else
        m_formerProperties.setStyle(KoListStyle::NoItem);
}

ChangeListCommand::~ChangeListCommand()
{
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
        if (QTextList *list = m_block.textList()) {
            list->remove(m_block);
            recalcList(m_block);
        }
    } else {
        m_list->add(m_block, 0);
    }
}

void ChangeListCommand::undo()
{
    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this, m_tool);

    if (m_formerProperties.style() == KoListStyle::NoItem)
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
