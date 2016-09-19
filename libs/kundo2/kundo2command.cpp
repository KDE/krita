/*
 * Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#include "kundo2command.h"
#include <QDebug>
#include <klocalizedstring.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include "kundo2stack.h"
#include "kundo2stack_p.h"
#include "kundo2group.h"
#include <KoIcon.h>
#include <QtGlobal>


/*!
    \class KUndo2Command
    \brief The KUndo2Command class is the base class of all commands stored on a KUndo2QStack.
    \since 4.2

    For an overview of Qt's Undo Framework, see the
    \l{Overview of Qt's Undo Framework}{overview document}.

    A KUndo2Command represents a single editing action on a document; for example,
    inserting or deleting a block of text in a text editor. KUndo2Command can apply
    a change to the document with redo() and undo the change with undo(). The
    implementations for these functions must be provided in a derived class.

    \snippet doc/src/snippets/code/src_gui_util_qundostack.cpp 0

    A KUndo2Command has an associated text(). This is a short string
    describing what the command does. It is used to update the text
    properties of the stack's undo and redo actions; see
    KUndo2QStack::createUndoAction() and KUndo2QStack::createRedoAction().

    KUndo2Command objects are owned by the stack they were pushed on.
    KUndo2QStack deletes a command if it has been undone and a new command is pushed. For example:

\snippet doc/src/snippets/code/src_gui_util_qundostack.cpp 1

    In effect, when a command is pushed, it becomes the top-most command
    on the stack.

    To support command compression, KUndo2Command has an id() and the virtual function
    mergeWith(). These functions are used by KUndo2QStack::push().

    To support command macros, a KUndo2Command object can have any number of child
    commands. Undoing or redoing the parent command will cause the child
    commands to be undone or redone. A command can be assigned
    to a parent explicitly in the constructor. In this case, the command
    will be owned by the parent.

    The parent in this case is usually an empty command, in that it doesn't
    provide its own implementation of undo() and redo(). Instead, it uses
    the base implementations of these functions, which simply call undo() or
    redo() on all its children. The parent should, however, have a meaningful
    text().

    \snippet doc/src/snippets/code/src_gui_util_qundostack.cpp 2

    Another way to create macros is to use the convenience functions
    KUndo2QStack::beginMacro() and KUndo2QStack::endMacro().

    \sa KUndo2QStack
*/

/*!
    Constructs a KUndo2Command object with the given \a parent and \a text.

    If \a parent is not 0, this command is appended to parent's child list.
    The parent command then owns this command and will delete it in its
    destructor.

    \sa ~KUndo2Command()
*/

KUndo2Command::KUndo2Command(const KUndo2MagicString &text, KUndo2Command *parent)
    : QObject()
    , m_hasParent(parent != 0)
    , m_timedID(0)
    , m_endOfCommand(QTime::currentTime())
{
    d = new KUndo2CommandPrivate;
    if (parent != 0) {
        parent->d->child_list.append(this);
    }
    setText(text);
    setTime();
}

/*!
    Constructs a KUndo2Command object with parent \a parent.

    If \a parent is not 0, this command is appended to parent's child list.
    The parent command then owns this command and will delete it in its
    destructor.

    \sa ~KUndo2Command()
*/

KUndo2Command::KUndo2Command(KUndo2Command *parent):
    m_hasParent(parent != 0),m_timedID(0)
{
    d = new KUndo2CommandPrivate;
    if (parent != 0)
        parent->d->child_list.append(this);
    setTime();
}

/*!
    Destroys the KUndo2Command object and all child commands.

    \sa KUndo2Command()
*/

KUndo2Command::~KUndo2Command()
{
    qDeleteAll(d->child_list);
    delete d;
}

/*!
    Returns the ID of this command.

    A command ID is used in command compression. It must be an integer unique to
    this command's class, or -1 if the command doesn't support compression.

    If the command supports compression this function must be overridden in the
    derived class to return the correct ID. The base implementation returns -1.

    KUndo2QStack::push() will only try to merge two commands if they have the
    same ID, and the ID is not -1.

    \sa mergeWith(), KUndo2QStack::push()
*/

int KUndo2Command::id() const
{
    return -1;
}

/*!
    Attempts to merge this command with \a command. Returns true on
    success; otherwise returns false.

    If this function returns true, calling this command's redo() must have the same
    effect as redoing both this command and \a command.
    Similarly, calling this command's undo() must have the same effect as undoing
    \a command and this command.

    KUndo2QStack will only try to merge two commands if they have the same id, and
    the id is not -1.

    The default implementation returns false.

    \snippet doc/src/snippets/code/src_gui_util_qundostack.cpp 3

    \sa id() KUndo2QStack::push()
*/

