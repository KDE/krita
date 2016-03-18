/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KisMimeDatabase.h"

#include <QMimeDatabase>
#include <QMimeType>
#include <QFileInfo>
#include <KritaPluginDebug.h>

#include <klocalizedstring.h>

QList<KisMimeDatabase::KisMimeType> KisMimeDatabase::s_mimeDatabase;


QString KisMimeDatabase::mimeTypeForFile(const QString &file)
{
    fillMimeData();
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(file);
    if (mime.name() != "application/octet-stream") {
        debugPlugin << "mimeTypeForFile(). QMimeDatabase returned" << mime.name() << "for" << file;
        return mime.name();
    }
    QFileInfo fi(file);
    QString suffix = fi.suffix();
    Q_FOREACH(const KisMimeDatabase::KisMimeType &mimeType, s_mimeDatabase) {
        if (mimeType.suffixes.contains("*." + suffix)) {
            debugPlugin << "mimeTypeForFile(). KisMimeDatabase returned" << mimeType.mimeType << "for" << file;
            return mimeType.mimeType;
        }
    }
    return QString();
}

QString KisMimeDatabase::mimeTypeForSuffix(const QString &suffix)
{
    fillMimeData();
    QMimeDatabase db;

    QString s = suffix;
    if (!s.startsWith("*.")) {
        s = "*." + s;
    }

    QMimeType mime = db.mimeTypeForFile(s);
    if (mime.name() != "application/octet-stream") {
        debugPlugin << "mimeTypeForSuffix(). QMimeDatabase returned" << mime.name() << "for" << s;
        return mime.name();
    }

    Q_FOREACH(const KisMimeDatabase::KisMimeType &mimeType, s_mimeDatabase) {
        if (mimeType.suffixes.contains(s)) {
            debugPlugin << "mimeTypeForSuffix(). KisMimeDatabase returned" << mimeType.mimeType << "for" << s;
            return mimeType.mimeType;
        }
    }
    return QString();
}

QString KisMimeDatabase::mimeTypeForData(const QByteArray ba)
{
    QMimeDatabase db;
    QMimeType mtp = db.mimeTypeForData(ba);
    debugPlugin << "mimeTypeForData(). QMimeDatabase returned" << mtp.name();
    return mtp.name();
}

QString KisMimeDatabase::descriptionForMimeType(const QString &mimeType)
{
    fillMimeData();
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForName(mimeType);
    if (mime.name() != "application/octet-stream") {
        debugPlugin << "descriptionForMimeType. QMimeDatabase returned" << mime.comment() << "for" << mimeType;
        return mime.comment();
    }
    Q_FOREACH(const KisMimeDatabase::KisMimeType &m, s_mimeDatabase) {
        if (m.mimeType == mimeType) {
            debugPlugin << "descriptionForMimeType. KisMimeDatabase returned" << m.description << "for" << mimeType;
            return m.description;
        }
    }
    return QString();
}

QStringList KisMimeDatabase::suffixesForMimeType(const QString &mimeType)
{
    fillMimeData();
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForName(mimeType);
    if (mime.name() != "application/octet-stream") {
        QString preferredSuffix = mime.preferredSuffix();
        QStringList suffixes = mime.suffixes();
        if (preferredSuffix != suffixes.first()) {
            suffixes.removeAll(preferredSuffix);
            suffixes.prepend(preferredSuffix);

        }
        debugPlugin << "suffixesForMimeType. QMimeDatabase returned" << suffixes;
        return suffixes;
    }
    Q_FOREACH(const KisMimeDatabase::KisMimeType &m, s_mimeDatabase) {
        if (m.mimeType == mimeType) {
            debugPlugin << "suffixesForMimeType. KisMimeDatabase returned" << m.suffixes;
            return m.suffixes;
        }
    }
    return QStringList(".kra");
}

QString KisMimeDatabase::iconNameForMimeType(const QString &mimeType)
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForName(mimeType);
    debugPlugin << "iconNameForMimeType" << mime.iconName();
    return mime.iconName();
}

void KisMimeDatabase::fillMimeData()
{
    // This should come from the import/export plugins, but the json files aren't translated,
    // which is bad for the description field
    if (s_mimeDatabase.isEmpty()) {

        KisMimeType mimeType;

        mimeType.mimeType = "image/x-gimp-brush";
        mimeType.description = i18nc("description of a file type", "Gimp Brush");
        mimeType.suffixes << "*.gbr,*vbr";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "image/x-gimp-brush";
        mimeType.description = i18nc("description of a file type", "Gimp Image Hose Brush");
        mimeType.suffixes << "*.gih";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-krita-paintoppreset";
        mimeType.description = i18nc("description of a file type", "Krita Brush Preset");
        mimeType.suffixes << "*.kpp";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "applicat/x-krita-assistant";
        mimeType.description = i18nc("description of a file type", "Krita Assistant");
        mimeType.suffixes << "*.paintingassistant";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "image/x-r16";
        mimeType.description = i18nc("description of a file type", "R16 Heightmap");
        mimeType.suffixes << "*.r16";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "image/x-r8";
        mimeType.description = i18nc("description of a file type", "R8 Heightmap");
        mimeType.suffixes << "*.r8";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-spriter";
        mimeType.description = i18nc("description of a file type", "Spriter SCML");
        mimeType.suffixes << "*.scml";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "";
        mimeType.description = i18nc("description of a file type", "");
        mimeType.suffixes << "*.";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "image/x-svm";
        mimeType.description = i18nc("description of a file type", "Starview Metafile");
        mimeType.suffixes << "*.svm";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "image/openraster";
        mimeType.description = i18nc("description of a file type", "OpenRaster Image");
        mimeType.suffixes << "*.ora";
        s_mimeDatabase << mimeType;

        debugPlugin << "Filled mimedatabase with" << s_mimeDatabase.count() << "special mimetypes";
    }
}
