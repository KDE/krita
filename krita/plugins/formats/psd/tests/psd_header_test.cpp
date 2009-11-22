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
#include <klocale.h>
#include <qtest_kde.h>
#include "../psd_header.h"
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
    QString filename = QString(FILES_DATA_DIR) + "/sources/1.psd";
    QFile f(filename);
    f.open(QIODevice::ReadOnly);
    PSDHeader header;
    header.read(&f);

    QVERIFY(header.m_signature == QString("8BPS"));
    QVERIFY(header.m_version == 1);
    QVERIFY(header.m_nChannels == 3);
    QVERIFY(header.m_width == 100 );
    QVERIFY(header.m_height == 100);
    QVERIFY(header.m_channelDepth == 16);
    QVERIFY(header.m_colormode == RGB);

}

void PSDHeaderTest::testRoundTripping()
{
    QString filename = "test.psd";
    QFile f(filename);
    f.open(QIODevice::ReadWrite);
    PSDHeader header;
    Q_ASSERT(!header.valid());
    header.m_signature = "8BPS";
    header.m_version = 1;
    header.m_nChannels = 3;
    header.m_width = 1000;
    header.m_height = 1000;
    header.m_channelDepth = 8;
    header.m_colormode = RGB;
    Q_ASSERT(header.valid());
    bool retval = header.write(&f);
    Q_ASSERT(retval);

    f.close();
    f.open(QIODevice::ReadOnly);
    PSDHeader header2;
    retval = header2.read(&f);
    Q_ASSERT(retval);

    QCOMPARE(header.m_signature, header2.m_signature);
    QVERIFY(header.m_version == header2.m_version);
    QVERIFY(header.m_nChannels == header2.m_nChannels);
    QVERIFY(header.m_width == header2.m_width);
    QVERIFY(header.m_height == header2.m_height);
    QVERIFY(header.m_channelDepth == header2.m_channelDepth);
    QVERIFY(header.m_colormode == header2.m_colormode);
}



QTEST_KDEMAIN(PSDHeaderTest, GUI)

#include "psd_header_test.moc"
