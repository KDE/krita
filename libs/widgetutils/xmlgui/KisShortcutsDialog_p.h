/* This file is part of the KDE libraries
    Copyright (C) 2006,2007 Andreas Hartmetz (ahartmetz@gmail.com)
    Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>
    Copyright (C) 2008 Alexander Dymo <adymo@kdevelop.org>

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
    Boston, MA 02110-1301, USA.
*/

#ifndef KISSHORTCUTSDIALOG_P_H
#define KISSHORTCUTSDIALOG_P_H

#include "KisShortcutsEditor.h"
#include "kkeysequencewidget.h"
#include "KisShortcutsDialog.h"

#include <kextendableitemdelegate.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>

#include <QKeySequence>
#include <QMetaType>
#include <QModelIndex>
#include <QTreeWidget>
#include <QtCore/QList>
#include <QtCore/QCollator>
#include <QGroupBox>

class QLabel;
class QTreeWidget;
class QTreeWidgetItem;
class QRadioButton;
class QAction;
class KActionCollection;
class QPushButton;
class QComboBox;
class KisShortcutsDialog;
class QAction;


// What is this for??  (will use abbreviation WTF)
enum ColumnDesignation {
    Name = 0,
    LocalPrimary,
    LocalAlternate,
    GlobalPrimary,
    GlobalAlternate,
    Id
};

// XXX: Hmm
enum MyRoles {
    ShortcutRole = Qt::UserRole,
    DefaultShortcutRole,
    ObjectRole
};


/**
 * Type used for QTreeWidgetItems
 * WTF?
 *
 * @internal
 */
enum ItemTypes {
    NonActionItem = 0,
    ActionItem = 1
};

// Return the first item of the list, if it exists
QKeySequence primarySequence(const QList<QKeySequence> &sequences);

// Return the second item of the list, if it exists
QKeySequence alternateSequence(const QList<QKeySequence> &sequences);



class KisShortcutsDialog::KisShortcutsDialogPrivate
{
public:
    KisShortcutsDialogPrivate(KisShortcutsDialog *q);
    void changeShortcutScheme(const QString &scheme);
    void undoChanges();
    void save();

    QList<KActionCollection *> m_collections;
    KisShortcutsDialog *q;
    KisShortcutsEditor *m_shortcutsEditor;
    bool m_saveSettings{false};
};


/**
 * Mixes the KShortcutWidget into the treeview used by KisShortcutsEditor. When
 * selecting an shortcut it changes the display from "CTRL-W" to the Widget.
 *
 * @bug That delegate uses KExtendableItemDelegate. That means a cell can be
 * expanded. When selected a cell is replaced by a KisShortcutsEditor. When
 * painting the widget KExtendableItemDelegate reparents the widget to the
 * viewport of the itemview it belongs to. The widget is destroyed when the user
 * selects another shortcut or explicitly issues a contractItem event. But when
 * the user clears the model the delegate misses that event and doesn't delete
 * the KShortcutseditor. And remains as a visible artefact in your treeview.
 * Additionally when closing your application you get an assertion failure from
 * KExtendableItemDelegate.
 *
 * @internal
 */
class KisShortcutsEditorDelegate : public KExtendableItemDelegate
{
    Q_OBJECT
public:
    KisShortcutsEditorDelegate(QTreeWidget *parent, bool allowLetterShortcuts);
    //reimplemented to have some extra height
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

    /**
     * Set a list of action collections to check against for conflicting
     * shortcuts.
     *
     * @see KKeySequenceWidget::setCheckActionCollections
     */
    void setCheckActionCollections(const QList<KActionCollection *> checkActionCollections);
    bool eventFilter(QObject *, QEvent *) Q_DECL_OVERRIDE;
private:
    mutable QPersistentModelIndex m_editingIndex;
    bool m_allowLetterShortcuts;
    QWidget *m_editor;

    //! List of actionCollections to check for conflicts.
    QList<KActionCollection *> m_checkActionCollections;


Q_SIGNALS:
  void shortcutChanged(QVariant, const QModelIndex &);

public Q_SLOTS:
  void hiddenBySearchLine(QTreeWidgetItem *, bool);

private Q_SLOTS:
    void itemActivated(QModelIndex index);

    /**
     * When the user collapses a hole subtree of shortcuts then remove eventually
     * extended items. Else we get that artefact bug. See above.
     */
    void itemCollapsed(QModelIndex index);

    /**
     * If the user allowed stealing a shortcut we want to be able to undo
     * that.
     */
    void stealShortcut(const QKeySequence &seq, QAction *action);

    void keySequenceChanged(const QKeySequence &);

};


/**
 * Edit a shortcut. Let you select between using the default shortcut and configuring your own.
 *
 * @internal
 */
class ShortcutEditWidget : public QWidget
{
    Q_OBJECT
public:
    ShortcutEditWidget(QWidget *viewport, const QKeySequence &defaultSeq, const QKeySequence &activeSeq,
                       bool allowLetterShortcuts);

    //! @see KKeySequenceWidget::setCheckActionCollections()
    void setCheckActionCollections(const QList<KActionCollection *> checkActionCollections);

    //@{
    //! @see KKeySequenceWidget::checkAgainstStandardShortcuts()
    KKeySequenceWidget::ShortcutTypes checkForConflictsAgainst() const;
    void setCheckForConflictsAgainst(KKeySequenceWidget::ShortcutTypes);
    //@}

