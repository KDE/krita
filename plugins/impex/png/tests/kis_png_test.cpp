/*
 * Copyright (C) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_png_test.h"


#include <QTest>
#include <QCoreApplication>

#include "filestest.h"

#include  <sdk/tests/kistest.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif


void KisPngTest::testFiles()
{
    TestUtil::testFiles(QString(FILES_DATA_DIR) + "/sources", QStringList(), QString(), 1);
}

void roudTripHdrImage(const KoColorSpace *savingColorSpace)
{
    qDebug() << "Test saving" << savingColorSpace->id() << savingColorSpace->profile()->name();

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
        exportConfiguration->setProperty("saveAsHDR", true);
        exportConfiguration->setProperty("saveSRGBProfile", false);
        exportConfiguration->setProperty("forceSRGB", false);
        doc->exportDocumentSync(QUrl::fromLocalFile("test.png"), "image/png", exportConfiguration);
    }

    {
        QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());
        KisImportExportManager manager(doc.data());
        doc->setFileBatchMode(true);

        KisImportExportFilter::ConversionStatus loadingStatus =
            manager.importDocument("test.png", QString());

        QCOMPARE(loadingStatus, KisImportExportFilter::OK);

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
    colorDepthIds << Float16BitsColorDepthID;
    colorDepthIds << Float32BitsColorDepthID;

    QVector<const KoColorProfile*> profiles;
    profiles << KoColorSpaceRegistry::instance()->p709G10Profile();
    profiles << KoColorSpaceRegistry::instance()->p2020G10Profile();
    profiles << KoColorSpaceRegistry::instance()->p2020PQProfile();


    Q_FOREACH(const KoID &depth, colorDepthIds) {
        Q_FOREACH(const KoColorProfile *profile, profiles) {
            roudTripHdrImage(
                KoColorSpaceRegistry::instance()->colorSpace(
                            RGBAColorModelID.id(),
                            depth.id(),
                            profile));
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

KISTEST_MAIN(KisPngTest)

