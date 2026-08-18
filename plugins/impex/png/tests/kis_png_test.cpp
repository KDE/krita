/*
 * SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_png_test.h"


#include <simpletest.h>
#include <QCoreApplication>

#include <KoColorProfileQuery.h>
#include <kis_hdr_metadata.h>
#include "filestest.h"

#include <testui.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif


const QString PngMimetype = "image/png";

void KisPngTest::testFiles()
{
    TestUtil::testFiles(QString(FILES_DATA_DIR) + "/sources", QStringList(), QString(), 1);
}

void KisPngTest::testWriteonly()
{
    TestUtil::testImportFromWriteonly(PngMimetype);
}

void roudTripHdrImage(const KoColorSpace *savingColorSpace)
{
    const KoColorSpace * scRGBF32 =
        KoColorSpaceRegistry::instance()->colorSpace(
            RGBAColorModelID.id(),
            Float32BitsColorDepthID.id(),
            KoColorSpaceRegistry::instance()->p709G10Profile());

    KoColor fillColor(scRGBF32);
    float *pixelPtr = reinterpret_cast<float*>(fillColor.data());

    pixelPtr[0] = 2.7;
    pixelPtr[1] = 1.6;
    pixelPtr[2] = 0.8;
    pixelPtr[3] = 0.9;

    {
        QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());

        KisImageSP image = new KisImage(0, 3, 3, scRGBF32, "png test");
        KisPaintLayerSP paintLayer0 = new KisPaintLayer(image, "paint0", OPACITY_OPAQUE_U8);
        paintLayer0->paintDevice()->fill(image->bounds(), fillColor);
        image->addNode(paintLayer0, image->root());

        // convert image color space before saving
        image->convertImageColorSpace(savingColorSpace, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
        image->waitForDone();

        KisImportExportManager manager(doc.data());
        doc->setFileBatchMode(true);
        doc->setCurrentImage(image);

        KisPropertiesConfigurationSP exportConfiguration = new KisPropertiesConfiguration();
        exportConfiguration->setProperty("floatingPointConversionOption", "Rec2100PQ");
        exportConfiguration->setProperty("writeCicpIfPossible", true);
        exportConfiguration->setProperty("storeColorSpaceInfo", true);
        exportConfiguration->setProperty("forceSRGB", false);
        doc->exportDocumentSync("test.png", "image/png", exportConfiguration);
    }

    {
        QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());
        KisImportExportManager manager(doc.data());
        doc->setFileBatchMode(true);

        KisImportExportErrorCode loadingStatus =
            manager.importDocument("test.png", QString());

        QVERIFY(loadingStatus.isOk());

        KisImageSP image = doc->image();
        image->initialRefreshGraph();

        KoColor resultColor;

//        qDebug() << ppVar(image->colorSpace()) << image->colorSpace()->profile()->name();
//        image->projection()->pixel(1, 1, &resultColor);
//        qDebug() << ppVar(resultColor);

        image->convertImageColorSpace(scRGBF32, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
        image->waitForDone();

        image->projection()->pixel(1, 1, &resultColor);
//        qDebug() << ppVar(resultColor);

        const float tolerance = savingColorSpace->colorDepthId() == Integer8BitsColorDepthID ? 0.02 : 0.01;
        bool resultIsValid = true;
        float *resultPtr = reinterpret_cast<float*>(resultColor.data());
        for (int i = 0; i < 4; i++) {
            resultIsValid &= qAbs(resultPtr[i] - pixelPtr[i]) < tolerance;
        }

        if (!resultIsValid) {
            qDebug() << ppVar(fillColor) << ppVar(resultColor);
        }
        QVERIFY(resultIsValid);
    }
}

void KisPngTest::testSaveHDR()
{
    QVector<KoID> colorDepthIds;
#ifdef HAVE_OPENEXR
    colorDepthIds << Float16BitsColorDepthID;
#endif
    colorDepthIds << Float32BitsColorDepthID;

    QVector<const KoColorProfile*> profiles;
    const KoColorProfile *profile = KoColorSpaceRegistry::instance()->p709G10Profile();
    if (!profile) {
        qWarning() << "Could not get a p709G10 Profile";
    }
    else {
        profiles << profile;
    }
    profile = KoColorSpaceRegistry::instance()->p2020G10Profile();
    if (!profile) {
        qWarning() << "Could not get a p2020G10 Profile";
    }
    else {
        profiles << profile;
    }
    profile = KoColorSpaceRegistry::instance()->p2020PQProfile();;
    if (!profile) {
        qWarning() << "Could not get a p2020PQ Profile";
    }
    else {
        profiles << profile;
    }

    Q_FOREACH(const KoID &depth, colorDepthIds) {
        Q_FOREACH(const KoColorProfile *profile, profiles) {
            if (profile) {
                roudTripHdrImage(
                    KoColorSpaceRegistry::instance()->colorSpace(
                                RGBAColorModelID.id(),
                                depth.id(),
                                profile));
            }
        }
    }

    roudTripHdrImage(
        KoColorSpaceRegistry::instance()->colorSpace(
                    RGBAColorModelID.id(),
                    Integer16BitsColorDepthID.id(),
                    KoColorSpaceRegistry::instance()->p2020PQProfile()));

    roudTripHdrImage(
        KoColorSpaceRegistry::instance()->colorSpace(
                    RGBAColorModelID.id(),
                    Integer8BitsColorDepthID.id(),
                    KoColorSpaceRegistry::instance()->p2020PQProfile()));
}

Q_DECLARE_METATYPE(ColorPrimaries)
Q_DECLARE_METATYPE(TransferCharacteristics)

void KisPngTest::testRoundtripCicpIccProfile_data()
{
    QTest::addColumn<ColorPrimaries>("primaries");
    QTest::addColumn<TransferCharacteristics>("transfer");
    QTest::addColumn<bool>("cicpOnly");

    QTest::addRow("rec2020") << PRIMARIES_ITU_R_BT_2020_2_AND_2100_0 << TRC_ITU_R_BT_2020_2_12bit << false;
    QTest::addRow("rec2100 PQ") << PRIMARIES_ITU_R_BT_2020_2_AND_2100_0 << TRC_ITU_R_BT_2100_0_PQ << false;
    QTest::addRow("rec709") << PRIMARIES_ITU_R_BT_709_5 << TRC_ITU_R_BT_709_5 << false;

    QTest::addRow("rec2020 cicp") << PRIMARIES_ITU_R_BT_2020_2_AND_2100_0 << TRC_ITU_R_BT_2020_2_12bit << true;
    QTest::addRow("rec2100 PQ cicp") << PRIMARIES_ITU_R_BT_2020_2_AND_2100_0 << TRC_ITU_R_BT_2100_0_PQ << true;
    QTest::addRow("rec709 cicp") << PRIMARIES_ITU_R_BT_709_5 << TRC_ITU_R_BT_709_5 << true;
}

void KisPngTest::testRoundtripCicpIccProfile()
{
    QFETCH(ColorPrimaries, primaries);
    QFETCH(TransferCharacteristics, transfer);
    QFETCH(bool, cicpOnly);
    QVector<double> colorants;
    const KoColorProfile *profile = KoColorSpaceRegistry::instance()->profileFor(KoColorProfileQuery(primaries, transfer));

    QRect imageRect(0,0,512,512);
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), profile);

    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());
    doc->setFileBatchMode(true);
    KisImageSP image = new KisImage(new KisSurrogateUndoStore(), imageRect.width(), imageRect.height(), cs, "test image");
    doc->setCurrentImage(image);
    const QString cicpOnlyString = cicpOnly? "_cicp_only": QString();
    const QString name = "roundtrip_cicp_test_"+QString::number(primaries)+"_"+QString::number(transfer)+cicpOnlyString+".png";

    KisPropertiesConfigurationSP exportConfiguration = new KisPropertiesConfiguration();
    exportConfiguration->setProperty("saveSRGBProfile", false);
    exportConfiguration->setProperty("forceSRGB", false);
    exportConfiguration->setProperty("storeColorSpaceInfo", true);
    exportConfiguration->setProperty("writeCicpIfPossible", cicpOnly);
    doc->exportDocumentSync(name, "image/png", exportConfiguration);

    //---//

    QScopedPointer<KisDocument> doc2(KisPart::instance()->createDocument());
    KisImportExportManager manager(doc2.data());
    doc2->setFileBatchMode(true);

    KisImportExportErrorCode loadingStatus =
        manager.importDocument(name, QString());

    QVERIFY(loadingStatus.isOk());

    KisImageSP image2 = doc2->image();
    image2->initialRefreshGraph();
    image2->waitForDone();
    QVERIFY(image2->colorSpace()->profile()->getColorPrimaries() == primaries);
    QVERIFY(image2->colorSpace()->profile()->getTransferCharacteristics() == transfer);
}

void KisPngTest::testLoadPngWithLegacyProfile()
{
    const QString pngFilePath = TestUtil::fetchDataFileLazy("test-png-with-legacy-hdr-profile.png");
    KIS_ASSERT(QFile::exists(pngFilePath));

    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());
    KisImportExportManager manager(doc.data());
    doc->setFileBatchMode(true);

    KisImportExportErrorCode loadingStatus =
        manager.importDocument(pngFilePath, QString());

    QVERIFY(loadingStatus.isOk());

    /**
     * A PNG with the legacy profile should use the default Krita profille
     * for a PQ space with all the cicp information set properly. The embedded
     * profile should be ignored.
     */
    const KoColorProfile *profile = doc->image()->colorSpace()->profile();

    QCOMPARE(profile->name(), KoColorSpaceRegistry::instance()->p2020PQProfile()->name());
    QCOMPARE(profile->getColorPrimaries(), PRIMARIES_ITU_R_BT_2020_2_AND_2100_0);
    QCOMPARE(profile->getTransferCharacteristics(), TRC_ITU_R_BT_2100_0_PQ);
}

