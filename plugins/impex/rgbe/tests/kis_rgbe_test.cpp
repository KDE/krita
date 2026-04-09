/*
 * SPDX-FileCopyrightText: 2023 Rasyuqa A. H. <qampidh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_rgbe_test.h"

#include <simpletest.h>

#include <QString>

#include <QBuffer>
#include <QDataStream>
#include <RGBEImportUtils.h>

#include <filestest.h>
#include <kis_meta_data_backend_registry.h>
#include <testui.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif

const QString MIMETYPE = "image/vnd.radiance";

void KisRGBETest::testFiles()
{
    KisMetadataBackendRegistry::instance();

    const int fuzziness = 1;

    TestUtil::testFiles(QString(FILES_DATA_DIR) + "/sources", {}, {}, fuzziness, 0, true);
}

void KisRGBETest::testHDR()
{
    const QString inputFileName = TestUtil::fetchDataFileLazy("/sources/hdr/objects.hdr");

    QScopedPointer<KisDocument> doc1(qobject_cast<KisDocument *>(KisPart::instance()->createDocument()));

    KisImportExportManager manager(doc1.data());
    doc1->setFileBatchMode(true);

    const KisImportExportErrorCode status = manager.importDocument(inputFileName, {});
    QVERIFY(status.isOk());

    KisImageSP image = doc1->image();

    {
        const QString outputFileName = TestUtil::fetchDataFileLazy("/results/objects.kra");

        KisDocument *doc2 = KisPart::instance()->createDocument();
        doc2->setFileBatchMode(true);
        const bool r = doc2->importDocument(outputFileName);

        QVERIFY(r);
        QVERIFY(doc2->errorMessage().isEmpty());
        QVERIFY(doc2->image());

        QVERIFY(TestUtil::comparePaintDevicesClever<float>(doc1->image()->root()->firstChild()->paintDevice(),
                                                           doc2->image()->root()->firstChild()->paintDevice(),
                                                           0.01f /* meaningless alpha */));

        delete doc2;
    }
}

inline void testSaveColorSpace(const QString &colorModel, const QString &colorDepth, const QString &colorProfile)
{
    const KoColorSpace *space = KoColorSpaceRegistry::instance()->colorSpace(colorModel, colorDepth, colorProfile);
    if (space) {
        TestUtil::testExportToColorSpace(MIMETYPE, space, ImportExportCodes::OK);
    }
}
void KisRGBETest::testSaveRgbaColorSpace()
{
    QString profile = "sRGB-elle-V2-g10";
    testSaveColorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), profile);
}

