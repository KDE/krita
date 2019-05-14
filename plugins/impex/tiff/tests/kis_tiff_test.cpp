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

#include "kis_tiff_test.h"


#include <QTest>
#include <QCoreApplication>

#include "filestest.h"

#include <KoColorModelStandardIds.h>
#include <KoColor.h>

#include "kisexiv2/kis_exiv2.h"
#include  <sdk/tests/kistest.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif


const QString TiffMimetype = "image/tiff";

void KisTiffTest::testFiles()
{
    // XXX: make the exiv io backends real plugins
    KisExiv2::initialize();

    QStringList excludes;

#ifndef CPU_32_BITS
    excludes << "flower-minisblack-06.tif";
#endif

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
    excludes << "text.tif" << "ycbcr-cat.tif";

    TestUtil::testFiles(QString(FILES_DATA_DIR) + "/sources", excludes, QString(), 1);
}

void KisTiffTest::testRoundTripRGBF16()
{
    // Disabled for now, it's broken because we assumed integers.
#if 0

    QRect testRect(0,0,1000,1000);
    QRect fillRect(100,100,100,100);

    const KoColorSpace *csf16 = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float16BitsColorDepthID.id(), 0);
    KisDocument *doc0 = qobject_cast<KisDocument*>(KisPart::instance()->createDocument());
    doc0->newImage("test", testRect.width(), testRect.height(), csf16, KoColor(Qt::blue, csf16), QString(), 1.0);

    QTemporaryFile tmpFile(QDir::tempPath() + QLatin1String("/krita_XXXXXX") + QLatin1String(".tiff"));

    tmpFile.open();
    doc0->setBackupFile(false);
    doc0->setOutputMimeType("image/tiff");
    doc0->setFileBatchMode(true);
    doc0->saveAs(QUrl::fromLocalFile(tmpFile.fileName()));

    KisNodeSP layer0 = doc0->image()->root()->firstChild();
    Q_ASSERT(layer0);
    layer0->paintDevice()->fill(fillRect, KoColor(Qt::red, csf16));


    KisDocument *doc1 = qobject_cast<KisDocument*>(KisPart::instance()->createDocument());

    KisImportExportManager manager(doc1);
    doc1->setFileBatchMode(false);

    KisImportExportErrorCode status;

    QString s = manager.importDocument(tmpFile.fileName(),
                                       QString(),
                                       status);

    dbgKrita << s;
    Q_ASSERT(doc1->image());

    QImage ref0 = doc0->image()->projection()->convertToQImage(0, testRect);
    QImage ref1 = doc1->image()->projection()->convertToQImage(0, testRect);

    QCOMPARE(ref0, ref1);
#endif
}


void KisTiffTest::testImportFromWriteonly()
{
    TestUtil::testImportFromWriteonly(QString(FILES_DATA_DIR), TiffMimetype);
}


void KisTiffTest::testExportToReadonly()
{
    TestUtil::testExportToReadonly(QString(FILES_DATA_DIR), TiffMimetype);
}


void KisTiffTest::testImportIncorrectFormat()
{
    TestUtil::testImportIncorrectFormat(QString(FILES_DATA_DIR), TiffMimetype);
}



KISTEST_MAIN(KisTiffTest)

