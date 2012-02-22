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

/** Calligra's undo/redo framework.
    The @class KoTextEditor undo/redo framework sits between the @class QTextDocument and the apllication's undo/redo stack.

    When the @class QTextDocument is changed by an editing action, it internally creates an undo/redo command. When doing so a signal (undoCommandAdded()) is emitted by the @class QTextDocument in order for applications to update their undo/redo stack accordingly.
    Each @class QTextDocument used in Calligra is handled by a specific @class KoTextEditor. It is responsible for on the one hand edit the @class QTextDocument, and on the other hand to listen for the QTextDocument's signal.

    Calligra uses a @class KUndo2Stack as its application undo/redo stack. This stack is populated by @class KUndo2Command or sub-classes of it.

    In order to limit the number of command sub-classes, KoTextEditor provides a framework which uses a generic command.

    The framework is based on a sort of state machine. The KoTextEditor can be in several different states (see @enum KoTextEditor::Private::State).
    These are:
    NoOp: this states indicates that the KoTextEditor is not editing the QTextDocument.
    KeyPress: this state indicates that the user is typing text. All text typed in succession should correspond to one undo command. To be used when entering text outside of an insertTextCommand.
    Delete: this state indicates that the user is deleting characters. All deletions done in succession should correspond to one undo command. To be used for deleting outside a deleteCommand. Currently not in use, our deltion is done through a command because of inline objects.
    Format: this state indicates that we are formatting text. To be used when formatting outside of a command.
    Custom: this state indicates that the QTextDocument is changed through a KUndo2Command.

    KoTextEditor reacts differently when receiving the QTextDocument's signal, depending on its state.

    In addition the framework allows to encapsulate modifications in a on-the-fly created custom command (\sa beginEditBlock() endEditBlock()).
    Furthermore the framework allows to push complete KUndo2Commands.

    See the documentation file for how to use this framework.
*/

/*
  Important members:

  commandStack: This stack holds the headCommands. These parent the generated UndoTextCommands. When undo or redo is called, they will in turn call UndoTextCommand::undo/redo.

  editorState: Holds the state of the KoTextEditor. see above

  commandTitle: Holds the title which is to be used when creating a headCommand.

  addNewCommand: bool used to tell the framework to create a new headCommand and push it on the commandStack, when receiving an undoCommandAdded signal from QTextDocument.

  inCustomCommand: counter used to keep track of nested KUndo2Commands that are pushed on the KoTextEditor.
  */


// This slot is called when the KoTextEditor receives the signal undoCommandAdded() from QTextDocument. A generic UndoTextCommand is used to match the QTextDocument's internal undo command. This command calls QTextDocument::undo() or QTextDocument::redo() respectively, which triggers the undo redo actions on the document.
//In order to allow nesting commands, we maintain a stack of commands. The top command will be the parent of our auto generated UndoTextCommands.
//Depending on the case, we might create a command which will serve as head command. This is pushed to our commandStack and eventually to the application's stack.
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
        //We have an empty stack. We need a head command which is to be pushed onto our commandStack and on the application stack if there is one.
        //This command will serve as a parent for the auto-generated UndoTextCommands.
        commandStack.push(new KUndo2Command(commandTitle));
        if (KoTextDocument(document).undoStack()) {
            KoTextDocument(document).undoStack()->push(commandStack.top());
        }
        addNewCommand = false;
    }
    else if (addNewCommand) {
        //We have already a headCommand on the commandStack. However we want a new child headCommand (nested commands) on the commandStack for parenting further UndoTextCommands. This shouldn't be pushed on the application's stack because it is a child of the current commandStack's top.
        commandStack.push(new KUndo2Command(commandTitle, commandStack.top()));
        addNewCommand = false;
    }
    else if ((editorState == KeyPress || editorState == Delete) && !commandStack.isEmpty() && commandStack.top()->childCount()) {
        //QTextDocument emits a signal on the first key press (or delte) and "merges" the subsequent ones, until an incompatible one is done. In which case it re-emit a signal.
        //Here we are in KeyPress (or Delete) state. The fact that the commandStack isn't empty and its top command has children means that we just received such a signal. We therefore need to pop the previous headCommand (which were either key press or delete) and create a new one to parent the UndoTextCommands. This command also need to be pushed on the application's stack.
        commandStack.pop();
        commandStack.push(new KUndo2Command(commandTitle, !commandStack.isEmpty()?commandStack.top():0));
        if (KoTextDocument(document).undoStack()) {
            KoTextDocument(document).undoStack()->push(commandStack.top());
        }
    }

    //Now we can create our UndoTextCommand which is parented to the commandStack't top headCommand.
    new UndoTextCommand(document, this, commandStack.top());
}

