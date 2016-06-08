/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2013 Aman Madaan <madaan.amanmadaan@gmail.com>
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

#include "ManageBookmarkDialog.h"

#include <KMessageBox>
#include <QInputDialog>

static QString lastBookMarkItem;

ManageBookmark::ManageBookmark(const QList<QString> &nameList, KoTextEditor *editor, QWidget *parent)
    : QWidget(parent)
    , m_editor(editor)
{
    widget.setupUi(this);
    widget.bookmarkList->addItems(nameList);
    widget.bookmarkList->setFocus(Qt::ActiveWindowFocusReason);
    const int count = widget.bookmarkList->count();
    if (count > 0) {
        int row = 0;
        if (!lastBookMarkItem.isNull()) {
            QList<QListWidgetItem *> items = widget.bookmarkList->findItems(lastBookMarkItem, Qt::MatchExactly);
            if (items.count() > 0) {
                row = widget.bookmarkList->row(items[0]);
            }
        }
        widget.bookmarkList->setCurrentRow(row);
    }

    connect(widget.bookmarkList, SIGNAL(currentRowChanged(int)), this, SLOT(selectionChanged(int)));
    connect(widget.buttonRename, SIGNAL(clicked()), this, SLOT(slotBookmarkRename()));
    connect(widget.buttonDelete, SIGNAL(clicked()), this, SLOT(slotBookmarkDelete()));
    connect(widget.buttonInsert, SIGNAL(clicked()), this, SLOT(slotBookmarkInsert()));
    connect(widget.bookmarkList, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(slotBookmarkItemActivated(QListWidgetItem*)));
    selectionChanged(bookmarkRow());
}

QString ManageBookmark::bookmarkName() const
{
    const QListWidgetItem *item = widget.bookmarkList->currentItem();
    return item ? item->text() : QString();
}

int ManageBookmark::bookmarkRow() const
{
    return widget.bookmarkList->currentRow();
}

void ManageBookmark::selectionChanged(int currentRow)
{
    widget.buttonRename->setEnabled(currentRow != -1);
    widget.buttonDelete->setEnabled(currentRow != -1);
    emit bookmarkSelectionChanged(currentRow);
}

void ManageBookmark::slotBookmarkRename()
{
    bool ok = 0;
    QListWidgetItem *item = widget.bookmarkList->currentItem();
    Q_ASSERT(item);
    QString curName = item->text();
    QString newName = item->text();
    while (true) {
        newName = QInputDialog::getText(parentWidget(),
                                        i18n("Rename Bookmark"),
                                        i18n("Please provide a new name for the bookmark"),
                                        QLineEdit::Normal,
                                        newName,
                                        &ok);
        if (curName != newName  && ok) {
            QList<QListWidgetItem *> items = widget.bookmarkList->findItems(newName, Qt::MatchExactly);
            if (items.count() > 0) {
                KMessageBox::error(parentWidget(), i18n("A bookmark with the name \"%1\" already exists.", newName));
                continue;
            }
            item->setText(newName);
            emit bookmarkNameChanged(curName, newName);
        }
        break;
    }
}

void ManageBookmark::slotBookmarkDelete()
{
    int currentRow = widget.bookmarkList->currentRow();
    Q_ASSERT(currentRow >= 0);
    QListWidgetItem *deletedItem = widget.bookmarkList->takeItem(currentRow);
    QString deletedName = deletedItem->text();
    emit bookmarkItemDeleted(deletedName);
    delete deletedItem;
}

void ManageBookmark::slotBookmarkItemActivated(QListWidgetItem *item)
{
    Q_ASSERT(item);
    lastBookMarkItem = item->text();
    emit bookmarkItemDoubleClicked(item);
}

void ManageBookmark::slotBookmarkInsert()
{
    QString bookmarkName;
    bool ok = 0;
    while (true) {
        bookmarkName = QInputDialog::getText(parentWidget(),
                                             i18n("Insert Bookmark"),
                                             i18n("Please provide a name for the bookmark"),
                                             QLineEdit::Normal,
                                             bookmarkName,
                                             &ok);
        if (ok) {
            QList<QListWidgetItem *> items = widget.bookmarkList->findItems(bookmarkName, Qt::MatchExactly);
            if (items.count() > 0) {
                KMessageBox::error(parentWidget(), i18n("A bookmark with the name \"%1\" already exists.", bookmarkName));
                continue;
            } else {
                m_editor->addBookmark(bookmarkName);
                widget.bookmarkList->insertItem(widget.bookmarkList->count(), bookmarkName);
            }
        }
        break;
    }
}

ManageBookmarkDialog::ManageBookmarkDialog(const QList<QString> &nameList, KoTextEditor *editor, QWidget *parent)
    : KoDialog(parent)
{
    ui = new ManageBookmark(nameList, editor, this);
    setMainWidget(ui);
    setCaption(i18n("Manage Bookmarks"));
    setModal(true);
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    showButtonSeparator(true);
    connect(ui, SIGNAL(bookmarkSelectionChanged(int)), this, SLOT(selectionChanged(int)));
    connect(ui, SIGNAL(bookmarkNameChanged(QString,QString)),
            this, SIGNAL(nameChanged(QString,QString)));
    connect(ui, SIGNAL(bookmarkItemDeleted(QString)),
            this, SIGNAL(bookmarkDeleted(QString)));
    connect(ui, SIGNAL(bookmarkItemDoubleClicked(QListWidgetItem*)),
            this, SLOT(bookmarkDoubleClicked(QListWidgetItem*)));
    selectionChanged(ui->bookmarkRow());
}

QString ManageBookmarkDialog::selectedBookmarkName()
{
    return ui->bookmarkName();
}

void ManageBookmarkDialog::selectionChanged(int currentRow)
{
    enableButtonOk(currentRow != -1);
}

void ManageBookmarkDialog::bookmarkDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);
    accept();
}
