/*
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_psd_test.h"


#include <QTest>
#include <QCoreApplication>

#include  <sdk/tests/testui.h>

#include "filestest.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif

#include <resources/KoPattern.h>
#include "kis_group_layer.h"
#include "kis_psd_layer_style.h"
#include "kis_paint_device_debug_utils.h"
#include <KisImportExportErrorCode.h>



const QString PSDMimetype = "image/vnd.adobe.photoshop";


void KisPSDTest::testFiles()
{
    TestUtil::testFiles(QString(FILES_DATA_DIR) + "/sources", QStringList());
}

void KisPSDTest::testOpening()
{
    QFileInfo sourceFileInfo(QString(FILES_DATA_DIR) + '/' + "testing_psd_ls.psd");

    QScopedPointer<KisDocument> doc(qobject_cast<KisDocument*>(KisPart::instance()->createDocument()));

    KisImportExportManager manager(doc.data());
    doc->setFileBatchMode(true);

    KisImportExportErrorCode status = manager.importDocument(sourceFileInfo.absoluteFilePath(), QString());
    QVERIFY(status.isOk());

    Q_ASSERT(doc->image());
}

QSharedPointer<KisDocument> openPsdDocument(const QFileInfo &fileInfo)
{
    QSharedPointer<KisDocument> doc(qobject_cast<KisDocument*>(KisPart::instance()->createDocument()));

    KisImportExportManager manager(doc.data());
    doc->setFileBatchMode(true);

    KisImportExportErrorCode status = manager.importDocument(fileInfo.absoluteFilePath(), QString());
    Q_UNUSED(status);

    return doc;
}

void KisPSDTest::testTransparencyMask()
{
    QFileInfo sourceFileInfo(QString(FILES_DATA_DIR) + '/' + "sources/masks.psd");

    Q_ASSERT(sourceFileInfo.exists());

    QSharedPointer<KisDocument> doc = openPsdDocument(sourceFileInfo);
    QVERIFY(doc->image());

    QImage result = doc->image()->projection()->convertToQImage(0, doc->image()->bounds());
    QVERIFY(TestUtil::checkQImageExternal(result, "psd_test", "transparency_masks", "kiki_single", 1, 1));


    doc->setFileBatchMode(true);
    doc->setMimeType("image/vnd.adobe.photoshop");

    QFileInfo dstFileInfo(QDir::currentPath() + '/' + "test_tmask.psd");
    bool retval = doc->exportDocumentSync(QUrl::fromLocalFile(dstFileInfo.absoluteFilePath()), "image/vnd.adobe.photoshop");
    QVERIFY(retval);

    {
        QSharedPointer<KisDocument> doc = openPsdDocument(dstFileInfo);
        QVERIFY(doc->image());

        QImage result = doc->image()->projection()->convertToQImage(0, doc->image()->bounds());
        QVERIFY(TestUtil::checkQImageExternal(result, "psd_test", "transparency_masks", "kiki_single", 1, 1));

        QVERIFY(doc->image()->root()->lastChild());
        QVERIFY(doc->image()->root()->lastChild()->firstChild());
        QVERIFY(doc->image()->root()->lastChild()->firstChild()->inherits("KisTransparencyMask"));
    }
}

void KisPSDTest::testOpenGrayscaleMultilayered()
{
    QFileInfo sourceFileInfo(QString(FILES_DATA_DIR) + '/' + "sources/gray.psd");
    //QFileInfo sourceFileInfo(QString(FILES_DATA_DIR) + '/' + "sources/100x100gray8.psd");

    Q_ASSERT(sourceFileInfo.exists());

    QSharedPointer<KisDocument> doc = openPsdDocument(sourceFileInfo);
    QVERIFY(doc->image());
}

void KisPSDTest::testOpenGroupLayers()
{
    QFileInfo sourceFileInfo(QString(FILES_DATA_DIR) + '/' + "group_layers.psd");

    Q_ASSERT(sourceFileInfo.exists());

    QSharedPointer<KisDocument> doc = openPsdDocument(sourceFileInfo);
    QVERIFY(doc->image());

    KisNodeSP node = TestUtil::findNode(doc->image()->root(), "Group 1 PT");
    KisGroupLayer *group = dynamic_cast<KisGroupLayer*>(node.data());
    QVERIFY(group);

    QVERIFY(group->passThroughMode());
}

void KisPSDTest::testOpenLayerStyles()
{
    QFileInfo sourceFileInfo(QString(FILES_DATA_DIR) + '/' + "testing_psd_ls.psd");

    Q_ASSERT(sourceFileInfo.exists());

    QSharedPointer<KisDocument> doc = openPsdDocument(sourceFileInfo);
    QVERIFY(doc->image());

    KisLayerSP layer = qobject_cast<KisLayer*>(doc->image()->root()->lastChild().data());
    QVERIFY(layer->layerStyle());
    QVERIFY(layer->layerStyle()->dropShadow());
    QVERIFY(layer->layerStyle()->dropShadow()->effectEnabled());
}

void KisPSDTest::testOpenLayerStylesWithPattern()
{
    QFileInfo sourceFileInfo(QString(FILES_DATA_DIR) + '/' + "test_ls_pattern.psd");

    Q_ASSERT(sourceFileInfo.exists());

    QSharedPointer<KisDocument> doc = openPsdDocument(sourceFileInfo);
    QVERIFY(doc->image());

    KisLayerSP layer = qobject_cast<KisLayer*>(doc->image()->root()->lastChild().data());
    QVERIFY(layer->layerStyle());
    QVERIFY(layer->layerStyle()->patternOverlay());
    QVERIFY(layer->layerStyle()->patternOverlay()->effectEnabled());
    QVERIFY(layer->layerStyle()->patternOverlay()->pattern());
    QVERIFY(layer->layerStyle()->patternOverlay()->pattern()->valid());
}

void KisPSDTest::testOpenLayerStylesWithPatternMulti()
{
    QFileInfo sourceFileInfo(QString(FILES_DATA_DIR) + '/' + "test_ls_pattern_multi.psd");

    Q_ASSERT(sourceFileInfo.exists());

    QSharedPointer<KisDocument> doc = openPsdDocument(sourceFileInfo);
    QVERIFY(doc->image());

    KisLayerSP layer = qobject_cast<KisLayer*>(doc->image()->root()->lastChild().data());
    QVERIFY(layer->layerStyle());

    QVERIFY(layer->layerStyle()->patternOverlay());
    QVERIFY(layer->layerStyle()->patternOverlay()->effectEnabled());
    QVERIFY(layer->layerStyle()->patternOverlay()->pattern());
    QVERIFY(layer->layerStyle()->patternOverlay()->pattern()->valid());

    QVERIFY(layer->layerStyle()->stroke());
    QVERIFY(layer->layerStyle()->stroke()->effectEnabled());
    QVERIFY(layer->layerStyle()->stroke()->pattern());
    QVERIFY(layer->layerStyle()->stroke()->pattern()->valid());
}

void KisPSDTest::testSaveLayerStylesWithPatternMulti()
{
    QFileInfo sourceFileInfo(QString(FILES_DATA_DIR) + '/' + "test_ls_pattern_multi.psd");

    Q_ASSERT(sourceFileInfo.exists());

    QSharedPointer<KisDocument> doc = openPsdDocument(sourceFileInfo);
    QVERIFY(doc->image());

    KisLayerSP layer = qobject_cast<KisLayer*>(doc->image()->root()->lastChild().data());
    QVERIFY(layer->layerStyle());

    QVERIFY(layer->layerStyle()->patternOverlay());
    QVERIFY(layer->layerStyle()->patternOverlay()->effectEnabled());
    QVERIFY(layer->layerStyle()->patternOverlay()->pattern());
    QVERIFY(layer->layerStyle()->patternOverlay()->pattern()->valid());

    QVERIFY(layer->layerStyle()->stroke());
    QVERIFY(layer->layerStyle()->stroke()->effectEnabled());
    QVERIFY(layer->layerStyle()->stroke()->pattern());
    QVERIFY(layer->layerStyle()->stroke()->pattern()->valid());

    doc->setFileBatchMode(true);
    const QByteArray mimeType("image/vnd.adobe.photoshop");
    QFileInfo dstFileInfo(QDir::currentPath() + '/' + "test_save_styles.psd");
    bool retval = doc->exportDocumentSync(QUrl::fromLocalFile(dstFileInfo.absoluteFilePath()), mimeType);
    QVERIFY(retval);

    {
        QSharedPointer<KisDocument> doc = openPsdDocument(dstFileInfo);
        QVERIFY(doc->image());

        QImage result = doc->image()->projection()->convertToQImage(0, doc->image()->bounds());
        //QVERIFY(TestUtil::checkQImageExternal(result, "psd_test", "transparency_masks", "kiki_single"));

        KisLayerSP layer = qobject_cast<KisLayer*>(doc->image()->root()->lastChild().data());
        QVERIFY(layer->layerStyle());

        QVERIFY(layer->layerStyle()->patternOverlay());
        QVERIFY(layer->layerStyle()->patternOverlay()->effectEnabled());
        QVERIFY(layer->layerStyle()->patternOverlay()->pattern());
        QVERIFY(layer->layerStyle()->patternOverlay()->pattern()->valid());

        QVERIFY(layer->layerStyle()->stroke());
        QVERIFY(layer->layerStyle()->stroke()->effectEnabled());
        QVERIFY(layer->layerStyle()->stroke()->pattern());
        QVERIFY(layer->layerStyle()->stroke()->pattern()->valid());
    }

}

void KisPSDTest::testOpeningFromOpenCanvas()
{
    QFileInfo sourceFileInfo(QString(FILES_DATA_DIR) + '/' + "test_krita_psd_from_opencanvas.psd");

    Q_ASSERT(sourceFileInfo.exists());

    QSharedPointer<KisDocument> doc = openPsdDocument(sourceFileInfo);
    QVERIFY(doc->image());
    QVERIFY(doc->image()->root()->firstChild());
}

void KisPSDTest::testOpeningAllFormats()
{
    QString path = TestUtil::fetchExternalDataFileName("psd_format_test_files");
    QDir dirSources(path);

    if (path.isEmpty()) {
        qWarning() << "External folder is not present, skipping...";
        return;
    }

    bool shouldFailTheTest = false;

    Q_FOREACH (QFileInfo sourceFileInfo, dirSources.entryInfoList()) {
        Q_ASSERT(sourceFileInfo.exists());

        if (sourceFileInfo.isHidden() || sourceFileInfo.isDir()) {
            continue;
        }

        if (sourceFileInfo.fileName() != "ml_cmyk_16b.psd") {
            //continue;
        }

        //dbgKrita << "Opening" << ppVar(sourceFileInfo.fileName());

        QSharedPointer<KisDocument> doc = openPsdDocument(sourceFileInfo);

        if (!doc->image()) {
            /**
             * 32bit images are expected to fail atm, their loading is not implemented
             */
            if (!sourceFileInfo.fileName().contains("_32b")) {
                shouldFailTheTest = true;
            }

            errKrita << "FAILED to open" << sourceFileInfo.fileName();
            continue;
        }

        // just check visually if the file loads fine
        KIS_DUMP_DEVICE_2(doc->image()->projection(), QRect(0,0,100,100), sourceFileInfo.fileName(), "dd");
    }

    QVERIFY(!shouldFailTheTest);
}

