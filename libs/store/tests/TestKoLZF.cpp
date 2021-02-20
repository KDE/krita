/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2015 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "TestKoLZF.h"

#include <KoLZF.h>

#include <QTest>

void TestKoLZF::testArrayCompressionEmpty_data()
{
    QTest::addColumn<char>("canary");
    QTest::newRow("00") << '\0';
    QTest::newRow("FF") << '\xFF';
}

void TestKoLZF::testArrayCompressionEmpty()
{
    QFETCH(char, canary);

    const char inputData[] = "test";
    // use one more byte and see if it stays untouched
    char outputdata[1];
    outputdata[0] = canary;

    const int compressedDataLength = KoLZF::compress(inputData, 0, outputdata, 0);

    QCOMPARE(compressedDataLength, 0);
    QCOMPARE(outputdata[0], canary);
}

void TestKoLZF::testArrayCompressionNullPointerInput()
{
    char outputdata[4];

    const int compressedDataLength = KoLZF::compress(0, 4, outputdata, 4);

    QCOMPARE(compressedDataLength, 0);
}

void TestKoLZF::testArrayCompressionNullPointerOutput()
{
    const char inputData[] = "test";

    const int compressedDataLength = KoLZF::compress(inputData, 4, 0, 4);

    QCOMPARE(compressedDataLength, 0);
}

void TestKoLZF::testArrayDecompressionEmpty_data()
{
    QTest::addColumn<char>("canary");
    QTest::newRow("00") << '\0';
    QTest::newRow("FF") << '\xFF';
}

void TestKoLZF::testArrayDecompressionEmpty()
{
    QFETCH(char, canary);

    const char inputData[] = "test";
    char outputdata[1];
    outputdata[0] = canary;

    const int uncompressedDataLength = KoLZF::decompress(inputData, 0, outputdata, 0);

    QCOMPARE(uncompressedDataLength, 0);
    QCOMPARE(outputdata[0], canary);
}

void TestKoLZF::testArrayDecompressionNullPointerInput()
{
    char outputdata[4];

    const int uncompressedDataLength = KoLZF::decompress(0, 0, outputdata, 4);

    QCOMPARE(uncompressedDataLength, 0);
}

void TestKoLZF::testArrayDecompressionNullPointerOutput()
{
    const char inputData[] = "test";

    const int uncompressedDataLength = KoLZF::decompress(inputData, 4, 0, 4);

    QCOMPARE(uncompressedDataLength, 0);
}

Q_DECLARE_METATYPE(char*)

void TestKoLZF::testArrayRoundtripDifferentSizes_data()
{
    QTest::addColumn<char*>("data");
    QTest::addColumn<int>("size");
    QTest::addColumn<char>("canary");

    static const char canary[] = {'\0',  '\xFF'};
    static const int canaryCount = sizeof( canary ) / sizeof( canary[0] );
    static const int fillMethodCount = 2;
    static const char * const fillMethodName[fillMethodCount] = {"uni",  "series"};

    for(int c = 0; c < canaryCount; ++c) {
        for(int i = 1; i < 512; ++i) {
            for (int f = 0; f < 2; ++f) {
                char *data = new char[i];
                if (f == 0) {
                    memset(data, 't', i);
                } else {
                    for (int b = 0; b < i; ++b) {
                        data[b] = b % 256;
                    }
                }

                QByteArray ba = QByteArray::number(i)+'-'+QByteArray(1, canary[c]).toHex()+'-'+QByteArray(fillMethodName[f]);
                QTest::newRow(ba.constData())
                    << data << i << canary[c];
            }
        }
    }
}

void TestKoLZF::testArrayRoundtripDifferentSizes()
{
    QFETCH(char*, data);
    QFETCH(int, size);
    QFETCH(char, canary);

    char * compressedData = new char[size+1];
    compressedData[size] = canary;

    // try
    const int compressedDataLength = KoLZF::compress(data, size, compressedData, size);

    QCOMPARE(compressedData[size], canary);

    const bool compressed = (compressedDataLength != 0);
    // done with testing if not compressed
    if (!compressed) {
        delete [] data;
        delete [] compressedData;
        return;
    }

    // now try uncompressing
    char * uncompressedData = new char[size+1];
    uncompressedData[size] = canary;

    const int uncompressedDataLength =
        KoLZF::decompress(compressedData, compressedDataLength, uncompressedData, size);

    QVERIFY(uncompressedDataLength != 0);
    QCOMPARE(uncompressedDataLength, size);
    QCOMPARE(memcmp(uncompressedData, data, size), 0 );
    QCOMPARE(uncompressedData[size], canary);

    delete [] data;
    delete [] compressedData;
    delete [] uncompressedData;
}


void TestKoLZF::testByteArrayCompressionEmpty()
{
    const QByteArray empty;

    // try
    const QByteArray compressed = KoLZF::compress(empty);

    // only metadata
    QCOMPARE(compressed.size(), 5);
    // size is 0
    QCOMPARE(compressed.at(0), static_cast<char>(0));
    QCOMPARE(compressed.at(1), static_cast<char>(0));
    QCOMPARE(compressed.at(2), static_cast<char>(0));
    QCOMPARE(compressed.at(3), static_cast<char>(0));
    // uncompressed
    QCOMPARE(compressed.at(4), static_cast<char>(0));
}

void TestKoLZF::testByteArrayDecompressionEmpty()
{
    const char emptyCompressedRaw[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
    const QByteArray emptyCompressed = QByteArray::fromRawData(emptyCompressedRaw, 5);

    QByteArray uncompressed;

    // try
    KoLZF::decompress(emptyCompressed, uncompressed);

    // empty
    QCOMPARE(uncompressed.size(), 0);
}

void TestKoLZF::testByteArrayRoundtripDifferentSizes_data()
{
    QTest::addColumn<QByteArray>("data");

    static const int fillMethodCount = 2;
    static const char * const fillMethodName[fillMethodCount] = {"uni",  "series"};

    for(int i = 0; i < 512; ++i) {
        for (int f = 0; f < 2; ++f) {
            QByteArray data;

            if (f == 0) {
                data = QByteArray(i, 't');
            } else {
                data.resize(i);
                for (int b = 0; b < i; ++b) {
                    data[b] = b % 256;
                }
            }
            QByteArray ba = QByteArray::number(i)+'-'+fillMethodName[f];
            QTest::newRow(ba.constData()) << data;
        }
    }
}

void TestKoLZF::testByteArrayRoundtripDifferentSizes()
{
    QFETCH(QByteArray, data);

    // try
    const QByteArray compressed = KoLZF::compress(data);
    QByteArray uncompressed;
    KoLZF::decompress(compressed, uncompressed);

    QCOMPARE(uncompressed, data);
}


QTEST_GUILESS_MAIN(TestKoLZF)
