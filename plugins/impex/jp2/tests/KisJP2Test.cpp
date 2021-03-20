/*
 * SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KisJP2Test.h>

#include <simpletest.h>
#include <QCoreApplication>

#include "filestest.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif


const QString JP2Mimetype = "image/jp2";



void KisJP2Test::testImportFromWriteonly()
{
    TestUtil::testImportFromWriteonly(QString(FILES_DATA_DIR), JP2Mimetype);
}


void KisJP2Test::testExportToReadonly()
{
    TestUtil::testExportToReadonly(QString(FILES_DATA_DIR), JP2Mimetype);
}


void KisJP2Test::testImportIncorrectFormat()
{
    TestUtil::testImportIncorrectFormat(QString(FILES_DATA_DIR), JP2Mimetype);
}



KISTEST_MAIN(KisJP2Test)


