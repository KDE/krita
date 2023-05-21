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

QString KisAndroidFileProxy::getFileFromContentUri(QString contentUri)
{
    QFile file = contentUri;
    const QString documentsLocation = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString savePath = documentsLocation + "/audio-files";

    QDir dirPath = savePath;

    if (!dirPath.exists()) {
        QDir(documentsLocation).mkpath("audio-files");
    }

    const int maxDirCount = 10;
    if (dirPath.count() > maxDirCount) {
        // if the number of files in the directory is greater than 10, delete the oldest file
        dirPath.remove(dirPath.entryList(QDir::NoDotAndDotDot | QDir::Files, QDir::Time).first());
    }

    QString filename = file.fileName();
    if (dirPath.exists(filename)) {
        // if the file already exists, delete it
        dirPath.remove(filename);
    }

    bool copyResult = file.copy(contentUri, savePath + "/" + filename);
    if (!copyResult) {
        qWarning() << "Failed to copy file from content uri" << contentUri << "to" << savePath + "/" + filename;
        return "";
    }
    return savePath + "/" + filename;
}
