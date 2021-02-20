/*
 *  SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

