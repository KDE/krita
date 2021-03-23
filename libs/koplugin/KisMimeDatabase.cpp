/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KisMimeDatabase.h"

#include <QMimeDatabase>
#include <QMimeType>
#include <QFileInfo>
#include <KritaPluginDebug.h>

#include <klocalizedstring.h>

QList<KisMimeDatabase::KisMimeType> KisMimeDatabase::s_mimeDatabase;

static QString sanitizeSuffix(const QString &suffix)
{
    return suffix.split(QLatin1Char(' ')).first();
}

QString KisMimeDatabase::mimeTypeForFile(const QString &file, bool checkExistingFiles)
{
    fillMimeData();

    QFileInfo fi(file);
    QString suffix = fi.suffix().toLower();

#ifdef Q_OS_ANDROID
    // HACK: on Android we can save as .kra with no extension or as something like:
    // "untitled.kra (1)", (1) being added by the SAF because we can't overwrite the duplicate.
    // So, we need to be able to remove that number and get extension. If there is no extension,
    // perhaps try "kra"
    suffix = sanitizeSuffix(suffix);
    if (suffix.isEmpty())
        suffix = "kra";
#endif

    Q_FOREACH(const KisMimeDatabase::KisMimeType &mimeType, s_mimeDatabase) {
        if (mimeType.suffixes.contains(suffix)) {
            debugPlugin << "mimeTypeForFile(). KisMimeDatabase returned" << mimeType.mimeType << "for" << file;
            return mimeType.mimeType;
        }
    }

    QMimeDatabase db;
    QMimeType mime;
    if (checkExistingFiles && fi.size() > 0) {
        mime = db.mimeTypeForFile(file, QMimeDatabase::MatchContent);
        if (mime.name() != "application/octet-stream" && mime.name() != "application/zip") {
            debugPlugin << "mimeTypeForFile(). QMimeDatabase returned" << mime.name() << "for" << file;
            return mime.name();
        }
    }

#ifdef Q_OS_ANDROID
    QString basename = fi.baseName();
    // HACK: because we use sanitzed suffix
    mime = db.mimeTypeForFile(basename + "." + suffix);
#else
    mime = db.mimeTypeForFile(file);
#endif
    if (mime.name() != "application/octet-stream") {
        debugPlugin << "mimeTypeForFile(). QMimeDatabase returned" << mime.name() << "for" << file;
        return mime.name();
    }
    return "";
}

QString KisMimeDatabase::mimeTypeForSuffix(const QString &suffix)
{
    fillMimeData();
    QMimeDatabase db;

    QString s = suffix.toLower();

    Q_FOREACH(const KisMimeDatabase::KisMimeType &mimeType, s_mimeDatabase) {
        if (mimeType.suffixes.contains(s)) {
            debugPlugin << "mimeTypeForSuffix(). KisMimeDatabase returned" << mimeType.mimeType << "for" << s;
            return mimeType.mimeType;
        }
    }


    // make the file look like a file so Qt would recognize it
    s = "file." + s;

    return mimeTypeForFile(s);
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

    Q_FOREACH(const KisMimeDatabase::KisMimeType &m, s_mimeDatabase) {
        if (m.mimeType == mimeType) {
            debugPlugin << "descriptionForMimeType. KisMimeDatabase returned" << m.description << "for" << mimeType;
            return m.description;
        }
    }

    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForName(mimeType);
    if (mime.name() != "application/octet-stream") {
        debugPlugin  << "descriptionForMimeType. QMimeDatabase returned" << mime.comment() << "for" << mimeType;
        return mime.comment();
    }

    return mimeType;
}

