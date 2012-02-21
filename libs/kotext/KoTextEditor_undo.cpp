/* This file is part of the KDE project
 * Copyright (C) 2009-2012 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (c) 2011 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (C) 2011-2012 C. Boemann <cbo@boemann.dk>
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

#include "KoTextEditor.h"
#include "KoTextEditor_p.h"

#include "KoTextDocument.h"

#include <kundo2command.h>

#include <KLocale>

#include <QString>
#include <QTextDocument>
#include <QWeakPointer>

#include <KDebug>

void KoTextEditor::Private::documentCommandAdded()
{
    class UndoTextCommand : public KUndo2Command
    {
    public:
        UndoTextCommand(QTextDocument *document, KoTextEditor::Private *p, KUndo2Command *parent = 0)
            : KUndo2Command(i18nc("(qtundo-format)", "Text"), parent),
              m_document(document)
            , m_p(p)
        {}

        void undo() {
            QTextDocument *doc = m_document.data();
            if (doc == 0)
                return;
            KoTextDocument(doc).textEditor()->cursor()->beginEditBlock(); //These are needed to get around a Qt bug in 4.8. A change through a QTextCursor outside an editBlock will crash (cursor position calculation on non layed out doc).
            doc->undo(KoTextDocument(doc).textEditor()->cursor());
            KoTextDocument(doc).textEditor()->cursor()->endEditBlock();
            m_p->emitTextFormatChanged();
        }

        void redo() {
            QTextDocument *doc = m_document.data();
            if (doc == 0)
                return;
            KoTextDocument(doc).textEditor()->cursor()->beginEditBlock(); //Same as above
            doc->redo(KoTextDocument(doc).textEditor()->cursor());
            KoTextDocument(doc).textEditor()->cursor()->endEditBlock();
            m_p->emitTextFormatChanged();
        }

        QWeakPointer<QTextDocument> m_document;
        KoTextEditor::Private *m_p;
    };

    if (commandStack.isEmpty()) {
        commandStack.push(new KUndo2Command(commandTitle));
        if (KoTextDocument(document).undoStack()) {
            KoTextDocument(document).undoStack()->push(commandStack.top());
        }
        addNewCommand = false;
    }
    else if (addNewCommand) {
        commandStack.push(new KUndo2Command(commandTitle, commandStack.top()));
        addNewCommand = false;
    }
    else if ((editorState == KeyPress || editorState == Delete) && !commandStack.isEmpty() && commandStack.top()->childCount()) {
        commandStack.pop();
        commandStack.push(new KUndo2Command(commandTitle, !commandStack.isEmpty()?commandStack.top():0));
        if (KoTextDocument(document).undoStack()) {
            KoTextDocument(document).undoStack()->push(commandStack.top());
        }
    }

    new UndoTextCommand(document, this, commandStack.top());
}

void KoTextEditor::Private::updateState(KoTextEditor::Private::State newState, QString title)
{
    if (editorState == Custom && newState != NoOp) {
        addNewCommand = true;
        if (!title.isEmpty())
            commandTitle = title;
        else
            commandTitle = i18n("Text");
        return;
    }
    if (newState == NoOp && !commandStack.isEmpty()) {
        commandStack.pop();
        if (commandStack.isEmpty()) {
            addNewCommand = true;
            editorState = NoOp;
        }
        return;
    }
    if (editorState != newState || commandTitle != title) {
        if (!commandStack.isEmpty()) {
            commandStack.pop();
            addNewCommand = true;
        }
    }
    editorState = newState;
    if (!title.isEmpty())
        commandTitle = title;
    else
        commandTitle = i18n("Text");
}

void KoTextEditor::addCommand(KUndo2Command *command)
{
    ++d->inCustomCommand;
    d->updateState(KoTextEditor::Private::Custom, (!command->text().isEmpty())?command->text():i18n("Text"));
    d->addNewCommand = false;
    if (d->commandStack.isEmpty()) {
        d->commandStack.push(command);
        KUndo2QStack *stack = KoTextDocument(d->document).undoStack();
        if (stack && !command->hasParent()) {
            stack->push(command);
        } else {
            command->redo();
        }
    }
    else {
        d->commandStack.push(command);
        command->redo();
    }
    while(d->commandStack.top() != command) { //clean auto generated commands which at that point should not be here anymore. in particular insertText being open ended will not have been cleared.
        d->commandStack.pop();
    }
    d->updateState(KoTextEditor::Private::NoOp);
    --d->inCustomCommand;
}

void KoTextEditor::instantlyExecuteCommand(KUndo2Command *command)
{
    d->updateState(KoTextEditor::Private::Custom, (!command->text().isEmpty())?command->text():i18n("Text"));
    command->redo();
    // instant replay done let's not keep it dangling
    if (!command->hasParent()) {
        d->updateState(KoTextEditor::Private::NoOp);
    }
}

KUndo2Command *KoTextEditor::beginEditBlock(QString title)
{
    if (!d->inCustomCommand) {
        d->updateState(KoTextEditor::Private::Custom, title);
        if (d->commandStack.isEmpty()) {
            KUndo2Command *command = new KUndo2Command(title);
            d->commandStack.push(command);
            ++d->inCustomCommand;
            d->dummyMacroAdded = true;
            KUndo2QStack *stack = KoTextDocument(d->document).undoStack();
            if (stack) {
                stack->push(command);
            } else {
                command->redo();
            }
        }
    }
    if (!(d->dummyMacroAdded && d->inCustomCommand == 1)) {
        d->caret.beginEditBlock();
    }
    return (d->commandStack.isEmpty())?0:d->commandStack.top();
}

void KoTextEditor::endEditBlock()
{
    if (d->dummyMacroAdded && d->inCustomCommand == 1) {
        --d->inCustomCommand;
        d->dummyMacroAdded = false;
    } else {
        d->caret.endEditBlock();
    }
    if (!d->inCustomCommand) {
        d->updateState(KoTextEditor::Private::NoOp);
    }
}
