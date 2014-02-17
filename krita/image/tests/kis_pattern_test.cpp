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

#include <qtest_kde.h>
#include "KoPattern.h"

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

    qDebug() << "PNG Name:" << pngPattern.name();
    qDebug() << "PNG Filename:" << pngPattern.filename();

    pngPattern.setFilename(patFilename);
    pngPattern.save();

    KoPattern patPattern(patFilename);
    QVERIFY(patPattern.load());

    qDebug() << "PAT Name:" << patPattern.name();
    qDebug() << "PAT Filename:" << patPattern.filename();

    qDebug() << pngPattern.image().format();
    qDebug() << patPattern.image().format();

    QCOMPARE(pngPattern.image().convertToFormat(QImage::Format_ARGB32), patPattern.image().convertToFormat(QImage::Format_ARGB32));
    QCOMPARE(pngPattern.md5(), patPattern.md5());
}


QTEST_KDEMAIN(KoPatternTest, GUI)
#include "kis_pattern_test.moc"
