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

#include <qtest_kde.h>

#include "filestest.h"

#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>

#include "kisexiv2/kis_exiv2.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif


void KisTiffTest::testFiles()
{
    // XXX: make the exiv io backends real plugins
    KisExiv2::initialize();

    QStringList excludes;
    if (!KoColorSpaceRegistry::instance()->colorModelsList(KoColorSpaceRegistry::AllColorSpaces).contains(YCbCrAColorModelID)) {
        excludes << "ycbcr-cat.tif";
    }

    excludes << "text.tif";

    TestUtil::testFiles(QString(FILES_DATA_DIR) + "/sources", excludes);
}
QTEST_KDEMAIN(KisTiffTest, GUI)

#include "kis_tiff_test.moc"
