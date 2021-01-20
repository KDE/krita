/*
 * SPDX-FileCopyrightText: 2018 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef RESOURCETESTHELPER_H
#define RESOURCETESTHELPER_H

#include <QImageReader>
#include <QDir>
#include <QStandardPaths>
#include <QDirIterator>

#include <KisMimeDatabase.h>
#include <KisResourceLoaderRegistry.h>

#include <KisResourceCacheDb.h>
#include "KisResourceTypes.h"
#include <DummyResource.h>
#include <KisStoragePlugin.h>
#include <QtTest>
#include "kis_debug.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

#ifndef FILES_DEST_DIR
#error "FILES_DEST_DIR not set. A directory where data will be written to for testing installing resources"
#endif

namespace ResourceTestHelper {

void rmTestDb() {
    QDir dbLocation(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QFile(dbLocation.path() + "/" + KisResourceCacheDb::resourceCacheDbFilename).remove();
    dbLocation.rmpath(dbLocation.path());
}


class KisDummyResourceLoader : public KisResourceLoaderBase {
public:
    KisDummyResourceLoader(const QString &id, const QString &folder, const QString &name, const QStringList &mimetypes)
        : KisResourceLoaderBase(id, folder, name, mimetypes)
    {
    }

    virtual KoResourceSP create(const QString &name)
    {
        QSharedPointer<DummyResource> resource = QSharedPointer<DummyResource>::create(name, resourceType());
        return resource;
    }
};

void createDummyLoaderRegistry() {

    KisResourceLoaderRegistry *reg = KisResourceLoaderRegistry::instance();
    reg->add(new KisDummyResourceLoader(ResourceType::PaintOpPresets, ResourceType::PaintOpPresets,  i18n("Brush presets"), QStringList() << "application/x-krita-paintoppreset"));
    reg->add(new KisDummyResourceLoader(ResourceSubType::GbrBrushes, ResourceType::Brushes, i18n("Brush tips"), QStringList() << "image/x-gimp-brush"));
    reg->add(new KisDummyResourceLoader(ResourceSubType::GihBrushes, ResourceType::Brushes, i18n("Brush tips"), QStringList() << "image/x-gimp-brush-animated"));
    reg->add(new KisDummyResourceLoader(ResourceSubType::SvgBrushes, ResourceType::Brushes, i18n("Brush tips"), QStringList() << "image/svg+xml"));
    reg->add(new KisDummyResourceLoader(ResourceSubType::PngBrushes, ResourceType::Brushes, i18n("Brush tips"), QStringList() << "image/png"));
    reg->add(new KisDummyResourceLoader(ResourceSubType::SegmentedGradients, ResourceType::Gradients, i18n("Gradients"), QStringList() << "application/x-gimp-gradient"));
    reg->add(new KisDummyResourceLoader(ResourceSubType::StopGradients, ResourceType::Gradients, i18n("Gradients"), QStringList() << "application/x-karbon-gradient" << "image/svg+xml"));
    reg->add(new KisDummyResourceLoader(ResourceType::Palettes, ResourceType::Palettes, i18n("Palettes"),
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

    reg->add(new KisDummyResourceLoader(ResourceType::Patterns, ResourceType::Patterns, i18n("Patterns"), allImageMimes));
    reg->add(new KisDummyResourceLoader(ResourceType::Workspaces, ResourceType::Workspaces, i18n("Workspaces"), QStringList() << "application/x-krita-workspace"));
    reg->add(new KisDummyResourceLoader(ResourceType::Symbols, ResourceType::Symbols, i18n("SVG symbol libraries"), QStringList() << "image/svg+xml"));
    reg->add(new KisDummyResourceLoader(ResourceType::WindowLayouts, ResourceType::WindowLayouts, i18n("Window layouts"), QStringList() << "application/x-krita-windowlayout"));
    reg->add(new KisDummyResourceLoader(ResourceType::Sessions, ResourceType::Sessions, i18n("Sessions"), QStringList() << "application/x-krita-session"));
    reg->add(new KisDummyResourceLoader(ResourceType::GamutMasks, ResourceType::GamutMasks, i18n("Gamut masks"), QStringList() << "application/x-krita-gamutmask"));

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

void initTestDb()
{
    rmTestDb();
    cleanDstLocation(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
}

void overrideResourceVesion(KoResourceSP resource, int version)
{
    resource->setVersion(version);
}

void testVersionedStorage(KisStoragePlugin &storage, const QString &resourceType, const QString &resourceUrl, const QString &optionalFolderCheck = QString())
{
    const QFileInfo fileInfo(resourceUrl);

    auto verifyFileExists = [optionalFolderCheck, resourceType] (KoResourceSP res) {
        if (optionalFolderCheck.isEmpty()) return;

        const QString filePath = optionalFolderCheck + "/" + resourceType + "/" + res->filename();

        if (!QFileInfo(filePath).exists()) {
            qWarning() << "Couldn't find a file in the resource storage:";
            qWarning() << "    " << ppVar(res->filename());
            qWarning() << "    " << ppVar(optionalFolderCheck);
            qWarning() << "    " << ppVar(filePath);
        }

        QVERIFY(QFileInfo(filePath).exists());
    };

    KoResourceSP res1 = storage.resource(resourceUrl);
    QCOMPARE(res1->filename(), fileInfo.fileName()); // filenames are not URLs
    QCOMPARE(res1->version(), -1); // storages don't work with versions
    QCOMPARE(res1->valid(), true);

    const QString originalSomething = res1.dynamicCast<DummyResource>()->something();

    KoResourceSP res2 = storage.resource(resourceUrl);
    QCOMPARE(res2->filename(), fileInfo.fileName());
    QCOMPARE(res2->version(), -1); // storages don't work with versions
    QCOMPARE(res2->valid(), true);

    QVERIFY(res1 != res2);

    res2.dynamicCast<DummyResource>()->setSomething("It's changed");
    QCOMPARE(res1.dynamicCast<DummyResource>()->something(), originalSomething);
    QCOMPARE(res2.dynamicCast<DummyResource>()->something(), "It's changed");

    KoResourceSP res3 = storage.resource(resourceUrl);
    QCOMPARE(res3->filename(), fileInfo.fileName());
    QCOMPARE(res3->version(), -1); // storages don't work with versions
    QCOMPARE(res3->valid(), true);
    QCOMPARE(res3.dynamicCast<DummyResource>()->something(), originalSomething);

    const QString versionedName = fileInfo.baseName() + ".0001." + fileInfo.suffix();

    storage.addResource(resourceType, res2);
    QCOMPARE(res2->filename(), versionedName);
    QCOMPARE(res2->version(), -1); // storages don't work with versions
    QCOMPARE(res2->valid(), true);
    verifyFileExists(res2);

    KoResourceSP res4 = storage.resource(resourceType + "/" + versionedName);
    QCOMPARE(res4->filename(), versionedName);
    QCOMPARE(res4->version(), -1); // storages don't work with versions
    QCOMPARE(res4->valid(), true);
    QCOMPARE(res4.dynamicCast<DummyResource>()->something(), "It's changed");
    verifyFileExists(res4);

    overrideResourceVesion(res4, 10000);
    storage.addResource(resourceType, res4);
    QCOMPARE(res4->filename(), fileInfo.baseName() + ".10000." + fileInfo.suffix());
    verifyFileExists(res4);

    overrideResourceVesion(res4, -1);
    const QString versionedName2 = fileInfo.baseName() + ".10001." + fileInfo.suffix();

    storage.addResource(resourceType, res4);
    QCOMPARE(res4->filename(), versionedName2);
    QCOMPARE(res4->version(), -1); // storages don't work with versions
    QCOMPARE(res4->valid(), true);
    verifyFileExists(res4);
}

}

#endif // RESOURCETESTHELPER_H
