/*
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_psd_test.h"


#include <simpletest.h>
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
#include <kis_generator_layer.h>
#include <kis_filter_configuration.h>
#include <KisGlobalResourcesInterface.h>



const QString PSDMimetype = "image/vnd.adobe.photoshop";


void KisPSDTest::testFiles()
{
    QStringList exclusions;
    exclusions << "100x100indexed.psd";
    exclusions << "100x100rgb16.psd";
    exclusions << "100x100cmyk16.psd";
    exclusions << "100x100cmyk8.psd";
    exclusions << "gray.psd";
    exclusions << "vector.psd";
    exclusions << "masks.psd";
    exclusions << "angle2.psd";
    exclusions << "diamond2.psd";
    exclusions << "linear3.psd";
    exclusions << "cmyk8-pantone_solid_coated_688c-L51_a33_b-8.psd";
    exclusions << "pattern2_uncompressed.psd";
    exclusions << "pattern4_rle.psd";


    TestUtil::testFiles(QString(FILES_DATA_DIR) + "/sources", exclusions, QString(), 2);
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
    bool retval = doc->exportDocumentSync(dstFileInfo.absoluteFilePath(), "image/vnd.adobe.photoshop");
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

void KisPSDTest::testOpenFillLayers()
{
    QFileInfo sourceFileInfo(QString(FILES_DATA_DIR) + '/' + "sources/angle2.psd");

        Q_ASSERT(sourceFileInfo.exists());

        QSharedPointer<KisDocument> doc = openPsdDocument(sourceFileInfo);
        QVERIFY(doc->image());
        KisGeneratorLayerSP layer = qobject_cast<KisGeneratorLayer*>(doc->image()->root()->lastChild().data());
        QVERIFY(layer);
        QVERIFY(layer->filter()->name() == "gradient");
        QVERIFY(layer->filter()->getDouble("end_position_angle") == 180);
        QVERIFY(layer->filter()->getDouble("end_position_distance") == 50);
        QVERIFY(layer->filter()->getString("shape") == "conical");

        QFileInfo sourceFileInfo2(QString(FILES_DATA_DIR) + '/' + "sources/diamond2.psd");

        Q_ASSERT(sourceFileInfo2.exists());

        doc = openPsdDocument(sourceFileInfo2);
        QVERIFY(doc->image());
        layer = qobject_cast<KisGeneratorLayer*>(doc->image()->root()->lastChild().data());
        QVERIFY(layer);
        QVERIFY(layer->filter()->name() == "gradient");
        QVERIFY(layer->filter()->getDouble("end_position_angle") == 16.5);
        QVERIFY(layer->filter()->getDouble("end_position_distance") - double(40.2306) < 0.001);
        QVERIFY(layer->filter()->getString("shape") == "square");

        QFileInfo sourceFileInfo3(QString(FILES_DATA_DIR) + '/' + "sources/linear3.psd");

        Q_ASSERT(sourceFileInfo3.exists());

        doc = openPsdDocument(sourceFileInfo3);
        QVERIFY(doc->image());
        layer = qobject_cast<KisGeneratorLayer*>(doc->image()->root()->lastChild().data());
        QVERIFY(layer);
        QVERIFY(layer->filter()->name() == "gradient");
        QVERIFY(layer->filter()->getDouble("end_position_angle") == 270);
        QVERIFY(layer->filter()->getString("shape") == "linear");

        QFileInfo sourceFileInfo4(QString(FILES_DATA_DIR) + '/' + "sources/cmyk8-pantone_solid_coated_688c-L51_a33_b-8.psd");

        Q_ASSERT(sourceFileInfo4.exists());

        doc = openPsdDocument(sourceFileInfo4);
        QVERIFY(doc->image());
        layer = qobject_cast<KisGeneratorLayer*>(doc->image()->root()->lastChild().data());
        QVERIFY(layer);
        QVERIFY(layer->filter()->name() == "color");
        KoColor c = layer->filter()->getColor("color");
        KoColor l = KoColor::fromXML("<color channeldepth='U16'><Lab space='"+KoColorSpaceRegistry::instance()->lab16()->name()+"' L='51.0' a='33.0' b='-8.0' /></color>");
        c.convertTo(l.colorSpace());
        QVERIFY(doc->image()->colorSpace()->difference(c.data(), l.data()) < 3);
        QVERIFY(c.metadata().value("spotName", QVariant()).toString() == "PANTONE 688 C");
        QVERIFY(c.metadata().value("psdSpotBook", QVariant()).toString().contains("Solid Coated"));
        QVERIFY(c.metadata().value("psdSpotBookId", QVariant()).toInt() == 3060);

        QFileInfo sourceFileInfo5(QString(FILES_DATA_DIR) + '/' + "sources/pattern2_uncompressed.psd");

        Q_ASSERT(sourceFileInfo5.exists());

        doc = openPsdDocument(sourceFileInfo5);
        QVERIFY(doc->image());
        layer = qobject_cast<KisGeneratorLayer*>(doc->image()->root()->lastChild().data());
        QVERIFY(layer);
        QVERIFY(layer->filter()->name() == "pattern");
        if (layer) {
            const QString patternMD5 = layer->filter()->getString("md5", "");
            const QString patternNameTemp = layer->filter()->getString("pattern", "Grid01.pat");
            const QString patternFileName = layer->filter()->getString("fileName", "");

            KoResourceLoadResult res = KisGlobalResourcesInterface::instance()->source(ResourceType::Patterns).bestMatchLoadResult(patternMD5, patternFileName, patternNameTemp);
            QVERIFY(res.resource<KoPattern>());
        }
        QVERIFY(layer->filter()->getDouble("transform_rotation_z") - 30.85 < 0.001);
        QVERIFY(layer->filter()->getDouble("transform_scale_x") - 3.63 < 0.001);
        QVERIFY(layer->filter()->getDouble("transform_scale_y") - 3.63 < 0.001);

        QFileInfo sourceFileInfo6(QString(FILES_DATA_DIR) + '/' + "sources/pattern4_rle.psd");

        Q_ASSERT(sourceFileInfo6.exists());

        doc = openPsdDocument(sourceFileInfo6);
        QVERIFY(doc->image());
        layer = qobject_cast<KisGeneratorLayer*>(doc->image()->root()->lastChild().data());
        QVERIFY(layer);
        QVERIFY(layer->filter()->name() == "pattern");
        if (layer) {
            const QString patternMD5 = layer->filter()->getString("md5", "");
            const QString patternNameTemp = layer->filter()->getString("pattern", "Grid01.pat");
            const QString patternFileName = layer->filter()->getString("fileName", "");

            KoResourceLoadResult res = KisGlobalResourcesInterface::instance()->source(ResourceType::Patterns).bestMatchLoadResult(patternMD5, patternFileName, patternNameTemp);
            QVERIFY(res.resource<KoPattern>());
        }
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

    KoPatternSP pattern =
        layer->layerStyle()->patternOverlay()->pattern(
                layer->layerStyle()->resourcesInterface());

    QVERIFY(pattern);
    QVERIFY(pattern->valid());
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
    {
        KoPatternSP pattern =
            layer->layerStyle()->patternOverlay()->pattern(
                    layer->layerStyle()->resourcesInterface());

        QVERIFY(pattern);
        QVERIFY(pattern->valid());
    }

    QVERIFY(layer->layerStyle()->stroke());
    QVERIFY(layer->layerStyle()->stroke()->effectEnabled());
    {
        KoPatternSP pattern =
            layer->layerStyle()->stroke()->pattern(
                layer->layerStyle()->resourcesInterface());

        QVERIFY(pattern);
        QVERIFY(pattern->valid());
    }
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
    {
        KoPatternSP pattern =
            layer->layerStyle()->patternOverlay()->pattern(
                    layer->layerStyle()->resourcesInterface());

        QVERIFY(pattern);
        QVERIFY(pattern->valid());
    }

    QVERIFY(layer->layerStyle()->stroke());
    QVERIFY(layer->layerStyle()->stroke()->effectEnabled());
    {
        KoPatternSP pattern =
            layer->layerStyle()->stroke()->pattern(
                    layer->layerStyle()->resourcesInterface());

        QVERIFY(pattern);
        QVERIFY(pattern->valid());
    }

    doc->setFileBatchMode(true);
    const QByteArray mimeType("image/vnd.adobe.photoshop");
    QFileInfo dstFileInfo(QDir::currentPath() + '/' + "test_save_styles.psd");
    bool retval = doc->exportDocumentSync(dstFileInfo.absoluteFilePath(), mimeType);
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
        {
            KoPatternSP pattern =
                layer->layerStyle()->patternOverlay()->pattern(
                        layer->layerStyle()->resourcesInterface());

            QVERIFY(pattern);
            QVERIFY(pattern->valid());
        }

        QVERIFY(layer->layerStyle()->stroke());
        QVERIFY(layer->layerStyle()->stroke()->effectEnabled());
        {
            KoPatternSP pattern =
                layer->layerStyle()->stroke()->pattern(
                        layer->layerStyle()->resourcesInterface());

            QVERIFY(pattern);
            QVERIFY(pattern->valid());
        }
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

        bool retval = doc->exportDocumentSync(dstFileInfo.absoluteFilePath(), "image/vnd.adobe.photoshop");
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

