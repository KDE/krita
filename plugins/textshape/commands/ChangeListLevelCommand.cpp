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

#include "ChangeListLevelCommand.h"

#include <KoParagraphStyle.h>
#include <KoTextBlockData.h>
#include <KoTextDocument.h>
#include <KoList.h>
#include "KoList_p.h"
#include <KoListLevelProperties.h>
#include <KLocale>
#include <kdebug.h>

#include <QTextCursor>

ChangeListLevelCommand::ChangeListLevelCommand(const QTextBlock &block, ChangeListLevelCommand::CommandType type, 
                                               int level, QUndoCommand *parent)
    : TextCommandBase(parent),
      m_block(block),
      m_type(type),
      m_level(level),
      m_first(true)
{
    setText(i18n("Change List Level"));
}

ChangeListLevelCommand::~ChangeListLevelCommand()
{
}

int ChangeListLevelCommand::effectiveLevel(int level)
{
    int result;
    if (m_type == IncreaseLevel) {
        result = level + m_level;
    } else if (m_type == DecreaseLevel) {
        result = level - m_level;
    } else if (m_type == SetLevel) {
        result = level;
    }
    result = qMax(1, qMin(10, result));
    return result;
}

void ChangeListLevelCommand::redo()
{
    if (!m_first) {
        TextCommandBase::redo();
        UndoRedoFinalizer finalizer(this, m_tool);
        m_list->listPrivate()->textLists[m_block.textList()->format().property(KoListStyle::Level).toInt() - 1] = m_block.textList();
        KoListPrivate::invalidate(m_block);
    }
    else {
        QTextList *textList = m_block.textList();
        if (!textList)
            return;
        m_list = KoTextDocument(m_block.document()).list(textList);
        Q_ASSERT(m_list);
        m_oldLevel = m_list->level(m_block);
        int newLevel = effectiveLevel(m_oldLevel);

        if (!m_list->style()->hasLevelProperties(newLevel)) {
            KoListLevelProperties llp = m_list->style()->levelProperties(newLevel);
            llp.setIndent((newLevel-1) * 20); // make this configurable
            m_list->style()->setLevelProperties(llp);
        }
        m_list->add(m_block, newLevel);
    }
    m_first = false;
}

void ChangeListLevelCommand::undo()
{
    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this, m_tool);
    if (m_block.textList())
        m_list->listPrivate()->textLists[m_block.textList()->format().property(KoListStyle::Level).toInt() - 1] = m_block.textList();
    KoListPrivate::invalidate(m_block);
}

bool ChangeListLevelCommand::mergeWith(const QUndoCommand *other)
{
    return false;
}
