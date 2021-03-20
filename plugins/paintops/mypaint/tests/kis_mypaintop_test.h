/*
 *  SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MYPAINTOP_TEST_H
#define KIS_MYPAINTOP_TEST_H

#include <QObject>
#include <stroke_testing_utils.h>
#include <qimage_based_test.h>
#include <simpletest.h>
#include <QtTest/QtTest>

class KisMyPaintOpTest: public QObject, public TestUtil::QImageBasedTest
{
    Q_OBJECT
public:
    KisMyPaintOpTest();
    virtual ~KisMyPaintOpTest() {}

private Q_SLOTS:
    void testDab();
    void testGetColor();
    void testLoading();
};

#endif // KIS_MYPAINTOP_TEST_H
