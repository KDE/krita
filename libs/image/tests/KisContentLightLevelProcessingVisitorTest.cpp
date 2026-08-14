/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisContentLightLevelProcessingVisitorTest.h"

#include <KoColorSpaceRegistry.h>
#include "kis_paint_layer.h"
#include "kis_paint_device.h"
#include <kis_group_layer.h>
#include <kis_types.h>
#include <KisContentLightLevelProcessingVistor.h>

#include "kis_undo_stores.h"
#include "kis_image.h"
#include <kis_processing_applicator.h>
#include <kis_hdr_metadata.h>



KisImageSP createImage() {
    KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id());
    KisImageSP image = new KisImage(undoStore, 300, 300, cs, "test");

    QRect fillRect1(50,50,100,100);
    QRect fillRect2(75,75,50,50);
    KisPaintLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisPaintLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);

    paintLayer1->paintDevice()->fill(fillRect1, KoColor(Qt::green, cs));
    paintLayer2->paintDevice()->fill(fillRect2, KoColor(Qt::red, cs));

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(paintLayer2, image->rootLayer());

    image->initialRefreshGraph();
    return image;
}


void KisContentLightLevelProcessingVisitorTest::testRgbColorSpace_data()
{
    QTest::addColumn<int>("type");
    QTest::addColumn<double>("expectedMaxCLL");
    QTest::addColumn<double>("expectedMaxFALL");

    QTest::addRow("XYZ Luminance") << int(KisRelativeContentLightLevelInformation::XYZLuminance) << 0.716905 << 0.0659222;
    QTest::addRow("Rec2020 Component") << int(KisRelativeContentLightLevelInformation::Rec2020Component) << 0.919528 << 0.0940548;
    QTest::addRow("RGB Component") << int(KisRelativeContentLightLevelInformation::RGBComponent) << 1.00002 << 0.111112;
}

void KisContentLightLevelProcessingVisitorTest::testRgbColorSpace()
{
    QFETCH(int, type);
    QFETCH(double, expectedMaxCLL);
    QFETCH(double, expectedMaxFALL);

    KisImageSP image = createImage();


    KisProcessingApplicator::ProcessingFlags signalFlags = KisProcessingApplicator::NO_UI_UPDATES | KisProcessingApplicator::RECURSIVE_FRAME_TIMES;
    KisProcessingApplicator applicator(image, image->rootLayer(),
                                       signalFlags);

    KisSharedPtr<KisContentLightLevelProcessingVistor> visitor =
        new KisContentLightLevelProcessingVistor(KisRelativeContentLightLevelInformation::CalculationType(type), image->bounds());
    applicator.applyVisitorAllFrames(visitor, KisStrokeJobData::SEQUENTIAL);
    applicator.end();

    image->waitForDone();

    KisRelativeContentLightLevelInformation info = visitor->contentLightLevelInformation();


    QVERIFY(info.maxContentLightLevel - expectedMaxCLL < 0.00001);
    QVERIFY(info.maxFrameAverageLightLevel - expectedMaxFALL < 0.00001);

}

void KisContentLightLevelProcessingVisitorTest::testCmykColorSpace_data()
{
    QTest::addColumn<int>("type");
    QTest::addColumn<double>("expectedMaxCLL");
    QTest::addColumn<double>("expectedMaxFALL");

    QTest::addRow("XYZ Luminance") << int(KisRelativeContentLightLevelInformation::XYZLuminance) << 0.360151 << 0.0195933;
    QTest::addRow("Rec2020 Component") << int(KisRelativeContentLightLevelInformation::Rec2020Component) << 0.514917 << 0.0538389;
    // Because we're testing cmyk, and not rgb, this will fallback onto xyzLuminance.
    QTest::addRow("RGB Component") << int(KisRelativeContentLightLevelInformation::RGBComponent) << 0.360151 << 0.0195933;
}

void KisContentLightLevelProcessingVisitorTest::testCmykColorSpace()
{
    QFETCH(int, type);
    QFETCH(double, expectedMaxCLL);
    QFETCH(double, expectedMaxFALL);
    KisImageSP image = createImage();
    const KoColorSpace *cmyk = KoColorSpaceRegistry::instance()->colorSpace(CMYKAColorModelID.id(), Float32BitsColorDepthID.id(), "Chemical proof");
    image->convertImageColorSpace(cmyk, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());


    KisProcessingApplicator::ProcessingFlags signalFlags = KisProcessingApplicator::NO_UI_UPDATES | KisProcessingApplicator::RECURSIVE_FRAME_TIMES;
    KisProcessingApplicator applicator(image, image->rootLayer(),
                                       signalFlags);

    KisSharedPtr<KisContentLightLevelProcessingVistor> visitor =
        new KisContentLightLevelProcessingVistor(KisRelativeContentLightLevelInformation::CalculationType(type), image->bounds());
    applicator.applyVisitorAllFrames(visitor, KisStrokeJobData::SEQUENTIAL);
    applicator.end();

    image->waitForDone();

    KisRelativeContentLightLevelInformation info = visitor->contentLightLevelInformation();

    QVERIFY(info.maxContentLightLevel - expectedMaxCLL < 0.00001);
    QVERIFY(info.maxFrameAverageLightLevel - expectedMaxFALL < 0.00001);
}

void KisContentLightLevelProcessingVisitorTest::testEmptyDocument()
{
    KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id());
    KisImageSP image = new KisImage(undoStore, 300, 300, cs, "test");

    //-----------------------//

    KisProcessingApplicator::ProcessingFlags signalFlags = KisProcessingApplicator::NO_UI_UPDATES | KisProcessingApplicator::RECURSIVE_FRAME_TIMES;
    KisProcessingApplicator applicator(image, image->rootLayer(),
                                       signalFlags);

    KisSharedPtr<KisContentLightLevelProcessingVistor> visitor =
        new KisContentLightLevelProcessingVistor(KisRelativeContentLightLevelInformation::XYZLuminance, image->bounds());
    applicator.applyVisitorAllFrames(visitor, KisStrokeJobData::SEQUENTIAL);
    applicator.end();

    image->waitForDone();
}

SIMPLE_TEST_MAIN(KisContentLightLevelProcessingVisitorTest)
