/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qdebug.h>
#include <KDE/KLocale>
#include <kstandardaction.h>
#include <kicon.h>
#include <kactioncollection.h>
#include "kundo2stack.h"
#include "kundo2stack_p.h"
#include "kundo2group.h"

#ifndef QT_NO_UNDOCOMMAND

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

KUndo2Command::KUndo2Command(const QString &text, KUndo2Command *parent)
{
    d = new KUndo2CommandPrivate;
    if (parent != 0)
        parent->d->child_list.append(this);
    setText(text);
}

/*!
    Constructs a KUndo2Command object with parent \a parent.

    If \a parent is not 0, this command is appended to parent's child list.
    The parent command then owns this command and will delete it in its
    destructor.

    \sa ~KUndo2Command()
*/

KUndo2Command::KUndo2Command(KUndo2Command *parent)
{
    d = new KUndo2CommandPrivate;
    if (parent != 0)
        parent->d->child_list.append(this);
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
    return d->actionText;
}

/*!
    Returns a short text string describing what this command does; for example,
    "insert text".

    The text is used when the text properties of the stack's undo and redo
    actions are updated.

    \sa setText(), KUndo2QStack::createUndoAction(), KUndo2QStack::createRedoAction()
*/

QString KUndo2Command::text() const
{
    return d->text;
}

/*!
    Sets the command's text to be the \a text specified.

    The specified text should be a short user-readable string describing what this
    command does.

    \sa text() KUndo2QStack::createUndoAction() KUndo2QStack::createRedoAction()
*/

