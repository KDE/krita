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
    : KUndo2Command(parent)
    , m_changeFollower(changeFollower)
    , m_origCharacterStyles(origCharacterStyles)
    , m_origParagraphStyles(origParagraphStyles)
    , m_changedStyles(changedStyles)
    , m_first(true)
    , m_macroFirst(parent != 0)
{
}

ChangeStylesCommand::~ChangeStylesCommand()
{
}

void ChangeStylesCommand::redo()
{
    // We need this weird construct if we are played within a macro then our first job
    // is to call instantlyExecuteCommand which will "redo" us immediately in what is the normal
    // "first" time
    if (m_macroFirst) {
        m_macroFirst = false;
        KoTextEditor *editor = KoTextDocument(m_changeFollower->document()).textEditor();
        editor->instantlyExecuteCommand(this);
        return;
    }

    KUndo2Command::redo();

    if (m_first) {
        m_first = false;
        m_changeFollower->processUpdates(m_changedStyles);
    }
}

void ChangeStylesCommand::undo()
{
    KUndo2Command::undo();
}
