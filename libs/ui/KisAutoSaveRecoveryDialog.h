/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2012 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KISAUTOSAVERECOVERYDIALOG_H
#define KISAUTOSAVERECOVERYDIALOG_H

#include <KoDialog.h>
#include <QStringList>
#include <QModelIndex>

class QListView;

Q_DECLARE_METATYPE(QModelIndex)

class KisAutoSaveRecoveryDialog : public KoDialog
{
    Q_OBJECT
public:

    explicit KisAutoSaveRecoveryDialog(const QStringList &filenames, QWidget *parent = 0);
    ~KisAutoSaveRecoveryDialog() override;
    QStringList recoverableFiles();

public Q_SLOTS:

    void toggleFileItem(bool toggle);
    void slotDeleteAll();

private:

    QListView *m_listView;

    class FileItemModel;
    FileItemModel *m_model;
};


#endif // KOAUTOSAVERECOVERYDIALOG_H
