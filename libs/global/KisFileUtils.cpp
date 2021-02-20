/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KisFileUtils.h"

#include <QString>
#include <QFileInfo>
#include <QDir>

namespace KritaUtils {

QString resolveAbsoluteFilePath(const QString &baseDir, const QString &fileName)
{
    if (QFileInfo(fileName).isAbsolute()) {
        return fileName;
    }

    QFileInfo fallbackBaseDirInfo(baseDir);

    return QFileInfo(QDir(fallbackBaseDirInfo.isDir() ?
                              fallbackBaseDirInfo.absoluteFilePath() :
                              fallbackBaseDirInfo.absolutePath()),
                     fileName).absoluteFilePath();
}

}
