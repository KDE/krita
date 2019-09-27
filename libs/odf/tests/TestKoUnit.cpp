/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright 2012 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "TestKoUnit.h"

#include <KoUnit.h>

#include <QTest>

Q_DECLARE_METATYPE(KoUnit::Type)
Q_DECLARE_METATYPE(KoUnit::ListOptions)


void TestKoUnit::testSimpleConstructor()
{
    KoUnit unit;
    QCOMPARE(unit.type(), KoUnit::Point);

    KoUnit otherUnit;
    QCOMPARE(unit, otherUnit);
}


void TestKoUnit::testConstructor_data()
{
    QTest::addColumn<KoUnit::Type>("type");

    QTest::newRow("point") << KoUnit::Point;
    QTest::newRow("pica") << KoUnit::Pica;
    QTest::newRow("millimeter") << KoUnit::Millimeter;
}

void TestKoUnit::testConstructor()
{
    QFETCH(KoUnit::Type, type);

    KoUnit unit(type);
    QCOMPARE(unit.type(), type);
}

void TestKoUnit::testPixelConstructor()
{
    KoUnit unit(KoUnit::Pixel, 0.5);
    QCOMPARE(unit.type(), KoUnit::Pixel);
    QCOMPARE(unit.toUserValue(100, false), (qreal)50);
}

void TestKoUnit::testAssignOperator_data()
{
    QTest::addColumn<KoUnit::Type>("type");
    QTest::addColumn<KoUnit::Type>("otherType");

    QTest::newRow("point-pica") << KoUnit::Point << KoUnit::Pica;
    QTest::newRow("pica-point") << KoUnit::Pica << KoUnit::Point;
    QTest::newRow("millimeter-inch") << KoUnit::Millimeter << KoUnit::Inch;
}

void TestKoUnit::testAssignOperator()
{
    QFETCH(KoUnit::Type, type);
    QFETCH(KoUnit::Type, otherType);

    KoUnit unit(type);

    KoUnit otherUnit(otherType);
    unit = otherUnit;
    QCOMPARE(unit, otherUnit);
}

void TestKoUnit::testVariant()
{
    KoUnit unit(KoUnit::Pixel, 0.5);

    QVariant variant;
    variant.setValue(unit);
    QCOMPARE(variant.value<KoUnit>(), unit);
}

void TestKoUnit::testFromSymbol_data()
{
    QTest::addColumn<KoUnit::Type>("type");
    QTest::addColumn<QString>("symbol");
    QTest::addColumn<bool>("isOkay");

    QTest::newRow("point") << KoUnit::Point << QString::fromLatin1("pt") << true;
    QTest::newRow("pica") << KoUnit::Pica << QString::fromLatin1("pi") << true;
    QTest::newRow("pixel") << KoUnit::Pixel << QString::fromLatin1("px") << true;
    QTest::newRow("inch") << KoUnit::Inch << QString::fromLatin1("in") << true;
    QTest::newRow("inch2") << KoUnit::Inch << QString::fromLatin1("inch") << true;
    QTest::newRow("decimeter") << KoUnit::Decimeter << QString::fromLatin1("dm") << true;
    QTest::newRow("badSymbol") << KoUnit::Point << QString::fromLatin1("badSymbol") << false;
}

void TestKoUnit::testFromSymbol()
{
    QFETCH(KoUnit::Type, type);
    QFETCH(QString, symbol);
    QFETCH(bool, isOkay);

    bool ok;
    KoUnit unit = KoUnit::fromSymbol(symbol, &ok);

    QCOMPARE(ok, isOkay);
    if (isOkay) {
        QCOMPARE(unit.type(), type);
    }
}