QStringList KisMimeDatabase::suffixesForMimeType(const QString &mimeType)
{
    fillMimeData();
    Q_FOREACH(const KisMimeDatabase::KisMimeType &m, s_mimeDatabase) {
        if (m.mimeType == mimeType) {
            debugPlugin << "suffixesForMimeType. KisMimeDatabase returned" << m.suffixes;
            return m.suffixes;
        }
    }

    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForName(mimeType);
    if (mime.name() != "application/octet-stream" && !mime.suffixes().isEmpty()) {
        QString preferredSuffix = mime.preferredSuffix();
        if (mimeType == "image/x-tga") {
            preferredSuffix = "tga";
        }
        if (mimeType == "image/jpeg") {
            preferredSuffix = "jpg";
        }
        QStringList suffixes = mime.suffixes();
        if (preferredSuffix != suffixes.first()) {
            suffixes.removeAll(preferredSuffix);
            suffixes.prepend(preferredSuffix);

        }
        debugPlugin << "suffixesForMimeType. QMimeDatabase returned" << suffixes;
        return suffixes;
    }
    return QStringList();
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
        mimeType.suffixes = QStringList() << "gbr" << "vbr";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "image/x-gimp-brush-animated";
        mimeType.description = i18nc("description of a file type", "Gimp Image Hose Brush");
        mimeType.suffixes = QStringList() << "gih";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "image/x-adobe-brushlibrary";
        mimeType.description = i18nc("description of a file type", "Adobe Brush Library");
        mimeType.suffixes = QStringList() << "abr";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-krita-paintoppreset";
        mimeType.description = i18nc("description of a file type", "Krita Brush Preset");
        mimeType.suffixes = QStringList() << "kpp";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-mypaint-brush";
        mimeType.description = i18nc("description of a file type", "MyPaint Brush");
        mimeType.suffixes = QStringList() << "myb";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-krita-assistant";
        mimeType.description = i18nc("description of a file type", "Krita Assistant");
        mimeType.suffixes = QStringList() << "paintingassistant";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "image/x-r32";
        mimeType.description = i18nc("description of a file type", "R32 Heightmap");
        mimeType.suffixes = QStringList() <<  "r32";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "image/x-r16";
        mimeType.description = i18nc("description of a file type", "R16 Heightmap");
        mimeType.suffixes = QStringList() <<  "r16";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "image/x-r8";
        mimeType.description = i18nc("description of a file type", "R8 Heightmap");
        mimeType.suffixes = QStringList() << "r8";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-spriter";
        mimeType.description = i18nc("description of a file type", "Spriter SCML");
        mimeType.suffixes = QStringList() << "scml";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "image/x-svm";
        mimeType.description = i18nc("description of a file type", "Starview Metafile");
        mimeType.suffixes = QStringList() << "svm";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "image/openraster";
        mimeType.description = i18nc("description of a file type", "OpenRaster Image");
        mimeType.suffixes = QStringList() << "ora";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-photoshop-style-library";
        mimeType.description = i18nc("description of a file type", "Photoshop Layer Style Library");
        mimeType.suffixes = QStringList() << "asl";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-gimp-color-palette";
        mimeType.description = i18nc("description of a file type", "Color Palette");
        mimeType.suffixes = QStringList() << "gpl" << "pal" << "act" << "aco" << "colors" << "xml" << "sbz";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "krita/x-colorset";
        mimeType.description = i18nc("description of a file type", "Krita Color Palette");
        mimeType.suffixes = QStringList() << "kpl";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-opencolorio-configuration";
        mimeType.description = i18nc("description of a file type", "OpenColorIO Configuration");
        mimeType.suffixes = QStringList() << "ocio";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-gimp-gradient";
        mimeType.description = i18nc("description of a file type", "GIMP Gradients");
        mimeType.suffixes = QStringList() << "ggr";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-gimp-pattern";
        mimeType.description = i18nc("description of a file type", "GIMP Patterns");
        mimeType.suffixes = QStringList() << "pat";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-krita-bundle";
        mimeType.description = i18nc("description of a file type", "Krita Resource Bundle");
        mimeType.suffixes = QStringList() << "bundle";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-krita-workspace";
        mimeType.description = i18nc("description of a file type", "Krita Workspace");
        mimeType.suffixes = QStringList() << "kws";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-krita-windowlayout";
        mimeType.description = i18nc("description of a file type", "Krita Window Layout");
        mimeType.suffixes = QStringList() << "kwl";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-krita-session";
        mimeType.description = i18nc("description of a file type", "Krita Session");
        mimeType.suffixes = QStringList() << "ksn";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-krita-taskset";
        mimeType.description = i18nc("description of a file type", "Krita Taskset");
        mimeType.suffixes = QStringList() << "kts";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-krita-reference-images";
        mimeType.description = i18nc("description of a file type", "Krita Reference Image Collection");
        mimeType.suffixes = QStringList() << "krf";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-krita-gamutmasks";
        mimeType.description = i18nc("description of a file type", "Krita Gamut Mask");
        mimeType.suffixes = QStringList() << "kgm";
        s_mimeDatabase << mimeType;
        
        mimeType.mimeType = "application/x-krita-shortcuts";
        mimeType.description = i18nc("description of a file type", "Krita Shortcut Scheme");
        mimeType.suffixes = QStringList() << "shortcuts";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "image/x-krita-raw";
        mimeType.description = i18nc("description of a file type", "Camera Raw Files");
        mimeType.suffixes = QStringList() << "bay" << "bmq" << "cr2" << "crw" << "cs1" << "dc2" << "dcr" << "dng" << "erf" << "fff" << "hdr" << "k25" << "kdc" << "mdc" << "mos" << "mrw" << "nef" << "orf" << "pef" << "pxn" << "raf" << "raw" << "rdc" << "sr2" << "srf" << "x3f" << "arw" << "3fr" << "cine" << "ia" << "kc2" << "mef" << "nrw" << "qtk" << "rw2" << "sti" << "rwl" << "srw";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-extension-exr";
        mimeType.description = i18nc("description of a file type", "OpenEXR (Extended)");
        mimeType.suffixes = QStringList() << "exr";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "image/x-psb";
        mimeType.description = i18nc("description of a file type", "Photoshop Image (Large)");
        mimeType.suffixes = QStringList() << "psb";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "image/heic";
        mimeType.description = i18nc("description of a file type", "HEIC/HEIF Image");
        mimeType.suffixes = QStringList() << "heic" << "heif";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "image/avif";
        mimeType.description = i18nc("description of a file type", "AVIF Image");
        mimeType.suffixes = QStringList() << "avif";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "image/jp2";
        mimeType.description = i18nc("description of a file type", "JP2 Image");
        mimeType.suffixes = QStringList() << "jp2" << "j2k";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-krita-seexpr-script";
        mimeType.description = i18nc("description of a file type", "SeExpr script package");
        mimeType.suffixes = QStringList() << "kse";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "application/x-krita-archive";
        mimeType.description = i18nc("description of a file type", "Krita Archival Image Format");
        mimeType.suffixes = QStringList() << "krz";
        s_mimeDatabase << mimeType;

        mimeType.mimeType = "image/apng";
        mimeType.description = i18nc("description of a file type", "Animated PNG Image");
        mimeType.suffixes = QStringList() << "apng";
        s_mimeDatabase << mimeType;

        debugPlugin << "Filled mimedatabase with" << s_mimeDatabase.count() << "special mimetypes";
    }
}
