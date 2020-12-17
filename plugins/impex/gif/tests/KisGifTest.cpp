/*
 * SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisGifTest.h"


#include <QTest>
#include <QCoreApplication>

#include "filestest.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif


const QString GifMimetype = "image/gif";



void KisGifTest::testImportFromWriteonly()
{
    TestUtil::testImportFromWriteonly(QString(FILES_DATA_DIR), GifMimetype);
}


void KisGifTest::testExportToReadonly()
{
    TestUtil::testExportToReadonly(QString(FILES_DATA_DIR), GifMimetype);
}


void KisGifTest::testImportIncorrectFormat()
{
    TestUtil::testImportIncorrectFormat(QString(FILES_DATA_DIR), GifMimetype);
}



KISTEST_MAIN(KisGifTest)


