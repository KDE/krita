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

#include "kis_jpeg_test.h"

#include <QTest>
#include <QCoreApplication>

#include "kisexiv2/kis_exiv2.h"
#include "filestest.h"
#include "jpeglib.h"
#include  <sdk/tests/kistest.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif

#ifndef JPEG_LIB_VERSION
#error "JPEG_LIB_VERSION not set. libjpeg should set it."
#endif

const QString JpegMimetype = "image/jpeg";

void KisJpegTest::testFiles()
{
    KisExiv2::initialize();
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

void KisJpegTest::testWriteonly()
{
    TestUtil::testWriteonly(QString(FILES_DATA_DIR), JpegMimetype);
}


KISTEST_MAIN(KisJpegTest)

