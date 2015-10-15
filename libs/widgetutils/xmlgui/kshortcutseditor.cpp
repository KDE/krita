/* This file is part of the KDE libraries Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2007 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

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

#include "kshortcutseditor.h"
#include "config-xmlgui.h"
// The following is needed for KShortcutsEditorPrivate and QTreeWidgetHack
#include "kshortcutsdialog_p.h"

#include <QAction>
#include <QHeaderView>
#include <QList>
#include <QObject>
#include <QTimer>
#include <QTextDocument>
#include <QTextTable>
#include <QTextCursor>
#include <QTextTableFormat>
#include <QPrinter>
#include <QPrintDialog>
#include <QDebug>

#include <kconfig.h>
#include <kconfiggroup.h>
#ifdef HAVE_GLOBALACCEL
#include <kglobalaccel.h>
#endif
#include <kmessagebox.h>
#include "kactioncollection.h"
#include "kactioncategory.h"
#include <ktreewidgetsearchline.h>

//---------------------------------------------------------------------
// KShortcutsEditor
//---------------------------------------------------------------------

KShortcutsEditor::KShortcutsEditor(KActionCollection *collection, QWidget *parent, ActionTypes actionType,
                                   LetterShortcuts allowLetterShortcuts)
    : QWidget(parent)
    , d(new KShortcutsEditorPrivate(this))
{
    d->initGUI(actionType, allowLetterShortcuts);
    addCollection(collection);
}

KShortcutsEditor::KShortcutsEditor(QWidget *parent, ActionTypes actionType, LetterShortcuts allowLetterShortcuts)
    : QWidget(parent)
    , d(new KShortcutsEditorPrivate(this))
{
    d->initGUI(actionType, allowLetterShortcuts);
}

KShortcutsEditor::~KShortcutsEditor()
{
    delete d;
}

bool KShortcutsEditor::isModified() const
{
    // Iterate over all items
    QTreeWidgetItemIterator it(d->ui.list, QTreeWidgetItemIterator::NoChildren);

    for (; (*it); ++it) {
        KShortcutsEditorItem *item = dynamic_cast<KShortcutsEditorItem *>(*it);
        if (item && item->isModified()) {
            return true;
        }
    }
    return false;
}

void KShortcutsEditor::clearCollections()
{
    d->delegate->contractAll();
    d->ui.list->clear();
    d->actionCollections.clear();
    QTimer::singleShot(0, this, SLOT(resizeColumns()));
}

void KShortcutsEditor::addCollection(KActionCollection *collection, const QString &title)
{
    // KXmlGui add action collections unconditionally. If some plugin doesn't
    // provide actions we don't want to create empty subgroups.
    if (collection->isEmpty()) {
        return;
    }

    // We add a bunch of items. Prevent the treewidget from permanently
    // updating.
    setUpdatesEnabled(false);

    d->actionCollections.append(collection);
    // Forward our actionCollections to the delegate which does the conflict
    // checking.
    d->delegate->setCheckActionCollections(d->actionCollections);
    QString displayTitle = title;
    if (displayTitle.isEmpty()) {
        // Use the programName (Translated).
        displayTitle = collection->componentDisplayName();
    }

    QTreeWidgetItem *hier[3];
    hier[KShortcutsEditorPrivate::Root] = d->ui.list->invisibleRootItem();
    hier[KShortcutsEditorPrivate::Program] = d->findOrMakeItem(hier[KShortcutsEditorPrivate::Root], displayTitle);
    hier[KShortcutsEditorPrivate::Action] = NULL;

    // Set to remember which actions we have seen.
    QSet<QAction *> actionsSeen;

    // Add all categories in their own subtree below the collections root node
    QList<KActionCategory *> categories = collection->findChildren<KActionCategory *>();
    foreach (KActionCategory *category, categories) {
        hier[KShortcutsEditorPrivate::Action] = d->findOrMakeItem(hier[KShortcutsEditorPrivate::Program], category->text());
        foreach (QAction *action, category->actions()) {
            // Set a marker that we have seen this action
            actionsSeen.insert(action);
            d->addAction(action, hier, KShortcutsEditorPrivate::Action);
        }
    }

    // The rest of the shortcuts is added as a direct shild of the action
    // collections root node
    foreach (QAction *action, collection->actions()) {
        if (actionsSeen.contains(action)) {
            continue;
        }

        d->addAction(action, hier, KShortcutsEditorPrivate::Program);
    }

    // sort the list
    d->ui.list->sortItems(Name, Qt::AscendingOrder);

    // reenable updating
    setUpdatesEnabled(true);

    QTimer::singleShot(0, this, SLOT(resizeColumns()));
}

void KShortcutsEditor::clearConfiguration()
{
    d->clearConfiguration();
}

void KShortcutsEditor::importConfiguration(KConfigBase *config)
{
    d->importConfiguration(config);
}

void KShortcutsEditor::exportConfiguration(KConfigBase *config) const
{
    Q_ASSERT(config);
    if (!config) {
        return;
    }

    if (d->actionTypes & KShortcutsEditor::GlobalAction) {
        QString groupName(QStringLiteral("Global Shortcuts"));
        KConfigGroup group(config, groupName);
        foreach (KActionCollection *collection, d->actionCollections) {
            collection->exportGlobalShortcuts(&group, true);
        }
    }
    if (d->actionTypes & ~KShortcutsEditor::GlobalAction) {
        QString groupName(QStringLiteral("Shortcuts"));
        KConfigGroup group(config, groupName);
        foreach (KActionCollection *collection, d->actionCollections) {
            collection->writeSettings(&group, true);
        }
    }
}

void KShortcutsEditor::writeConfiguration(KConfigGroup *config) const
{
    foreach (KActionCollection *collection, d->actionCollections) {
        collection->writeSettings(config);
    }
}

//slot
void KShortcutsEditor::resizeColumns()
{
    for (int i = 0; i < d->ui.list->columnCount(); i++) {
        d->ui.list->resizeColumnToContents(i);
    }
}

void KShortcutsEditor::commit()
{
    for (QTreeWidgetItemIterator it(d->ui.list); (*it); ++it) {
        if (KShortcutsEditorItem *item = dynamic_cast<KShortcutsEditorItem *>(*it)) {
            item->commit();
        }
    }
}

void KShortcutsEditor::save()
{
    writeConfiguration();
    // we have to call commit. If we wouldn't do that the changes would be
    // undone on deletion! That would lead to weird problems. Changes to
    // Global Shortcuts would vanish completely. Changes to local shortcuts
    // would vanish for this session.
    commit();
}

// KDE5 : rename to undo()
void KShortcutsEditor::undoChanges()
{
    //This function used to crash sometimes when invoked by clicking on "cancel"
    //with Qt 4.2.something. Apparently items were deleted too early by Qt.
    //It seems to work with 4.3-ish Qt versions. Keep an eye on this.
    for (QTreeWidgetItemIterator it(d->ui.list); (*it); ++it) {
        if (KShortcutsEditorItem *item = dynamic_cast<KShortcutsEditorItem *>(*it)) {
            item->undo();
        }
    }
}

//We ask the user here if there are any conflicts, as opposed to undoChanges().
//They don't do the same thing anyway, this just not to confuse any readers.
//slot
void KShortcutsEditor::allDefault()
{
    d->allDefault();
}

void KShortcutsEditor::printShortcuts() const
{
    d->printShortcuts();
}

KShortcutsEditor::ActionTypes KShortcutsEditor::actionTypes() const
{
    return d->actionTypes;
}

void KShortcutsEditor::setActionTypes(ActionTypes actionTypes)
{
    d->setActionTypes(actionTypes);
}

//---------------------------------------------------------------------
// KShortcutsEditorPrivate
//---------------------------------------------------------------------

KShortcutsEditorPrivate::KShortcutsEditorPrivate(KShortcutsEditor *q)
    :   q(q),
        delegate(0)
{}

void KShortcutsEditorPrivate::initGUI(KShortcutsEditor::ActionTypes types, KShortcutsEditor::LetterShortcuts allowLetterShortcuts)
{
    actionTypes = types;

    ui.setupUi(q);
    q->layout()->setMargin(0);
    ui.searchFilter->searchLine()->setTreeWidget(ui.list); // Plug into search line
    ui.list->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui.list->header()->hideSection(ShapeGesture);  //mouse gestures didn't make it in time...
    ui.list->header()->hideSection(RockerGesture);
    if (!(actionTypes & KShortcutsEditor::GlobalAction)) {
        ui.list->header()->hideSection(GlobalPrimary);
        ui.list->header()->hideSection(GlobalAlternate);
    } else if (!(actionTypes & ~KShortcutsEditor::GlobalAction)) {
        ui.list->header()->hideSection(LocalPrimary);
        ui.list->header()->hideSection(LocalAlternate);
    }

    // Create the Delegate. It is responsible for the KKeySeqeunceWidgets that
    // really change the shortcuts.
    delegate = new KShortcutsEditorDelegate(
        ui.list,
        allowLetterShortcuts == KShortcutsEditor::LetterShortcutsAllowed);

    ui.list->setItemDelegate(delegate);
    ui.list->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui.list->setSelectionMode(QAbstractItemView::SingleSelection);
    //we have our own editing mechanism
    ui.list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.list->setAlternatingRowColors(true);

    //TODO listen to changes to global shortcuts
    QObject::connect(delegate, SIGNAL(shortcutChanged(QVariant,QModelIndex)),
                     q, SLOT(capturedShortcut(QVariant,QModelIndex)));
    //hide the editor widget chen its item becomes hidden
    QObject::connect(ui.searchFilter->searchLine(), SIGNAL(hiddenChanged(QTreeWidgetItem*,bool)),
                     delegate, SLOT(hiddenBySearchLine(QTreeWidgetItem*,bool)));

    ui.searchFilter->setFocus();
}

void KShortcutsEditorPrivate::setActionTypes(KShortcutsEditor::ActionTypes types)
{
    if (actionTypes == types) {
        return;
    }
    actionTypes = types;

    // show/hide the sections based on new selection
    QHeaderView *header = ui.list->header();
    if (actionTypes & KShortcutsEditor::GlobalAction) {
        header->showSection(GlobalPrimary);
        header->showSection(GlobalAlternate);
    } else {
        header->hideSection(GlobalPrimary);
        header->hideSection(GlobalAlternate);
    }
    if (actionTypes & ~KShortcutsEditor::GlobalAction) {
        header->showSection(LocalPrimary);
        header->showSection(LocalAlternate);
    } else {
        header->hideSection(LocalPrimary);
        header->hideSection(LocalAlternate);
    }
}

bool KShortcutsEditorPrivate::addAction(QAction *action, QTreeWidgetItem *hier[], hierarchyLevel level)
{
    // If the action name starts with unnamed- spit out a warning and ignore
    // it. That name will change at will and will break loading and writing
    QString actionName = action->objectName();
    if (actionName.isEmpty() || actionName.startsWith(QStringLiteral("unnamed-"))) {
        qCritical() << "Skipping action without name " << action->text() << "," << actionName << "!";
        return false;
    }

    // This code doesn't allow editing of QAction. It can not distinguish
    // between default and active shortcuts. This breaks many assumptions the
    // editor makes.
    const QVariant value = action->property("isShortcutConfigurable");
    if (!value.isValid() || value.toBool()) {
        new KShortcutsEditorItem((hier[level]), action);
        return true;
    }

    return false;
}

void KShortcutsEditorPrivate::allDefault()
{
    for (QTreeWidgetItemIterator it(ui.list); (*it); ++it) {
        if (!(*it)->parent() || (*it)->type() != ActionItem) {
            continue;
        }

        KShortcutsEditorItem *item = static_cast<KShortcutsEditorItem *>(*it);
        QAction *act = item->m_action;

        QList<QKeySequence> defaultShortcuts = act->property("defaultShortcuts").value<QList<QKeySequence> >();
        if (act->shortcuts() != defaultShortcuts) {
            QKeySequence primary = defaultShortcuts.isEmpty() ? QKeySequence() : defaultShortcuts.at(0);
            QKeySequence alternate = defaultShortcuts.size() <= 1 ? QKeySequence() : defaultShortcuts.at(1);
            changeKeyShortcut(item, LocalPrimary, primary);
            changeKeyShortcut(item, LocalAlternate, alternate);
        }
#ifdef HAVE_GLOBALACCEL
        if (KGlobalAccel::self()->shortcut(act) != KGlobalAccel::self()->defaultShortcut(act)) {
            QList<QKeySequence> defaultShortcut = KGlobalAccel::self()->defaultShortcut(act);
            changeKeyShortcut(item, GlobalPrimary, primarySequence(defaultShortcut));
            changeKeyShortcut(item, GlobalAlternate, alternateSequence(defaultShortcut));
        }
#endif
#if 0
        KShapeGesture actShapeGesture = KGestureMap::self()->shapeGesture(act);
        KShapeGesture actDefaultShapeGesture = KGestureMap::self()->defaultShapeGesture(act);
        if (actShapeGesture != actDefaultShapeGesture) {
            changeShapeGesture(item, actDefaultShapeGesture);
        }

        KRockerGesture actRockerGesture = KGestureMap::self()->rockerGesture(act);
        KRockerGesture actDefaultRockerGesture = KGestureMap::self()->defaultRockerGesture(act);
        if (actRockerGesture != actDefaultRockerGesture) {
            changeRockerGesture(item, actDefaultRockerGesture);
        }
#endif
    }
}

//static
KShortcutsEditorItem *KShortcutsEditorPrivate::itemFromIndex(QTreeWidget *const w,
        const QModelIndex &index)
{
    QTreeWidgetItem *item = static_cast<QTreeWidgetHack *>(w)->itemFromIndex(index);
    if (item && item->type() == ActionItem) {
        return static_cast<KShortcutsEditorItem *>(item);
    }
    return 0;
}

QTreeWidgetItem *KShortcutsEditorPrivate::findOrMakeItem(QTreeWidgetItem *parent, const QString &name)
{
    for (int i = 0; i < parent->childCount(); i++) {
        QTreeWidgetItem *child = parent->child(i);
        if (child->text(0) == name) {
            return child;
        }
    }
    QTreeWidgetItem *ret = new QTreeWidgetItem(parent, NonActionItem);
    ret->setText(0, name);
    ui.list->expandItem(ret);
    ret->setFlags(ret->flags() & ~Qt::ItemIsSelectable);
    return ret;
}

//private slot
void KShortcutsEditorPrivate::capturedShortcut(const QVariant &newShortcut, const QModelIndex &index)
{
    //dispatch to the right handler
    if (!index.isValid()) {
        return;
    }
    int column = index.column();
    KShortcutsEditorItem *item = itemFromIndex(ui.list, index);
    Q_ASSERT(item);

    if (column >= LocalPrimary && column <= GlobalAlternate) {
        changeKeyShortcut(item, column, newShortcut.value<QKeySequence>());
    }
#if 0
    else if (column == ShapeGesture) {
        changeShapeGesture(item, newShortcut.value<KShapeGesture>());
    } else if (column == RockerGesture) {
        changeRockerGesture(item, newShortcut.value<KRockerGesture>());
    }
#endif
}

void KShortcutsEditorPrivate::changeKeyShortcut(KShortcutsEditorItem *item, uint column, const QKeySequence &capture)
{
    // The keySequence we get is cleared by KKeySequenceWidget. No conflicts.
    if (capture == item->keySequence(column)) {
        return;
    }

    item->setKeySequence(column, capture);
    q->keyChange();
    //force view update
    item->setText(column, capture.toString(QKeySequence::NativeText));
}

#if 0
void KShortcutsEditorPrivate::changeShapeGesture(KShortcutsEditorItem *item, const KShapeGesture &capture)
{
    if (capture == KGestureMap::self()->shapeGesture(item->m_action)) {
        return;
    }

    if (capture.isValid()) {
        bool conflict = false;
        KShortcutsEditorItem *otherItem;

        //search for conflicts
        for (QTreeWidgetItemIterator it(ui.list); (*it); ++it) {
            if (!(*it)->parent() || (*it == item)) {
                continue;
            }

            otherItem = static_cast<KShortcutsEditorItem *>(*it);

            //comparisons are possibly expensive
            KShapeGesture otherGesture = KGestureMap::self()->shapeGesture(otherItem->m_action);
            if (!otherGesture.isValid()) {
                continue;
            }

            if (capture == otherGesture) {
                conflict = true;
                break;
            }
        }

        if (conflict && !stealShapeGesture(otherItem, capture)) {
            return;
        }
    }

    item->setShapeGesture(capture);
}
#endif

#if 0
void KShortcutsEditorPrivate::changeRockerGesture(KShortcutsEditorItem *item, const KRockerGesture &capture)
{
    if (capture == KGestureMap::self()->rockerGesture(item->m_action)) {
        return;
    }

    if (capture.isValid()) {
        bool conflict = false;
        KShortcutsEditorItem *otherItem;

        for (QTreeWidgetItemIterator it(ui.list); (*it); ++it) {
            if (!(*it)->parent() || (*it == item)) {
                continue;
            }

            otherItem = static_cast<KShortcutsEditorItem *>(*it);

            KRockerGesture otherGesture = KGestureMap::self()->rockerGesture(otherItem->m_action);
            if (capture == otherGesture) {
                conflict = true;
                break;
            }
        }

        if (conflict && !stealRockerGesture(otherItem, capture)) {
            return;
        }
    }

    item->setRockerGesture(capture);
}
#endif

void KShortcutsEditorPrivate::clearConfiguration()
{
    for (QTreeWidgetItemIterator it(ui.list); (*it); ++it) {
        if (!(*it)->parent()) {
            continue;
        }

        KShortcutsEditorItem *item = static_cast<KShortcutsEditorItem *>(*it);

        changeKeyShortcut(item, LocalPrimary,   QKeySequence());
        changeKeyShortcut(item, LocalAlternate, QKeySequence());

        changeKeyShortcut(item, GlobalPrimary,   QKeySequence());
        changeKeyShortcut(item, GlobalAlternate, QKeySequence());

#if 0
        changeShapeGesture(item, KShapeGesture());
#endif
    }
}

void KShortcutsEditorPrivate::importConfiguration(KConfigBase *config)
{
    Q_ASSERT(config);
    if (!config) {
        return;
    }

    KConfigGroup globalShortcutsGroup(config, QStringLiteral("Global Shortcuts"));
    if ((actionTypes & KShortcutsEditor::GlobalAction) && globalShortcutsGroup.exists()) {

        for (QTreeWidgetItemIterator it(ui.list); (*it); ++it) {

            if (!(*it)->parent()) {
                continue;
            }

            KShortcutsEditorItem *item = static_cast<KShortcutsEditorItem *>(*it);
            const QString actionId = item->data(Id).toString();
            if (!globalShortcutsGroup.hasKey(actionId))
                continue;

            QList<QKeySequence> sc = QKeySequence::listFromString(globalShortcutsGroup.readEntry(actionId, QString()));
            changeKeyShortcut(item, GlobalPrimary, primarySequence(sc));
            changeKeyShortcut(item, GlobalAlternate, alternateSequence(sc));
        }
    }

    if (actionTypes & ~KShortcutsEditor::GlobalAction) {
        const KConfigGroup localShortcutsGroup(config, QStringLiteral("Shortcuts"));
        for (QTreeWidgetItemIterator it(ui.list); (*it); ++it) {

            if (!(*it)->parent()) {
                continue;
            }
            KShortcutsEditorItem *item = static_cast<KShortcutsEditorItem *>(*it);
            const QString actionId = item->data(Id).toString();
            if (!localShortcutsGroup.hasKey(actionId))
                continue;

            QList<QKeySequence> sc = QKeySequence::listFromString(localShortcutsGroup.readEntry(actionId, QString()));
            changeKeyShortcut(item, LocalPrimary, primarySequence(sc));
            changeKeyShortcut(item, LocalAlternate, alternateSequence(sc));
        }
    }
}

#if 0
bool KShortcutsEditorPrivate::stealShapeGesture(KShortcutsEditorItem *item, const KShapeGesture &gst)
{
    QString title = i18n("Key Conflict");
    QString message = i18n("The '%1' shape gesture has already been allocated to the \"%2\" action.\n"
                           "Do you want to reassign it from that action to the current one?",
                           gst.shapeName(), item->m_action->text());

    if (KMessageBox::warningContinueCancel(q, message, title, KGuiItem(i18n("Reassign")))
            != KMessageBox::Continue) {
        return false;
    }
    item->setShapeGesture(KShapeGesture());

    return true;
}
#endif

#if 0
bool KShortcutsEditorPrivate::stealRockerGesture(KShortcutsEditorItem *item, const KRockerGesture &gst)
{
    QString title = i18n("Key Conflict");
    QString message = i18n("The '%1' rocker gesture has already been allocated to the \"%2\" action.\n"
                           "Do you want to reassign it from that action to the current one?",
                           gst.rockerName(), item->m_action->text());

    if (KMessageBox::warningContinueCancel(q, message, title, KGuiItem(i18n("Reassign")))
            != KMessageBox::Continue) {
        return false;
    }

    item->setRockerGesture(KRockerGesture());

    return true;
}
#endif

/*TODO for the printShortcuts function
Nice to have features (which I'm not sure I can do before may due to
more important things):

- adjust the general page borders, IMHO they're too wide

- add a custom printer options page that allows to filter out all
  actions that don't have a shortcut set to reduce this list. IMHO this
  should be optional as people might want to simply print all and  when
  they find a new action that they assign a shortcut they can simply use
  a pen to fill out the empty space

- find a way to align the Main/Alternate/Global entries in the shortcuts
  column without adding borders. I first did this without a nested table
  but instead simply added 3 rows and merged the 3 cells in the Action
  name and description column, but unfortunately I didn't find a way to
  remove the borders between the 6 shortcut cells.
*/
void KShortcutsEditorPrivate::printShortcuts() const
{
// One cant print on wince
#ifndef _WIN32_WCE
    QTreeWidgetItem *root = ui.list->invisibleRootItem();
    QTextDocument doc;

    doc.setDefaultFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));

    QTextCursor cursor(&doc);
    cursor.beginEditBlock();
    QTextCharFormat headerFormat;
    headerFormat.setProperty(QTextFormat::FontSizeAdjustment, 3);
    headerFormat.setFontWeight(QFont::Bold);
    cursor.insertText(i18nc("header for an applications shortcut list", "Shortcuts for %1",
                            QGuiApplication::applicationDisplayName()),
                      headerFormat);
    QTextCharFormat componentFormat;
    componentFormat.setProperty(QTextFormat::FontSizeAdjustment, 2);
    componentFormat.setFontWeight(QFont::Bold);
    QTextBlockFormat componentBlockFormat = cursor.blockFormat();
    componentBlockFormat.setTopMargin(16);
    componentBlockFormat.setBottomMargin(16);

    QTextTableFormat tableformat;
    tableformat.setHeaderRowCount(1);
    tableformat.setCellPadding(4.0);
    tableformat.setCellSpacing(0);
    tableformat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tableformat.setBorder(0.5);

    QList<QPair<QString, ColumnDesignation> > shortcutTitleToColumn;
    shortcutTitleToColumn << qMakePair(i18n("Main:"), LocalPrimary);
    shortcutTitleToColumn << qMakePair(i18n("Alternate:"), LocalAlternate);
    shortcutTitleToColumn << qMakePair(i18n("Global:"), GlobalPrimary);
    shortcutTitleToColumn << qMakePair(i18n("Global alternate:"), GlobalAlternate);

    for (int i = 0; i < root->childCount(); i++) {
        QTreeWidgetItem *item = root->child(i);
        cursor.insertBlock(componentBlockFormat, componentFormat);
        cursor.insertText(item->text(0));

        QTextTable *table = cursor.insertTable(1, 3);
        table->setFormat(tableformat);
        int currow = 0;

        QTextTableCell cell = table->cellAt(currow, 0);
        QTextCharFormat format = cell.format();
        format.setFontWeight(QFont::Bold);
        cell.setFormat(format);
        cell.firstCursorPosition().insertText(i18n("Action Name"));

        cell = table->cellAt(currow, 1);
        cell.setFormat(format);
        cell.firstCursorPosition().insertText(i18n("Shortcuts"));

        cell = table->cellAt(currow, 2);
        cell.setFormat(format);
        cell.firstCursorPosition().insertText(i18n("Description"));
        currow++;

        for (QTreeWidgetItemIterator it(item); *it; ++it) {
            if ((*it)->type() != ActionItem) {
                continue;
            }

            KShortcutsEditorItem *editoritem = static_cast<KShortcutsEditorItem *>(*it);
            table->insertRows(table->rows(), 1);
            QVariant data = editoritem->data(Name, Qt::DisplayRole);
            table->cellAt(currow, 0).firstCursorPosition().insertText(data.toString());

            QTextTable *shortcutTable = 0;
            for (int k = 0; k < shortcutTitleToColumn.count(); k++) {
                data = editoritem->data(shortcutTitleToColumn.at(k).second, Qt::DisplayRole);
                QString key = data.value<QKeySequence>().toString();

                if (!key.isEmpty()) {
                    if (!shortcutTable) {
                        shortcutTable = table->cellAt(currow, 1).firstCursorPosition().insertTable(1, 2);
                        QTextTableFormat shortcutTableFormat = tableformat;
                        shortcutTableFormat.setCellSpacing(0.0);
                        shortcutTableFormat.setHeaderRowCount(0);
                        shortcutTableFormat.setBorder(0.0);
                        shortcutTable->setFormat(shortcutTableFormat);
                    } else {
                        shortcutTable->insertRows(shortcutTable->rows(), 1);
                    }
                    shortcutTable->cellAt(shortcutTable->rows() - 1, 0).firstCursorPosition().insertText(shortcutTitleToColumn.at(k).first);
                    shortcutTable->cellAt(shortcutTable->rows() - 1, 1).firstCursorPosition().insertText(key);
                }
            }

            QAction *action = editoritem->m_action;
            cell = table->cellAt(currow, 2);
            format = cell.format();
            format.setProperty(QTextFormat::FontSizeAdjustment, -1);
            cell.setFormat(format);
            cell.firstCursorPosition().insertHtml(action->whatsThis());

            currow++;
        }
        cursor.movePosition(QTextCursor::End);
    }
    cursor.endEditBlock();

    QPrinter printer;
    QPrintDialog *dlg = new QPrintDialog(&printer, q);
    if (dlg->exec() == QDialog::Accepted) {
        doc.print(&printer);
    }
    delete dlg;
#endif
}

#include "moc_kshortcutseditor.cpp"