void KUndo2Command::setText(const QString &text)
{
    int cdpos = text.indexOf(QLatin1Char('\n'));
    if (cdpos > 0)
    {
        d->text = text.left(cdpos);
        d->actionText = text.mid(cdpos + 1);
    }
    else
    {
        d->text = text;
        d->actionText = text;
    }
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

const KUndo2Command *KUndo2Command::child(int index) const
{
    if (index < 0 || index >= d->child_list.count())
        return 0;
    return d->child_list.at(index);
}

#endif // QT_NO_UNDOCOMMAND

#ifndef QT_NO_UNDOSTACK

/*!
    \class KUndo2QStack
    \brief The KUndo2QStack class is a stack of KUndo2Command objects.
    \since 4.2

    For an overview of Qt's Undo Framework, see the
    \l{Overview of Qt's Undo Framework}{overview document}.

    An undo stack maintains a stack of commands that have been applied to a
    document.

    New commands are pushed on the stack using push(). Commands can be
    undone and redone using undo() and redo(), or by triggering the
    actions returned by createUndoAction() and createRedoAction().

    KUndo2QStack keeps track of the \a current command. This is the command
    which will be executed by the next call to redo(). The index of this
    command is returned by index(). The state of the edited object can be
    rolled forward or back using setIndex(). If the top-most command on the
    stack has already been redone, index() is equal to count().

    KUndo2QStack provides support for undo and redo actions, command
    compression, command macros, and supports the concept of a
    \e{clean state}.

    \section1 Undo and Redo Actions

    KUndo2QStack provides convenient undo and redo QAction objects, which
    can be inserted into a menu or a toolbar. When commands are undone or
    redone, KUndo2QStack updates the text properties of these actions
    to reflect what change they will trigger. The actions are also disabled
    when no command is available for undo or redo. These actions
    are returned by KUndo2QStack::createUndoAction() and KUndo2QStack::createRedoAction().

    \section1 Command Compression and Macros

    Command compression is useful when several commands can be compressed
    into a single command that can be undone and redone in a single operation.
    For example, when a user types a character in a text editor, a new command
    is created. This command inserts the character into the document at the
    cursor position. However, it is more convenient for the user to be able
    to undo or redo typing of whole words, sentences, or paragraphs.
    Command compression allows these single-character commands to be merged
    into a single command which inserts or deletes sections of text.
    For more information, see KUndo2Command::mergeWith() and push().

    A command macro is a sequence of commands, all of which are undone and
    redone in one go. Command macros are created by giving a command a list
    of child commands.
    Undoing or redoing the parent command will cause the child commands to
    be undone or redone. Command macros may be created explicitly
    by specifying a parent in the KUndo2Command constructor, or by using the
    convenience functions beginMacro() and endMacro().

    Although command compression and macros appear to have the same effect to the
    user, they often have different uses in an application. Commands that
    perform small changes to a document may be usefully compressed if there is
    no need to individually record them, and if only larger changes are relevant
    to the user.
    However, for commands that need to be recorded individually, or those that
    cannot be compressed, it is useful to use macros to provide a more convenient
    user experience while maintaining a record of each command.

    \section1 Clean State

    KUndo2QStack supports the concept of a clean state. When the
    document is saved to disk, the stack can be marked as clean using
    setClean(). Whenever the stack returns to this state through undoing and
    redoing commands, it emits the signal cleanChanged(). This signal
    is also emitted when the stack leaves the clean state. This signal is
    usually used to enable and disable the save actions in the application,
    and to update the document's title to reflect that it contains unsaved
    changes.

    \sa KUndo2Command, KUndo2View
*/

#ifndef QT_NO_ACTION

KUndo2Action::KUndo2Action(const QString &textTemplate, const QString &defaultText, QObject *parent)
    : QAction(parent)
{
    m_textTemplate = textTemplate;
    m_defaultText = defaultText;
}

void KUndo2Action::setPrefixedText(const QString &text)
{
    if (text.isEmpty())
        setText(m_defaultText);
    else
        setText(m_textTemplate.arg(text));
}

#endif // QT_NO_ACTION

/*! \internal
    Sets the current index to \a idx, emitting appropriate signals. If \a clean is true,
    makes \a idx the clean index as well.
*/

void KUndo2QStack::setIndex(int idx, bool clean)
{
    bool was_clean = m_index == m_clean_index;

    if (idx != m_index) {
        m_index = idx;
        emit indexChanged(m_index);
        emit canUndoChanged(canUndo());
        emit undoTextChanged(undoText());
        emit canRedoChanged(canRedo());
        emit redoTextChanged(redoText());
    }

    if (clean)
        m_clean_index = m_index;

    bool is_clean = m_index == m_clean_index;
    if (is_clean != was_clean)
        emit cleanChanged(is_clean);
}

/*! \internal
    If the number of commands on the stack exceedes the undo limit, deletes commands from
    the bottom of the stack.

    Returns true if commands were deleted.
*/

bool KUndo2QStack::checkUndoLimit()
{
    if (m_undo_limit <= 0 || !m_macro_stack.isEmpty() || m_undo_limit >= m_command_list.count())
        return false;

    int del_count = m_command_list.count() - m_undo_limit;

    for (int i = 0; i < del_count; ++i)
        delete m_command_list.takeFirst();

    m_index -= del_count;
    if (m_clean_index != -1) {
        if (m_clean_index < del_count)
            m_clean_index = -1; // we've deleted the clean command
        else
            m_clean_index -= del_count;
    }

    return true;
}

/*!
    Constructs an empty undo stack with the parent \a parent. The
    stack will initially be in the clean state. If \a parent is a
    KUndo2Group object, the stack is automatically added to the group.

    \sa push()
*/

KUndo2QStack::KUndo2QStack(QObject *parent)
    : QObject(parent), m_index(0), m_clean_index(0), m_group(0), m_undo_limit(0)
{
#ifndef QT_NO_UNDOGROUP
    if (KUndo2Group *group = qobject_cast<KUndo2Group*>(parent))
        group->addStack(this);
#endif
}

/*!
    Destroys the undo stack, deleting any commands that are on it. If the
    stack is in a KUndo2Group, the stack is automatically removed from the group.

    \sa KUndo2QStack()
*/

KUndo2QStack::~KUndo2QStack()
{
#ifndef QT_NO_UNDOGROUP
    if (m_group != 0)
        m_group->removeStack(this);
#endif
    clear();
}

/*!
    Clears the command stack by deleting all commands on it, and returns the stack
    to the clean state.

    Commands are not undone or redone; the state of the edited object remains
    unchanged.

    This function is usually used when the contents of the document are
    abandoned.

    \sa KUndo2QStack()
*/

void KUndo2QStack::clear()
{
    if (m_command_list.isEmpty())
        return;

    bool was_clean = isClean();

    m_macro_stack.clear();
    qDeleteAll(m_command_list);
    m_command_list.clear();

    m_index = 0;
    m_clean_index = 0;

    emit indexChanged(0);
    emit canUndoChanged(false);
    emit undoTextChanged(QString());
    emit canRedoChanged(false);
    emit redoTextChanged(QString());

    if (!was_clean)
        emit cleanChanged(true);
}

/*!
    Pushes \a cmd on the stack or merges it with the most recently executed command.
    In either case, executes \a cmd by calling its redo() function.

    If \a cmd's id is not -1, and if the id is the same as that of the
    most recently executed command, KUndo2QStack will attempt to merge the two
    commands by calling KUndo2Command::mergeWith() on the most recently executed
    command. If KUndo2Command::mergeWith() returns true, \a cmd is deleted.

    In all other cases \a cmd is simply pushed on the stack.

    If commands were undone before \a cmd was pushed, the current command and
    all commands above it are deleted. Hence \a cmd always ends up being the
    top-most on the stack.

    Once a command is pushed, the stack takes ownership of it. There
    are no getters to return the command, since modifying it after it has
    been executed will almost always lead to corruption of the document's
    state.

    \sa KUndo2Command::id() KUndo2Command::mergeWith()
*/

void KUndo2QStack::push(KUndo2Command *cmd)
{
    cmd->redo();

    bool macro = !m_macro_stack.isEmpty();

    KUndo2Command *cur = 0;
    if (macro) {
        KUndo2Command *macro_cmd = m_macro_stack.last();
        if (!macro_cmd->d->child_list.isEmpty())
            cur = macro_cmd->d->child_list.last();
    } else {
        if (m_index > 0)
            cur = m_command_list.at(m_index - 1);
        while (m_index < m_command_list.size())
            delete m_command_list.takeLast();
        if (m_clean_index > m_index)
            m_clean_index = -1; // we've deleted the clean state
    }

    bool try_merge = cur != 0
                        && cur->id() != -1
                        && cur->id() == cmd->id()
                        && (macro || m_index != m_clean_index);

    if (try_merge && cur->mergeWith(cmd)) {
        delete cmd;
        if (!macro) {
            emit indexChanged(m_index);
            emit canUndoChanged(canUndo());
            emit undoTextChanged(undoText());
            emit canRedoChanged(canRedo());
            emit redoTextChanged(redoText());
        }
    } else {
        if (macro) {
            m_macro_stack.last()->d->child_list.append(cmd);
        } else {
            m_command_list.append(cmd);
            checkUndoLimit();
            setIndex(m_index + 1, false);
        }
    }
}

/*!
    Marks the stack as clean and emits cleanChanged() if the stack was
    not already clean.

    Whenever the stack returns to this state through the use of undo/redo
    commands, it emits the signal cleanChanged(). This signal is also
    emitted when the stack leaves the clean state.

    \sa isClean(), cleanIndex()
*/

void KUndo2QStack::setClean()
{
    if (!m_macro_stack.isEmpty()) {
        qWarning("KUndo2QStack::setClean(): cannot set clean in the middle of a macro");
        return;
    }

    setIndex(m_index, true);
}

/*!
    If the stack is in the clean state, returns true; otherwise returns false.

    \sa setClean() cleanIndex()
*/

bool KUndo2QStack::isClean() const
{
    if (!m_macro_stack.isEmpty())
        return false;
    return m_clean_index == m_index;
}

/*!
    Returns the clean index. This is the index at which setClean() was called.

    A stack may not have a clean index. This happens if a document is saved,
    some commands are undone, then a new command is pushed. Since
    push() deletes all the undone commands before pushing the new command, the stack
    can't return to the clean state again. In this case, this function returns -1.

    \sa isClean() setClean()
*/

int KUndo2QStack::cleanIndex() const
{
    return m_clean_index;
}

/*!
    Undoes the command below the current command by calling KUndo2Command::undo().
    Decrements the current command index.

    If the stack is empty, or if the bottom command on the stack has already been
    undone, this function does nothing.

    \sa redo() index()
*/

void KUndo2QStack::undo()
{
    if (m_index == 0)
        return;

    if (!m_macro_stack.isEmpty()) {
        qWarning("KUndo2QStack::undo(): cannot undo in the middle of a macro");
        return;
    }

    int idx = m_index - 1;
    m_command_list.at(idx)->undo();
    setIndex(idx, false);
}

/*!
    Redoes the current command by calling KUndo2Command::redo(). Increments the current
    command index.

    If the stack is empty, or if the top command on the stack has already been
    redone, this function does nothing.

    \sa undo() index()
*/

void KUndo2QStack::redo()
{
    if (m_index == m_command_list.size())
        return;

    if (!m_macro_stack.isEmpty()) {
        qWarning("KUndo2QStack::redo(): cannot redo in the middle of a macro");
        return;
    }

    m_command_list.at(m_index)->redo();
    setIndex(m_index + 1, false);
}

/*!
    Returns the number of commands on the stack. Macro commands are counted as
    one command.

    \sa index() setIndex() command()
*/

int KUndo2QStack::count() const
{
    return m_command_list.size();
}

/*!
    Returns the index of the current command. This is the command that will be
    executed on the next call to redo(). It is not always the top-most command
    on the stack, since a number of commands may have been undone.

    \sa undo() redo() count()
*/

int KUndo2QStack::index() const
{
    return m_index;
}

/*!
    Repeatedly calls undo() or redo() until the current command index reaches
    \a idx. This function can be used to roll the state of the document forwards
    of backwards. indexChanged() is emitted only once.

    \sa index() count() undo() redo()
*/

void KUndo2QStack::setIndex(int idx)
{
    if (!m_macro_stack.isEmpty()) {
        qWarning("KUndo2QStack::setIndex(): cannot set index in the middle of a macro");
        return;
    }

    if (idx < 0)
        idx = 0;
    else if (idx > m_command_list.size())
        idx = m_command_list.size();

    int i = m_index;
    while (i < idx)
        m_command_list.at(i++)->redo();
    while (i > idx)
        m_command_list.at(--i)->undo();

    setIndex(idx, false);
}

/*!
    Returns true if there is a command available for undo; otherwise returns false.

    This function returns false if the stack is empty, or if the bottom command
    on the stack has already been undone.

    Synonymous with index() == 0.

    \sa index() canRedo()
*/

bool KUndo2QStack::canUndo() const
{
    if (!m_macro_stack.isEmpty())
        return false;
    return m_index > 0;
}

/*!
    Returns true if there is a command available for redo; otherwise returns false.

    This function returns false if the stack is empty or if the top command
    on the stack has already been redone.

    Synonymous with index() == count().

    \sa index() canUndo()
*/

bool KUndo2QStack::canRedo() const
{
    if (!m_macro_stack.isEmpty())
        return false;
    return m_index < m_command_list.size();
}

/*!
    Returns the text of the command which will be undone in the next call to undo().

    \sa KUndo2Command::text() redoActionText() undoItemText()
*/

QString KUndo2QStack::undoText() const
{
    if (!m_macro_stack.isEmpty())
        return QString();
    if (m_index > 0)
        return m_command_list.at(m_index - 1)->actionText();
    return QString();
}

/*!
    Returns the text of the command which will be redone in the next call to redo().

    \sa KUndo2Command::text() undoActionText() redoItemText()
*/

QString KUndo2QStack::redoText() const
{
    if (!m_macro_stack.isEmpty())
        return QString();
    if (m_index < m_command_list.size())
        return m_command_list.at(m_index)->actionText();
    return QString();
}

#ifndef QT_NO_ACTION

/*!
    Creates an undo QAction object with the given \a parent.

    Triggering this action will cause a call to undo(). The text of this action
    is the text of the command which will be undone in the next call to undo(),
    prefixed by the specified \a prefix. If there is no command available for undo,
    this action will be disabled.

    If \a prefix is empty, the default prefix "Undo" is used.

    \sa createRedoAction(), canUndo(), KUndo2Command::text()
*/

QAction *KUndo2QStack::createUndoAction(QObject *parent) const
{
    KUndo2Action *result = new KUndo2Action(i18n("Undo %1"), i18nc("Default text for undo action", "Undo"), parent);
    result->setEnabled(canUndo());
    result->setPrefixedText(undoText());
    connect(this, SIGNAL(canUndoChanged(bool)),
            result, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(undoTextChanged(QString)),
            result, SLOT(setPrefixedText(QString)));
    connect(result, SIGNAL(triggered()), this, SLOT(undo()));
    return result;
}

/*!
    Creates an redo QAction object with the given \a parent.

    Triggering this action will cause a call to redo(). The text of this action
    is the text of the command which will be redone in the next call to redo(),
    prefixed by the specified \a prefix. If there is no command available for redo,
    this action will be disabled.

    If \a prefix is empty, the default prefix "Redo" is used.

    \sa createUndoAction(), canRedo(), KUndo2Command::text()
*/

QAction *KUndo2QStack::createRedoAction(QObject *parent) const
{
    KUndo2Action *result = new KUndo2Action(i18n("Redo %1"), i18nc("Default text for redo action", "Redo"), parent);
    result->setEnabled(canRedo());
    result->setPrefixedText(redoText());
    connect(this, SIGNAL(canRedoChanged(bool)),
            result, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(redoTextChanged(QString)),
            result, SLOT(setPrefixText(QString)));
    connect(result, SIGNAL(triggered()), this, SLOT(redo()));
    return result;
}

#endif // QT_NO_ACTION

/*!
    Begins composition of a macro command with the given \a text description.

    An empty command described by the specified \a text is pushed on the stack.
    Any subsequent commands pushed on the stack will be appended to the empty
    command's children until endMacro() is called.

    Calls to beginMacro() and endMacro() may be nested, but every call to
    beginMacro() must have a matching call to endMacro().

    While a macro is composed, the stack is disabled. This means that:
    \list
    \i indexChanged() and cleanChanged() are not emitted,
    \i canUndo() and canRedo() return false,
    \i calling undo() or redo() has no effect,
    \i the undo/redo actions are disabled.
    \endlist

    The stack becomes enabled and appropriate signals are emitted when endMacro()
    is called for the outermost macro.

    \snippet doc/src/snippets/code/src_gui_util_qundostack.cpp 4

    This code is equivalent to:

    \snippet doc/src/snippets/code/src_gui_util_qundostack.cpp 5

    \sa endMacro()
*/

void KUndo2QStack::beginMacro(const QString &text)
{
    KUndo2Command *cmd = new KUndo2Command();
    cmd->setText(text);

    if (m_macro_stack.isEmpty()) {
        while (m_index < m_command_list.size())
            delete m_command_list.takeLast();
        if (m_clean_index > m_index)
            m_clean_index = -1; // we've deleted the clean state
        m_command_list.append(cmd);
    } else {
        m_macro_stack.last()->d->child_list.append(cmd);
    }
    m_macro_stack.append(cmd);

    if (m_macro_stack.count() == 1) {
        emit canUndoChanged(false);
        emit undoTextChanged(QString());
        emit canRedoChanged(false);
        emit redoTextChanged(QString());
    }
}

/*!
    Ends composition of a macro command.

    If this is the outermost macro in a set nested macros, this function emits
    indexChanged() once for the entire macro command.

    \sa beginMacro()
*/

void KUndo2QStack::endMacro()
{
    if (m_macro_stack.isEmpty()) {
        qWarning("KUndo2QStack::endMacro(): no matching beginMacro()");
        return;
    }

    m_macro_stack.removeLast();

    if (m_macro_stack.isEmpty()) {
        checkUndoLimit();
        setIndex(m_index + 1, false);
    }
}

/*!
  \since 4.4

  Returns a const pointer to the command at \a index.

  This function returns a const pointer, because modifying a command,
  once it has been pushed onto the stack and executed, almost always
  causes corruption of the state of the document, if the command is 
  later undone or redone.

  \sa KUndo2Command::child()
*/
const KUndo2Command *KUndo2QStack::command(int index) const
{
    if (index < 0 || index >= m_command_list.count())
        return 0;
    return m_command_list.at(index);
}

/*!
    Returns the text of the command at index \a idx.

    \sa beginMacro()
*/

QString KUndo2QStack::text(int idx) const
{
    if (idx < 0 || idx >= m_command_list.size())
        return QString();
    return m_command_list.at(idx)->text();
}

/*!
    \property KUndo2QStack::undoLimit
    \brief the maximum number of commands on this stack.
    \since 4.3

    When the number of commands on a stack exceedes the stack's undoLimit, commands are
    deleted from the bottom of the stack. Macro commands (commands with child commands)
    are treated as one command. The default value is 0, which means that there is no
    limit.

    This property may only be set when the undo stack is empty, since setting it on a
    non-empty stack might delete the command at the current index. Calling setUndoLimit()
    on a non-empty stack prints a warning and does nothing.
*/

void KUndo2QStack::setUndoLimit(int limit)
{
    if (!m_command_list.isEmpty()) {
        qWarning("KUndo2QStack::setUndoLimit(): an undo limit can only be set when the stack is empty");
        return;
    }

    if (limit == m_undo_limit)
        return;
    m_undo_limit = limit;
    checkUndoLimit();
}

int KUndo2QStack::undoLimit() const
{
    return m_undo_limit;
}

/*!
    \property KUndo2QStack::active
    \brief the active status of this stack.

    An application often has multiple undo stacks, one for each opened document. The active
    stack is the one associated with the currently active document. If the stack belongs
    to a KUndo2Group, calls to KUndo2Group::undo() or KUndo2Group::redo() will be forwarded
    to this stack when it is active. If the KUndo2Group is watched by a KUndo2View, the view
    will display the contents of this stack when it is active. If the stack does not belong to
    a KUndo2Group, making it active has no effect.

    It is the programmer's responsibility to specify which stack is active by
    calling setActive(), usually when the associated document window receives focus.

    \sa KUndo2Group
*/

void KUndo2QStack::setActive(bool active)
{
#ifdef QT_NO_UNDOGROUP
    Q_UNUSED(active);
#else
    if (m_group != 0) {
        if (active)
            m_group->setActiveStack(this);
        else if (m_group->activeStack() == this)
            m_group->setActiveStack(0);
    }
#endif
}

bool KUndo2QStack::isActive() const
{
#ifdef QT_NO_UNDOGROUP
    return true;
#else
    return m_group == 0 || m_group->activeStack() == this;
#endif
}

QAction* KUndo2Stack::createRedoAction(KActionCollection* actionCollection, const QString& actionName)
{
    QAction* action = KUndo2QStack::createRedoAction(actionCollection);

    if(actionName.isEmpty()) {
        action->setObjectName(KStandardAction::name(KStandardAction::Redo));
    } else {
        action->setObjectName(actionName);
    }

    action->setIcon(KIcon("edit-redo"));
    action->setIconText(i18n("Redo"));
    action->setShortcuts(KStandardShortcut::redo());

    actionCollection->addAction(action->objectName(), action);

    return action;
}

QAction* KUndo2Stack::createUndoAction(KActionCollection* actionCollection, const QString& actionName)
{
    QAction* action = KUndo2QStack::createUndoAction(actionCollection);

    if(actionName.isEmpty()) {
        action->setObjectName(KStandardAction::name(KStandardAction::Undo));
    } else {
        action->setObjectName(actionName);
    }

    action->setIcon(KIcon("edit-undo"));
    action->setIconText(i18n("Undo"));
    action->setShortcuts(KStandardShortcut::undo());

    actionCollection->addAction(action->objectName(), action);

    return action;
}

/*!
    \fn void KUndo2QStack::indexChanged(int idx)

    This signal is emitted whenever a command modifies the state of the document.
    This happens when a command is undone or redone. When a macro
    command is undone or redone, or setIndex() is called, this signal
    is emitted only once.

    \a idx specifies the index of the current command, ie. the command which will be
    executed on the next call to redo().

    \sa index() setIndex()
*/

/*!
    \fn void KUndo2QStack::cleanChanged(bool clean)

    This signal is emitted whenever the stack enters or leaves the clean state.
    If \a clean is true, the stack is in a clean state; otherwise this signal
    indicates that it has left the clean state.

    \sa isClean() setClean()
*/

/*!
    \fn void KUndo2QStack::undoTextChanged(const QString &undoText)

    This signal is emitted whenever the value of undoText() changes. It is
    used to update the text property of the undo action returned by createUndoAction().
    \a undoText specifies the new text.
*/

/*!
    \fn void KUndo2QStack::canUndoChanged(bool canUndo)

    This signal is emitted whenever the value of canUndo() changes. It is
    used to enable or disable the undo action returned by createUndoAction().
    \a canUndo specifies the new value.
*/

/*!
    \fn void KUndo2QStack::redoTextChanged(const QString &redoText)

    This signal is emitted whenever the value of redoText() changes. It is
    used to update the text property of the redo action returned by createRedoAction().
    \a redoText specifies the new text.
*/

/*!
    \fn void KUndo2QStack::canRedoChanged(bool canRedo)

    This signal is emitted whenever the value of canRedo() changes. It is
    used to enable or disable the redo action returned by createRedoAction().
    \a canRedo specifies the new value.
*/

KUndo2Stack::KUndo2Stack(QObject *parent):
	KUndo2QStack(parent)
{
}

#endif // QT_NO_UNDOSTACK
