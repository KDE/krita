/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2015 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "TestKoXmlVector.h"

#include <KoXmlVector.h>

#include <QTest>
#include <QDebug>


enum TestEnum
{
    FirstType = 0,
    SecondType = 1,
    ThirdType = 2,
    FourthType = 3,
    FifthType = 4
};

class TestStruct
{
public:
    bool attr: 1;
    TestEnum type: 3;
    unsigned int number: 28;
    QString string;
};

Q_DECLARE_TYPEINFO(TestStruct, Q_MOVABLE_TYPE);

static QDataStream& operator<<(QDataStream& s, const TestStruct& item)
{
    quint8 flag = item.attr ? 1 : 0;

    s << flag;
    s << (quint8) item.type;
    s << item.number;
    s << item.string;

    return s;
}

static QDataStream& operator>>(QDataStream& s, TestStruct& item)
{
    quint8 flag;
    quint8 type;
    int number;
    QString string;

    s >> flag;
    s >> type;
    s >> number;
    s >> string;

    item.attr = (flag != 0);
    item.type = (TestEnum) type;
    item.number = number;
    item.string = string;

    return s;
}

void TestKoXmlVector::simpleConstructor()
{
    KoXmlVector<TestStruct> vector;

    QCOMPARE(vector.count(), 0);
    QCOMPARE(vector.size(), 0);
    QCOMPARE(vector.isEmpty(), true);
}


static const int writeAndReadUncompressedCount = 5;

void TestKoXmlVector::writeAndRead_data()
{
    QTest::addColumn<unsigned int>("itemCount");
    for(unsigned int i = 0; i < writeAndReadUncompressedCount*3+1; ++i) {
        QTest::newRow(QByteArray::number(i)) << i;
    }
}


void TestKoXmlVector::writeAndRead()
{
    QFETCH(unsigned int, itemCount);

    KoXmlVector<TestStruct, writeAndReadUncompressedCount+1> vector;

    // add 3x items than what would not be compressed
    for (unsigned int i = 0; i < itemCount; ++i) {
        // test adding
        TestStruct &item = vector.newItem();

        const bool attr = (i % 2) == 0;
        const TestEnum type = (TestEnum)(i % 5);
        const unsigned int number = i;
        const QString string = QString::number(i);

        item.attr = attr;
        item.type = type;
        item.number = number;
        item.string = string;

        QCOMPARE(vector.count(), (signed)i+1);
        QCOMPARE(vector.size(), (signed)i+1);
        QCOMPARE(vector.isEmpty(), false);
    }

    vector.squeeze();

    // now check all in a row again, so including all the uncompressed
    for (unsigned int i = 0; i < itemCount; ++i) {
        const bool attr = (i % 2) == 0;
        const TestEnum type = (TestEnum)(i % 5);
        const unsigned int number = i;
        const QString string = QString::number(i);

        const TestStruct &readItem = vector[i];

        QCOMPARE(readItem.attr, attr);
        QCOMPARE(readItem.type, type);
        QCOMPARE(readItem.number, number);
        QCOMPARE(readItem.string, string);
    }
    // and backwards
    for (unsigned int ri = itemCount; ri > 0; --ri) {
        const unsigned int i = ri-1;
        const bool attr = (i % 2) == 0;
        const TestEnum type = (TestEnum)(i % 5);
        const unsigned int number = i;
        const QString string = QString::number(i);

        const TestStruct &readItem = vector[i];

        QCOMPARE(readItem.attr, attr);
        QCOMPARE(readItem.type, type);
        QCOMPARE(readItem.number, number);
        QCOMPARE(readItem.string, string);
    }
}



QTEST_GUILESS_MAIN(TestKoXmlVector)
