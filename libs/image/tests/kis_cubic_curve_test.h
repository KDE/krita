/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_CUBIC_CURVE_TEST_H_
#define _KIS_CUBIC_CURVE_TEST_H_

#include <QtTest>

#include <QPointF>

class KisCubicCurveTest : public QObject
{
    Q_OBJECT
public:
    KisCubicCurveTest();
private Q_SLOTS:

    void testCreation();
    void testCopy();
    void testEdition();
    void testComparison();
    void testSerialization();
    void testValue();
    void testNull();
    void testTransfer();
private:
    QPointF pt0, pt1, pt2, pt3, pt4, pt5;
};


#endif
