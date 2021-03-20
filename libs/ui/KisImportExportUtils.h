/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISIMPORTEXPORTUTILS_H
#define KISIMPORTEXPORTUTILS_H

#include <QFlags>
#include <QString>

namespace KritaUtils {

enum SaveFlag {
    SaveNone = 0,
    SaveShowWarnings = 0x1,
    SaveIsExporting = 0x2,
    SaveInAutosaveMode = 0x4
};

Q_DECLARE_FLAGS(SaveFlags, SaveFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(SaveFlags)

struct ExportFileJob {
    ExportFileJob()
        : flags(SaveNone)
    {
    }

    ExportFileJob(QString _filePath, QByteArray _mimeType, SaveFlags _flags = SaveNone)
        : filePath(_filePath), mimeType(_mimeType), flags(_flags)
    {
    }

    bool isValid() const {
        return !filePath.isEmpty();
    }

    QString filePath;
    QByteArray mimeType;
    SaveFlags flags;
};

}

#endif // KISIMPORTEXPORTUTILS_H
