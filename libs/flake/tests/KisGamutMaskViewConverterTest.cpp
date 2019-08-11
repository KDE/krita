/*
 *  Copyright (c) 2019 Anna Medonosova <anna.medonosova@gmail.com>
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

#include <QScopedPointer>

#include <KisGamutMaskViewConverter.h>
#include <kis_assert_exception.h>

#include "KisGamutMaskViewConverterTest.h"



KisGamutMaskViewConverterTest::KisGamutMaskViewConverterTest(QObject *parent) : QObject(parent)
{

}

void KisGamutMaskViewConverterTest::testDocumentToViewX()
{
    QFETCH(qreal, input);
    QFETCH(qreal, expectedOutput);
    QFETCH(QSizeF, maskSize);
    QFETCH(QSize, viewSize);

    QScopedPointer<KisGamutMaskViewConverter> converter(new KisGamutMaskViewConverter());
    converter->setMaskSize(maskSize);
    converter->setViewSize(viewSize);

    qreal converterOutput = converter->documentToViewX(input);
    QCOMPARE(converterOutput, expectedOutput);
}

void KisGamutMaskViewConverterTest::testDocumentToViewX_data()
{
    QTest::addColumn<qreal>("input");
    QTest::addColumn<qreal>("expectedOutput");
    QTest::addColumn<QSizeF>("maskSize");
    QTest::addColumn<QSize>("viewSize");

    QTest::newRow("mask < view") << 2.0 << 4.0 << QSizeF(200.0, 200.0) << QSize(400, 400);
    QTest::newRow("view < mask") << 2.0 << 1.0 << QSizeF(400.0, 400.0) << QSize(200, 200);

    QTest::newRow("view not square") << 2.0 << 4.0 << QSizeF(200.0, 300.0) << QSize(400, 400);
    QTest::newRow("mask not square") << 2.0 << 4.0 << QSizeF(200.0, 200.0) << QSize(400, 600);
}

void KisGamutMaskViewConverterTest::testDocumentToViewY()
{
    QFETCH(qreal, input);
    QFETCH(qreal, expectedOutput);
    QFETCH(QSizeF, maskSize);
    QFETCH(QSize, viewSize);

    QScopedPointer<KisGamutMaskViewConverter> converter(new KisGamutMaskViewConverter());
    converter->setMaskSize(maskSize);
    converter->setViewSize(viewSize);

    qreal converterOutput = converter->documentToViewY(input);
    QCOMPARE(converterOutput, expectedOutput);
}

void KisGamutMaskViewConverterTest::testDocumentToViewY_data()
{

    QTest::addColumn<qreal>("input");
    QTest::addColumn<qreal>("expectedOutput");
    QTest::addColumn<QSizeF>("maskSize");
    QTest::addColumn<QSize>("viewSize");

    QTest::newRow("mask < view") << 2.0 << 4.0 << QSizeF(200.0, 200.0) << QSize(400, 400);
    QTest::newRow("view < mask") << 2.0 << 1.0 << QSizeF(400.0, 400.0) << QSize(200, 200);

    QTest::newRow("view not square") << 2.0 << 4.0 << QSizeF(200.0, 300.0) << QSize(400, 400);
    QTest::newRow("mask not square") << 2.0 << 4.0 << QSizeF(200.0, 200.0) << QSize(400, 600);
}

void KisGamutMaskViewConverterTest::testViewToDocumentX()
{
    QFETCH(qreal, input);
    QFETCH(qreal, expectedOutput);
    QFETCH(QSizeF, maskSize);
    QFETCH(QSize, viewSize);

    QScopedPointer<KisGamutMaskViewConverter> converter(new KisGamutMaskViewConverter());
    converter->setMaskSize(maskSize);
    converter->setViewSize(viewSize);

    qreal converterOutput = converter->viewToDocumentX(input);
    QCOMPARE(converterOutput, expectedOutput);
}

void KisGamutMaskViewConverterTest::testViewToDocumentX_data()
{
    QTest::addColumn<qreal>("input");
    QTest::addColumn<qreal>("expectedOutput");
    QTest::addColumn<QSizeF>("maskSize");
    QTest::addColumn<QSize>("viewSize");

    QTest::newRow("mask < view") << 4.0 << 2.0 << QSizeF(200.0, 200.0) << QSize(400, 400);
    QTest::newRow("view < mask") << 1.0 << 2.0 << QSizeF(400.0, 400.0) << QSize(200, 200);

    QTest::newRow("view not square") << 4.0 << 2.0 << QSizeF(200.0, 300.0) << QSize(400, 400);
    QTest::newRow("mask not square") << 4.0 << 2.0 << QSizeF(200.0, 200.0) << QSize(400, 600);
}

void KisGamutMaskViewConverterTest::testViewToDocumentY()
{
    QFETCH(qreal, input);
    QFETCH(qreal, expectedOutput);
    QFETCH(QSizeF, maskSize);
    QFETCH(QSize, viewSize);

    QScopedPointer<KisGamutMaskViewConverter> converter(new KisGamutMaskViewConverter());
    converter->setMaskSize(maskSize);
    converter->setViewSize(viewSize);

    qreal converterOutput = converter->viewToDocumentY(input);
    QCOMPARE(converterOutput, expectedOutput);
}

void KisGamutMaskViewConverterTest::testViewToDocumentY_data()
{
    QTest::addColumn<qreal>("input");
    QTest::addColumn<qreal>("expectedOutput");
    QTest::addColumn<QSizeF>("maskSize");
    QTest::addColumn<QSize>("viewSize");


    QTest::newRow("mask < view") << 4.0 << 2.0 << QSizeF(200.0, 200.0) << QSize(400, 400);
    QTest::newRow("view < mask") << 1.0 << 2.0 << QSizeF(400.0, 400.0) << QSize(200, 200);

    QTest::newRow("view not square") << 4.0 << 2.0 << QSizeF(200.0, 300.0) << QSize(400, 400);
    QTest::newRow("mask not square") << 4.0 << 2.0 << QSizeF(200.0, 200.0) << QSize(400, 600);
}

QTEST_MAIN(KisGamutMaskViewConverterTest);
