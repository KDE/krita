/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include <KLocale>
#include <KDebug>

ChangeListCommand::ChangeListCommand(const QTextBlock &block, KoListStyle::Style style, QUndoCommand *parent)
: TextCommandBase( parent ),
    m_block(block),
    m_listStyle(0)
{
// kDebug() <<"ChangeListCommand" << style;
    Q_ASSERT(block.isValid());
    storeOldProperties();

    if(style != KoListStyle::NoItem) {
        QTextBlock prev = block.previous();
        if(prev.isValid() && prev.textList()) {
            QTextListFormat format = prev.textList()->format();
            if(format.intProperty(QTextListFormat::ListStyle) == static_cast<int> (style))
{ //kDebug() <<" merge with prev";
                m_listStyle = KoListStyle::fromTextList(prev.textList());
}
        }
        QTextBlock next = block.next();
        if(m_listStyle == 0 && next.isValid() && next.textList()) {
            QTextListFormat format = next.textList()->format();
            if(format.intProperty(QTextListFormat::ListStyle) == static_cast<int> (style))
{ //kDebug() <<" merge with next";
                m_listStyle = KoListStyle::fromTextList(next.textList());
}
        }
        if(m_listStyle == 0) { // create a new one
            m_listStyle = new KoListStyle();
            KoListLevelProperties llp;
            if(block.textList()) // find out current list-level / etc
{
                llp = KoListLevelProperties::fromTextList(block.textList());
 //kDebug() <<" reuse current (level:" << llp.level() <<")";
}
            else
{ //kDebug() <<" create new level 1";
                 llp = m_listStyle->level(1);
}
            llp.setStyle(style);
            if(style == KoListStyle::SquareItem || style == KoListStyle::DiscItem ||
                    style == KoListStyle::CircleItem || style == KoListStyle::BoxItem ||
                    style == KoListStyle::RhombusItem || style == KoListStyle::HeavyCheckMarkItem ||
                    style == KoListStyle::BallotXItem || style == KoListStyle::RightArrowItem ||
                    style == KoListStyle::RightArrowHeadItem)
                llp.setListItemSuffix(""); // for non-numbered items, remove any suffix.
            else
                llp.setListItemSuffix("."); // for numbered items, add a trailing dot.
            m_listStyle->setLevel(llp);
        }
    }

    setText( i18n("Change List") );
}

ChangeListCommand::ChangeListCommand(const QTextBlock &block, KoListStyle style, bool exact, QUndoCommand *parent)
: TextCommandBase( parent ),
    m_block(block)
{
    Q_ASSERT(block.isValid());
    Q_ASSERT(style.isValid());
    storeOldProperties();
    if(! exact) {
        // search for similar ones in the next / prev parags.
        // TODO
    }
    m_listStyle = new KoListStyle(style);
    setText( i18n("Change List") );
}

void ChangeListCommand::storeOldProperties() {
    if(m_block.textList())
        m_formerProperties = KoListLevelProperties::fromTextList(m_block.textList());
    else
        m_formerProperties.setStyle(KoListStyle::NoItem);
}

ChangeListCommand::~ChangeListCommand() {
    delete m_listStyle;
}

void ChangeListCommand::redo() {
    TextCommandBase::redo();
    UndoRedoFinalizer finalizer(this, m_tool);
    if(m_listStyle == 0) { // no list item (anymore)
        QTextList *list = m_block.textList();
        if(list == 0) // nothing to do!
            return;
        bool shouldReset = list->count() > 1;
        list->remove(m_block);
        if( shouldReset )
            recalcList(list->item(0));
        return;
    }

    if(m_block.textList() && m_block.textList()->count() != 1) { // we need to split the list.
        QTextList *list = m_block.textList();
        list->remove(m_block);
        recalcList(list->item(0));
    }
    m_listStyle->applyStyle(m_block);
    recalcList(m_block);
}

void ChangeListCommand::recalcList(const QTextBlock &block) const {
    KoTextBlockData *userData = dynamic_cast<KoTextBlockData*> (block.userData());
    if(userData)
        userData->setCounterWidth(-1.0);
}

void ChangeListCommand::undo() {
    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this, m_tool);

    if(m_formerProperties.style() == KoListStyle::NoItem)
        return;
    Q_ASSERT(m_block.textList());
    QTextListFormat format;
    m_formerProperties.applyStyle(format);
    m_block.textList()->setFormat(format);
    KoTextBlockData *userData = dynamic_cast<KoTextBlockData*> (m_block.userData());
    if(userData) // force a recalc of my listitem.
        userData->setCounterWidth(-1.0);
}

bool ChangeListCommand::mergeWith (const QUndoCommand *other) {
    const ChangeListCommand *clc = dynamic_cast<const ChangeListCommand*> (other);
    if(clc == 0)
        return false;
    if(clc->m_block != m_block)
        return false;

    m_formerProperties = clc->m_formerProperties;
    return true;
}
