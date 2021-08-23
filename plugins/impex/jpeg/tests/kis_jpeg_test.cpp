/*
 * SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_jpeg_test.h"

#include <simpletest.h>
#include <QCoreApplication>

#include "filestest.h"
#include "jpeglib.h"
#include <kis_meta_data_backend_registry.h>
#include <sdk/tests/testui.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif

#ifndef JPEG_LIB_VERSION
#error "JPEG_LIB_VERSION not set. libjpeg should set it."
#endif

const QString JpegMimetype = "image/jpeg";

void KisJpegTest::testFiles()
{
    KisMetadataBackendRegistry::instance();
    /**
     * Different versions of JPEG library may produce a bit different
     * result, so just compare in a weak way, i.e, only the size for real
     */
    const int fuzziness = 1;
    const int maxNumFailingPixels = 2592 * 1952; // All pixels can be different...
    const bool showDebug = false; // No need to write down all pixels that are different since all of them can be.

    if (JPEG_LIB_VERSION == 80){
        TestUtil::testFiles(QString(FILES_DATA_DIR) + "/sources", QStringList(), "_80", fuzziness, maxNumFailingPixels, showDebug);
    }else {
        TestUtil::testFiles(QString(FILES_DATA_DIR) + "/sources", QStringList(), QString(), fuzziness, maxNumFailingPixels, showDebug);
    }
}

void KisJpegTest::testImportFromWriteonly()
{
    TestUtil::testImportFromWriteonly(QString(FILES_DATA_DIR), JpegMimetype);
}


void KisJpegTest::testExportToReadonly()
{
    TestUtil::testExportToReadonly(QString(FILES_DATA_DIR), JpegMimetype);
}


void KisJpegTest::testImportIncorrectFormat()
{
    TestUtil::testImportIncorrectFormat(QString(FILES_DATA_DIR), JpegMimetype);
}
KISTEST_MAIN(KisJpegTest)

