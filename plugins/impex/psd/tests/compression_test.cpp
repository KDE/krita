/*
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "compression_test.h"

#include <simpletest.h>
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

SIMPLE_TEST_MAIN(CompressionTest)

