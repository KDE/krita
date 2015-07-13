/* This file is part of the KDE libraries
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
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
#include "KisShortcutsDialog_p.h"

#include <QApplication>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QTreeWidgetItemIterator>

KShortcutsEditorDelegate::KShortcutsEditorDelegate(QTreeWidget *parent, bool allowLetterShortcuts)
    : KExtendableItemDelegate(parent),
      m_allowLetterShortcuts(allowLetterShortcuts),
      m_editor(0)
{
    Q_ASSERT(qobject_cast<QAbstractItemView *>(parent));

    QPixmap pixmap(16, 16);
    pixmap.fill(QColor(Qt::transparent));
    QPainter p(&pixmap);
    QStyleOption option;
    option.rect = pixmap.rect();

    bool isRtl = QApplication::isRightToLeft();
    QApplication::style()->drawPrimitive(isRtl ? QStyle::PE_IndicatorArrowLeft : QStyle::PE_IndicatorArrowRight, &option, &p);
    p.end();
    setExtendPixmap(pixmap);

    pixmap.fill(QColor(Qt::transparent));
    p.begin(&pixmap);
    QApplication::style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &option, &p);
    p.end();
    setContractPixmap(pixmap);

    parent->installEventFilter(this);

    // Listen to activation signals
    // connect(parent, SIGNAL(activated(QModelIndex)), this, SLOT(itemActivated(QModelIndex)));
    connect(parent, SIGNAL(clicked(QModelIndex)), this, SLOT(itemActivated(QModelIndex)));

    // Listen to collapse signals
    connect(parent, SIGNAL(collapsed(QModelIndex)), this, SLOT(itemCollapsed(QModelIndex)));
}

void KShortcutsEditorDelegate::stealShortcut(
    const QKeySequence &seq,
    QAction *action)
{
    QTreeWidget *view = static_cast<QTreeWidget *>(parent());

    // Iterate over all items
    QTreeWidgetItemIterator it(view, QTreeWidgetItemIterator::NoChildren);

    for (; (*it); ++it) {
        KShortcutsEditorItem *item = dynamic_cast<KShortcutsEditorItem *>(*it);
        if (item && item->data(0, ObjectRole).value<QObject *>() == action) {

            // We found the action, snapshot the current state. Steal the
            // shortcut. We will save the change later.
            const QList<QKeySequence> cut = action->shortcuts();
            const QKeySequence primary = cut.isEmpty() ? QKeySequence() : cut.at(0);
            const QKeySequence alternate = cut.size() <= 1 ? QKeySequence() : cut.at(1);

            if (primary.matches(seq) != QKeySequence::NoMatch
                    || seq.matches(primary) != QKeySequence::NoMatch) {
                item->setKeySequence(LocalPrimary, QKeySequence());
            }

            if (alternate.matches(seq) != QKeySequence::NoMatch
                    || seq.matches(alternate) != QKeySequence::NoMatch) {
                item->setKeySequence(LocalAlternate, QKeySequence());
            }
            break;
        }
    }

}

QSize KShortcutsEditorDelegate::sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    QSize ret(KExtendableItemDelegate::sizeHint(option, index));
    ret.rheight() += 4;
    return ret;
}

//slot
void KShortcutsEditorDelegate::itemActivated(QModelIndex index)
{
    //As per our constructor our parent *is* a QTreeWidget
    QTreeWidget *view = static_cast<QTreeWidget *>(parent());

    KShortcutsEditorItem *item = KShortcutsEditorPrivate::itemFromIndex(view, index);
    if (!item) {
        //that probably was a non-leaf (type() !=ActionItem) item
        return;
    }

    int column = index.column();
    if (column == Name) {
        // If user click in the name column activate the (Global|Local)Primary
        // column if possible.
        if (!view->header()->isSectionHidden(LocalPrimary)) {
            column = LocalPrimary;
        } else if (!view->header()->isSectionHidden(GlobalPrimary)) {
            column = GlobalPrimary;
        } else {
            // do nothing.
        }
        index = index.sibling(index.row(), column);
        view->selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
    }

    // Check if the models wants us to edit the item at index
    if (!index.data(ShowExtensionIndicatorRole).value<bool>()) {
        return;
    }

    if (!isExtended(index)) {
        //we only want maximum ONE extender open at any time.
        if (m_editingIndex.isValid()) {
            KShortcutsEditorItem *oldItem = KShortcutsEditorPrivate::itemFromIndex(view,
                                            m_editingIndex);
            Q_ASSERT(oldItem); //here we really expect nothing but a real KShortcutsEditorItem

            oldItem->setNameBold(false);
            contractItem(m_editingIndex);
        }

        m_editingIndex = index;
        QWidget *viewport = static_cast<QAbstractItemView *>(parent())->viewport();

        if (column >= LocalPrimary && column <= GlobalAlternate) {
            ShortcutEditWidget *editor = new ShortcutEditWidget(viewport,
                    index.data(DefaultShortcutRole).value<QKeySequence>(),
                    index.data(ShortcutRole).value<QKeySequence>(),
                    m_allowLetterShortcuts);
            if (column == GlobalPrimary) {
                QObject *action = index.data(ObjectRole).value<QObject *>();
                editor->setAction(action);
                editor->setMultiKeyShortcutsAllowed(false);
                QString componentName = action->property("componentName").toString();
                if (componentName.isEmpty()) {
                    componentName = QCoreApplication::applicationName();
                }
                editor->setComponentName(componentName);
            }

            m_editor = editor;
            // For global shortcuts check against the kde standard shortcuts
            if (column == GlobalPrimary || column == GlobalAlternate) {
                editor->setCheckForConflictsAgainst(
                    KKeySequenceWidget::LocalShortcuts
                    | KKeySequenceWidget::GlobalShortcuts
                    | KKeySequenceWidget::StandardShortcuts);
            }

            editor->setCheckActionCollections(m_checkActionCollections);

            connect(m_editor, SIGNAL(keySequenceChanged(QKeySequence)),
                    this, SLOT(keySequenceChanged(QKeySequence)));
            connect(m_editor, SIGNAL(stealShortcut(QKeySequence,QAction*)),
                    this, SLOT(stealShortcut(QKeySequence,QAction*)));

        } else if (column == RockerGesture) {
            m_editor = new QLabel(QStringLiteral("A lame placeholder"), viewport);

        } else if (column == ShapeGesture) {
            m_editor = new QLabel(QStringLiteral("<i>A towel</i>"), viewport);

        } else {
            return;
        }

        m_editor->installEventFilter(this);
        item->setNameBold(true);
        extendItem(m_editor, index);

    } else {
        //the item is extended, and clicking on it again closes it
        item->setNameBold(false);
        contractItem(index);
        view->selectionModel()->select(index, QItemSelectionModel::Clear);
        m_editingIndex = QModelIndex();
        m_editor = 0;
    }
}

//slot
void KShortcutsEditorDelegate::itemCollapsed(QModelIndex index)
{
    if (!m_editingIndex.isValid()) {
        return;
    }

    const QAbstractItemModel *model = index.model();
    for (int row = 0; row < model->rowCount(index); ++row) {
        for (int col = 0; col < index.model()->columnCount(index); ++col) {
            QModelIndex colIndex = model->index(row, col, index);

            if (colIndex == m_editingIndex) {
                itemActivated(m_editingIndex); //this will *close* the item's editor because it's already open
            }
        }
    }
}

//slot
void KShortcutsEditorDelegate::hiddenBySearchLine(QTreeWidgetItem *item, bool hidden)
{
    if (!hidden || !item) {
        return;
    }
    QTreeWidget *view = static_cast<QTreeWidget *>(parent());
    QTreeWidgetItem *editingItem = KShortcutsEditorPrivate::itemFromIndex(view, m_editingIndex);
    if (editingItem == item) {
        itemActivated(m_editingIndex); //this will *close* the item's editor because it's already open
    }
}

bool KShortcutsEditorDelegate::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_editor) {
        //Prevent clicks in the empty part of the editor widget from closing the editor
        //because they would propagate to the itemview and be interpreted as a click in
        //an item's rect. That in turn would lead to an itemActivated() call, closing
        //the current editor.

        switch (e->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
            return true;
        default:
            return false;
        }
    } else if (o == parent()) {
        // Make left/right cursor keys switch items instead of operate the scroll bar
        // (subclassing QtreeView/Widget would be cleaner but much more of a hassle)
        // Note that in our case we know that the selection mode is SingleSelection,
        // so we don't have to ask QAbstractItemView::selectionCommand() et al.

        if (e->type() != QEvent::KeyPress) {
            return false;
        }
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        QTreeWidget *view = static_cast<QTreeWidget *>(parent());
        QItemSelectionModel *selection = view->selectionModel();
        QModelIndex index = selection->currentIndex();

        switch (ke->key()) {
        case Qt::Key_Space:
        case Qt::Key_Select:
            // we are not using the standard "open editor" mechanism of QAbstractItemView,
            // so let's emulate that here.
            itemActivated(index);
            return true;
        case Qt::Key_Left:
            index = index.sibling(index.row(), index.column() - 1);
            break;
        case Qt::Key_Right:
            index = index.sibling(index.row(), index.column() + 1);
            break;
        default:
            return false;
        }
        // a cursor key was pressed
        if (index.isValid()) {
            selection->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
            //### using PositionAtCenter for now;
            // EnsureVisible has no effect which seems to be a bug.
            view->scrollTo(index, QAbstractItemView::PositionAtCenter);
        }
        return true;
    }
    return false;
}

//slot
void KShortcutsEditorDelegate::keySequenceChanged(const QKeySequence &seq)
{
    QVariant ret = QVariant::fromValue(seq);
    emit shortcutChanged(ret, m_editingIndex);
}

void KShortcutsEditorDelegate::setCheckActionCollections(
    const QList<KActionCollection *> checkActionCollections)
{
    m_checkActionCollections = checkActionCollections;
}

#if 0
//slot
void KShortcutsEditorDelegate::shapeGestureChanged(const KShapeGesture &gest)
{
    //this is somewhat verbose because the gesture types are not "built in" to QVariant
    QVariant ret = QVariant::fromValue(gest);
    emit shortcutChanged(ret, m_editingIndex);
}
#endif

#if 0
//slot
void KShortcutsEditorDelegate::rockerGestureChanged(const KRockerGesture &gest)
{
    QVariant ret = QVariant::fromValue(gest);
    emit shortcutChanged(ret, m_editingIndex);
}
#endif

