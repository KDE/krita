 /*
  * This file is part of the KDE project
  * SPDX-FileCopyrightText: 2019 Sharaf Zaman <sharafzaz121@gmail.com>
  *
  * SPDX-License-Identifier: GPL-2.0-or-later
  */

#ifndef KISANDROIDFILEMANAGER_H
#define KISANDROIDFILEMANAGER_H

#include <QUrl>
#include <QtAndroid>
#include <QtAndroidExtras/QAndroidActivityResultReceiver>

#include <KisMainWindow.h>

class KisAndroidFileManager: public QObject
{
    Q_OBJECT
public:
    KisAndroidFileManager(KisMainWindow* mainWindow);
    ~KisAndroidFileManager();

    void openImportFile();

    static void takePersistableUriPermission(const QUrl &url);

    static QString mimeType(const QString& uri);

Q_SIGNALS:
    void sigFileSelected(QUrl path);
    void sigEmptyFilePath();
    void cancelled();

private:
    static void takePersistableUriPermission(const QAndroidJniObject &uri);

    // Request codes
    const static int FILE_PICK_RC = 1;   /// to import a file
    const static int FILE_SAVE_RC = 2;   /// to save/export a file

    // Actions
    const QAndroidJniObject ACTION_OPEN_DOCUMENT;
    const QAndroidJniObject ACTION_GET_CONTENT;
    const QAndroidJniObject ACTION_CREATE_DOCUMENT;

    const QAndroidJniObject genericMIME;

    class ActivityResultReceiver;
    ActivityResultReceiver *resultReceiver;
};
#endif // KISANDROIDFILEMANAGER_H