bool KUndo2Command::mergeWith(const KUndo2Command *command)
{
    Q_UNUSED(command);
    return false;
}

/*!
    Applies a change to the document. This function must be implemented in
    the derived class. Calling KUndo2QStack::push(),
    KUndo2QStack::undo() or KUndo2QStack::redo() from this function leads to
    undefined beahavior.

    The default implementation calls redo() on all child commands.

    \sa undo()
*/

void KUndo2Command::redo()
{
    for (int i = 0; i < d->child_list.size(); ++i)
        d->child_list.at(i)->redo();
}

/*!
    Reverts a change to the document. After undo() is called, the state of
    the document should be the same as before redo() was called. This function must
    be implemented in the derived class. Calling KUndo2QStack::push(),
    KUndo2QStack::undo() or KUndo2QStack::redo() from this function leads to
    undefined beahavior.

    The default implementation calls undo() on all child commands in reverse order.

    \sa redo()
*/

void KUndo2Command::undo()
{
    for (int i = d->child_list.size() - 1; i >= 0; --i)
        d->child_list.at(i)->undo();
}

/*!
    Returns a short text string describing what this command does; for example,
    "insert text".

    The text is used when the text properties of the stack's undo and redo
    actions are updated.

    \sa setText(), KUndo2QStack::createUndoAction(), KUndo2QStack::createRedoAction()
*/

QString KUndo2Command::actionText() const
{
    if(d->actionText!=0)
        return d->actionText;
    else
        return QString();
}

/*!
    Returns a short text string describing what this command does; for example,
    "insert text".

    The text is used when the text properties of the stack's undo and redo
    actions are updated.

    \sa setText(), KUndo2QStack::createUndoAction(), KUndo2QStack::createRedoAction()
*/

KUndo2MagicString KUndo2Command::text() const
{
    return d->text;
}

/*!
    Sets the command's text to be the \a text specified.

    The specified text should be a short user-readable string describing what this
    command does.

    \sa text() KUndo2QStack::createUndoAction() KUndo2QStack::createRedoAction()
*/

void KUndo2Command::setText(const KUndo2MagicString &undoText)
{
    d->text = undoText;
    d->actionText = undoText.toSecondaryString();
}

/*!
    \since 4.4

    Returns the number of child commands in this command.

    \sa child()
*/

int KUndo2Command::childCount() const
{
    return d->child_list.count();
}

/*!
    \since 4.4

    Returns the child command at \a index.

    \sa childCount(), KUndo2QStack::command()
*/

//const KUndo2Command *KUndo2Command::child(int index) const
//{
//    if (index < 0 || index >= d->child_list.count())
//        return 0;
//    return d->child_list.at(index);
//}

bool KUndo2Command::hasParent()
{
    return m_hasParent;
}
int KUndo2Command::timedId()
{
    return m_timedID;
}
void KUndo2Command::setTimedID(int value)
{
    m_timedID = value;
}

bool KUndo2Command::timedMergeWith(KUndo2Command *other)
{
    if(other->timedId() == this->timedId() && other->timedId()!=-1 )
        m_mergeCommandsVector.append(other);
    else
        return false;
    return true;
}
void KUndo2Command::setTime()
{
    m_timeOfCreation = QTime::currentTime();
}
QTime KUndo2Command::time()
{
    return m_timeOfCreation;
}
void KUndo2Command::setEndTime()
{
    m_endOfCommand  = QTime::currentTime();
}
QTime KUndo2Command::endTime()
{
    return m_endOfCommand;
}

void KUndo2Command::undoMergedCommands()
{

    undo();
    if (!mergeCommandsVector().isEmpty()) {
        QVectorIterator<KUndo2Command*> it(mergeCommandsVector());
        it.toFront();
        while (it.hasNext()) {
            KUndo2Command* cmd = it.next();
            cmd->undoMergedCommands();
        }
    }
}

void KUndo2Command::redoMergedCommands()
{
    if (!mergeCommandsVector().isEmpty()) {

        QVectorIterator<KUndo2Command*> it(mergeCommandsVector());
        it.toBack();
        while (it.hasPrevious()) {
            KUndo2Command* cmd = it.previous();
            cmd->redoMergedCommands();
        }
    }
    redo();
}
QVector<KUndo2Command*> KUndo2Command::mergeCommandsVector()
{
    return m_mergeCommandsVector;
}
bool KUndo2Command::isMerged()
{
    return !m_mergeCommandsVector.isEmpty();
}

KUndo2CommandExtraData* KUndo2Command::extraData() const
{
    return d->extraData.data();
}

void KUndo2Command::setExtraData(KUndo2CommandExtraData *data)
{
    d->extraData.reset(data);
}
