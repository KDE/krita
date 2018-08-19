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

#include "psd_header_test.h"

#include <QTest>
#include <QCoreApplication>
#include <klocalizedstring.h>
#include <psd_header.h>
#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif

void PSDHeaderTest::testCreation()
{
    PSDHeader header;
    Q_ASSERT(!header.valid());
}

void PSDHeaderTest::testLoading()
{
    QString filename = QString(FILES_DATA_DIR) + "/sources/2.psd";
    QFile f(filename);
    f.open(QIODevice::ReadOnly);
    PSDHeader header;
    header.read(&f);

    QVERIFY(header.signature == QString("8BPS"));
    QVERIFY(header.version == 1);
    QVERIFY(header.nChannels == 3);
    QVERIFY(header.width == 100 );
    QVERIFY(header.height == 100);
    QVERIFY(header.channelDepth == 8);
    QVERIFY(header.colormode == RGB);

}

void PSDHeaderTest::testRoundTripping()
{
    QString filename = "test.psd";
    QFile f(filename);
    f.open(QIODevice::ReadWrite);
    PSDHeader header;
    Q_ASSERT(!header.valid());
    header.signature = "8BPS";
    header.version = 1;
    header.nChannels = 3;
    header.width = 1000;
    header.height = 1000;
    header.channelDepth = 8;
    header.colormode = RGB;
    Q_ASSERT(header.valid());
    bool retval = header.write(&f);
    Q_ASSERT(retval); Q_UNUSED(retval);

    f.close();
    f.open(QIODevice::ReadOnly);
    PSDHeader header2;
    retval = header2.read(&f);
    Q_ASSERT(retval);

    QCOMPARE(header.signature, header2.signature);
    QVERIFY(header.version == header2.version);
    QVERIFY(header.nChannels == header2.nChannels);
    QVERIFY(header.width == header2.width);
    QVERIFY(header.height == header2.height);
    QVERIFY(header.channelDepth == header2.channelDepth);
    QVERIFY(header.colormode == header2.colormode);
}



QTEST_MAIN(PSDHeaderTest)