void KisRGBETest::testRLEOverflow_data()
{
    QTest::addColumn<QByteArray>("srcData");
    QTest::addColumn<bool>("expectedSuccess");

    struct DataGenerator {
        QByteArray component0;

        QByteArray generate() const {
            QByteArray srcData;

            // Construct HDR RLE data that attempts buffer overflow
            // Format: [2] [2] [width_msb] [width_lsb] [RLE_component_0] [RLE_component_1] [RLE_component_2]
            // [RLE_component_3]

            srcData.append(static_cast<char>(2)); // Header marker for new scanline format
            srcData.append(static_cast<char>(2)); // Confirms new format
            srcData.append(static_cast<char>(0x00)); // Width MSB (16 = 0x0010)
            srcData.append(static_cast<char>(0x10)); // Width LSB

            srcData.append(component0);

            // Components 1, 2, 3: Add valid RLE data to reach end of stream
            for (int i = 1; i < 4; i++) {
                srcData.append(static_cast<char>(16)); // Read 16 bytes in non-run mode
                for (int j = 0; j < 16; ++j) {
                    srcData.append(static_cast<char>(0x80 + i)); // Dummy data byte
                }
            }

            return srcData;
        }
    };

    struct OldDataGenerator {
        QByteArray component0;

        QByteArray generate() const {
            QByteArray srcData;

            // Construct HDR RLE data that attempts buffer overflow
            // Format: [2] [2] [width_msb] [width_lsb] [RLE_component_0] [RLE_component_1] [RLE_component_2]
            // [RLE_component_3]

            srcData.append(static_cast<char>(2)); // Header marker for new scanline format
            srcData.append(static_cast<char>(2)); // Confirms new format
            srcData.append(static_cast<char>(0x00)); // Width MSB (16 = 0x0010)
            srcData.append(static_cast<char>(0x10)); // Width LSB

            srcData.append(component0);

            // Components 1, 2, 3: Add valid RLE data to reach end of stream
            for (int i = 1; i < 4; i++) {
                srcData.append(static_cast<char>(16)); // Read 16 bytes in non-run mode
                for (int j = 0; j < 16; ++j) {
                    srcData.append(static_cast<char>(0x80 + i)); // Dummy data byte
                }
            }

            return srcData;
        }
    };


    {
        DataGenerator g;

        // Component 0: RLE run that attempts overflow
        g.component0.append(static_cast<char>(72 | 128)); // RLE run marker is 128, repeat 72 times
        g.component0.append(static_cast<char>(0x80)); // Value to repeat

        QTest::addRow("rle-deep-past-the-end") << g.generate() << false;
    }

    {
        DataGenerator g;

        // Component 0: RLE run that attempts overflow
        g.component0.append(static_cast<char>(17 | 128)); // RLE run marker is 128, repeat 17 times
        g.component0.append(static_cast<char>(0x80)); // Value to repeat

        QTest::addRow("rle-1byte-past-the-end") << g.generate() << false;
    }

    {
        DataGenerator g;

        // Component 0: RLE run that attempts overflow
        g.component0.append(static_cast<char>(1)); // Directly copy one byte
        g.component0.append(static_cast<char>(0x70));

        g.component0.append(static_cast<char>(16 | 128)); // RLE run marker is 128, repeat 16 times
        g.component0.append(static_cast<char>(0x80)); // Value to repeat

        QTest::addRow("rle-1byte-past-the-end-with-offset") << g.generate() << false;
    }

    {
        DataGenerator g;

        // Component 0: RLE run that expands to exactly 16 pixels
        g.component0.append(static_cast<char>(16 | 128)); // RLE run marker is 128, repeat 16 times
        g.component0.append(static_cast<char>(0x80)); // Value to repeat

        QTest::addRow("rle-correct") << g.generate() << true;
    }

    {
        DataGenerator g;

        // Component 0: RLE run that expands to exactly 16 pixels
        g.component0.append(static_cast<char>(1)); // Directly copy one byte
        g.component0.append(static_cast<char>(0x70));

        g.component0.append(static_cast<char>(15 | 128)); // RLE run marker is 128, repeat 15 times
        g.component0.append(static_cast<char>(0x80)); // Value to repeat

        QTest::addRow("rle-correct-with-offset") << g.generate() << true;
    }

    {
        DataGenerator g;

        // Component 0: non-run mode that attempts overflow
        g.component0.append(static_cast<char>(17)); // Directly copy 17 bytes
        for (int j = 0; j < 17; j++) {
            g.component0.append(static_cast<char>(0x70));
        }

        QTest::addRow("non-run-1byte-past-the-end") << g.generate() << false;
    }

    {
        DataGenerator g;

        // Component 0: non-run mode that expands to exactly 16 pixels
        g.component0.append(static_cast<char>(16)); // Directly copy 16 bytes
        for (int j = 0; j < 16; j++) {
            g.component0.append(static_cast<char>(0x70));
        }

        QTest::addRow("non-run-correct") << g.generate() << true;
    }

    {
        QByteArray srcData;
        // Each record is one HDR pixel in RGBe form: [R][G][B][E],
        // pixel [1][1][1][count] is a special marker to repeat the
        // previous pixel `count` times

        for (int j = 0; j < 16; j++) {
            srcData.append(static_cast<char>(0x80));
            srcData.append(static_cast<char>(0x81));
            srcData.append(static_cast<char>(0x82));
            srcData.append(static_cast<char>(0x83));
        }

        QTest::addRow("old-format-uncompressed") << srcData << true;
    }

    {
        QByteArray srcData;
        // Each record is one HDR pixel in RGBe form: [R][G][B][E],
        // pixel [1][1][1][count] is a special marker to repeat the
        // previous pixel `count` times

        srcData.append(static_cast<char>(0x80));
        srcData.append(static_cast<char>(0x81));
        srcData.append(static_cast<char>(0x82));
        srcData.append(static_cast<char>(0x83));
        srcData.append(static_cast<char>(1));
        srcData.append(static_cast<char>(1));
        srcData.append(static_cast<char>(1));
        srcData.append(static_cast<char>(15));

        QTest::addRow("old-format-expand-exact") << srcData << true;
    }

    {
        QByteArray srcData;
        // Each record is one HDR pixel in RGBe form: [R][G][B][E],
        // pixel [1][1][1][count] is a special marker to repeat the
        // previous pixel `count` times

        srcData.append(static_cast<char>(0x80));
        srcData.append(static_cast<char>(0x81));
        srcData.append(static_cast<char>(0x82));
        srcData.append(static_cast<char>(0x83));
        srcData.append(static_cast<char>(1));
        srcData.append(static_cast<char>(1));
        srcData.append(static_cast<char>(1));
        srcData.append(static_cast<char>(16));

        QTest::addRow("old-format-1px-past-the-end") << srcData << false;
    }

}

void KisRGBETest::testRLEOverflow()
{
    //QByteArray dstData;
    QFETCH(QByteArray, srcData);
    QFETCH(bool, expectedSuccess);

    const int width = 16;

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP device = new KisPaintDevice(cs);
    KisSequentialIterator it(device, {0, 0, width, 1});

    QBuffer buffer(&srcData);
    buffer.open(QIODevice::ReadOnly);

    QDataStream stream(&buffer);
    bool result = RGBEIMPORT::LoadHDR(stream, width, 1, it);

    QCOMPARE(result, expectedSuccess);
}

#ifndef _WIN32
void KisRGBETest::testImportFromWriteonly()
{
    TestUtil::testImportFromWriteonly(MIMETYPE);
}

void KisRGBETest::testExportToReadonly()
{
    TestUtil::testExportToReadonly(MIMETYPE);
}
#endif

void KisRGBETest::testImportIncorrectFormat()
{
    TestUtil::testImportIncorrectFormat(MIMETYPE);
}

KISTEST_MAIN(KisRGBETest)
