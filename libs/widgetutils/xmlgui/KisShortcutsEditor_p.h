/**
 *  SPDX-FileCopyrightText: 1997 Nicolas Hadacek <hadacek@kde.org>
 *  SPDX-FileCopyrightText: 1998 Matthias Ettrich <ettrich@kde.org>
 *  SPDX-FileCopyrightText: 2001 Ellis Whitehead <ellis@kde.org>
 *  SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
 *  SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
 *  SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>
 *  SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */


#ifndef KISSHORTCUTSEDITOR_P
#define KISSHORTCUTSEDITOR_P

#include "KisShortcutsEditor.h"
#include <QTreeWidget>
#include <QScroller>
#include <ktreewidgetsearchline.h>

#include "KisShortcutsDialog_p.h"

// NEEDED FOR KisShortcutsEditorPrivate
#include "ui_KisShortcutsDialog.h"




/// Declared at the end of the file - provides the basic storage unit for this class
class KisShortcutsEditorItem;

/**
 * @internal
 */

class KisShortcutsEditorPrivate
{
public:

  //! Represents the three hierarchies the dialog displays.
  enum hierarchyLevel {Root = 0,  /* Base level node (Tools, Krita...) */
                       Program,   /* We use this like "Path Tool, Default Tool," */
                       Action};   /* Individual actions */



    KisShortcutsEditorPrivate(KisShortcutsEditor *q);

    void initGUI(KisShortcutsEditor::ActionTypes actionTypes, KisShortcutsEditor::LetterShortcuts allowLetterShortcuts);
    void appendToView(uint nList, const QString &title = QString());
    //used in appendToView
    QTreeWidgetItem *findOrMakeItem(QTreeWidgetItem *parent, const QString &name);

    // Set all shortcuts to their default values (bindings).
    void allDefault();

    // clear all shortcuts
    void clearConfiguration();

    //changeXXX were described as "conflict resolution functions"
    void changeKeyShortcut(KisShortcutsEditorItem *item, uint column, const QKeySequence &capture);

    //this invokes the appropriate conflict resolution function
    void capturedShortcut(const QVariant &, const QModelIndex &);

    /**
     * Add @p action at hierarchy level @p level.
     *
     * Filters out QActions (TODO: hmm) and unnamed actions before adding.
     *
     * @return true if the actions was successfully added
     */
    bool addAction(QAction *action, QTreeWidgetItem *hier[], hierarchyLevel level);

    void printShortcuts() const;

    // TODO: Is this necessary w/o global actions?
    void setActionTypes(KisShortcutsEditor::ActionTypes actionTypes);

public:

    // Members
    QList<KActionCollection *> actionCollections;
    KisShortcutsEditor *q;

    Ui::KisShortcutsDialog ui;

    KisShortcutsEditor::ActionTypes actionTypes;
    KisShortcutsEditorDelegate *delegate;
};



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
 * A QTreeWidgetItem that can handle QActions. It also provides undo functionality.
 *
 * Call commit() to save pending changes.
 *
 * @internal
 */
class KisShortcutsEditorItem : public QTreeWidgetItem
{
public:

    KisShortcutsEditorItem(QTreeWidgetItem *parent, QAction *action);

    //! Destructor will erase unsaved changes.
    ~KisShortcutsEditorItem() override;

    //! Undo the changes since the last commit.
    void undo();

    //! Commit the changes.
    void commit();

    QVariant data(int column, int role = Qt::DisplayRole) const override;
    bool operator<(const QTreeWidgetItem &other) const override;

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
    bool m_isNameBold{false};

    //@{
    //! The original shortcuts before user changes. 0 means no change.
    QList<QKeySequence> *m_oldLocalShortcut{0};
    //@}

    //! The localized action name
    QString m_actionNameInTable;

    //! The action id. Needed for exporting and importing
    QString m_id;

    //! The collator, for sorting
    QCollator m_collator;
};



#endif // KISSHORTCUTSEDITOR_P
