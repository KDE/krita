/*
 *  SPDX-FileCopyrightText: 2023 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "TestEllipseInPolygon.h"


#include <testui.h>
#include <testutil.h>


#include <kis_painting_assistant.h>
#include <PerspectiveBasedAssistantHelper.h>
#include <kis_algebra_2d.h>
#include <kis_global.h>

#include <ConcentricEllipseAssistant.h>
#include "EllipseInPolygon.h"


void TestEllipseInPolygon::testConicCalculationsPhaseAB()
{
    QRandomGenerator random(1234);
    int n = 100;
    auto rand = [random] () mutable {return random.generateDouble();};
    double eps = 1e-10;
    for(int i = 0; i < n; i++) {
        ConicFormula c;
        c.setFormulaActual(rand(), rand(), rand(), rand(), rand(), rand());
        QPointF point(rand(), rand());

        auto result = ConicCalculations::normalizeFormula(c, point);
        ConicFormula res = std::get<0>(result);
        QPointF resPoint = std::get<1>(result);
        double normalizeBy = std::get<2>(result);

        QCOMPARE(c.calculateFormulaForPoint(point), res.calculateFormulaForPoint(resPoint));

        if (qAbs(normalizeBy) > eps) {
            QCOMPARE(point, resPoint*normalizeBy);
            for(int i = 0; i < 6; i++) {
                if (i >= 0 && i <= 2) {
                    QCOMPARE(c.getFormulaActual()[i], res.getFormulaActual()[i]/normalizeBy/normalizeBy);
                } else if (i == 3 || i == 4) {
                    QCOMPARE(c.getFormulaActual()[i], res.getFormulaActual()[i]/normalizeBy);
                } else {
                    QCOMPARE(c.getFormulaActual()[i], res.getFormulaActual()[i]);
                }
            }
        }
    }

}

void TestEllipseInPolygon::testConicCalculationsPhaseBC()
{
    QRandomGenerator random(1234);
    int n = 100;
    auto rand = [random] () mutable {return random.generateDouble();};
    auto sq = [] (qreal a) {return a*a;};

    for(int i = 0; i < n; i++) {
        ConicFormula c;
        c.setFormulaActual(rand(), rand(), rand(), rand(), rand(), rand());
        QPointF point(rand(), rand());

        ConicFormula result = ConicCalculations::canonizeFormula(c);

        QCOMPARE(c.calculateFormulaForPoint(point), result.calculateFormulaForPoint(point));
        qreal checkIfReallyCanonized = sq(result.A) + sq(result.B) + sq(result.C)
                + sq(result.D) + sq(result.E) + sq(result.F);

        QCOMPARE(checkIfReallyCanonized, 1.0);
        QVERIFY(result.isSpecial());
        QVERIFY(!c.isSpecial());

    }

}

void TestEllipseInPolygon::testConicCalculationsPhaseCD()
{
    QRandomGenerator random(1234);
    int n = 100;
    auto rand = [random] () mutable {return random.generateDouble();};

    for(int i = 0; i < n; i++) {
        ConicFormula c;
        c.setFormulaActual(rand(), rand(), rand(), rand(), rand(), rand());
        QPointF point(rand(), rand());

        auto result = ConicCalculations::rotateFormula(c, point);
        ConicFormula resFormula = std::get<0>(result);
        double K = std::get<1>(result);
        double L = std::get<2>(result);
        QPointF resPoint = std::get<3>(result);

        // check if the point still solves the formula the same way
        QCOMPARE(c.calculateFormulaForPoint(point), resFormula.calculateFormulaForPoint(resPoint));

        // check if derotated point is the same as rotated
        QPointF resPoint2 = EllipseInPolygon::getRotatedPoint(resPoint, K, L, true);
        QCOMPARE(point, resPoint2);

        // check if derotated formula is the same as rotated
        QVector<double> resFormula2 = EllipseInPolygon::getRotatedFormula(resFormula.getFormulaActual(), K, L, true);
        for(int i = 0; i < 6; i++) {
            QCOMPARE(resFormula2[i], c.getFormulaActual()[i]);
        }



        // TODO: add checking for actual rotation as well





    }
}

void TestEllipseInPolygon::testConicCalculationsPhaseDE()
{

}

void TestEllipseInPolygon::testConicCalculationsPhaseEF()
{

}

void TestEllipseInPolygon::testConicCalculationsPhaseFG()
{

}

void TestEllipseInPolygon::testConicCalculationsPhaseGH()
{

}

void TestEllipseInPolygon::testConicCalculationsPhaseHI()
{

}

KISTEST_MAIN(TestEllipseInPolygon)