void KisPSDTest::testSavingAllFormats()
{
    QString path = TestUtil::fetchExternalDataFileName("psd_format_test_files");
    QDir dirSources(path);

    if (path.isEmpty()) {
        qWarning() << "External folder is not present, skipping...";
        return;
    }

    Q_FOREACH (QFileInfo sourceFileInfo, dirSources.entryInfoList()) {
        Q_ASSERT(sourceFileInfo.exists());

        if (sourceFileInfo.isHidden() || sourceFileInfo.isDir()) {
            continue;
        }

        if (sourceFileInfo.fileName() != "sl_rgb_8b.psd") {
            //continue;
        }

        dbgKrita << "Opening" << ppVar(sourceFileInfo.fileName());

        QSharedPointer<KisDocument> doc = openPsdDocument(sourceFileInfo);

        if (!doc->image()) {
            errKrita << "FAILED to open" << sourceFileInfo.fileName();
            continue;
        }

        QString baseName = sourceFileInfo.fileName();

        //QString originalName = QString("%1_0orig").arg(baseName);
        //QString resultName = QString("%1_1result").arg(baseName);
        QString tempPsdName = QString("%1_3interm.psd").arg(baseName);

        QImage refImage = doc->image()->projection()->convertToQImage(0, QRect(0,0,100,100));

        // uncomment to do a visual check
        // KIS_DUMP_DEVICE_2(doc->image()->projection(), QRect(0,0,100,100), originalName, "dd");

        doc->setFileBatchMode(true);
        doc->setMimeType("image/vnd.adobe.photoshop");

        QFileInfo dstFileInfo(QDir::currentPath() + '/' + tempPsdName);

        dbgKrita << "Saving" << ppVar(dstFileInfo.fileName());

        bool retval = doc->exportDocumentSync(QUrl::fromLocalFile(dstFileInfo.absoluteFilePath()), "image/vnd.adobe.photoshop");
        QVERIFY(retval);

        {
            QSharedPointer<KisDocument> doc = openPsdDocument(dstFileInfo);
            QVERIFY(doc->image());

            // uncomment to do a visual check
            //KIS_DUMP_DEVICE_2(doc->image()->projection(), QRect(0,0,100,100), resultName, "dd");

            QImage resultImage = doc->image()->projection()->convertToQImage(0, QRect(0,0,100,100));
            QCOMPARE(resultImage, refImage);
        }
    }
}



void KisPSDTest::testImportFromWriteonly()
{
    TestUtil::testImportFromWriteonly(QString(FILES_DATA_DIR), PSDMimetype);
}


void KisPSDTest::testExportToReadonly()
{
    TestUtil::testExportToReadonly(QString(FILES_DATA_DIR), PSDMimetype);
}


void KisPSDTest::testImportIncorrectFormat()
{
    TestUtil::testImportIncorrectFormat(QString(FILES_DATA_DIR), PSDMimetype);
}




KISTEST_MAIN(KisPSDTest)

