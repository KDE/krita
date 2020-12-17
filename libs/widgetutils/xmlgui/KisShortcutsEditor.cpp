/* This file is part of the KDE libraries SPDX-FileCopyrightText: 1998 Mark Donohoe <donohoe@kde.org>
    SPDX-FileCopyrightText: 1997 Nicolas Hadacek <hadacek@kde.org>
    SPDX-FileCopyrightText: 1998 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 2001 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KisShortcutsEditor.h"
#include "KisShortcutsEditor_p.h"
#include "kshortcutschemeshelper_p.h"
#include "config-xmlgui.h"
#include "kis_action_registry.h"

// The following is needed for KisShortcutsEditorPrivate and QTreeWidgetHack
// #include "KisShortcutsDialog_p.h"

#include <QAction>
#include <QList>
#include <QObject>
#include <QTimer>
#include <QTextDocument>
#include <QTextTable>
#include <QTextCursor>
#include <QTextTableFormat>
#include <QPrinter>
#include <QDebug>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kmessagebox.h>
#include "kactioncollection.h"
#include "kactioncategory.h"
#include <ktreewidgetsearchline.h>

//---------------------------------------------------------------------
// KisShortcutsEditor
//---------------------------------------------------------------------

Q_DECLARE_METATYPE(KisShortcutsEditorItem *)


KisShortcutsEditor::KisShortcutsEditor(QWidget *parent, ActionTypes actionType, LetterShortcuts allowLetterShortcuts)
    : QWidget(parent)
    , d(new KisShortcutsEditorPrivate(this))
{
    d->initGUI(actionType, allowLetterShortcuts);
}

KisShortcutsEditor::~KisShortcutsEditor()
{
    delete d;
}

bool KisShortcutsEditor::isModified() const
{
    // Iterate over all items
    QTreeWidgetItemIterator it(d->ui.list, QTreeWidgetItemIterator::NoChildren);

    for (; (*it); ++it) {
        KisShortcutsEditorItem *item = dynamic_cast<KisShortcutsEditorItem *>(*it);
        if (item && item->isModified()) {
            return true;
        }
    }
    return false;
}

void KisShortcutsEditor::clearCollections()
{
    d->delegate->contractAll();
    d->ui.list->clear();
    d->actionCollections.clear();
    QTimer::singleShot(0, this, SLOT(resizeColumns()));
}

void KisShortcutsEditor::clearSearch()
{
    d->ui.searchFilter->searchLine()->clear();
}


void KisShortcutsEditor::addCollection(KActionCollection *collection, const QString &title)
{
    // KXmlGui add action collections unconditionally. If some plugin doesn't
    // provide actions we don't want to create empty subgroups.
    if (collection->isEmpty()) {
        return;
    }

    // Pause updating.
    setUpdatesEnabled(false);


    /**
     * Forward this actioncollection to the delegate which will do conflict checking.
     * This _replaces_ existing collections in the delegate.
     */
    d->actionCollections.append(collection);
    d->delegate->setCheckActionCollections(d->actionCollections);


    // Determine how we should label this collection in the widget.
    QString collectionTitle;
    if (!title.isEmpty()) {
      collectionTitle = title;
    } else {
      // Use the programName (Translated).
      collectionTitle = collection->componentDisplayName();
    }

    // Create the collection root node.
    QTreeWidgetItem *hierarchy[3];
    hierarchy[KisShortcutsEditorPrivate::Root] = d->ui.list->invisibleRootItem();
    hierarchy[KisShortcutsEditorPrivate::Program] =
      d->findOrMakeItem(hierarchy[KisShortcutsEditorPrivate::Root], collectionTitle);
    hierarchy[KisShortcutsEditorPrivate::Action] = 0;

    // Remember which actions we have seen. We will be adding categorized
    // actions first, so this will help us keep track of which actions haven't
    // been categorized yet, so we can add them as uncategorized at the end.
    QSet<QAction *> actionsSeen;

    // Add a subtree for each category? Perhaps easier to think that this
    // doesn't exist. Basically you add KActionCategory as a QObject child of
    // KActionCollection, and then tag objects as belonging to the category.
    foreach (KActionCategory *category, collection->categories()) {

        // Don't display empty categories.
        if (category->actions().isEmpty()) {
            continue;
        }

        hierarchy[KisShortcutsEditorPrivate::Action] =
          d->findOrMakeItem(hierarchy[KisShortcutsEditorPrivate::Program], category->text());

        // Add every item from the category.
        foreach (QAction *action, category->actions()) {
            actionsSeen.insert(action);
            d->addAction(action, hierarchy, KisShortcutsEditorPrivate::Action);
        }

        // Fold in each KActionCategory by default.
        hierarchy[KisShortcutsEditorPrivate::Action]->setExpanded(false);

    }

    // Finally, tack on any uncategorized actions.
    foreach (QAction *action, collection->actions()) {
        if (!actionsSeen.contains(action)) {
            d->addAction(action, hierarchy, KisShortcutsEditorPrivate::Program);
        }
    }

    // sort the list
    d->ui.list->sortItems(Name, Qt::AscendingOrder);

    // Now turn on updating again.
    setUpdatesEnabled(true);

    QTimer::singleShot(0, this, SLOT(resizeColumns()));
}

