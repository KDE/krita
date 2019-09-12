/*
 * Copyright (C) 2019 Wolthera van Hövell tot Westerflier
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
#include <QTest>
#include <QCoreApplication>

#include  <sdk/tests/kistest.h>

#include "filestest.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif

#include "kis_sai_test.h"

const QString SaiMimeType = "application/x-painttool-sai";

void KisSaiTest::testOpening()
{
        QFileInfo sourceFileInfo(QString(FILES_DATA_DIR) + QDir::separator() + "sai_rgba_test.sai");

        QScopedPointer<KisDocument> doc(qobject_cast<KisDocument*>(KisPart::instance()->createDocument()));

        KisImportExportManager manager(doc.data());
        doc->setFileBatchMode(true);

        KisImportExportErrorCode status = manager.importDocument(sourceFileInfo.absoluteFilePath(), QString());
        QVERIFY(status.isOk());

        Q_ASSERT(doc->image());

}


QSharedPointer<KisDocument> openSaiDocument(const QFileInfo &fileInfo)
{
    QSharedPointer<KisDocument> doc(qobject_cast<KisDocument*>(KisPart::instance()->createDocument()));

    KisImportExportManager manager(doc.data());
    doc->setFileBatchMode(true);

    KisImportExportErrorCode status = manager.importDocument(fileInfo.absoluteFilePath(), QString());
    Q_UNUSED(status);

    return doc;
}

void KisSaiTest::testColorData()
{
    QFileInfo sourceFileInfo(QString(FILES_DATA_DIR) + QDir::separator() + "sai_rgba_test.sai");
    //Because the projection color is set to white, this needs to be undone here.
    //QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "sai_rgba_test.png");
    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "sai_rgb_test.png");

    Q_ASSERT(sourceFileInfo.exists());

    QSharedPointer<KisDocument> doc = openSaiDocument(sourceFileInfo);
    QVERIFY(doc->image());


    QPoint errpoint;
    int fuzzy = 0;
    int fuzzyAlpha = 1;
    if(!TestUtil::compareQImages(errpoint, qimage,
                                     doc->image()->projection()->convertToQImage(0, 0, 0, qimage.width(), qimage.height()), fuzzy, fuzzyAlpha)) {
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisSaiTest::testBlendingModes()
{
    QFileInfo sourceFileInfo(QString(FILES_DATA_DIR) + QDir::separator() + "sai_test_blending_modes.sai");
    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "sai_test_blending_modes.png");

    Q_ASSERT(sourceFileInfo.exists());

    QSharedPointer<KisDocument> doc = openSaiDocument(sourceFileInfo);
    QVERIFY(doc->image());

    QPoint errpoint;
    int fuzzy = 1;
    int fuzzyAlpha = 1;
    if(!TestUtil::compareQImages(errpoint, qimage,
                                     doc->image()->projection()->convertToQImage(0, 0, 0, qimage.width(), qimage.height()), fuzzy, fuzzyAlpha)) {
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisSaiTest::testLayerConfiguration()
{
    QFileInfo sourceFileInfo(QString(FILES_DATA_DIR) + QDir::separator() + "sai_test_layer_configurations.sai");
    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "sai_test_layer_configurations.png");

    Q_ASSERT(sourceFileInfo.exists());

    QSharedPointer<KisDocument> doc = openSaiDocument(sourceFileInfo);
    QVERIFY(doc->image());

    QPoint errpoint;
    int fuzzy = 1;
    int fuzzyAlpha = 1;
    //Two for masks
    //Two for textures and layerstyles.
    int maximumFailures = 4;
    if(!TestUtil::compareQImages(errpoint, qimage,
                                     doc->image()->projection()->convertToQImage(0, 0, 0, qimage.width(), qimage.height()), fuzzy, fuzzyAlpha, maximumFailures)) {
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisSaiTest::testClippingAndGroups()
{
    QFileInfo sourceFileInfo(QString(FILES_DATA_DIR) + QDir::separator() + "sai_test_clipping_groups.sai");
    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "sai_test_clipping_groups.png");

    Q_ASSERT(sourceFileInfo.exists());

    QSharedPointer<KisDocument> doc = openSaiDocument(sourceFileInfo);
    QVERIFY(doc->image());

    QPoint errpoint;
    int fuzzy = 1;
    int fuzzyAlpha = 1;
    if(!TestUtil::compareQImages(errpoint, qimage,
                                     doc->image()->projection()->convertToQImage(0, 0, 0, qimage.width(), qimage.height()), fuzzy, fuzzyAlpha)) {
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisSaiTest::testLayerOffset()
{
    QFileInfo sourceFileInfo(QString(FILES_DATA_DIR) + QDir::separator() + "sai_test_layer_offset.sai");
    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "sai_test_layer_offset.png");

    Q_ASSERT(sourceFileInfo.exists());

    QSharedPointer<KisDocument> doc = openSaiDocument(sourceFileInfo);
    QVERIFY(doc->image());

    QPoint errpoint;
    int fuzzy = 1;
    int fuzzyAlpha = 1;
    if(!TestUtil::compareQImages(errpoint, qimage,
                                     doc->image()->projection()->convertToQImage(0, 0, 0, qimage.width(), qimage.height()), fuzzy, fuzzyAlpha)) {
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisSaiTest::testImportFromWriteonly()
{
    TestUtil::testImportFromWriteonly(QString(FILES_DATA_DIR), SaiMimeType);
}

void KisSaiTest::testImportIncorrectFormat()
{
    TestUtil::testImportIncorrectFormat(QString(FILES_DATA_DIR), SaiMimeType);
}

KISTEST_MAIN(KisSaiTest)
