/* This file is part of the KDE project
   Copyright (C) 2012 Boudewijn Rempt <boud@valdyas.org>

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
#ifndef KOAUTOSAVERECOVERYDIALOG_H
#define KOAUTOSAVERECOVERYDIALOG_H

#include <KoDialog.h>
#include <QStringList>
#include <QModelIndex>

class QListView;

Q_DECLARE_METATYPE(QModelIndex)

class KoAutoSaveRecoveryDialog : public KoDialog
{
    Q_OBJECT
public:

    explicit KoAutoSaveRecoveryDialog(const QStringList &filenames, QWidget *parent = 0);
    QStringList recoverableFiles();

public Q_SLOTS:

    void toggleFileItem(bool toggle);

private:

    QListView *m_listView;

    class FileItemModel;
    FileItemModel *m_model;
};


#endif // KOAUTOSAVERECOVERYDIALOG_H
