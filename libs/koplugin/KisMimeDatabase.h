    /*
 * Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KISMIMEDATABASE_H
#define KISMIMEDATABASE_H

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QList>

#include "kritaplugin_export.h"

/**
 * @brief The KisMimeDatabase class maps file extensions to mimetypes and vice versa
 */
class KRITAPLUGIN_EXPORT KisMimeDatabase
{
public:

    /// Find the mimetype for the given filename. The filename must include a suffix.
    static QString mimeTypeForFile(const QString &file, bool checkExistingFiles = true);
    /// Find the mimetype for a given extension. The extension may have the form "*.xxx" or "xxx"
    static QString mimeTypeForSuffix(const QString &suffix);
    /// Find the mimetype through analyzing the contents. This does not work for Krita's
    /// extended mimetypes.
    static QString mimeTypeForData(const QByteArray ba);
    /// Find the user-readable description for the given mimetype
    static QString descriptionForMimeType(const QString &mimeType);
    /// Find the list of possible extensions for the given mimetype.
    /// The preferred suffix is the first one.
    static QStringList suffixesForMimeType(const QString &mimeType);
    /// The default icon name for the given mimetype
    static QString iconNameForMimeType(const QString &mimeType);

private:

    struct KisMimeType {
        QByteArray mimeType;
        QStringList suffixes;
        QString description;
    };

    static QList<KisMimeType> s_mimeDatabase;
    static void fillMimeData();

};

#endif // KISMIMEDATABASE_H
