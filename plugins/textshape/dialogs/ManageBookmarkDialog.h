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

#ifndef SELECTBOOKMARKDIALOG_H
#define SELECTBOOKMARKDIALOG_H
#include <KoTextEditor.h>
#include <ui_SelectBookmark.h>

#include <QWidget>
#include <KDialog>

class ManageBookmark : public QWidget
{
    Q_OBJECT
public:
    explicit ManageBookmark(QList<QString> nameList, KoTextEditor *editor, QWidget *parent = 0);
    QString bookmarkName() const;
    int bookmarkRow() const;

signals:
    void bookmarkSelectionChanged(int currentRow);
    void bookmarkNameChanged(const QString &oldName, const QString &newName);
    void bookmarkItemDeleted(const QString &deletedName);
    void bookmarkItemDoubleClicked(QListWidgetItem *item);

private slots:
    void selectionChanged(int currentRow);
    void slotBookmarkRename();
    void slotBookmarkDelete();
    void slotBookmarkInsert();
    void slotBookmarkItemActivated(QListWidgetItem *item);

private:
    Ui::SelectBookmark widget;
    KoTextEditor *m_editor;
};

class ManageBookmarkDialog : public KDialog
{
    Q_OBJECT
public:
    explicit ManageBookmarkDialog(QList<QString> nameList, KoTextEditor *editor, QWidget *parent = 0);
    QString selectedBookmarkName();

signals:
    void nameChanged(const QString &oldName, const QString &newName);
    void bookmarkDeleted(const QString &deletedName);

private slots:
    void selectionChanged(int currentRow);
    void bookmarkDoubleClicked(QListWidgetItem *item);

private:
    ManageBookmark *ui;

};

#endif

