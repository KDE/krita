/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_pattern_test.h"

#include <simpletest.h>
#include <resources/KoPattern.h>

#include <QCryptographicHash>
#include <QByteArray>

#include <KoMD5Generator.h>
#include <kis_debug.h>
#include <KisGlobalResourcesInterface.h>

void KoPatternTest::testCreation()
{
    KoPattern test(QString(FILES_DATA_DIR) + '/' + "pattern.pat");
}

void KoPatternTest::testRoundTripMd5()
{
    QString filename(QString(FILES_DATA_DIR) + '/' + "test_pattern.png");
    QString patFilename("test_pattern.pat");

    KoPattern pngPattern(filename);
    QVERIFY(pngPattern.load(KisGlobalResourcesInterface::instance()));
    pngPattern.setMD5(KoMD5Generator::generateHash(filename));

    dbgKrita << "PNG Name:" << pngPattern.name();
    dbgKrita << "PNG Filename:" << pngPattern.filename();

    pngPattern.setFilename(patFilename);
    pngPattern.save();

    KoPattern patPattern(patFilename);
    QVERIFY(patPattern.load(KisGlobalResourcesInterface::instance()));
    patPattern.setMD5(KoMD5Generator::generateHash(patFilename));

    dbgKrita << "PAT Name:" << patPattern.name();
    dbgKrita << "PAT Filename:" << patPattern.filename();

    dbgKrita << pngPattern.pattern().format();
    dbgKrita << patPattern.pattern().format();

    QCOMPARE(pngPattern.pattern().convertToFormat(QImage::Format_ARGB32), patPattern.pattern().convertToFormat(QImage::Format_ARGB32));
    QImage im1 = pngPattern.pattern().convertToFormat(QImage::Format_ARGB32);
    QImage im2 = patPattern.pattern().convertToFormat(QImage::Format_ARGB32);

    QCryptographicHash h1(QCryptographicHash::Md5);
#if QT_VERSION >= QT_VERSION_CHECK(5,10,0)
    h1.addData(QByteArray::fromRawData((const char*)im1.constBits(), im1.sizeInBytes()));
#else
    h1.addData(QByteArray::fromRawData((const char*)im1.constBits(), im1.byteCount()));
#endif

    QCryptographicHash h2(QCryptographicHash::Md5);
#if QT_VERSION >= QT_VERSION_CHECK(5,10,0)
    h2.addData(QByteArray::fromRawData((const char*)im2.constBits(), im2.sizeInBytes()));
#else
    h2.addData(QByteArray::fromRawData((const char*)im2.constBits(), im2.byteCount()));
#endif

    // Compares the images: they should be the same
    QCOMPARE(h1.result(), h2.result());
    QCOMPARE(im1, im2);
    // Compares the md5sum taken from the file: they should be different
    QVERIFY(pngPattern.md5() != patPattern.md5());
}


SIMPLE_TEST_MAIN(KoPatternTest)
