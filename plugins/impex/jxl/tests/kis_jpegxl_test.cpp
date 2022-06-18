/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_jpegxl_test.h"
#include "kis_image_animation_interface.h"
#include "kis_keyframe_channel.h"

#include <simpletest.h>

#include <QString>

#include <kis_meta_data_backend_registry.h>
#include <sdk/tests/filestest.h>
#include <sdk/tests/testui.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif

const QString MIMETYPE = "image/jxl";

void KisJPEGXLTest::testFiles()
{
    KisMetadataBackendRegistry::instance();

    const int fuzziness = 1;

    TestUtil::testFiles(QString(FILES_DATA_DIR) + "/sources", {}, {}, fuzziness, 0, true);

    TestUtil::testFiles(QString(FILES_DATA_DIR) + "/sources/netflix",
                        {"hdr_cosmos01000_cicp9-16-0_lossless.jxl", "LICENSE.txt"},
                        {},
                        fuzziness,
                        0,
                        true);
}

void KisJPEGXLTest::testAnimation()
{
    const auto inputFileName = TestUtil::fetchDataFileLazy("/sources/DX-MON/loading_16.jxl");

    QScopedPointer<KisDocument> doc(qobject_cast<KisDocument *>(KisPart::instance()->createDocument()));

    KisImportExportManager manager(doc.data());
    doc->setFileBatchMode(true);

    const auto status = manager.importDocument(inputFileName, QString());
    QVERIFY(status.isOk());

    KisImageSP image = doc->image();

    // Check that it's a 32 FPS document with 24 frames.
    auto node1 = doc->image()->root()->firstChild();

    QVERIFY(node1->inherits("KisPaintLayer"));

    KisPaintLayerSP layer1 = qobject_cast<KisPaintLayer *>(node1.data());

    QVERIFY(layer1->isAnimated());

    const auto *channel1 = layer1->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    QVERIFY(channel1);
    QCOMPARE(channel1->keyframeCount(), 24);

    QCOMPARE(image->animationInterface()->framerate(), 32);
    QCOMPARE(image->animationInterface()->fullClipRange(),
             KisTimeSpan::fromTimeToTime(0, 23));
    QCOMPARE(image->animationInterface()->currentTime(), 0);
}

void KisJPEGXLTest::testAnimationWithTail()
{
    const auto inputFileName =
        TestUtil::fetchDataFileLazy("/sources/animated/animation_test.jxl");

    QScopedPointer<KisDocument> doc(
        qobject_cast<KisDocument *>(KisPart::instance()->createDocument()));

    KisImportExportManager manager(doc.data());
    doc->setFileBatchMode(true);

    const auto status = manager.importDocument(inputFileName, QString());
    QVERIFY(status.isOk());

    KisImageSP image = doc->image();

    auto node1 = doc->image()->root()->firstChild();

    QVERIFY(node1->inherits("KisPaintLayer"));

    KisPaintLayerSP layer1 = qobject_cast<KisPaintLayer *>(node1.data());

    QVERIFY(layer1->isAnimated());

    const auto *channel1 =
        layer1->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    QVERIFY(channel1);
    QCOMPARE(channel1->keyframeCount(), 4);

    QCOMPARE(image->animationInterface()->framerate(), 5);
    QCOMPARE(image->animationInterface()->fullClipRange(),
             KisTimeSpan::fromTimeToTime(0, 19));
    QCOMPARE(image->animationInterface()->currentTime(), 0);
}

void KisJPEGXLTest::testHDR()
{
    const auto inputFileName = TestUtil::fetchDataFileLazy("/sources/netflix/hdr_cosmos01000_cicp9-16-0_lossless.jxl");

    QScopedPointer<KisDocument> doc1(qobject_cast<KisDocument *>(KisPart::instance()->createDocument()));

    KisImportExportManager manager(doc1.data());
    doc1->setFileBatchMode(true);

    const auto status = manager.importDocument(inputFileName, {});
    QVERIFY(status.isOk());

    KisImageSP image = doc1->image();

    {
        const auto outputFileName = TestUtil::fetchDataFileLazy("/results/hdr_cosmos01000_cicp9-16-0_lossless.kra");

        KisDocument *doc2 = KisPart::instance()->createDocument();
        doc2->setFileBatchMode(true);
        const auto r = doc2->importDocument(outputFileName);

        QVERIFY(r);
        QVERIFY(doc2->errorMessage().isEmpty());
        QVERIFY(doc2->image());

        doc1->image()->root()->firstChild()->paintDevice()->convertToQImage(nullptr).save("1.png");
        doc2->image()->root()->firstChild()->paintDevice()->convertToQImage(nullptr).save("2.png");

        QVERIFY(TestUtil::comparePaintDevicesClever<float>(doc1->image()->root()->firstChild()->paintDevice(),
                                                           doc2->image()->root()->firstChild()->paintDevice(),
                                                           0.01f /* meaningless alpha */));

        delete doc2;
    }
}

#ifndef _WIN32
void KisJPEGXLTest::testImportFromWriteonly()
{
    TestUtil::testImportFromWriteonly(MIMETYPE);
}

void KisJPEGXLTest::testExportToReadonly()
{
    TestUtil::testExportToReadonly(MIMETYPE);
}
#endif

void KisJPEGXLTest::testImportIncorrectFormat()
{
    TestUtil::testImportIncorrectFormat(MIMETYPE);
}

KISTEST_MAIN(KisJPEGXLTest)