//This method is used to update the KoTextEditor state, which will condition how the QTextDocument::undoCommandAdded signal will get handled.
void KoTextEditor::Private::updateState(KoTextEditor::Private::State newState, QString title)
{
    if (editorState == Custom && newState != NoOp) {
        //We already are in a custom state (meaning that either a KUndo2Command was pushed on us, an on-the-fly macro command was started or we are executing a complex editing from within the KoTextEditor.
        //In that state any update of the state different from NoOp is part of that "macro". However, updating the state means that we are now wanting to have a new command for parenting the UndoTextCommand generated after the signal from QTextDocument. This is to ensure that undo/redo actions are done in the proper order. Setting addNewCommand will ensure that we create such a child headCommand on the commandStack. This command will not be pushed on the application's stack.
        addNewCommand = true;
        if (!title.isEmpty())
            commandTitle = title;
        else
            commandTitle = i18n("Text");
        return;
    }
    if (newState == NoOp && !commandStack.isEmpty()) {
        //Calling updateState to NoOp when the commandStack isn't empty means that the current headCommand on the commandStack is finished. Further UndoTextCommands do not belong to it. So we pop it.
        //If after poping the headCommand we still have some commands on the commandStack means we have not finished with the highest "macro". In that case we need to stay in the "Custom" state.
        //On the contrary, an empty commandStack means we have finished with the "macro". In that case, we set the editor to NoOp state. A signal from the QTextDocument should also generate a new headCommand.
        commandStack.pop();
        if (commandStack.isEmpty()) {
            addNewCommand = true;
            editorState = NoOp;
        }
        return;
    }
    if (editorState != newState || commandTitle != title) {
        //We are not in "Custom" state but either are moving to a new state (from editing to format,...) or the command type is the same, but not the command itself (like format:bold, format:italic). The later case is caracterised by a different command title.
        //When we change command, we need to pop the current commandStack's top and ask for a new headCommand to be created.
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

/// This method is used to push a complete KUndo2Command on the KoTextEditor. This command will be pushed on the application's stack if needed. The logic allows to push several commands which are then going to be nested, provided these children are pushed from within the redo method of their parent.
void KoTextEditor::addCommand(KUndo2Command *command)
{
    //We increase the inCustomCommand counter to inform the framework that we are having a further KUndo2Command and update the KoTextEditor's state to Custom.
    //However, this update will request a new headCommand to be pushed on the commandStack. This is what we want for internal complex editions but not in this case. Indeed, it must be the KUndo2Command which will parent the UndoTextCommands. Therefore we set the addNewCommand back to false.
    //If the commandStack is empty, we are the highest "macro" command and we should therefore push the KUndo2Command on the application's stack.
    //On the contrary, if the commandStack is not empty, or the pushed command has a parent, it means that we are adding a nested KUndo2Command. In which case we just want to put it on the commandStack to parent UndoTextCommands. We need to call the redo method manually though.
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

    //When we reach that point, the command has been executed. We first need to clean up all the automatically generated headCommand on our commandStack, which could potentially have been created during the editing. When we reach our pushed command, the commandStack is clean. We can then call a state update to NoOp and decrease the inCustomCommand counter.
    while(d->commandStack.top() != command) {
        d->commandStack.pop();
    }
    d->updateState(KoTextEditor::Private::NoOp);
    --d->inCustomCommand;
}

/// DO NOT USE THIS. It stays here for compiling reasons. But it will severely break everything. Again: DO NOT USE THIS.
void KoTextEditor::instantlyExecuteCommand(KUndo2Command *command)
{
    d->updateState(KoTextEditor::Private::Custom, (!command->text().isEmpty())?command->text():i18n("Text"));
    command->redo();
    // instant replay done let's not keep it dangling
    if (!command->hasParent()) {
        d->updateState(KoTextEditor::Private::NoOp);
    }
}

/// This method is used to start an on-the-fly macro command. Use KoTextEditor::endEditBlock to stop it.
/// ***
/// Important note:
/// ***
/// The framework does not allow to push a complete KUndo2Command (through KoTextEditor::addCommand) from within an EditBlock. Doing so will lead in the best case to several undo/redo commands on the application's stack instead of one, in the worst case to an out of sync application's stack.
/// ***
KUndo2Command *KoTextEditor::beginEditBlock(QString title)
{
    if (!d->inCustomCommand) {
        // We are not in a custom macro command. So we first need to update the KoTextEditor's state to Custom. Additionnaly, if the commandStack is empty, we need to create a master headCommand for our macro and push it on the stack.
        d->updateState(KoTextEditor::Private::Custom, title);
        if (d->commandStack.isEmpty()) {
            KUndo2Command *command = new KUndo2Command(title);
            d->commandStack.push(command);
            ++d->inCustomCommand;
            d->dummyMacroAdded = true; //This bool is used to tell endEditBlock that we have created a master headCommand.
            KUndo2QStack *stack = KoTextDocument(d->document).undoStack();
            if (stack) {
                stack->push(command);
            } else {
                command->redo();
            }
        }
    }
    //QTextDocument sends the undoCommandAdded signal at the end of the QTextCursor edit block. Since we want our master headCommand to parent the signal induced UndoTextCommands, we should not call QTextCursor::beginEditBlock for the headCommand.
    if (!(d->dummyMacroAdded && d->inCustomCommand == 1)) {
        //we don't call beginEditBlock for the first headCommand because we want the signals to be sent before we finished our command.
        d->caret.beginEditBlock();
    }
    return (d->commandStack.isEmpty())?0:d->commandStack.top();
}

void KoTextEditor::endEditBlock()
{
    //Only the self created master headCommand (see beginEditBlock) is left on the commandStack, we need to decrease the inCustomCommand counter that we increased on creation.
    //If we are not yet at this master headCommand, we can call QTextCursor::endEditBlock
    if (d->dummyMacroAdded && d->inCustomCommand == 1) {
        //we don't call caret.endEditBlock because we did not begin a block for the first headCommand
        --d->inCustomCommand;
        d->dummyMacroAdded = false;
    } else {
        d->caret.endEditBlock();
    }
    if (!d->inCustomCommand) {
        //We have now finished completely the macro, set the editor state to NoOp then.
        d->updateState(KoTextEditor::Private::NoOp);
    }
}
