 /*
  * This file is part of the KDE project
  * SPDX-FileCopyrightText: 2019 Sharaf Zaman <sharafzaz121@gmail.com>
  *
  * SPDX-License-Identifier: GPL-2.0-or-later
  */

#include "KisAndroidFileManager.h"
#include <QUrl>

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

void KisAndroidFileManager::takePersistableUriPermission(const QUrl &url)
{
    QAndroidJniObject rawUri = QAndroidJniObject::fromString(url.toString());
    QAndroidJniObject uri = QAndroidJniObject::callStaticObjectMethod("android/net/Uri",
                                                                      "parse",
                                                                      "(Ljava/lang/String;)Landroid/net/Uri;",
                                                                      rawUri.object());
    if (uri.isValid()) {
        takePersistableUriPermission(uri);
    } else {
        warnKrita << "Uri returned is not valid";
    }
}

QString KisAndroidFileManager::mimeType(const QString& uri)
{
    QAndroidJniObject mimeType = QAndroidJniObject::callStaticObjectMethod(
        "org/qtproject/qt5/android/QtNative",
        "getMimeTypeFromUri",
        "(Landroid/content/Context;Ljava/lang/String;)Ljava/lang/String;",
        QtAndroid::androidContext().object(),
        QAndroidJniObject::fromString(uri).object());

    if (mimeType.isValid()) {
        return mimeType.toString();
    } else {
        return QString();
    }
}

void KisAndroidFileManager::takePersistableUriPermission(const QAndroidJniObject& uri)
{
    int mode = QAndroidJniObject::getStaticField<jint>("android/content/Intent",
                                                       "FLAG_GRANT_WRITE_URI_PERMISSION");

    mode |= QAndroidJniObject::getStaticField<jint>("android/content/Intent",
                                                    "FLAG_GRANT_READ_URI_PERMISSION");

    QAndroidJniObject contentResolver = QtAndroid::androidActivity()
            .callObjectMethod("getContentResolver",
                              "()Landroid/content/ContentResolver;");

    // This protects us SecurityException, which might be hard to figure out in future..
    contentResolver.callMethod<void>("takePersistableUriPermission",
                                     "(Landroid/net/Uri;I)V",
                                     uri.object(),
                                     mode);
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
                _manager->takePersistableUriPermission(uri);

                if (path.isEmpty())
                {
                    emit _manager->sigEmptyFilePath();
                    return;
                }
                emit _manager->sigFileSelected(QUrl(path));
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
