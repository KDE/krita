/*
 * SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisQImageIOTest.h"


#include <QTest>
#include <QCoreApplication>

#include "filestest.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif


const QString QImageIOMimetype = "image/x-gimp-brush";



void KisQImageIOTest::testImportFromWriteonly()
{
    TestUtil::testImportFromWriteonly(QString(FILES_DATA_DIR), QImageIOMimetype);
}


void KisQImageIOTest::testExportToReadonly()
{
    TestUtil::testExportToReadonly(QString(FILES_DATA_DIR), QImageIOMimetype);
}


void KisQImageIOTest::testImportIncorrectFormat()
{
    TestUtil::testImportIncorrectFormat(QString(FILES_DATA_DIR), QImageIOMimetype);
}



KISTEST_MAIN(KisQImageIOTest)


