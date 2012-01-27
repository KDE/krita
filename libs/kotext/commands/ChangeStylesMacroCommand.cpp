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

#include "ChangeStylesMacroCommand.h"

#include "KoStyleManager.h"
#include "ChangeStylesCommand.h"
#include "KoCharacterStyle.h"
#include "KoParagraphStyle.h"

#include <KoTextDocument.h>

#include <KLocale>

#include <QTextDocument>

ChangeStylesMacroCommand::ChangeStylesMacroCommand(const QList<ChangeFollower *> &changeFollowers
        , KoStyleManager *styleManager)
    : KUndo2Command(i18nc("(qtundo-format)", "Change Styles"))
    , m_changeFollowers(changeFollowers)
    , m_styleManager(styleManager)
    , m_first(true)
{
}

ChangeStylesMacroCommand::~ChangeStylesMacroCommand()
{
}

// on first pass the subcommands are created (where they collect needed info)
//     then styles are changed in the styleManager
//     finally the new styles are applied to the documents through super::redo()
void ChangeStylesMacroCommand::redo()
{
    if (m_first) {
        foreach(ChangeFollower *cf, m_changeFollowers) {
            new ChangeStylesCommand(cf, m_origCharacterStyles, m_origParagraphStyles, m_changedStyles, this);
        }
        m_first = false;
    }

    foreach(KoCharacterStyle *newStyle, m_changedCharacterStyles) {
        int id = newStyle->styleId();
        m_styleManager->characterStyle(id)->copyProperties(newStyle);

        emit m_styleManager->styleAltered(m_styleManager->characterStyle(id));
    }

    foreach(KoParagraphStyle *newStyle, m_changedParagraphStyles) {
        int id = newStyle->styleId();
        m_styleManager->paragraphStyle(id)->copyProperties(newStyle);

        emit m_styleManager->styleAltered(m_styleManager->paragraphStyle(id));
    }

    KUndo2Command::redo(); // calls redo on all children
}

void ChangeStylesMacroCommand::undo()
{
    foreach(KoCharacterStyle *oldStyle, m_origCharacterStyles) {
        int id = oldStyle->styleId();
        m_styleManager->characterStyle(id)->copyProperties(oldStyle);

        emit m_styleManager->styleAltered(m_styleManager->characterStyle(id));
    }

    foreach(KoParagraphStyle *oldStyle, m_origParagraphStyles) {
        int id = oldStyle->styleId();
        m_styleManager->paragraphStyle(id)->copyProperties(oldStyle);

        emit m_styleManager->styleAltered(m_styleManager->paragraphStyle(id));
    }

    KUndo2Command::undo(); // calls undo on all children
}
