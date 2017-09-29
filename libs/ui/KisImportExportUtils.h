/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
