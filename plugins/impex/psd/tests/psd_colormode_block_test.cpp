/*
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "psd_colormode_block_test.h"

#include <simpletest.h>
#include <QCoreApplication>
#include <klocalizedstring.h>
#include <psd.h>
#include <psd_header.h>
#include <psd_colormode_block.h>
#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif

void PSDColorModeBlockTest::testCreation()
{
    PSDColorModeBlock colorModeBlock1(Indexed);
    Q_ASSERT(!colorModeBlock1.valid());

    PSDColorModeBlock colorModeBlock2(DuoTone);
    Q_ASSERT(!colorModeBlock2.valid());

    PSDColorModeBlock colorModeBlock3(RGB);
    Q_ASSERT(colorModeBlock3.valid());
}

void PSDColorModeBlockTest::testLoadingRGB()
{
    QString filename = QString(FILES_DATA_DIR) + "/sources/2.psd";
    QFile f(filename);
    f.open(QIODevice::ReadOnly);
    PSDHeader header;
    header.read(&f);

    QVERIFY(header.colormode == RGB);

    PSDColorModeBlock colorModeBlock(header.colormode);
    bool retval = colorModeBlock.read(&f);
    Q_ASSERT(retval); Q_UNUSED(retval);
    Q_ASSERT(colorModeBlock.valid());

}

void PSDColorModeBlockTest::testLoadingIndexed()
{
    QString filename = QString(FILES_DATA_DIR) + "/sources/100x100indexed.psd";
    QFile f(filename);
    f.open(QIODevice::ReadOnly);
    PSDHeader header;
    header.read(&f);

    QVERIFY(header.colormode == Indexed);

    PSDColorModeBlock colorModeBlock(header.colormode);
    bool retval = colorModeBlock.read(&f);
    Q_ASSERT(retval); Q_UNUSED(retval);
    Q_ASSERT(colorModeBlock.valid());

}


SIMPLE_TEST_MAIN(PSDColorModeBlockTest)

