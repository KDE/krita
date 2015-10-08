/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include "kis_pattern_test.h"

#include <QTest>
#include "KoPattern.h"

#include <QCryptographicHash>
#include <QByteArray>

#include <kis_debug.h>

void KoPatternTest::testCreation()
{
    KoPattern test(QString(FILES_DATA_DIR) + QDir::separator() + "pattern.pat");
}

void KoPatternTest::testRoundTripMd5()
{
    QString filename(QString(FILES_DATA_DIR) + QDir::separator() + "test_pattern.png");
    QString patFilename("test_pattern.pat");

    KoPattern pngPattern(filename);
    QVERIFY(pngPattern.load());

    dbgKrita << "PNG Name:" << pngPattern.name();
    dbgKrita << "PNG Filename:" << pngPattern.filename();

    pngPattern.setFilename(patFilename);
    pngPattern.save();

    KoPattern patPattern(patFilename);
    QVERIFY(patPattern.load());

    dbgKrita << "PAT Name:" << patPattern.name();
    dbgKrita << "PAT Filename:" << patPattern.filename();

    dbgKrita << pngPattern.pattern().format();
    dbgKrita << patPattern.pattern().format();

    QCOMPARE(pngPattern.pattern().convertToFormat(QImage::Format_ARGB32), patPattern.pattern().convertToFormat(QImage::Format_ARGB32));
    QImage im1 = pngPattern.pattern().convertToFormat(QImage::Format_ARGB32);
    QImage im2 = patPattern.pattern().convertToFormat(QImage::Format_ARGB32);

    QCryptographicHash h1(QCryptographicHash::Md5);
    h1.addData(QByteArray::fromRawData((const char*)im1.constBits(), im1.byteCount()));

    QCryptographicHash h2(QCryptographicHash::Md5);
    h2.addData(QByteArray::fromRawData((const char*)im2.constBits(), im2.byteCount()));

    QCOMPARE(h1.result(), h2.result());
    QCOMPARE(im1, im2);
    QCOMPARE(pngPattern.md5(), patPattern.md5());
}


QTEST_MAIN(KoPatternTest)
