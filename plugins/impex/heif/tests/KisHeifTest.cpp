/*
 * SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 * SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisHeifTest.h"


#include <simpletest.h>
#include <QCoreApplication>
#include <KoColorSpaceEngine.h>

#include "filestest.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif


const QString HeifMimetype = "image/heic";
const QString AvifMimetype = "image/avif";



void KisHeifTest::testImportFromWriteonly()
{
    TestUtil::testImportFromWriteonly(QString(FILES_DATA_DIR), HeifMimetype);
    TestUtil::testImportFromWriteonly(QString(FILES_DATA_DIR), AvifMimetype);
}


void KisHeifTest::testExportToReadonly()
{
    TestUtil::testExportToReadonly(QString(FILES_DATA_DIR), HeifMimetype);
    TestUtil::testExportToReadonly(QString(FILES_DATA_DIR), AvifMimetype);
}


void KisHeifTest::testImportIncorrectFormat()
{
    TestUtil::testImportIncorrectFormat(QString(FILES_DATA_DIR), HeifMimetype);
    TestUtil::testImportIncorrectFormat(QString(FILES_DATA_DIR), AvifMimetype);
}

void KisHeifTest::testLoadMonochrome(int bitDepth)
{
    {
        QString file = QString("test_monochrome_%1.").arg(bitDepth);
        int error = 2;
        if (bitDepth == 10) {
            file = QString(FILES_DATA_DIR) + file;
            error = 25;
        }
        qDebug() << "Loading test for" << file;
        QScopedPointer<KisDocument> doc_png(KisPart::instance()->createDocument());
        doc_png->setFileBatchMode(true);
        QScopedPointer<KisDocument> doc_heif(KisPart::instance()->createDocument());
        doc_heif->setFileBatchMode(true);
        QScopedPointer<KisDocument> doc_avif(KisPart::instance()->createDocument());
        doc_avif->setFileBatchMode(true);

        KisImportExportManager manager(doc_png.data());

        KisImportExportErrorCode loadingStatus =
            manager.importDocument(file+"png", QString());

        QVERIFY(loadingStatus.isOk());
        KisImportExportManager (doc_heif.data()).importDocument(file+"heif", QString());
        KisImportExportManager (doc_avif.data()).importDocument(file+"avif", QString());

        doc_png->image()->initialRefreshGraph();
        doc_heif->image()->initialRefreshGraph();
        doc_avif->image()->initialRefreshGraph();

        KoColor pngColor;
        KoColor heifColor;
        KoColor avifColor;

        for (int y = 0; y<doc_png->image()->height(); y++) {
            for (int x = 0; x<doc_png->image()->width(); x++) {
                doc_png->image()->projection()->pixel(x, y, &pngColor);
                doc_heif->image()->projection()->pixel(x, y, &heifColor);
                doc_avif->image()->projection()->pixel(x, y, &avifColor);

                QVERIFY2(pngColor.colorSpace()->differenceA(pngColor.data(), heifColor.data()) < error,
                         QString("Heif %5 gray color doesn't match PNG color, (%1, %2) %3 %4")
                         .arg(x).arg(y).arg(KoColor::toQString(pngColor)).arg(KoColor::toQString(heifColor)).arg(bitDepth).toLatin1());
                QVERIFY2(pngColor.colorSpace()->differenceA(pngColor.data(), avifColor.data()) < error,
                         QString("Avif %5 gray color doesn't match PNG color, (%1, %2) %3 %4")
                         .arg(x).arg(y).arg(pngColor.toXML()).arg(avifColor.toXML()).arg(bitDepth).toLatin1());
                QVERIFY2(avifColor.colorSpace()->differenceA(avifColor.data(), heifColor.data()) < error,
                         QString("Heif %5 gray color doesn't match Avif color, (%1, %2) %3 %4")
                         .arg(x).arg(y).arg(heifColor.toXML()).arg(heifColor.toXML()).arg(bitDepth).toLatin1());
            }
        }
    }
}

void KisHeifTest::testLoadRGB(int bitDepth)
{
    {
        QString file = QString("test_rgba_%1.").arg(bitDepth);
        int error = 2;
        if (bitDepth == 10) {
            file = QString(FILES_DATA_DIR) + file;
            error = 25;
        }
        qDebug() << "Loading test for" << file;
        QScopedPointer<KisDocument> doc_png(KisPart::instance()->createDocument());
        doc_png->setFileBatchMode(true);
        QScopedPointer<KisDocument> doc_heif(KisPart::instance()->createDocument());
        doc_heif->setFileBatchMode(true);
        QScopedPointer<KisDocument> doc_avif(KisPart::instance()->createDocument());
        doc_avif->setFileBatchMode(true);

        KisImportExportManager manager(doc_png.data());

        KisImportExportErrorCode loadingStatus =
            manager.importDocument(file+"png", QString());

        QVERIFY(loadingStatus.isOk());
        KisImportExportManager (doc_heif.data()).importDocument(file+"heif", QString());
        KisImportExportManager (doc_avif.data()).importDocument(file+"avif", QString());

        doc_png->image()->initialRefreshGraph();
        doc_heif->image()->initialRefreshGraph();
        doc_avif->image()->initialRefreshGraph();

        KoColor pngColor(doc_png->image()->colorSpace());
        KoColor heifColor(doc_png->image()->colorSpace());
        KoColor avifColor(doc_png->image()->colorSpace());

        for (int y = 0; y<doc_png->image()->height(); y++) {
            for (int x = 0; x<doc_png->image()->width(); x++) {
                doc_png->image()->projection()->pixel(x, y, &pngColor);
                doc_heif->image()->projection()->pixel(x, y, &heifColor);
                doc_avif->image()->projection()->pixel(x, y, &avifColor);

                QVERIFY2(pngColor.colorSpace() == heifColor.colorSpace(), QString("%1 RGBA colorspace mismatch between png and heif").toLatin1());
                QVERIFY2(pngColor.colorSpace() == avifColor.colorSpace(), QString("%1 RGBA colorspace mismatch between png and avif").toLatin1());
                QVERIFY2(avifColor.colorSpace() == heifColor.colorSpace(), QString("%1 RGBA colorspace mismatch between avif and heif").toLatin1());

                QVERIFY2(pngColor.colorSpace()->differenceA(pngColor.data(), heifColor.data()) < error, QString("%5 RGBA Heif color doesn't match PNG color, (%1, %2) %3 %4")
                         .arg(x).arg(y).arg(pngColor.toXML()).arg(heifColor.toXML()).arg(bitDepth).toLatin1());
                QVERIFY2(pngColor.colorSpace()->differenceA(pngColor.data(), avifColor.data()) < error, QString("%5 RGBA Avif color doesn't match PNG color, (%1, %2) %3 %4")
                         .arg(x).arg(y).arg(pngColor.toXML()).arg(avifColor.toXML()).arg(bitDepth).toLatin1());
                QVERIFY2(avifColor.colorSpace()->differenceA(avifColor.data(), heifColor.data()) < error, QString("%5 RGBA Heif color doesn't match Avif color, (%1, %2) %3 %4")
                         .arg(x).arg(y).arg(heifColor.toXML()).arg(heifColor.toXML()).arg(bitDepth).toLatin1());
            }
        }
    }
}

void KisHeifTest::testSaveHDR()
{
    const KoColorSpace * cs =
        KoColorSpaceRegistry::instance()->colorSpace(
            RGBAColorModelID.id(),
            Float32BitsColorDepthID.id(),
                KoColorSpaceRegistry::instance()->p2020G10Profile());

    KoColor fillColor(cs);

    int blockSize = 32;
    int width  = blockSize * 2;
    int height = blockSize * 2;

    {
        QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());

        KisImageSP image = new KisImage(0, width, height, cs, "png test");
        KisPaintLayerSP paintLayer0 = new KisPaintLayer(image, "paint0", OPACITY_OPAQUE_U8);
        QVector<float> channelValues(4);

        float total = float((blockSize-1) * (blockSize-1));

        for (int y=0; y<blockSize; y++) {
            for (int x=0; x<blockSize; x++) {
                float value = float(x * y);

                channelValues[0] = value;
                channelValues[1] = total-value;
                channelValues[2] = total-value;
                channelValues[3] = 1.0;
                cs->fromNormalisedChannelsValue(fillColor.data(), channelValues);
                paintLayer0->paintDevice()->setPixel(x, y, fillColor);

                channelValues[1] = value;
                channelValues[0] = total-value;
                channelValues[2] = total-value;
                channelValues[3] = 1.0;
                cs->fromNormalisedChannelsValue(fillColor.data(), channelValues);
                paintLayer0->paintDevice()->setPixel(width - (x+1), y, fillColor);

                channelValues[2] = value;
                channelValues[0] = total-value;
                channelValues[1] = total-value;
                channelValues[3] = 1.0;
                cs->fromNormalisedChannelsValue(fillColor.data(), channelValues);
                paintLayer0->paintDevice()->setPixel(x, height - (y+1), fillColor);

                channelValues[3] = value/total;
                channelValues[0] = 0.5;
                channelValues[1] = 0.5;
                channelValues[2] = 0.5;
                cs->fromNormalisedChannelsValue(fillColor.data(), channelValues);
                paintLayer0->paintDevice()->setPixel(width - (x+1), height - (y+1), fillColor);
            }
        }

        image->addNode(paintLayer0, image->root());

        image->waitForDone();

        KisImportExportManager manager(doc.data());
        doc->setFileBatchMode(true);
        doc->setCurrentImage(image);

        qDebug() << "Saving HDR PNG";

        KisPropertiesConfigurationSP pngExportConfiguration = new KisPropertiesConfiguration();
        pngExportConfiguration->setProperty("saveAsHDR", true);
        pngExportConfiguration->setProperty("saveSRGBProfile", true);
        pngExportConfiguration->setProperty("forceSRGB", false);
        doc->exportDocumentSync(QUrl::fromLocalFile(QString("test_rgba_hdr.png")), "image/png", pngExportConfiguration);

        qDebug() << "Saving HDR heif and avif for PQ";
        KisPropertiesConfigurationSP heifExportConfiguration = new KisPropertiesConfiguration();
        heifExportConfiguration->setProperty("quality", 100);
        heifExportConfiguration->setProperty("lossless", true);
        heifExportConfiguration->setProperty("chroma", "444");
        heifExportConfiguration->setProperty("floatingPointConversionOption", "Rec2100PQ");
        doc->exportDocumentSync(QUrl::fromLocalFile(QString("test_rgba_hdr_pq.heif")), "image/heic", heifExportConfiguration);
        doc->exportDocumentSync(QUrl::fromLocalFile(QString("test_rgba_hdr_pq.avif")), "image/avif", heifExportConfiguration);

        qDebug() << "Saving HDR heif and avif for HLG";
        heifExportConfiguration->setProperty("HLGnominalPeak", 1000.0);
        heifExportConfiguration->setProperty("HLGgamma", 1.2);
        heifExportConfiguration->setProperty("removeHGLOOTF", true);
        heifExportConfiguration->setProperty("floatingPointConversionOption", "Rec2100HLG");
        doc->exportDocumentSync(QUrl::fromLocalFile(QString("test_rgba_hdr_hlg.heif")), "image/heic", heifExportConfiguration);
        doc->exportDocumentSync(QUrl::fromLocalFile(QString("test_rgba_hdr_hlg.avif")), "image/avif", heifExportConfiguration);

        qDebug() << "Saving HDR heif and avif for SMPTE 428";
        heifExportConfiguration->setProperty("floatingPointConversionOption", "ApplySMPTE428");
        doc->exportDocumentSync(QUrl::fromLocalFile(QString("test_rgba_hdr_smpte428.heif")), "image/heic", heifExportConfiguration);
        doc->exportDocumentSync(QUrl::fromLocalFile(QString("test_rgba_hdr_smpte428.avif")), "image/avif", heifExportConfiguration);
    }
}

void KisHeifTest::testLoadHDR()
{
    {
        QScopedPointer<KisDocument> doc_png(KisPart::instance()->createDocument());
        doc_png->setFileBatchMode(true);
        QScopedPointer<KisDocument> doc_avif_pq(KisPart::instance()->createDocument());
        doc_avif_pq->setFileBatchMode(true);
        QScopedPointer<KisDocument> doc_heif_pq(KisPart::instance()->createDocument());
        doc_heif_pq->setFileBatchMode(true);

        QScopedPointer<KisDocument> doc_avif_hlg(KisPart::instance()->createDocument());
        doc_avif_hlg->setFileBatchMode(true);
        QScopedPointer<KisDocument> doc_heif_hlg(KisPart::instance()->createDocument());
        doc_heif_hlg->setFileBatchMode(true);

        QScopedPointer<KisDocument> doc_avif_smpte428(KisPart::instance()->createDocument());
        doc_avif_smpte428->setFileBatchMode(true);
        QScopedPointer<KisDocument> doc_heif_smpte428(KisPart::instance()->createDocument());
        doc_heif_smpte428->setFileBatchMode(true);

        KisImportExportManager manager(doc_png.data());

        KisImportExportErrorCode loadingStatus =
            manager.importDocument(QString("test_rgba_hdr.png"), QString());

        QVERIFY(loadingStatus.isOk());
        qDebug() << "Loading test for PQ files";
        KisImportExportManager (doc_avif_pq.data()).importDocument(QString("test_rgba_hdr_pq.avif"), QString());
        KisImportExportManager (doc_heif_pq.data()).importDocument(QString("test_rgba_hdr_pq.heif"), QString());

        qDebug() << "Loading test for HLG files";
        KisImportExportManager (doc_avif_hlg.data()).importDocument(QString("test_rgba_hdr_hlg.avif"), QString());
        KisImportExportManager (doc_heif_hlg.data()).importDocument(QString("test_rgba_hdr_hlg.heif"), QString());

        qDebug() << "Loading test for smpte428 files";
        KisImportExportManager (doc_avif_smpte428.data()).importDocument(QString("test_rgba_hdr_smpte428.avif"), QString());
        KisImportExportManager (doc_heif_smpte428.data()).importDocument(QString("test_rgba_hdr_smpte428.heif"), QString());

        doc_png->image()->initialRefreshGraph();
        doc_avif_pq->image()->initialRefreshGraph();
        doc_heif_pq->image()->initialRefreshGraph();

        doc_avif_hlg->image()->initialRefreshGraph();
        doc_heif_hlg->image()->initialRefreshGraph();

        doc_avif_smpte428->image()->initialRefreshGraph();
        doc_heif_smpte428->image()->initialRefreshGraph();

        const KoColorSpace * cs =
            KoColorSpaceRegistry::instance()->colorSpace(
                RGBAColorModelID.id(),
                Float32BitsColorDepthID.id(),
                    KoColorSpaceRegistry::instance()->p2020G10Profile());
        doc_png->image()->convertImageColorSpace(cs, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
        doc_png->image()->waitForDone();

        KoColor pngColor(cs);
        KoColor heifColor(cs);
        KoColor avifColor(cs);

        for (int y = 0; y<doc_png->image()->height(); y++) {
            for (int x = 0; x<doc_png->image()->width(); x++) {
                doc_png->image()->projection()->pixel(x, y, &pngColor);
                doc_heif_pq->image()->projection()->pixel(x, y, &heifColor);
                doc_avif_pq->image()->projection()->pixel(x, y, &avifColor);

                QVERIFY2(cs->difference(pngColor.data(), avifColor.data()) <1, QString("Avif PQ color doesn't match PNG color, (%1, %2) %3 %4")
                         .arg(x).arg(y).arg(pngColor.toXML()).arg(avifColor.toXML()).toLatin1());

                QVERIFY2(cs->difference(pngColor.data(), heifColor.data()) <1, QString("Heif PQ color doesn't match PNG color, (%1, %2) %3 %4")
                         .arg(x).arg(y).arg(pngColor.toXML()).arg(heifColor.toXML()).toLatin1());

                doc_heif_hlg->image()->projection()->pixel(x, y, &heifColor);
                doc_avif_hlg->image()->projection()->pixel(x, y, &avifColor);

                QVERIFY2(cs->difference(heifColor.data(), avifColor.data()) <1, QString("Avif HLG color doesn't match heif color, (%1, %2) %3 %4")
                         .arg(x).arg(y).arg(heifColor.toXML()).arg(avifColor.toXML()).toLatin1());

                doc_heif_smpte428->image()->projection()->pixel(x, y, &heifColor);
                doc_avif_smpte428->image()->projection()->pixel(x, y, &avifColor);

                QVERIFY2(cs->difference(heifColor.data(), avifColor.data()) <1, QString("Avif smpte428 color doesn't match heif color, (%1, %2) %3 %4")
                         .arg(x).arg(y).arg(heifColor.toXML()).arg(avifColor.toXML()).toLatin1());
            }
        }
    }
}

void KisHeifTest::testSaveMonochrome(int bitDepth)
{
    qDebug() << "Testing saving monochrome for" << bitDepth;
    QString depth = Integer8BitsColorDepthID.id();
    if (bitDepth != 8) {
        depth = Integer16BitsColorDepthID.id();
    }

    const KoColorSpace * cs =
        KoColorSpaceRegistry::instance()->colorSpace(
            GrayAColorModelID.id(),
            depth);

    KoColor fillColor(cs);

    int blockSize = 32;
    int width  = blockSize * 2;
    int height = blockSize;

    {
        QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());

        KisImageSP image = new KisImage(0, width, height, cs, "png test");
        KisPaintLayerSP paintLayer0 = new KisPaintLayer(image, "paint0", OPACITY_OPAQUE_U8);
        QVector<float> channelValues(2);

        for (int y=0; y<blockSize; y++) {
            for (int x=0; x<blockSize; x++) {
                float value = float(x * y) / float((blockSize-1) * (blockSize-1));

                channelValues[0] = value;
                channelValues[1] = 1.0;
                cs->fromNormalisedChannelsValue(fillColor.data(), channelValues);

                paintLayer0->paintDevice()->setPixel(x, y, fillColor);
                channelValues[1] = value;
                channelValues[0] = 0.5;
                cs->fromNormalisedChannelsValue(fillColor.data(), channelValues);
                paintLayer0->paintDevice()->setPixel(width - (x+1), y, fillColor);
            }
        }

        image->addNode(paintLayer0, image->root());

        image->waitForDone();

        KisImportExportManager manager(doc.data());
        doc->setFileBatchMode(true);
        doc->setCurrentImage(image);

        KisPropertiesConfigurationSP pngExportConfiguration = new KisPropertiesConfiguration();
        pngExportConfiguration->setProperty("saveAsHDR", false);
        pngExportConfiguration->setProperty("saveSRGBProfile", false);
        pngExportConfiguration->setProperty("forceSRGB", false);

        KisPropertiesConfigurationSP heifExportConfiguration = new KisPropertiesConfiguration();
        heifExportConfiguration->setProperty("quality", 100);
        heifExportConfiguration->setProperty("lossless", true);
        doc->exportDocumentSync(QUrl::fromLocalFile(QString("test_monochrome_%1.png").arg(bitDepth)), "image/png", pngExportConfiguration);
        doc->exportDocumentSync(QUrl::fromLocalFile(QString("test_monochrome_%1.heif").arg(bitDepth)), "image/heic", heifExportConfiguration);
        doc->exportDocumentSync(QUrl::fromLocalFile(QString("test_monochrome_%1.avif").arg(bitDepth)), "image/avif", heifExportConfiguration);

    }
}

void KisHeifTest::testSaveRGB(int bitDepth)
{
    qDebug() << "Testing saving RGBA for" << bitDepth;
    QString depth = Integer8BitsColorDepthID.id();
    if (bitDepth != 8) {
        depth = Integer16BitsColorDepthID.id();
    }

    QString profileName;

    QVector<double> colorants;
    const KoColorProfile *testProfile = KoColorSpaceRegistry::instance()->profileFor(colorants
                                                                                     , PRIMARIES_SMPTE_240M
                                                                                     , TRC_SMPTE_240M);
    if (testProfile) {
        profileName = testProfile->name();
    }

    const KoColorSpace * cs =
        KoColorSpaceRegistry::instance()->colorSpace(
            RGBAColorModelID.id(),
            depth, profileName);

    KoColor fillColor(cs);

    int blockSize = 32;
    int width  = blockSize * 2;
    int height = blockSize * 2;

    {
        QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());

        KisImageSP image = new KisImage(0, width, height, cs, "png test");
        KisPaintLayerSP paintLayer0 = new KisPaintLayer(image, "paint0", OPACITY_OPAQUE_U8);
        QVector<float> channelValues(4);

        for (int y=0; y<blockSize; y++) {
            for (int x=0; x<blockSize; x++) {
                float value = float(x * y) / float((blockSize-1) * (blockSize-1));

                channelValues[0] = value;
                channelValues[1] = 1.0-value;
                channelValues[2] = 1.0-value;
                channelValues[3] = 1.0;
                cs->fromNormalisedChannelsValue(fillColor.data(), channelValues);
                paintLayer0->paintDevice()->setPixel(x, y, fillColor);

                channelValues[1] = value;
                channelValues[0] = 1.0-value;
                channelValues[2] = 1.0-value;
                channelValues[3] = 1.0;
                cs->fromNormalisedChannelsValue(fillColor.data(), channelValues);
                paintLayer0->paintDevice()->setPixel(width - (x+1), y, fillColor);

                channelValues[2] = value;
                channelValues[0] = 1.0-value;
                channelValues[1] = 1.0-value;
                channelValues[3] = 1.0;
                cs->fromNormalisedChannelsValue(fillColor.data(), channelValues);
                paintLayer0->paintDevice()->setPixel(x, height - (y+1), fillColor);

                channelValues[3] = value;
                channelValues[0] = 0.5;
                channelValues[1] = 0.5;
                channelValues[2] = 0.5;
                cs->fromNormalisedChannelsValue(fillColor.data(), channelValues);
                paintLayer0->paintDevice()->setPixel(width - (x+1), height - (y+1), fillColor);
            }
        }

        image->addNode(paintLayer0, image->root());

        image->waitForDone();

        KisImportExportManager manager(doc.data());
        doc->setFileBatchMode(true);
        doc->setCurrentImage(image);

        KisPropertiesConfigurationSP pngExportConfiguration = new KisPropertiesConfiguration();
        pngExportConfiguration->setProperty("saveAsHDR", false);
        pngExportConfiguration->setProperty("saveSRGBProfile", true);
        pngExportConfiguration->setProperty("forceSRGB", false);
        doc->exportDocumentSync(QUrl::fromLocalFile(QString("test_rgba_%1.png").arg(bitDepth)), "image/png", pngExportConfiguration);

        KisPropertiesConfigurationSP heifExportConfiguration = new KisPropertiesConfiguration();
        heifExportConfiguration->setProperty("quality", 100);
        heifExportConfiguration->setProperty("lossless", true);
        heifExportConfiguration->setProperty("chroma", "444");
        doc->exportDocumentSync(QUrl::fromLocalFile(QString("test_rgba_%1.heif").arg(bitDepth)), "image/heic", heifExportConfiguration);
        doc->exportDocumentSync(QUrl::fromLocalFile(QString("test_rgba_%1.avif").arg(bitDepth)), "image/avif", heifExportConfiguration);
    }
}

void KisHeifTest::testImages()
{
    testSaveMonochrome(8);
    testSaveMonochrome(12);
    testSaveRGB(8);
    testSaveRGB(12);
    testLoadRGB(8);
    testLoadRGB(10);
    testLoadRGB(12);
    testLoadMonochrome(8);
    testLoadMonochrome(10);
    testLoadMonochrome(12);
}



KISTEST_MAIN(KisHeifTest)


