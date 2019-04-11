/*
 *  Copyright (c) 2015 Agata Cacko <cacko.azh@gmail.com>
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

#ifndef KIS_ANIMATION_EXPORTER_TEST_H
#define KIS_ANIMATION_EXPORTER_TEST_H

#include <QtTest>
#include <KisSpinBoxSplineUnitConverter.h>

/**
 * KisSpinBoxSplineUnitConverterTest contains tests
 *  for class KisSpinBoxSplineUnitConverter
 */
class KisSpinBoxSplineUnitConverterTest : public QObject
{
    Q_OBJECT

    KisSpinBoxSplineUnitConverter converter;

private Q_SLOTS:
    void testCurveCalculationToSpinBox();
    void testCurveCalculationToSpinBox_data();

    void testCurveCalculationToCurve();
    void testCurveCalculationToCurve_data();

    void testCurveCalculationTwoWay();
    void testCurveCalculationTwoWay_data();

    void testCurveCalculationCase64();

};
#endif

