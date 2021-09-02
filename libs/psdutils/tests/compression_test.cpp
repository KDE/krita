/*
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "compression_test.h"

#include <QBuffer>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <cmath>
#include <klocalizedstring.h>

#include <compression.h>
#include <kis_debug.h>
#include <simpletest.h>

void CompressionTest::testCompressionRLE()
{
    QByteArray ba("Twee eeee aaaaa asdasda47892347981    wwwwwwwwwwwwWWWWWWWWWW");
    QByteArray compressed = Compression::compress(ba, psd_compression_type::RLE);
    QVERIFY(compressed.size() > 0);
    dbgKrita << compressed.size() << "uncompressed" << ba.size();

    QByteArray uncompressed = Compression::uncompress(ba.size(), compressed, psd_compression_type::RLE);
    QVERIFY(uncompressed.size() > 0);
    QVERIFY(qstrcmp(ba, uncompressed) == 0);

    ba.clear();
    QDataStream ds(&ba, QIODevice::WriteOnly);
    for (int i = 0; i < 500; ++i) {
        ds << rand();
    }
    compressed = Compression::compress(ba, psd_compression_type::RLE);
    dbgKrita << compressed.size() << "uncompressed" << ba.size();
    QVERIFY(compressed.size() > 0);
    uncompressed = Compression::uncompress(ba.size(), compressed, psd_compression_type::RLE);
    QVERIFY(uncompressed.size() > 0);
    QVERIFY(qstrcmp(ba, uncompressed) == 0);
}

void CompressionTest::testCompressionZIP()
{
    QByteArray ba("Twee eeee aaaaa asdasda47892347981    wwwwwwwwwwwwWWWWWWWWWW");
    QByteArray compressed = Compression::compress(ba, psd_compression_type::ZIP);
    QVERIFY(compressed.size() > 0);
    dbgKrita << compressed.size() << "uncompressed" << ba.size();

    QByteArray uncompressed = Compression::uncompress(ba.size(), compressed, psd_compression_type::ZIP);
    QVERIFY(uncompressed.size() > 0);
    QVERIFY(qstrcmp(ba, uncompressed) == 0);

    ba.clear();
    QDataStream ds(&ba, QIODevice::WriteOnly);
    for (int i = 0; i < 500; ++i) {
        ds << rand();
    }
    compressed = Compression::compress(ba, psd_compression_type::ZIP);
    QVERIFY(compressed.size() > 0);
    dbgKrita << compressed.size() << "uncompressed" << ba.size();
    uncompressed = Compression::uncompress(ba.size(), compressed, psd_compression_type::ZIP);
    QVERIFY(uncompressed.size() > 0);
    QVERIFY(qstrcmp(ba, uncompressed) == 0);
}

void CompressionTest::testCompressionUncompressed()
{
    QByteArray ba("Twee eeee aaaaa asdasda47892347981    wwwwwwwwwwwwWWWWWWWWWW");
    QByteArray compressed = Compression::compress(ba, psd_compression_type::Uncompressed);
    QVERIFY(compressed.size() > 0);
    dbgKrita << compressed.size() << "uncompressed" << ba.size();

    QByteArray uncompressed = Compression::uncompress(ba.size(), compressed, psd_compression_type::Uncompressed);
    QVERIFY(uncompressed.size() > 0);
    QVERIFY(qstrcmp(ba, uncompressed) == 0);

    ba.clear();
    QDataStream ds(&ba, QIODevice::WriteOnly);
    for (int i = 0; i < 500; ++i) {
        ds << rand();
    }
    compressed = Compression::compress(ba, psd_compression_type::Uncompressed);
    dbgKrita << compressed.size() << "uncompressed" << ba.size();
    QVERIFY(compressed.size() > 0);
    uncompressed = Compression::uncompress(ba.size(), compressed, psd_compression_type::Uncompressed);
    QVERIFY(uncompressed.size() > 0);
    QVERIFY(qstrcmp(ba, uncompressed) == 0);
}

SIMPLE_TEST_MAIN(CompressionTest)
