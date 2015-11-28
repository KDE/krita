/*
 *  Copyright (c) 2012 C. Boemann <cbo@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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
#include "OdfTextTrackStyles.h"

#include <KoTextDocument.h>
#include <KoTextEditor.h>

#include <klocalizedstring.h>

#include <QTextDocument>

ChangeStylesMacroCommand::ChangeStylesMacroCommand(const QList<QTextDocument *> &documents
        , KoStyleManager *styleManager)
    : KUndo2Command(kundo2_i18n("Change Styles"))
    , m_documents(documents)
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
    QList<ChangeStylesCommand *> commands;
    if (m_first) {
        // IMPORTANT: the sub commands needs to be created now so the can collect
        // info before we change the styles
        Q_FOREACH (QTextDocument *qDoc, m_documents) {
            commands.append(new ChangeStylesCommand(qDoc, m_origCharacterStyles, m_origParagraphStyles, m_changedStyles, this));
        }
    }

    // Okay so now it's safe to change the styles and this should always be done
    Q_FOREACH (KoCharacterStyle *newStyle, m_changedCharacterStyles) {
        int id = newStyle->styleId();
        m_styleManager->characterStyle(id)->copyProperties(newStyle);
    }

    Q_FOREACH (KoParagraphStyle *newStyle, m_changedParagraphStyles) {
        int id = newStyle->styleId();
        m_styleManager->paragraphStyle(id)->copyProperties(newStyle);
    }

    if (m_first) {
        int i = 0;
        Q_FOREACH (QTextDocument *qDoc, m_documents) {
            //add and execute it's redo
            // ToC documents doesn't have a texteditor so make sure we ignore that
            if (KoTextDocument(qDoc).textEditor()) {
                KoTextDocument(qDoc).textEditor()->addCommand(commands[i]);
            }
            i++;
        }
        m_first = false;
    } else {
        KUndo2Command::redo(); // calls redo on all children
    }
}

void ChangeStylesMacroCommand::undo()
{
    Q_FOREACH (KoCharacterStyle *oldStyle, m_origCharacterStyles) {
        int id = oldStyle->styleId();
        m_styleManager->characterStyle(id)->copyProperties(oldStyle);
    }

    Q_FOREACH (KoParagraphStyle *oldStyle, m_origParagraphStyles) {
        int id = oldStyle->styleId();
        m_styleManager->paragraphStyle(id)->copyProperties(oldStyle);
    }

    KUndo2Command::undo(); // calls undo on all children
}
