/*
 *  SPDX-FileCopyrightText: 2023 Alvin Wong <alvin@alvinhc.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <simpletest.h>
#include <testflake.h>

#include "MockShapes.h"

#include <SvgSavingContext.h>

#include <QBuffer>

class TestSvgSavingContext : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCreateUID1();
    void testCreateUID2();
    void testDuplicatedId1();
};

void TestSvgSavingContext::testCreateUID1()
{
    QBuffer buffer;
    buffer.open(QIODevice::Append);
    SvgSavingContext context(buffer);

    MockShape shape1;
    shape1.setName("shape0");

    MockShape shape2;
    shape2.setName("shape2");

    QCOMPARE(context.getID(&shape1), "shape0");
    QCOMPARE(context.createUID("shape"), "shape1");
    QCOMPARE(context.getID(&shape2), "shape2");
    QCOMPARE(context.createUID("shape"), "shape3");
}

void TestSvgSavingContext::testCreateUID2()
{
    QBuffer buffer;
    buffer.open(QIODevice::Append);
    SvgSavingContext context(buffer);

    MockShape shape1;
    shape1.setName("shape0");

    MockShape shape2;
    shape2.setName("shape01");

    QCOMPARE(context.getID(&shape1), "shape0");
    QCOMPARE(context.getID(&shape2), "shape01");
    QCOMPARE(context.createUID("shape"), "shape1");
    QCOMPARE(context.createUID("shape0"), "shape02");
    QCOMPARE(context.createUID("shape0"), "shape03");
}

void TestSvgSavingContext::testDuplicatedId1()
{
    QBuffer buffer;
    buffer.open(QIODevice::Append);
    SvgSavingContext context(buffer);

    MockShape shape1;
    shape1.setName("shape0");

    MockShape shape2;
    shape2.setName("shape0");

    MockShape shape3;
    shape3.setName("shape0");

    MockShape shape4;
    shape4.setName("shape01");

    QCOMPARE(context.getID(&shape1), "shape0");
    QCOMPARE(context.getID(&shape2), "shape01");
    QCOMPARE(context.getID(&shape3), "shape02");
    QCOMPARE(context.getID(&shape4), "shape011");
}

KISTEST_MAIN(TestSvgSavingContext)

#include "TestSvgSavingContext.moc"
