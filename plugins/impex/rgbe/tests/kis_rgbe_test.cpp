/*
 * SPDX-FileCopyrightText: 2023 Rasyuqa A. H. <qampidh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_rgbe_test.h"

#include <simpletest.h>

#include <QString>

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
