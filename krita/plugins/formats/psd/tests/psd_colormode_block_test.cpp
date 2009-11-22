/*
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
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

#include "psd_colormode_block_test.h"

#include <QTest>
#include <QCoreApplication>
#include <klocale.h>
#include <qtest_kde.h>
#include "../psd_header.h"
#include "../psd_colormode_block.h"
#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif

void PSDColorModeBlockTest::testCreation()
{
    PSDColorModeBlock colorModeBlock1(PSDHeader::Indexed);
    Q_ASSERT(!colorModeBlock1.valid());

    PSDColorModeBlock colorModeBlock2(PSDHeader::DuoTone);
    Q_ASSERT(!colorModeBlock2.valid());

    PSDColorModeBlock colorModeBlock3(PSDHeader::RGB);
    Q_ASSERT(colorModeBlock3.valid());
}

void PSDColorModeBlockTest::testLoadingRGB()
{
    QString filename = QString(FILES_DATA_DIR) + "/sources/1.psd";
    QFile f(filename);
    f.open(QIODevice::ReadOnly);
    PSDHeader header;
    header.read(&f);

    QVERIFY(header.m_colormode == PSDHeader::RGB);

    PSDColorModeBlock colorModeBlock(header.m_colormode);
    bool retval = colorModeBlock.read(&f);
    Q_ASSERT(retval);
    Q_ASSERT(colorModeBlock.valid());

}

void PSDColorModeBlockTest::testLoadingIndexed()
{
    QString filename = QString(FILES_DATA_DIR) + "/sources/indexed.psd";
    QFile f(filename);
    f.open(QIODevice::ReadOnly);
    PSDHeader header;
    header.read(&f);

    QVERIFY(header.m_colormode == PSDHeader::Indexed);

    PSDColorModeBlock colorModeBlock(header.m_colormode);
    bool retval = colorModeBlock.read(&f);
    Q_ASSERT(retval);
    Q_ASSERT(colorModeBlock.valid());

}


QTEST_KDEMAIN(PSDColorModeBlockTest, GUI)

#include "psd_colormode_block_test.moc"
