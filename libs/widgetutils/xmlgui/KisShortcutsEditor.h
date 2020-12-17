/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Nicolas Hadacek <hadacek@kde.org>
    SPDX-FileCopyrightText: 2001, 2001 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>
    SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>


    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KISSHORTCUTSEDITOR_H
#define KISSHORTCUTSEDITOR_H

#include <kritawidgetutils_export.h>

#include <QWidget>
#include <KisKineticScroller.h>

class KActionCollection;
class KConfig;
class KConfigBase;
class KConfigGroup;
class KGlobalAccel;
class KisShortcutsEditorPrivate;


/**
 * WARNING: KisShortcutsEditor expects that the list of existing shortcuts is
 * already free of conflicts. If it is not, nothing will crash, but your users
 * won't like the resulting behavior.
 *
 * TODO: Find the right place to check for conflicts.
 */


/**
 * @short Widget for configuration of KAccel and KGlobalAccel.
 *
 * Configure dictionaries of key/action associations for KActions,
 * including global shortcuts.
 *
 * The class takes care of all aspects of configuration, including
 * handling key conflicts internally. Connect to the allDefault()
 * slot if you want to set all configurable shortcuts to their
 * default values.
 *
 * @see KShortcutsDialog
 * @author Nicolas Hadacek <hadacek@via.ecp.fr>
 * @author Hamish Rodda <rodda@kde.org> (KDE 4 porting)
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class KRITAWIDGETUTILS_EXPORT KisShortcutsEditor : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(ActionTypes actionTypes READ actionTypes WRITE setActionTypes)

public:
    /*
     * These attempt to build some sort of characterization of all actions. The
     * idea is to determine which sorts of actions will be configured in the
     * dialog.
     *
     * Enumerating all possible actions is a sorrowful, pitiable endeavor,
     * useless for Krita. We should do something about this.
     */
    enum ActionType {
        /// Actions which are triggered by any keypress in a widget which has the action added to it
        WidgetAction      = Qt::WidgetShortcut      /*0*/,
        /// Actions which are triggered by any keypress in a window which has the action added to it or its child widget(s)
        WindowAction      = Qt::WindowShortcut      /*1*/,
        /// Actions which are triggered by any keypress in the application
        ApplicationAction = Qt::ApplicationShortcut /*2*/,
        /// Actions which are triggered by any keypress in the windowing system
        GlobalAction      = 4,
        /// All actions
        AllActions        = 0xffffffff
    };
    Q_DECLARE_FLAGS(ActionTypes, ActionType)

    enum LetterShortcuts {
        /// Shortcuts without a modifier are not allowed, so 'A' would not be
        /// valid, whereas 'Ctrl+A' would be. This only applies to printable
        /// characters, however. 'F1', 'Insert' etc. could still be used.
        LetterShortcutsDisallowed = 0,
        /// Letter shortcuts are allowed
        LetterShortcutsAllowed
    };

    /**
     * \overload
     *
     * Creates a key chooser without a starting action collection.
     *
     * @param parent parent widget
     * @param actionTypes types of actions to display in this widget.
     * @param allowLetterShortcuts set to LetterShortcutsDisallowed if unmodified alphanumeric
     *  keys ('A', '1', etc.) are not permissible shortcuts.
     */
    explicit KisShortcutsEditor(QWidget *parent,
                                ActionTypes actionTypes = AllActions, LetterShortcuts allowLetterShortcuts = LetterShortcutsAllowed);

    /// Destructor
    ~KisShortcutsEditor() override;

    /**
     * @return @c true if there are unsaved changes.
     */
    bool isModified() const;

    /**
     * Removes all action collections from the editor
     */
    void clearCollections();

    /**
     * Clears search area
     */
    void clearSearch();

    /**
     * Note: the reason this is so damn complicated is because it's supposed to
     * support having multiple applications running inside of each other through
     * KisParts. That means we have to be able to separate sections within each
     * configuration file.
     *
     * Insert an action collection, i.e. add all its actions to the ones
     * already associated with the KisShortcutsEditor object.
     *
     * @param collection the action collection.
     * @param title subtree title of this collection of shortcut.
     */
    void addCollection(KActionCollection *, const QString &title = QString());

    /**
     * Undo all change made since the last commit().
     */
    void undo();

    /**
     * Save the changes.
     *
     * Before saving the changes are committed. This saves the actions to disk.
     * Any KActionCollection objects with the xmlFile() value set will be
     * written to an XML file.  All other will be written to the application's
     * rc file.
     */
    void save();

    /**
     * Update the dialog entries without saving.
     *
     * @since 4.2
     */
    void commit();

    /**
     * Removes all configured shortcuts.
     */
    void clearConfiguration();

    /**
     * Write the current custom shortcut settings to the \p config object.
     *
     * @param config Config object to save to. Default is kritashortcutsrc.
     *
     */
    void saveShortcuts(KConfigGroup *config = 0) const;


    /**
     * Write the current shortcuts to a new scheme to configuration file
     *
     * @param config Config object to save to.
     */
    void exportConfiguration(KConfigBase *config) const;


    /**
     * Import a shortcut configuration file.
     *
     * @param config Config object to load from.
     * @param isScheme true for shortcut scheme, false for custom shortcuts
     */
    void importConfiguration(KConfigBase *config, bool isScheme);

    /**
     * Sets the types of actions to display in this widget.
     *
     * @param actionTypes New types of actions
     * @since 5.0
     */
    void setActionTypes(ActionTypes actionTypes);
    /**
     *
     * @return The types of actions currently displayed in this widget.
     * @since 5.0
     */
    ActionTypes actionTypes() const;

Q_SIGNALS:
    /**
     * Emitted when an action's shortcut has been changed.
     **/
    void keyChange();

public Q_SLOTS:
    /**
     * Resize columns to width required
     */
    void resizeColumns();

    /**
     * Set all shortcuts to their default values (bindings).
     **/
    void allDefault();

    /**
     * Opens a printing dialog to print all the shortcuts
     */
    void printShortcuts() const;

    /**
     * Expand or collapse the tree view when the search text changes
     */
    void searchUpdated(QString s);

    void slotScrollerStateChanged(QScroller::State state){KisKineticScroller::updateCursor(this, state);}

private:
    Q_PRIVATE_SLOT(d, void capturedShortcut(QVariant, const QModelIndex &))

private:
    friend class KisShortcutsDialog;
    friend class KisShortcutsEditorPrivate;
    KisShortcutsEditorPrivate *const d;
    Q_DISABLE_COPY(KisShortcutsEditor)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KisShortcutsEditor::ActionTypes)

#endif // KISSHORTCUTSEDITOR_H