void KisPngTest::testRoundtripHDRMetadata()
{
    const KoColorProfile *profile = KoColorSpaceRegistry::instance()->p2020PQProfile();

    QRect imageRect(0,0,512,512);
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), profile);

    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());
    doc->setFileBatchMode(true);
    KisImageSP image = new KisImage(new KisSurrogateUndoStore(), imageRect.width(), imageRect.height(), cs, "test image");

    KisRelativeContentLightLevelInformation clli;
    clli.maxFrameAverageLightLevel = 12.3456;
    clli.maxContentLightLevel = 6.54321;
    KisColorVolumeInformation cvi;
    cvi.white.x = 0.3;
    cvi.white.y = 0.3;
    cvi.red.x = 0.3;
    cvi.red.y = 0.3;
    cvi.green.x = 0.3;
    cvi.green.y = 0.3;
    cvi.blue.x = 0.3;
    cvi.blue.y = 0.3;

    image->setRelativeContentLightLevelInformation(clli);
    image->setColorVolumeInformation(cvi);

    doc->setCurrentImage(image);
    const QString name = "roundtrip_hdr_metadata.png";

    KisPropertiesConfigurationSP exportConfiguration = new KisPropertiesConfiguration();
    exportConfiguration->setProperty("saveSRGBProfile", false);
    exportConfiguration->setProperty("forceSRGB", false);
    doc->exportDocumentSync(name, "image/png", exportConfiguration);

    QScopedPointer<KisDocument> doc2(KisPart::instance()->createDocument());
    KisImportExportManager manager(doc2.data());
    doc2->setFileBatchMode(true);

    KisImportExportErrorCode loadingStatus =
        manager.importDocument(name, QString());

    QVERIFY(loadingStatus.isOk());

    KisImageSP image2 = doc2->image();
    image2->initialRefreshGraph();
    image2->waitForDone();

    QVERIFY(image2->relativeContentLightLevelInformation());
    QVERIFY(image2->colorVolumeInformation());

    QVERIFY(qAbs(clli.maxContentLightLevel - image2->relativeContentLightLevelInformation()->maxContentLightLevel) < 0.0001);
    QVERIFY(qAbs(clli.maxFrameAverageLightLevel - image2->relativeContentLightLevelInformation()->maxFrameAverageLightLevel) < 0.0001);
    QVERIFY(cvi == image2->colorVolumeInformation());
}

KISTEST_MAIN(KisPngTest)

