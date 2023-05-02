/*
 *  SPDX-FileCopyrightText: 2023 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef __TEST_ELLIPSE_IN_POLYGON_H
#define __TEST_ELLIPSE_IN_POLYGON_H

#include <simpletest.h>
#include "empty_nodes_test.h"

#include "kis_painting_assistant.h"


class TestEllipseInPolygon : public QObject
{
    Q_OBJECT
private Q_SLOTS:

// test all ConicCalculations
    void testConicCalculationsPhaseAB();
    void testConicCalculationsPhaseBC();
    void testConicCalculationsPhaseCD();
    void testConicCalculationsPhaseDE();
    void testConicCalculationsPhaseEF();
    void testConicCalculationsPhaseFG();
    void testConicCalculationsPhaseGH();
    void testConicCalculationsPhaseHI();




};

#endif /* __TEST_ELLIPSE_IN_POLYGON_H */