void KisShortcutsEditor::clearConfiguration()
{
    d->clearConfiguration();
}

void KisShortcutsEditor::importConfiguration(KConfigBase *config, bool isScheme)
{
    Q_ASSERT(config);
    if (!config) {
        return;
    }

    // If this is a shortcut scheme, apply it
    if (isScheme) {
        KisActionRegistry::instance()->applyShortcutScheme(config);
    }

    // Update the dialog entry items
    const KConfigGroup schemeShortcuts(config, QStringLiteral("Shortcuts"));
    for (QTreeWidgetItemIterator it(d->ui.list); (*it); ++it) {

        if (!(*it)->parent()) {
            continue;
        }
        KisShortcutsEditorItem *item = static_cast<KisShortcutsEditorItem *>(*it);
        const QString actionId = item->data(Id).toString();
        if (!schemeShortcuts.hasKey(actionId))
            continue;

        QList<QKeySequence> sc = QKeySequence::listFromString(schemeShortcuts.readEntry(actionId, QString()));
        d->changeKeyShortcut(item, LocalPrimary, primarySequence(sc));
        d->changeKeyShortcut(item, LocalAlternate, alternateSequence(sc));
    }
}

void KisShortcutsEditor::exportConfiguration(KConfigBase *config) const
{
    Q_ASSERT(config);
    if (!config) {
        return;
    }

    if (d->actionTypes) {
        KConfigGroup group(config,QStringLiteral("Shortcuts"));
        foreach (KActionCollection *collection, d->actionCollections) {
            collection->writeSettings(&group, true);
        }
    }

    KisActionRegistry::instance()->notifySettingsUpdated();
}

void KisShortcutsEditor::saveShortcuts(KConfigGroup *config) const
{
    // This is a horrible mess with pointers...
    KConfigGroup cg;
    if (config == 0) {
        cg = KConfigGroup(KSharedConfig::openConfig("kritashortcutsrc"),
                          QStringLiteral("Shortcuts"));
        config = &cg;
    }

    // Clear and reset temporary shortcuts
    config->deleteGroup();
    foreach (KActionCollection *collection, d->actionCollections) {
        collection->writeSettings(config, false);
    }

    KisActionRegistry::instance()->notifySettingsUpdated();
}

//slot
void KisShortcutsEditor::resizeColumns()
{
    for (int i = 0; i < d->ui.list->columnCount(); i++) {
        d->ui.list->resizeColumnToContents(i);
    }
}




void KisShortcutsEditor::commit()
{
    for (QTreeWidgetItemIterator it(d->ui.list); (*it); ++it) {
        if (KisShortcutsEditorItem *item = dynamic_cast<KisShortcutsEditorItem *>(*it)) {
            item->commit();
        }
    }
}

void KisShortcutsEditor::save()
{
    saveShortcuts();
    commit(); // Not doing this would be bad
}

void KisShortcutsEditor::undo()
{
    // TODO: is this working?
    for (QTreeWidgetItemIterator it(d->ui.list); (*it); ++it) {
        if (KisShortcutsEditorItem *item = dynamic_cast<KisShortcutsEditorItem *>(*it)) {
            item->undo();
        }
    }
}

void KisShortcutsEditor::allDefault()
{
    d->allDefault();
}

void KisShortcutsEditor::printShortcuts() const
{
    d->printShortcuts();
}

void KisShortcutsEditor::searchUpdated(QString s)
{
    if (s.isEmpty()) {
        // Reset the tree area
        d->ui.list->collapseAll();
        d->ui.list->expandToDepth(0);
    } else {
        d->ui.list->expandAll();
    }
}

KisShortcutsEditor::ActionTypes KisShortcutsEditor::actionTypes() const
{
    return d->actionTypes;
}

void KisShortcutsEditor::setActionTypes(ActionTypes actionTypes)
{
    d->setActionTypes(actionTypes);
}


#include "moc_KisShortcutsEditor.cpp"
