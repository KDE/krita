/* This file is part of the KDE project
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
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

#ifndef CREATENEWBOOKMARKDIALOG_H
#define CREATENEWBOOKMARKDIALOG_H

#include "ui_CreateNewBookmark.h"

#include <QWidget>
#include <QList>
#include <KDialog>

class CreateNewBookmark : public QWidget {
    Q_OBJECT
public:
    CreateNewBookmark(const QList<QString> &nameList, const QString &suggestedName, QWidget *parent = 0);
    QString bookmarkName();

signals:
    void bookmarkNameChanged(const QString &name);

private:
    Ui::CreateNewBookmark widget;
};

class CreateNewBookmarkDialog : public KDialog {
    Q_OBJECT
public:
    CreateNewBookmarkDialog(const QList<QString> &nameList, const QString &suggestedName, QWidget *parent = 0);
    QString newBookmarkName();

private slots:
    void nameChanged(const QString &name);

private:
    CreateNewBookmark *ui;
    QList<QString> m_nameList;
};

#endif

