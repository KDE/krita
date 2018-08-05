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

#include "compression_test.h"

#include <QTest>
#include <QCoreApplication>
#include <klocalizedstring.h>
#include <compression.h>
#include <QByteArray>
#include <QBuffer>
#include <QDataStream>

#include <math.h>

#include <kis_debug.h>

void CompressionTest::testCompressionRLE()
{
    QByteArray ba("Twee eeee aaaaa asdasda47892347981    wwwwwwwwwwwwWWWWWWWWWW");
    QByteArray compressed = Compression::compress(ba, Compression::RLE);
    dbgKrita << compressed.size() << "uncompressed" << ba.size();

    QByteArray uncompressed = Compression::uncompress(ba.size(), compressed, Compression::RLE);
    QVERIFY(qstrcmp(ba, uncompressed) == 0);

    ba.clear();
    QDataStream ds(&ba, QIODevice::WriteOnly);
    for (int i = 0; i < 500; ++i) {
        ds << rand();
    }
    compressed = Compression::compress(ba, Compression::RLE);
    dbgKrita << compressed.size() << "uncompressed" << ba.size();
    uncompressed = Compression::uncompress(ba.size(), compressed, Compression::RLE);
    QVERIFY(qstrcmp(ba, uncompressed) == 0);


}


void CompressionTest::testCompressionZIP()
{
    QByteArray ba("Twee eeee aaaaa asdasda47892347981    wwwwwwwwwwwwWWWWWWWWWW");
    QByteArray compressed = Compression::compress(ba, Compression::ZIP);
    dbgKrita << compressed.size() << "uncompressed" << ba.size();

    QByteArray uncompressed = Compression::uncompress(ba.size(), compressed, Compression::ZIP);
    QVERIFY(qstrcmp(ba, uncompressed) == 0);

    ba.clear();
    QDataStream ds(&ba, QIODevice::WriteOnly);
    for (int i = 0; i < 500; ++i) {
        ds << rand();
    }
    compressed = Compression::compress(ba, Compression::ZIP);
    dbgKrita << compressed.size() << "uncompressed" << ba.size();
    uncompressed = Compression::uncompress(ba.size(), compressed, Compression::ZIP);
    QVERIFY(qstrcmp(ba, uncompressed) == 0);

}


void CompressionTest::testCompressionUncompressed()
{
    QByteArray ba("Twee eeee aaaaa asdasda47892347981    wwwwwwwwwwwwWWWWWWWWWW");
    QByteArray compressed = Compression::compress(ba, Compression::Uncompressed);
    dbgKrita << compressed.size() << "uncompressed" << ba.size();

    QByteArray uncompressed = Compression::uncompress(ba.size(), compressed, Compression::Uncompressed);
    QVERIFY(qstrcmp(ba, uncompressed) == 0);

    ba.clear();
    QDataStream ds(&ba, QIODevice::WriteOnly);
    for (int i = 0; i < 500; ++i) {
        ds << rand();
    }
    compressed = Compression::compress(ba, Compression::Uncompressed);
    dbgKrita << compressed.size() << "uncompressed" << ba.size();
    uncompressed = Compression::uncompress(ba.size(), compressed, Compression::Uncompressed);
    QVERIFY(qstrcmp(ba, uncompressed) == 0);


}

QTEST_MAIN(CompressionTest)

