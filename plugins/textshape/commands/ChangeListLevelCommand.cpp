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
#include <KoListLevelProperties.h>

#include <QTextCursor>
#include <QDebug>

ChangeListLevelCommand::ChangeListLevelCommand(const QTextBlock &block, ChangeListLevelCommand::CommandType type, 
                                               int level, QUndoCommand *parent)
    : TextCommandBase(parent),
      m_block(block),
      m_type(type),
      m_level(level)
{
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
    TextCommandBase::redo();
    UndoRedoFinalizer finalizer(this, m_tool);

    QTextList *textList = m_block.textList();
    if (!textList)
        return;
    KoList *list = KoTextDocument(m_block.document()).list(textList);
    if (list) {
        QTextListFormat format = textList->format();
        m_oldLevel = format.intProperty(KoListStyle::Level);
        int newLevel = effectiveLevel(m_oldLevel);
        if (!list->style()->hasLevelProperties(newLevel)) {
            KoListLevelProperties llp = list->style()->levelProperties(newLevel);
            llp.setIndent(newLevel * 15);
            list->style()->setLevelProperties(llp);
        }
        list->add(m_block, newLevel);
    } else {
        QTextBlockFormat format = m_block.blockFormat();
        m_oldLevel = format.intProperty(KoParagraphStyle::ListLevel);
        format.setProperty(KoParagraphStyle::ListLevel, effectiveLevel(m_oldLevel));
        QTextCursor cursor(m_block);
        cursor.setBlockFormat(format);
    }
}

void ChangeListLevelCommand::undo()
{
    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this, m_tool);

    QTextList *textList = m_block.textList();
    if (!textList)
        return;
    KoList *list = KoTextDocument(m_block.document()).list(textList);
    if (list) {
        QTextListFormat format = textList->format();
        list->add(m_block, m_oldLevel);
    } else {
        QTextBlockFormat format = m_block.blockFormat();
        format.setProperty(KoParagraphStyle::ListLevel, m_oldLevel);
        QTextCursor cursor(m_block);
        cursor.setBlockFormat(format);
    }
}

bool ChangeListLevelCommand::mergeWith(const QUndoCommand *other)
{
    const ChangeListLevelCommand *cmd = dynamic_cast<const ChangeListLevelCommand *>(other);
    if (!cmd)
        return false;
    if (m_block != cmd->m_block || m_type != cmd->m_type)
        return false;
    m_level = cmd->m_level;
    return true;
}