void TestKoUnit::testListForUi_data()
{
    QTest::addColumn<KoUnit::ListOptions>("listOptions");
    QTest::addColumn<int>("index");

    const QVector<KoUnit::ListOptions> optionsList =
        QVector<KoUnit::ListOptions>() << KoUnit::HidePixel << KoUnit::ListAll;
    static const char* const optionsName[2] = {"HidePixel", "ListDefault"};
    static const char* const indexName[3] = {"-start", "-middle", "-end"};

    for (int o = 0; o < optionsList.count(); ++o) {
        const KoUnit::ListOptions options = optionsList.at(o);
        const int unitCount = KoUnit::listOfUnitNameForUi(options).count();
        for (int i = 0; i < 3; ++i) {
            const int index =
                (i == 0) ? 0 :
                (i == 1) ? unitCount/2 :
                /*i == 2*/ unitCount-1;

            const QString rowName = QLatin1String(optionsName[o]) + QLatin1String(indexName[i]);

            QTest::newRow(rowName.toLatin1().constData()) << options << index;
        }
    }
}

void TestKoUnit::testListForUi()
{
    QFETCH(KoUnit::ListOptions, listOptions);
    QFETCH(int, index);

    KoUnit unit = KoUnit::fromListForUi(index, listOptions);

    QCOMPARE(unit.indexInListForUi(listOptions), index);
}


void TestKoUnit::testToUserValue_data()
{
    // regression test to check whether changes in KoUnit changes the conversion output

    QTest::addColumn<KoUnit::Type>("type");
    QTest::addColumn<qreal>("value");
    QTest::addColumn<qreal>("expected");
    QTest::addColumn<qreal>("expectedRounded");


    // 1000.0
    QTest::newRow("point 1000") << KoUnit::Point << 1000.0 << 1000.0 << 1000.0;
    QTest::newRow("pica 1000") << KoUnit::Pica  << 1000.0 << 83.3333330000 << 83.3333300000;
    QTest::newRow("pixel 1000") << KoUnit::Pixel << 1000.0 << 1000.0 << 1000.0;
    QTest::newRow("inch 1000") << KoUnit::Inch << 1000.0 << 13.8888888889 << 13.8888800000;
    QTest::newRow("decimeter 1000") << KoUnit::Decimeter  << 1000.0 << 3.5277716700 << 3.5277000000;

    // 1.37
    QTest::newRow("point 1.37") << KoUnit::Point << 1.37 << 1.37 << 1.37;
    QTest::newRow("pica 1.37") << KoUnit::Pica  << 1.37 << 0.114166666210 << 0.1141600000;
    QTest::newRow("pixel 1.37") << KoUnit::Pixel << 1.37 << 1.37 << 1.37;
    QTest::newRow("inch 1.37") << KoUnit::Inch << 1.37 << 0.019027777777779 << 0.019020000000;
    QTest::newRow("decimeter 1.37") << KoUnit::Decimeter  << 1.37 <<  0.004833047187900 << 0.004800000000;

    // 0.0001111
    QTest::newRow("point 0.0001111") << KoUnit::Point << 0.0001111 << 0.000111100000  << 0.0 ;
    QTest::newRow("pica 0.0001111") << KoUnit::Pica  << 0.0001111 << 0.00000925833329630 << 0.0;
    QTest::newRow("pixel 0.0001111") << KoUnit::Pixel << 0.0001111 << 0.000111100000  << 0.000111100000 ;
    QTest::newRow("inch 0.0001111") << KoUnit::Inch << 0.0001111 << 0.00000154305555555568 << 0.0;
    QTest::newRow("decimeter 0.0001111") << KoUnit::Decimeter  << 0.0001111 << 0.00000039193543253700 << 0.0;
}

void TestKoUnit::testToUserValue()
{
    QFETCH(KoUnit::Type, type);
    QFETCH(qreal, value);
    QFETCH(qreal, expected);
    QFETCH(qreal, expectedRounded);

    KoUnit unit = KoUnit(type);

    qreal exphere = unit.toUserValue(value, false);
    qreal expRound = unit.toUserValue(value, true);

    QCOMPARE(exphere, expected);
    QCOMPARE(expRound, expectedRounded);

}

QTEST_GUILESS_MAIN(TestKoUnit)
