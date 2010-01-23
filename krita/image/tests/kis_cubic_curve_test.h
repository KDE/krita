/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_CUBIC_CURVE_TEST_H_
#define _KIS_CUBIC_CURVE_TEST_H_

#include <QtTest/QtTest>

#include <QPointF>

class KisCubicCurveTest : public QObject
{
    Q_OBJECT
public:
    KisCubicCurveTest();
private slots:

    void testCreation();
    void testCopy();
    void testEdition();
    void testComparison();
    void testSerialization();
    void testValue();
    void testTransfer();
private:
    QPointF pt0, pt1, pt2, pt3, pt4, pt5;
};


#endif
