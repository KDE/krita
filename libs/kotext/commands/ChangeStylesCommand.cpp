/*
 *  Copyright (c) 2012 C. Boemann <cbo@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ChangeStylesCommand.h"
#include "KoStyleManager.h"
#include "ChangeFollower.h"

#include <KoTextDocument.h>
#include <KoTextEditor.h>

#include <QTextDocument>

ChangeStylesCommand::ChangeStylesCommand(ChangeFollower *changeFollower
        , const QList<KoCharacterStyle *> &origCharacterStyles
        , const QList<KoParagraphStyle *> &origParagraphStyles
        , const QSet<int> &changedStyles
        , KUndo2Command *parent)
    : KUndo2Command("stylechangecommand",parent) //Don't translate
    , m_changeFollower(changeFollower)
    , m_origCharacterStyles(origCharacterStyles)
    , m_origParagraphStyles(origParagraphStyles)
    , m_changedStyles(changedStyles)
    , m_first(true)
{
    m_changeFollower->collectNeededInfo(m_changedStyles);
}

ChangeStylesCommand::~ChangeStylesCommand()
{
}

void ChangeStylesCommand::redo()
{
    KUndo2Command::redo();

    if (m_first) {
        m_first = false;
        m_changeFollower->processUpdates();
    }
}

void ChangeStylesCommand::undo()
{
    KUndo2Command::undo();
}
