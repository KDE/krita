/*
 * Copyright (c) 2018 boud <boud@valdyas.org>
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
#ifndef RESOURCETESTHELPER_H
#define RESOURCETESTHELPER_H

#include <QImageReader>
#include <QDir>
#include <QStandardPaths>

#include <KisMimeDatabase.h>
#include <KisResourceLoaderRegistry.h>

#include <KisResourceCacheDb.h>

#include <DummyResource.h>


#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

#ifndef FILES_DEST_DIR
#error "FILES_DEST_DIR not set. A directory where data will be written to for testing installing resources"
#endif

namespace ResourceTestHelper {

void initTestDb() {
    QDir dbLocation(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (dbLocation.exists()) {
        QFile(dbLocation.path() + "/" + KisResourceCacheDb::resourceCacheDbFilename).remove();
        dbLocation.rmpath(dbLocation.path());
    }
}

void rmTestDb() {
    QDir dbLocation(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    bool res = QFile(dbLocation.path() + "/" + KisResourceCacheDb::resourceCacheDbFilename).remove();
    Q_ASSERT(res);
    res = dbLocation.rmpath(dbLocation.path());
    Q_ASSERT(res);
}

void createDummyLoaderRegistry() {

    KisResourceLoaderRegistry *reg = KisResourceLoaderRegistry::instance();
    reg->add(new KisResourceLoader<DummyResource>("paintoppresets", "paintoppresets",  QStringList() << "application/x-krita-paintoppreset"));
    reg->add(new KisResourceLoader<DummyResource>("gbr_brushes", "brushes", QStringList() << "image/x-gimp-brush"));
    reg->add(new KisResourceLoader<DummyResource>("gih_brushes", "brushes", QStringList() << "image/x-gimp-brush-animated"));
    reg->add(new KisResourceLoader<DummyResource>("svg_brushes", "brushes", QStringList() << "image/svg+xml"));
    reg->add(new KisResourceLoader<DummyResource>("png_brushes", "brushes", QStringList() << "image/png"));
    reg->add(new KisResourceLoader<DummyResource>("segmented_gradients", "gradients", QStringList() << "application/x-gimp-gradient"));
    reg->add(new KisResourceLoader<DummyResource>("stop_gradients", "gradients", QStringList() << "application/x-karbon-gradient" << "image/svg+xml"));
    reg->add(new KisResourceLoader<DummyResource>("palettes", "palettes",
                                                  QStringList() << KisMimeDatabase::mimeTypeForSuffix("kpl")
                                                  << KisMimeDatabase::mimeTypeForSuffix("gpl")
                                                  << KisMimeDatabase::mimeTypeForSuffix("pal")
                                                  << KisMimeDatabase::mimeTypeForSuffix("act")
                                                  << KisMimeDatabase::mimeTypeForSuffix("aco")
                                                  << KisMimeDatabase::mimeTypeForSuffix("css")
                                                  << KisMimeDatabase::mimeTypeForSuffix("colors")
                                                  << KisMimeDatabase::mimeTypeForSuffix("xml")
                                                  << KisMimeDatabase::mimeTypeForSuffix("sbz")));

    QList<QByteArray> src = QImageReader::supportedMimeTypes();
    QStringList allImageMimes;
    Q_FOREACH(const QByteArray ba, src) {
        allImageMimes << QString::fromUtf8(ba);
    }
    allImageMimes << KisMimeDatabase::mimeTypeForSuffix("pat");

    reg->add(new KisResourceLoader<DummyResource>("patterns", "patterns", allImageMimes));
    reg->add(new KisResourceLoader<DummyResource>("workspaces", "workspaces", QStringList() << "application/x-krita-workspace"));
    reg->add(new KisResourceLoader<DummyResource>("symbols", "symbols", QStringList() << "image/svg+xml"));
    reg->add(new KisResourceLoader<DummyResource>("windowlayouts", "sessions", QStringList() << "application/x-krita-windowlayout"));
    reg->add(new KisResourceLoader<DummyResource>("sessions", "sessions", QStringList() << "application/x-krita-session"));
    reg->add(new KisResourceLoader<DummyResource>("gamutmasks", "gamutmasks", QStringList() << "application/x-krita-gamutmask"));

}

bool cleanDstLocation(const QString &dstLocation)
{
    if (QDir(dstLocation).exists()) {
        {
            QDirIterator iter(dstLocation, QStringList() << "*", QDir::Files, QDirIterator::Subdirectories);
            while (iter.hasNext()) {
                iter.next();
                QFile f(iter.filePath());
                f.remove();
                //qDebug() << (r ? "Removed" : "Failed to remove") << iter.filePath();
            }
        }
        {
            QDirIterator iter(dstLocation, QStringList() << "*", QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (iter.hasNext()) {
                iter.next();
                QDir(iter.filePath()).rmpath(iter.filePath());
                //qDebug() << (r ? "Removed" : "Failed to remove") << iter.filePath();
            }
        }

        return QDir().rmpath(dstLocation);
    }
    return true;
}


}

#endif // RESOURCETESTHELPER_H
