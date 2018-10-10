/* This file is part of the KDE project
 * Copyright (C) 2009-2012 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
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

#include <klocalizedstring.h>

#include <QTextDocument>
#include <QWeakPointer>

#include "TextDebug.h"

/** Calligra's undo/redo framework.
    The @class KoTextEditor undo/redo framework sits between the @class QTextDocument and the application's undo/redo stack.

    When the @class QTextDocument is changed by an editing action, it internally creates an undo/redo command. When doing so a signal (undoCommandAdded()) is emitted by the @class QTextDocument in order for applications to update their undo/redo stack accordingly.
    Each @class QTextDocument used in Calligra is handled by a specific @class KoTextEditor. It is responsible for on the one hand edit the @class QTextDocument, and on the other hand to listen for the QTextDocument's signal.

    Calligra uses a @class KUndo2Stack as its application undo/redo stack. This stack is populated by @class KUndo2Command or sub-classes of it.

    In order to limit the number of command sub-classes, KoTextEditor provides a framework which uses a generic command.

    The framework is based on a sort of state machine. The KoTextEditor can be in several different states (see @enum KoTextEditor::Private::State).
    These are:
    NoOp: this states indicates that the KoTextEditor is not editing the QTextDocument.
    KeyPress: this state indicates that the user is typing text. All text typed in succession should correspond to one undo command. To be used when entering text outside of an insertTextCommand.
    Delete: this state indicates that the user is deleting characters. All deletions done in succession should correspond to one undo command. To be used for deleting outside a deleteCommand. Currently not in use, our deletion is done through a command because of inline objects.
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

  customCommandCount: counter used to keep track of nested KUndo2Commands that are pushed on the KoTextEditor.
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
            : KUndo2Command(kundo2_i18n("Text"), parent),
              m_document(document)
            , m_p(p)
        {}

        void undo() override {
            QTextDocument *doc = m_document.data();
            if (doc == 0)
                return;
            doc->undo(KoTextDocument(doc).textEditor()->cursor());
            m_p->emitTextFormatChanged();
        }

        void redo() override {
            QTextDocument *doc = m_document.data();
            if (doc == 0)
                return;
            doc->redo(KoTextDocument(doc).textEditor()->cursor());
            m_p->emitTextFormatChanged();
        }

        QWeakPointer<QTextDocument> m_document;
        KoTextEditor::Private *m_p;
    };

    debugText << "received a QTextDocument undoCommand signal";
    debugText << "commandStack count: " << commandStack.count();
    debugText << "addCommand: " << addNewCommand;
    debugText << "editorState: " << editorState;
    if (commandStack.isEmpty()) {
        //We have an empty stack. We need a head command which is to be pushed onto our commandStack and on the application stack if there is one.
        //This command will serve as a parent for the auto-generated UndoTextCommands.
        debugText << "empty stack, will push a new headCommand on both commandStack and application's stack. title: " << commandTitle;
        commandStack.push(new KUndo2Command(commandTitle));
        if (KoTextDocument(document).undoStack()) {
            KoTextDocument(document).undoStack()->push(commandStack.top());
        }
        addNewCommand = false;
        debugText << "commandStack is now: " << commandStack.count();
    }
    else if (addNewCommand) {
        //We have already a headCommand on the commandStack. However we want a new child headCommand (nested commands) on the commandStack for parenting further UndoTextCommands. This shouldn't be pushed on the application's stack because it is a child of the current commandStack's top.
        debugText << "we have a headCommand on the commandStack but need a new child command. we will push it only on the commandStack: " << commandTitle;
        commandStack.push(new KUndo2Command(commandTitle, commandStack.top()));
        addNewCommand = false;
        debugText << "commandStack count is now: " << commandStack.count();
    }
    else if ((editorState == KeyPress || editorState == Delete) && !commandStack.isEmpty() && commandStack.top()->childCount()) {
        //QTextDocument emits a signal on the first key press (or delete) and "merges" the subsequent ones, until an incompatible one is done. In which case it re-emit a signal.
        //Here we are in KeyPress (or Delete) state. The fact that the commandStack isn't empty and its top command has children means that we just received such a signal. We therefore need to pop the previous headCommand (which were either key press or delete) and create a new one to parent the UndoTextCommands. This command also need to be pushed on the application's stack.
        debugText << "we are in subsequent keyPress/delete state and still received a signal. we need to create a new headCommand: " << commandTitle;
        debugText << "so we pop the current one and push the new one on both the commandStack and the application's stack";
        commandStack.pop();
        commandStack.push(new KUndo2Command(commandTitle, !commandStack.isEmpty()?commandStack.top():0));
        if (KoTextDocument(document).undoStack()) {
            KoTextDocument(document).undoStack()->push(commandStack.top());
        }
        debugText << "commandStack count: " << commandStack.count();
    }

    //Now we can create our UndoTextCommand which is parented to the commandStack't top headCommand.
    new UndoTextCommand(document, this, commandStack.top());
    debugText << "done creating the dummy UndoTextCommand";
}

//This method is used to update the KoTextEditor state, which will condition how the QTextDocument::undoCommandAdded signal will get handled.
void KoTextEditor::Private::updateState(KoTextEditor::Private::State newState, const KUndo2MagicString &title)
{
    debugText << "updateState from: " << editorState << " to: " << newState << " with: " << title;
    debugText << "commandStack count: " << commandStack.count();
    if (editorState == Custom && newState != NoOp) {
        //We already are in a custom state (meaning that either a KUndo2Command was pushed on us, an on-the-fly macro command was started or we are executing a complex editing from within the KoTextEditor.
        //In that state any update of the state different from NoOp is part of that "macro". However, updating the state means that we are now wanting to have a new command for parenting the UndoTextCommand generated after the signal
        //from QTextDocument. This is to ensure that undo/redo actions are done in the proper order. Setting addNewCommand will ensure that we create such a child headCommand on the commandStack. This command will not be pushed on the application's stack.
        debugText << "we are already in a custom state. a new state, which is not NoOp is part of the macro we are doing. we need however to create a new command on the commandStack to parent a signal induced UndoTextCommand";
        addNewCommand = true;
        if (!title.isEmpty())
            commandTitle = title;
        else
            commandTitle = kundo2_i18n("Text");
        debugText << "returning now. commandStack is not modified at this stage";
        return;
    }
    if (newState == NoOp && !commandStack.isEmpty()) {
        //Calling updateState to NoOp when the commandStack isn't empty means that the current headCommand on the commandStack is finished. Further UndoTextCommands do not belong to it. So we pop it.
        //If after popping the headCommand we still have some commands on the commandStack means we have not finished with the highest "macro". In that case we need to stay in the "Custom" state.
        //On the contrary, an empty commandStack means we have finished with the "macro". In that case, we set the editor to NoOp state. A signal from the QTextDocument should also generate a new headCommand.
        debugText << "we are in a macro and update the state to NoOp. this means that the command on top of the commandStack is finished. we should pop it";
        debugText << "commandStack count before: " << commandStack.count();
        commandStack.pop();
        debugText << "commandStack count after: " << commandStack.count();
        if (commandStack.isEmpty()) {
            debugText << "we have no more commands on the commandStack. the macro is complete. next signal induced command will need to be parented to a new headCommand. Also the editor should go to NoOp";
            addNewCommand = true;
            editorState = NoOp;
        }
        debugText << "returning now. commandStack count: " << commandStack.count();
        return;
    }
    if (editorState != newState || commandTitle != title) {
        //We are not in "Custom" state but either are moving to a new state (from editing to format,...) or the command type is the same, but not the command itself (like format:bold, format:italic). The later case is caracterised by a different command title.
        //When we change command, we need to pop the current commandStack's top and ask for a new headCommand to be created.
        debugText << "we are not in a custom state but change the command";
        debugText << "commandStack count: " << commandStack.count();
        if (!commandStack.isEmpty()) {
            debugText << "the commandStack is not empty. however the command on it is not a macro. so we pop it and ask to recreate a new one: " << title;
            commandStack.pop();
            addNewCommand = true;
        }
    }
    editorState = newState;
    if (!title.isEmpty())
        commandTitle = title;
    else
        commandTitle = kundo2_i18n("Text");
    debugText << "returning now. commandStack count: " << commandStack.count();
}

/// This method is used to push a complete KUndo2Command on the KoTextEditor. This command will be pushed on the application's stack if needed. The logic allows to push several commands which are then going to be nested, provided these children are pushed from within the redo method of their parent.
void KoTextEditor::addCommand(KUndo2Command *command)
{
    debugText << "we receive a command to add on the stack.";
    debugText << "commandStack count: " << d->commandStack.count();
    debugText << "customCommandCount counter: " << d->customCommandCount << " will increase";

    //We increase the customCommandCount counter to inform the framework that we are having a further KUndo2Command and update the KoTextEditor's state to Custom.
    //However, this update will request a new headCommand to be pushed on the commandStack. This is what we want for internal complex editions but not in this case. Indeed, it must be the KUndo2Command which will parent the UndoTextCommands. Therefore we set the addNewCommand back to false.
    //If the commandStack is empty, we are the highest "macro" command and we should therefore push the KUndo2Command on the application's stack.
    //On the contrary, if the commandStack is not empty, or the pushed command has a parent, it means that we are adding a nested KUndo2Command. In which case we just want to put it on the commandStack to parent UndoTextCommands. We need to call the redo method manually though.
    ++d->customCommandCount;
    debugText << "we will now go to custom state";
    d->updateState(KoTextEditor::Private::Custom, (!command->text().isEmpty())?command->text():kundo2_i18n("Text"));
    debugText << "but will set the addCommand to false. we don't want a new headCommand";
    d->addNewCommand = false;
    debugText << "commandStack count is: " << d->commandStack.count();
    if (d->commandStack.isEmpty()) {
        debugText << "the commandStack is empty. this means we are the top most command";
        d->commandStack.push(command);
        debugText << "command pushed on the commandStack. count: " << d->commandStack.count();
        KUndo2QStack *stack = KoTextDocument(d->document).undoStack();
        if (stack && !command->hasParent()) {
            debugText << "we have an application stack and the command is not a sub command of a non text command (which have been pushed outside kotext";
            stack->push(command);
            debugText << "so we pushed it on the application's' stack";
        } else {
            debugText << "we either have no application's stack, or our command is actually the child of a non kotext command";
            command->redo();
            debugText << "still called redo on it";
        }
    }
    else {
        debugText << "the commandStack is not empty, our command is actually nested in another kotext command. we don't push on the application stack but only on the commandStack";
        d->commandStack.push(command);
        debugText << "commandStack count after push: " << d->commandStack.count();
        command->redo();
        debugText << "called redo still";
    }

    //When we reach that point, the command has been executed. We first need to clean up all the automatically generated headCommand on our commandStack, which could potentially have been created during the editing. When we reach our pushed command, the commandStack is clean. We can then call a state update to NoOp and decrease the customCommandCount counter.
    debugText << "the command has been executed. we need to clean up the commandStack of the auto generated headCommands";
    debugText << "before cleaning. commandStack count: " << d->commandStack.count();
    while (d->commandStack.top() != command) {
        d->commandStack.pop();
    }
    debugText << "after cleaning. commandStack count: " << d->commandStack.count() << " will set NoOp";
    d->updateState(KoTextEditor::Private::NoOp);
    debugText << "after NoOp set. inCustomCounter: " << d->customCommandCount << " will decrease and return";
    --d->customCommandCount;
}

/// DO NOT USE THIS. It stays here for compiling reasons. But it will severely break everything. Again: DO NOT USE THIS.
void KoTextEditor::instantlyExecuteCommand(KUndo2Command *command)
{
    d->updateState(KoTextEditor::Private::Custom, (!command->text().isEmpty())?command->text():kundo2_i18n("Text"));
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
KUndo2Command *KoTextEditor::beginEditBlock(const KUndo2MagicString &title)
{
    debugText << "beginEditBlock";
    debugText << "commandStack count: " << d->commandStack.count();
    debugText << "customCommandCount counter: " << d->customCommandCount;
    if (!d->customCommandCount) {
        // We are not in a custom macro command. So we first need to update the KoTextEditor's state to Custom. Additionally, if the commandStack is empty, we need to create a master headCommand for our macro and push it on the stack.
        debugText << "we are not in a custom command. will update state to custom";
        d->updateState(KoTextEditor::Private::Custom, title);
        debugText << "commandStack count: " << d->commandStack.count();
        if (d->commandStack.isEmpty()) {
            debugText << "the commandStack is empty. we need a dummy headCommand both on the commandStack and on the application's stack";
            KUndo2Command *command = new KUndo2Command(title);
            d->commandStack.push(command);
            ++d->customCommandCount;
            d->dummyMacroAdded = true; //This bool is used to tell endEditBlock that we have created a master headCommand.
            KUndo2QStack *stack = KoTextDocument(d->document).undoStack();
            if (stack) {
                stack->push(command);
            } else {
                command->redo();
            }
            debugText << "done adding the headCommand. commandStack count: " << d->commandStack.count() << " inCommand counter: " << d->customCommandCount;
        }
    }
    //QTextDocument sends the undoCommandAdded signal at the end of the QTextCursor edit block. Since we want our master headCommand to parent the signal induced UndoTextCommands, we should not call QTextCursor::beginEditBlock for the headCommand.
    if (!(d->dummyMacroAdded && d->customCommandCount == 1)) {
        debugText << "we did not add a dummy command, or we are further down nesting. call beginEditBlock on the caret to nest the QTextDoc changes";
        //we don't call beginEditBlock for the first headCommand because we want the signals to be sent before we finished our command.
        d->caret.beginEditBlock();
    }
    debugText << "will return top od commandStack";
    return (d->commandStack.isEmpty())?0:d->commandStack.top();
}

void KoTextEditor::endEditBlock()
{
    debugText << "endEditBlock";
    //Only the self created master headCommand (see beginEditBlock) is left on the commandStack, we need to decrease the customCommandCount counter that we increased on creation.
    //If we are not yet at this master headCommand, we can call QTextCursor::endEditBlock
    if (d->dummyMacroAdded && d->customCommandCount == 1) {
        debugText << "only the created dummy headCommand from beginEditBlock is left. we need to decrease further the nesting counter";
        //we don't call caret.endEditBlock because we did not begin a block for the first headCommand
        --d->customCommandCount;
        d->dummyMacroAdded = false;
    } else {
        debugText << "we are not at our top dummy headCommand. call caret.endEditBlock";
        d->caret.endEditBlock();
    }
    if (!d->customCommandCount) {
        //We have now finished completely the macro, set the editor state to NoOp then.
        debugText << "we have finished completely the macro, set the state to NoOp now. commandStack count: " << d->commandStack.count();
        d->updateState(KoTextEditor::Private::NoOp);
        debugText << "done setting the state. editorState: " << d->editorState << " commandStack count: " << d->commandStack.count();
    }
}
