/*
 * SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tiff_test.h"


#include <simpletest.h>
#include <QCoreApplication>

#include "filestest.h"

#include <KoColorModelStandardIds.h>
#include <KoColor.h>

#include <KoColorModelStandardIdsUtils.h>
#include <kis_meta_data_backend_registry.h>
#include <testui.h>

#include <config-jpeg.h>

#include <KoConfig.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif


const QString TiffMimetype = "image/tiff";

void KisTiffTest::testFiles()
{
    KisMetadataBackendRegistry::instance();

    QStringList excludes;

#ifdef HAVE_LCMS2
    excludes << "flower-separated-contig-08.tif"
             << "flower-separated-contig-16.tif"
             << "flower-separated-planar-08.tif"
             << "flower-separated-planar-16.tif"
             << "flower-minisblack-02.tif"
             << "flower-minisblack-04.tif"
             << "flower-minisblack-08.tif"
             << "flower-minisblack-10.tif"
             << "flower-minisblack-12.tif"
             << "flower-minisblack-14.tif"
             << "flower-minisblack-16.tif"
             << "flower-minisblack-24.tif"
             << "flower-minisblack-32.tif"
             << "jim___dg.tif"
             << "jim___gg.tif"
             << "strike.tif";
#endif

#ifndef HAVE_JPEG_TURBO
    excludes << "quad-strip-jpeg.tif"
             << "quad-tile-jpeg.tif";
#endif

    TestUtil::testFiles(QString(FILES_DATA_DIR) + "/sources", excludes, QString(), 1);
}

void KisTiffTest::testSaveTiffColorSpace(QString colorModel, QString colorDepth, QString colorProfile)
{
    const KoColorSpace *space = KoColorSpaceRegistry::instance()->colorSpace(colorModel, colorDepth, colorProfile);
    if (space) {
        TestUtil::testExportToColorSpace(TiffMimetype, space, ImportExportCodes::OK);
    }
}

void KisTiffTest::testSaveTiffRgbaColorSpace()
{
    QString profile = "sRGB-elle-V2-srgbtrc";
    testSaveTiffColorSpace(RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), profile);
    profile = "sRGB-elle-V2-g10";
    testSaveTiffColorSpace(RGBAColorModelID.id(), Integer16BitsColorDepthID.id(), profile);
    testSaveTiffColorSpace(RGBAColorModelID.id(), Float16BitsColorDepthID.id(), profile);
    testSaveTiffColorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), profile);
}

void KisTiffTest::testSaveTiffGreyAColorSpace()
{
    QString profile = "Gray-D50-elle-V2-srgbtrc";
    testSaveTiffColorSpace(GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), profile);
    profile = "Gray-D50-elle-V2-g10";
    testSaveTiffColorSpace(GrayAColorModelID.id(), Integer16BitsColorDepthID.id(), profile);
    testSaveTiffColorSpace(GrayAColorModelID.id(), Float16BitsColorDepthID.id(), profile);
    testSaveTiffColorSpace(GrayAColorModelID.id(), Float32BitsColorDepthID.id(), profile);

}


void KisTiffTest::testSaveTiffCmykColorSpace()
{
    QString profile = "Chemical proof";
    testSaveTiffColorSpace(CMYKAColorModelID.id(), Integer8BitsColorDepthID.id(), profile);
    testSaveTiffColorSpace(CMYKAColorModelID.id(), Integer16BitsColorDepthID.id(), profile);
    testSaveTiffColorSpace(CMYKAColorModelID.id(), Float32BitsColorDepthID.id(), profile);
}

void KisTiffTest::testSaveTiffLabColorSpace()
{
    const QString profile = "Lab identity build-in";
    testSaveTiffColorSpace(LABAColorModelID.id(), Integer8BitsColorDepthID.id(), profile);
    testSaveTiffColorSpace(LABAColorModelID.id(), Integer16BitsColorDepthID.id(), profile);
    testSaveTiffColorSpace(LABAColorModelID.id(), Float32BitsColorDepthID.id(), profile);
}


void KisTiffTest::testSaveTiffYCbCrAColorSpace()
{
    const QString profile = "ITU-R BT.709-6 + BT.1886 YCbCr ICC V4 profile";
    testSaveTiffColorSpace(YCbCrAColorModelID.id(), Integer8BitsColorDepthID.id(), profile);
    testSaveTiffColorSpace(YCbCrAColorModelID.id(), Integer16BitsColorDepthID.id(), profile);
    testSaveTiffColorSpace(YCbCrAColorModelID.id(),
                           Float32BitsColorDepthID.id(),
                           profile);
}

void KisTiffTest::testImportFromWriteonly()
{
    TestUtil::testImportFromWriteonly(TiffMimetype);
}


void KisTiffTest::testExportToReadonly()
{
    TestUtil::testExportToReadonly(TiffMimetype);
}


void KisTiffTest::testImportIncorrectFormat()
{
    TestUtil::testImportIncorrectFormat(TiffMimetype);
}



KISTEST_MAIN(KisTiffTest)

