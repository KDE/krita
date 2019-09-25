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

#include "KisAndroidFileManager.h"

#include <kis_debug.h>

class KisAndroidFileManager::ActivityResultReceiver : public QAndroidActivityResultReceiver
{
public:
    ActivityResultReceiver(KisAndroidFileManager* manager)
        : _manager(manager)
    {
    }

    void handleActivityResult(int receiverRequestCode, int resultCode, const QAndroidJniObject &data) override;

private:
    const static jint RESULT_OK = -1;
    const static jint RESULT_CANCELED = 0;

    KisAndroidFileManager *_manager;
};

KisAndroidFileManager::KisAndroidFileManager(KisMainWindow* mainWindow)
     : QObject(mainWindow)
     , ACTION_OPEN_DOCUMENT(QAndroidJniObject::fromString("android.intent.action.OPEN_DOCUMENT"))
     , ACTION_GET_CONTENT(QAndroidJniObject::fromString("android.intent.action.GET_CONTENT"))
     , ACTION_CREATE_DOCUMENT(QAndroidJniObject::fromString("android.intent.action.CREATE_DOCUMENT"))
     , genericMIME(QAndroidJniObject::fromString("*/*"))
     , resultReceiver(new ActivityResultReceiver(this))
{
}

KisAndroidFileManager::~KisAndroidFileManager()
{
    delete resultReceiver;
}

void KisAndroidFileManager::openImportFile()
{
    QAndroidJniObject intent("android/content/Intent");
    if (intent.isValid())
    {
        intent.callObjectMethod("setAction", "(Ljava/lang/String;)Landroid/content/Intent;", ACTION_OPEN_DOCUMENT.object<jstring>());
        intent.callObjectMethod("setType", "(Ljava/lang/String;)Landroid/content/Intent;", genericMIME.object<jstring>());
        QtAndroid::startActivity(intent.object<jobject>(), FILE_PICK_RC, resultReceiver);
    }
    else
    {
        warnKrita << "Intent is null!";
    }
}

void KisAndroidFileManager::ActivityResultReceiver::handleActivityResult(int requestCode, int resultCode, const QAndroidJniObject &data)
{
    if (requestCode == FILE_PICK_RC)
    {
        if (resultCode == RESULT_OK)
        {
            QAndroidJniObject uri = data.callObjectMethod("getData", "()Landroid/net/Uri;");
            if (uri.isValid())
            {
                QAndroidJniObject pathObject =
                        QAndroidJniObject::callStaticObjectMethod("org/krita/android/FileUtils",
                                                                  "getPath",
                                                                  "(Landroid/content/Context;Landroid/net/Uri;)Ljava/lang/String;",
                                                                  QtAndroid::androidContext().object(),
                                                                  uri.object());
                QString path = pathObject.toString();
                dbgAndroid << path;
                if (path.isEmpty())
                {
                    emit _manager->sigEmptyFilePath();
                    return;
                }
                emit _manager->sigFileSelected(path);
            }
            else
            {
                warnKrita << "JNI Object returned:" << "data.callObjectMethod(\"getData\", \"()Landroid/net/Uri;\")" << "not valid";
            }
        }
        else if (resultCode == RESULT_CANCELED)
        {
            emit _manager->cancelled();
        }
    }
}