    //@{
    //! @see KKeySequenceWidget::checkAgainstStandardShortcuts()
    bool multiKeyShortcutsAllowed() const;
    void setMultiKeyShortcutsAllowed(bool);
    //@}

    //! @see KKeySequenceWidget::setComponentName
    void setComponentName(const QString componentName);

    void setAction(QObject *action);
    void paintEvent(QPaintEvent *pe) Q_DECL_OVERRIDE;


Q_SIGNALS:
    //! Emitted when the key sequence is changed.
    void keySequenceChanged(const QKeySequence &);

    //! @see KKeySequenceWidget::stealShortcut()
    void stealShortcut(const QKeySequence &seq, QAction *action);


public Q_SLOTS:
  //! Set the displayed sequences
  void setKeySequence(const QKeySequence &activeSeq);

private Q_SLOTS:
    void defaultToggled(bool);
    void setCustom(const QKeySequence &);



private:
    QLabel *m_defaultLabel;
    QKeySequence m_defaultKeySequence;
    QRadioButton *m_defaultRadio;
    QRadioButton *m_customRadio;
    KKeySequenceWidget *m_customEditor;
    bool m_isUpdating;
    QObject *m_action;
};



/**
 * A QTreeWidgetItem that can handle QActions.
 *
 * It provides undo, commit functionality for changes made. Changes are effective immediately. You
 * have to commit them or they will be undone when deleting the item.
 *
 * @internal
 */
class KisShortcutsEditorItem : public QTreeWidgetItem
{
public:

    KisShortcutsEditorItem(QTreeWidgetItem *parent, QAction *action);

    /**
     * Destructor
     *
     * Will undo pending changes. If you don't want that. Call commitChanges before
     */
    virtual ~KisShortcutsEditorItem();

    //! Undo the changes since the last commit.
    void undo();

    //! Commit the changes.
    void commit();

    QVariant data(int column, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool operator<(const QTreeWidgetItem &other) const Q_DECL_OVERRIDE;

    QKeySequence keySequence(uint column) const;
    void setKeySequence(uint column, const QKeySequence &seq);
    bool isModified(uint column) const;
    bool isModified() const;

    void setNameBold(bool flag)
    {
        m_isNameBold = flag;
    }

private:
    friend class KisShortcutsEditorPrivate;

    //! Recheck modified status - could have changed back to initial value
    void updateModified();

    //! The action this item is responsible for
    QAction *m_action;

    //! Should the Name column be painted in bold?
    bool m_isNameBold;

    //@{
    //! The original shortcuts before user changes. 0 means no change.
    QList<QKeySequence> *m_oldLocalShortcut;
    QList<QKeySequence> *m_oldGlobalShortcut;
    //@}

    //! The localized action name
    QString m_actionNameInTable;

    //! The action id. Needed for exporting and importing
    QString m_id;

    //! The collator, for sorting
    QCollator m_collator;

};

// NEEDED FOR KisShortcutsEditorPrivate
#include "ui_KisShortcutsDialog.h"

// Hack to make two protected methods public.
// Used by both KisShortcutsEditorPrivate and KisShortcutsEditorDelegate
class QTreeWidgetHack : public QTreeWidget
{
public:
    QTreeWidgetItem *itemFromIndex(const QModelIndex &index) const
    {
        return QTreeWidget::itemFromIndex(index);
    }
    QModelIndex indexFromItem(QTreeWidgetItem *item, int column) const
    {
        return QTreeWidget::indexFromItem(item, column);
    }
};

/**
 * This class should belong into kshortcutseditor.cpp. But kshortcutseditordelegate uses a static
 * function of this class. So for now it's here. But i will remove it later.
 *
 * @internal
 */
class KisShortcutsEditorPrivate
{
public:

    KisShortcutsEditorPrivate(KisShortcutsEditor *q);

    void initGUI(KisShortcutsEditor::ActionTypes actionTypes, KisShortcutsEditor::LetterShortcuts allowLetterShortcuts);
    void appendToView(uint nList, const QString &title = QString());
    //used in appendToView
    QTreeWidgetItem *findOrMakeItem(QTreeWidgetItem *parent, const QString &name);

    static KisShortcutsEditorItem *itemFromIndex(QTreeWidget *const w, const QModelIndex &index);

    // Set all shortcuts to their default values (bindings).
    void allDefault();

    // clear all shortcuts
    void clearConfiguration();

    // Import shortcuts from file
    void importConfiguration(KConfigBase *config);


    //conflict resolution functions
    void changeKeyShortcut(KisShortcutsEditorItem *item, uint column, const QKeySequence &capture);

// private slots
    //this invokes the appropriate conflict resolution function
    void capturedShortcut(const QVariant &, const QModelIndex &);

    //! Represents the three hierarchies the dialog handles.
    enum hierarchyLevel {Root = 0, Program, Action};

    /**
     * Add @a action at @a level. Checks for QActions and unnamed actions
     * before adding.
     *
     * @return true if the actions was really added, false if not
     */
    bool addAction(QAction *action, QTreeWidgetItem *hier[], hierarchyLevel level);

    void printShortcuts() const;

    void setActionTypes(KisShortcutsEditor::ActionTypes actionTypes);

// members
    QList<KActionCollection *> actionCollections;
    KisShortcutsEditor *q;

    Ui::KisShortcutsDialog ui;

    KisShortcutsEditor::ActionTypes actionTypes;
    KisShortcutsEditorDelegate *delegate;

};

Q_DECLARE_METATYPE(KisShortcutsEditorItem *)

#endif /* KISSHORTCUTSDIALOG_P_H */
