/*
 *  SPDX-FileCopyrightText: 2017 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_heightmap_test.h"

#include <simpletest.h>
#include <QCoreApplication>

#include  <sdk/tests/testui.h>

#include "filestest.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif


const QString HeightmapMimetype = "image/x-r8";


void KisHeightmapTest::testFiles()
{
    TestUtil::testFiles(QString(FILES_DATA_DIR) + "/sources", QStringList(), QString(), 1);
}



void KisHeightmapTest::testImportFromWriteonly()
{
    TestUtil::testImportFromWriteonly(QString(FILES_DATA_DIR), HeightmapMimetype);
}


void KisHeightmapTest::testExportToReadonly()
{
    TestUtil::testExportToReadonly(QString(FILES_DATA_DIR), HeightmapMimetype);
}


void KisHeightmapTest::testImportIncorrectFormat()
{
    TestUtil::testImportIncorrectFormat(QString(FILES_DATA_DIR), HeightmapMimetype);
}


KISTEST_MAIN(KisHeightmapTest)

