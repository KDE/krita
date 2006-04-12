/* This file is part of the KDE project
   Copyright (C) 2000 Werner Trobin <trobin@kde.org>
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef kocommandhistory_h
#define kocommandhistory_h

#include <q3ptrlist.h>
#include <qstring.h>
#include <qobject.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3PopupMenu>
#include <koffice_export.h>

class KAction;
class KActionCollection;
class Q3PopupMenu;
class KCommand;
#include <q3listbox.h>

class KoListBox : public Q3ListBox {
    Q_OBJECT
public:
    KoListBox( QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0 );
protected:
    virtual void contentsMouseMoveEvent ( QMouseEvent * );
    virtual QSize sizeHint() const;
signals:
    void changeNumberOfSelectedItem( int );
};

/**
 * The command history stores a (user) configurable amount of
 * Commands. It keeps track of its size and deletes commands
 * if it gets too large. The user can set a maximum undo and
 * a maximum redo limit (e.g. max. 50 undo / 30 redo commands).
 * The KoCommandHistory keeps track of the "borders" and deletes
 * commands, if appropriate. It also activates/deactivates the
 * undo/redo actions in the menu and changes the text according
 * to the name of the command.
 *
 * @short History of user commands (for undo/redo)
 */
class KOFFICEUI_EXPORT KoCommandHistory : public QObject {
    Q_OBJECT
public:
    /**
     * Creates a command history, to store commands.
     * This constructor doesn't create actions, so you need to call
     * #undo and #redo yourself.
     */
    KoCommandHistory();

    /**
     * Creates a command history, to store commands.
     * This also creates an undo and a redo action, in the @p actionCollection,
     * using the standard names ("edit_undo" and "edit_redo").
     *
     * @param actionCollection the collection to put the history in.
     * @param withMenus if true, the actions will display a menu when plugged
     * into a toolbar.
     */
    KoCommandHistory(KActionCollection *actionCollection, bool withMenus = true);

    /**
     * Destructs the command history object.
     */
    virtual ~KoCommandHistory();

    /**
     * Erases all the undo/redo history.
     * Use this when reloading the data, for instance, since this invalidates
     * all the commands.
     */
    void clear();

    /**
     * Adds a command to the history. Call this for each @p command you create.
     * Unless you set @p execute to false, this will also execute the command.
     * This means, most of the application's code will look like:
     *
     *    MyCommand * cmd = new MyCommand(i18n("The Name"), parameters);
     *    m_historyCommand.addCommand( cmd );
     */
    void addCommand(KCommand *command, bool execute=true);

    /**
     * @return the maximum number of items in the undo history
     */
    int undoLimit() const { return m_undoLimit; }
    /**
     * Sets the maximum number of items in the undo history.
     */
    void setUndoLimit(int limit);
    /**
     * @return the maximum number of items in the redo history
     */
    int redoLimit() const { return m_redoLimit; }
    /**
     * Sets the maximum number of items in the redo history.
     */
    void setRedoLimit(int limit);

    /**
     * Enable or disable the undo and redo actions.
     * This isn't usually necessary, but this method can be useful if
     * you disable all actions (to go to a "readonly" state), and then
     * want to come back to a readwrite mode.
     */
    void updateActions();

    /**
     * @return the current top item on the history stack
     */
    KCommand * presentCommand();

public slots:
    /**
     * Undoes the last action.
     * Call this if you don't use the builtin KActions.
     */
    virtual void undo();
    /**
     * Redoes the last undone action.
     * Call this if you don't use the builtin KActions.
     */
    virtual void redo();
    /**
     * Remembers when you saved the document.
     * Call this right after saving the document. As soon as
     * the history reaches the current index again (via some
     * undo/redo operations) it will emit @ref documentRestored
     * If you implemented undo/redo properly the document is
     * the same you saved before.
     */
    virtual void documentSaved();

protected slots:
    void slotUndoAboutToShow();
    void slotUndoActivated( int );
    void slotRedoAboutToShow();
    void slotRedoActivated( int );
    void slotUndoActivated( Q3ListBoxItem *);
    void slotRedoActivated( Q3ListBoxItem *);
    void slotChangeRedoNumberOfSelectedItem( int );
    void slotChangeUndoNumberOfSelectedItem( int );
signals:
    /**
     * Emitted every time a command is executed
     * (whether by addCommand, undo or redo).
     * You can use this to update the GUI, for instance.
     */
    void commandExecuted();

    /**
     * Emitted every time a command is executed
     * (whether by addCommand, undo or redo).
     * You can use this to update the GUI, for instance.
     *
     * @param cmd pointer to the command that was executed
     * @todo document who owns that pointer
     */
    void commandExecuted(KCommand *cmd);

    /**
     * Emitted every time we reach the index where you
     * saved the document for the last time. See @ref #documentSaved
     */
    void documentRestored();

private:
    void clipCommands();  // ensures that the limits are kept

    Q3PtrList<KCommand> m_commands;
    KAction *m_undo, *m_redo;
    Q3PopupMenu *m_undoPopup, *m_redoPopup;
    int m_undoLimit, m_redoLimit;
    bool m_first;  // attention: it's the first command in the list!
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KoCommandHistoryPrivate;
    KoCommandHistoryPrivate *d;
};

#endif
