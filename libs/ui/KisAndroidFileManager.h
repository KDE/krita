 /*
  * This file is part of the KDE project
  * Copyright (C) 2019 Sharaf Zaman <sharafzaz121@gmail.com>
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 2 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
  */

#ifndef KISANDROIDFILEMANAGER_H
#define KISANDROIDFILEMANAGER_H

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

Q_SIGNALS:
    void sigFileSelected(QString path);
    void sigEmptyFilePath();
    void cancelled();

private:
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
