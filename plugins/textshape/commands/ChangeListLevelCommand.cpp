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
#include "TextTool.h"
#include <KoListLevelProperties.h>
#include <KLocale>
#include <kdebug.h>

#include <QTextCursor>
#include <QHash>
#include <QList>

ChangeListLevelCommand::ChangeListLevelCommand(const QTextCursor &cursor, ChangeListLevelCommand::CommandType type,
                                               int coef, QUndoCommand *parent)
    : TextCommandBase(parent),
      m_type(type),
      m_coefficient(coef),
      m_first(true)
{
    setText(i18n("Change List Level"));

    int selectionStart = qMin(cursor.anchor(), cursor.position());
    int selectionEnd = qMax(cursor.anchor(), cursor.position());

    QTextBlock block = cursor.block().document()->findBlock(selectionStart);

    bool oneOf = (selectionStart == selectionEnd); //ensures the block containing the cursor is selected in that case

    while (block.isValid() && ((block.position() < selectionEnd) || oneOf)) {
        m_blocks.append(block);
        if (block.textList()) {
            m_lists.insert(m_blocks.size() - 1, KoTextDocument(block.document()).list(block.textList()));
            Q_ASSERT(m_lists.value(m_blocks.size() - 1));
            m_levels.insert(m_blocks.size() - 1, effectiveLevel(m_lists.value(m_blocks.size() - 1)->level(block)));
        }
        oneOf = false;
        block = block.next();
    }
}

ChangeListLevelCommand::~ChangeListLevelCommand()
{
}

int ChangeListLevelCommand::effectiveLevel(int level)
{
    int result = -1;
    if (m_type == IncreaseLevel) {
        result = level + m_coefficient;
    } else if (m_type == DecreaseLevel) {
        result = level - m_coefficient;
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
        UndoRedoFinalizer finalizer(this);
        for (int i = 0; i < m_blocks.size(); ++i) {
            m_lists.value(i)->updateStoredList(m_blocks.at(i));
            if (KoTextBlockData *userData = dynamic_cast<KoTextBlockData*>(m_blocks.at(i).userData()))
                userData->setCounterWidth(-1.0);
        }
    }
    else {
        for (int i = 0; i < m_blocks.size(); ++i) {
            if (!m_lists.value(i)->style()->hasLevelProperties(m_levels.value(i))) {
                KoListLevelProperties llp = m_lists.value(i)->style()->levelProperties(m_levels.value(i));
                llp.setIndent((m_levels.value(i)-1) * 20); //TODO make this configurable
                m_lists.value(i)->style()->setLevelProperties(llp);
            }
            m_lists.value(i)->add(m_blocks.at(i), m_levels.value(i));
        }
    }
    m_first = false;
}

void ChangeListLevelCommand::undo()
{
    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this);
    for (int i = 0; i < m_blocks.size(); ++i) {
        if (m_blocks.at(i).textList())
            m_lists.value(i)->updateStoredList(m_blocks.at(i));
        if (KoTextBlockData *userData = dynamic_cast<KoTextBlockData*>(m_blocks.at(i).userData()))
            userData->setCounterWidth(-1.0);

    }
}

bool ChangeListLevelCommand::mergeWith(const QUndoCommand *)
{
    return false;
}
