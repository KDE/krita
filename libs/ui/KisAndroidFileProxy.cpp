/*
 * SPDX-FileCopyrightText: 2023 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisAndroidFileProxy.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <kis_debug.h>

QString KisAndroidFileProxy::getFileFromContentUri(QString contentUri)
{
    QFile file = contentUri;
    const QString savePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);

    QDir dirPath = savePath;

    QString filename = file.fileName();
    if (dirPath.exists(filename)) {
        // if the file already exists, delete it
        dirPath.remove(filename);
    }

    bool copyResult = file.copy(savePath + "/" + filename);
    if (!copyResult) {
        qWarning() << "Failed to copy file from content uri" << contentUri << "to" << savePath + "/" + filename;
        qWarning() << "    " << ppVar(file.error()) << ppVar(file.errorString());
        return "";
    }
    return savePath + "/" + filename;
}
