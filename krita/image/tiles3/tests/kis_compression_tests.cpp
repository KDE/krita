/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_compression_tests.h"
#include <qtest_kde.h>

#include <QImage>

#include "../../../sdk/tests/testutil.h"
#include "tiles3/swap/kis_lzf_compression.h"


#define TEST_FILE "tile.png"
//#define TEST_FILE "hakonepa.png"


#define PRINT_COMPRESSION(title,src,dst)                               \
    (qDebug() << title << dst << "/" << src << "\t|" << double(dst)/src)


void KisCompressionTests::roundTrip(KisAbstractCompression *compression)
{
    QImage referenceImage(QString(FILES_DATA_DIR) + QDir::separator() + TEST_FILE);

    QImage image(referenceImage);
    qint32 srcSize = image.byteCount();

    qint32 outputSize = compression->outputBufferSize(srcSize);
    quint8 *output = new quint8[outputSize];

    qint32 compressedBytes;
    qint32 uncompressedBytes;

    compressedBytes = compression->compress(image.bits(), srcSize,
                                            output, outputSize);
    uncompressedBytes = compression->decompress(output, compressedBytes,
                                                image.bits(), srcSize);

    PRINT_COMPRESSION("Single-pass:\t", uncompressedBytes, compressedBytes);

    QCOMPARE(uncompressedBytes, srcSize);
    QVERIFY(referenceImage == image);
}

void KisCompressionTests::roundTripTwoPass(KisAbstractCompression *compression)
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + TEST_FILE);

    qint32 srcSize = image.byteCount();
    qint32 outputSize = compression->outputBufferSize(srcSize);

    quint8 *output = new quint8[outputSize];

    qint32 compressedBytes;
    qint32 uncompressedBytes;

    quint8 *tempBuffer = new quint8[srcSize];
    KisAbstractCompression::linearizeColors(image.bits(), tempBuffer,
                                            srcSize, 4);

    compressedBytes = compression->compress(tempBuffer, srcSize,
                                            output, outputSize);

    uncompressedBytes = compression->decompress(output, compressedBytes,
                                                tempBuffer, srcSize);

    KisAbstractCompression::delinearizeColors(tempBuffer, image.bits(),
                                              srcSize, 4);

    PRINT_COMPRESSION("Two-pass:\t", uncompressedBytes, compressedBytes);

    QCOMPARE(uncompressedBytes, srcSize);

    QImage referenceImage(QString(FILES_DATA_DIR) + QDir::separator() + TEST_FILE);
    QVERIFY(referenceImage == image);
}

void KisCompressionTests::benchmarkCompression(KisAbstractCompression *compression)
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + TEST_FILE);

    qint32 srcSize = image.byteCount();
    qint32 outputSize = compression->outputBufferSize(srcSize);

    quint8 *output = new quint8[outputSize];

    qint32 compressedBytes;

    QBENCHMARK {
        compressedBytes = compression->compress(image.bits(), srcSize,
                                                output, outputSize);
    }
    Q_UNUSED(compressedBytes);
}

void KisCompressionTests::benchmarkCompressionTwoPass(KisAbstractCompression *compression)
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + TEST_FILE);

    qint32 srcSize = image.byteCount();
    qint32 outputSize = compression->outputBufferSize(srcSize);

    quint8 *output = new quint8[outputSize];

    qint32 compressedBytes;

    quint8 *tempBuffer = new quint8[srcSize];

    QBENCHMARK {
        KisAbstractCompression::linearizeColors(image.bits(), tempBuffer,
                                                srcSize, 4);
        compressedBytes = compression->compress(tempBuffer, srcSize,
                                                output, outputSize);
    }
    Q_UNUSED(compressedBytes);
}

void KisCompressionTests::benchmarkDecompression(KisAbstractCompression *compression)
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + TEST_FILE);

    qint32 srcSize = image.byteCount();
    qint32 outputSize = compression->outputBufferSize(srcSize);

    quint8 *output = new quint8[outputSize];

    qint32 compressedBytes;
    qint32 uncompressedBytes;

    compressedBytes = compression->compress(image.bits(), srcSize,
                                            output, outputSize);

    QBENCHMARK {
        uncompressedBytes = compression->decompress(output, compressedBytes,
                                                    image.bits(), srcSize);
    }
    Q_UNUSED(uncompressedBytes);
}

void KisCompressionTests::benchmarkDecompressionTwoPass(KisAbstractCompression *compression)
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + TEST_FILE);

    qint32 srcSize = image.byteCount();
    qint32 outputSize = compression->outputBufferSize(srcSize);

    quint8 *output = new quint8[outputSize];

    qint32 compressedBytes;
    qint32 uncompressedBytes;

    quint8 *tempBuffer = new quint8[srcSize];
    KisAbstractCompression::linearizeColors(image.bits(), tempBuffer, srcSize, 4);

    compressedBytes = compression->compress(tempBuffer, srcSize,
                                            output, outputSize);

    QBENCHMARK {
        uncompressedBytes = compression->decompress(output, compressedBytes,
                                                    tempBuffer, srcSize);

        KisAbstractCompression::delinearizeColors(tempBuffer, image.bits(),
                                                  srcSize, 4);
    }
    Q_UNUSED(uncompressedBytes);
}

void KisCompressionTests::testOverflow(KisAbstractCompression *compression)
{
    QFile file(QString(FILES_DATA_DIR) + QDir::separator() + TEST_FILE);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QByteArray array = file.readAll();

    qint32 srcSize = array.count();
    qint32 outputSize = compression->outputBufferSize(srcSize);

    quint8 *output = new quint8[outputSize];

    qint32 compressedBytes;
    compressedBytes = compression->compress((quint8*)array.data(), srcSize,
                                            output, outputSize);

    PRINT_COMPRESSION("Uncompressable:\t", srcSize, compressedBytes);
    qDebug() << "Max buffer size:" << outputSize;
    QVERIFY(compressedBytes <= outputSize);
}

void KisCompressionTests::testLzfRoundTrip()
{
    KisAbstractCompression *compression = new KisLzfCompression();

    roundTrip(compression);
    roundTripTwoPass(compression);

    delete compression;
}

void KisCompressionTests::testLzfOverflow()
{
    KisAbstractCompression *compression = new KisLzfCompression();
    testOverflow(compression);
    delete compression;
}

void KisCompressionTests::benchmarkMemCpy()
{
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + TEST_FILE);
    qint32 srcSize = image.byteCount();
    quint8 *output = new quint8[srcSize];

    QBENCHMARK {
        memcpy(output, image.bits(), srcSize);
    }
}

void KisCompressionTests::benchmarkCompressionLzf()
{
    KisAbstractCompression *compression = new KisLzfCompression();
    benchmarkCompression(compression);
    delete compression;
}

void KisCompressionTests::benchmarkCompressionLzfTwoPass()
{
    KisAbstractCompression *compression = new KisLzfCompression();
    benchmarkCompressionTwoPass(compression);
    delete compression;
}

void KisCompressionTests::benchmarkDecompressionLzf()
{
    KisAbstractCompression *compression = new KisLzfCompression();
    benchmarkDecompression(compression);
    delete compression;
}

void KisCompressionTests::benchmarkDecompressionLzfTwoPass()
{
    KisAbstractCompression *compression = new KisLzfCompression();
    benchmarkDecompressionTwoPass(compression);
    delete compression;
}

QTEST_KDEMAIN(KisCompressionTests, NoGUI)
#include "kis_compression_tests.moc"

