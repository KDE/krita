/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KisFileUtils.h"

#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>

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

QString deduplicateFileName(const QString &fileName,
                            const QString &separator,
                            std::function<bool(QString)> fileAllowedCallback)
{
    const QFileInfo fileInfo(fileName);

    int counter = 0;
    QString proposedFileName = fileInfo.fileName();

    QString baseName = fileInfo.baseName();
    QString completeSuffix = fileInfo.completeSuffix();

    /**
     * Search for the separator around the leftmost dot in the filename
     * and try to reuse its counter.
     *
     * The design choice is that there cannot be any dots to the left
     * from the separator. Separator itself can have dots, but it cannot
     * be a part of the file extension.
     */
    QRegularExpression rex(QString("^([^.]+)%1\\d+(\\.(.+))?$").arg(separator));
    auto match = rex.match(proposedFileName);

    if (match.hasMatch()) {
        baseName = match.captured(1);
        completeSuffix = match.captured(2).removeFirst();
    }

    while (!fileAllowedCallback(proposedFileName)) {
        QStringList fileParts = {baseName, separator, QString::number(counter++)};

        if (!completeSuffix.isEmpty()) {
            fileParts += ".";
            fileParts += completeSuffix;
        }
        proposedFileName = fileParts.join("");
    }

    return proposedFileName;
}
}
