/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_webp_test.h"

#include <filestest.h>
#include <simpletest.h>
#include <testui.h>


#include <QString>

#include <kis_image_animation_interface.h>
#include <kis_keyframe_channel.h>
#include <kis_meta_data_backend_registry.h>

#ifndef FILES_DATA_DIR
#error                                                                         \
    "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif

const QString MIMETYPE = "image/webp";

void KisWebPTest::testFiles()
{
    KisMetadataBackendRegistry::instance();

    const int fuzziness = 1;

    TestUtil::testFiles(QString(FILES_DATA_DIR) + "/sources",
                        {},
                        {},
                        fuzziness,
                        0,
                        true);
}

void KisWebPTest::testAnimation()
{
    const QString inputFileName =
        TestUtil::fetchDataFileLazy("/sources/DX-MON/loading_16.webp");

    QScopedPointer<KisDocument> doc(
        qobject_cast<KisDocument *>(KisPart::instance()->createDocument()));

    KisImportExportManager manager(doc.data());
    doc->setFileBatchMode(true);

    const KisImportExportErrorCode status =
        manager.importDocument(inputFileName, QString());
    QVERIFY(status.isOk());

    KisImageSP image = doc->image();

    // Check that it's a 32 FPS document with 24 frames.
    KisNodeSP node1 = doc->image()->root()->firstChild();

    QVERIFY(node1->inherits("KisPaintLayer"));

    KisPaintLayerSP layer1 = qobject_cast<KisPaintLayer *>(node1.data());

    QVERIFY(layer1->isAnimated());

    const KisKeyframeChannel *channel1 =
        layer1->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    QVERIFY(channel1);
    QCOMPARE(channel1->keyframeCount(), 24);

    QCOMPARE(image->animationInterface()->framerate(), 32);
    QCOMPARE(image->animationInterface()->documentPlaybackRange(),
             KisTimeSpan::fromTimeToTime(0, 23));
    QCOMPARE(image->animationInterface()->currentTime(), 0);
}

void KisWebPTest::testAnimationWithTail()
{
    const QString inputFileName =
        TestUtil::fetchDataFileLazy("/sources/animated/animation_test.webp");

    QScopedPointer<KisDocument> doc(
        qobject_cast<KisDocument *>(KisPart::instance()->createDocument()));

    KisImportExportManager manager(doc.data());
    doc->setFileBatchMode(true);

    const KisImportExportErrorCode status =
        manager.importDocument(inputFileName, QString());
    QVERIFY(status.isOk());

    KisImageSP image = doc->image();

    KisNodeSP node1 = doc->image()->root()->firstChild();

    QVERIFY(node1->inherits("KisPaintLayer"));

    KisPaintLayerSP layer1 = qobject_cast<KisPaintLayer *>(node1.data());

    QVERIFY(layer1->isAnimated());

    const KisKeyframeChannel *channel1 =
        layer1->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    QVERIFY(channel1);
    QCOMPARE(channel1->keyframeCount(), 4);

    // Unlike the JPEG-XL original, WebP stores with an associated
    // duration, not framerate. They have no notion of blanking.
    QCOMPARE(image->animationInterface()->framerate(), 1);
    QCOMPARE(image->animationInterface()->documentPlaybackRange(),
             KisTimeSpan::fromTimeToTime(0, 3));
    QCOMPARE(image->animationInterface()->currentTime(), 0);
}

inline void testSaveColorSpace(const QString &colorModel, const QString &colorDepth, const QString &colorProfile)
{
    const KoColorSpace *space = KoColorSpaceRegistry::instance()->colorSpace(colorModel, colorDepth, colorProfile);
    if (space) {
        TestUtil::testExportToColorSpace(MIMETYPE, space, ImportExportCodes::OK);
    }
}

void KisWebPTest::testSaveRgbaColorSpace()
{
    QString profile = "sRGB-elle-V2-srgbtrc";
    testSaveColorSpace(RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), profile);
}

void KisWebPTest::testSaveUnsupportedColorSpace()
{
    // Non-RGBA color model will trigger color conversion and dithering path on WebP exports,
    // therefore it's sufficient to only test for U8 depths.

    QString profile = "Gray-D50-elle-V2-srgbtrc";
    testSaveColorSpace(GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), profile);

    profile = "Chemical proof";
    testSaveColorSpace(CMYKAColorModelID.id(), Integer8BitsColorDepthID.id(), profile);

    profile = "Lab identity built-in";
    testSaveColorSpace(LABAColorModelID.id(), Integer8BitsColorDepthID.id(), profile);
}

#ifndef _WIN32
void KisWebPTest::testImportFromWriteonly()
{
    TestUtil::testImportFromWriteonly(MIMETYPE);
}

void KisWebPTest::testExportToReadonly()
{
    TestUtil::testExportToReadonly(MIMETYPE);
}
#endif

void KisWebPTest::testImportIncorrectFormat()
{
    TestUtil::testImportIncorrectFormat(MIMETYPE);
}

KISTEST_MAIN(KisWebPTest)
