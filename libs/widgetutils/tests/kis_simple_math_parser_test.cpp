/*
 *  SPDX-FileCopyrightText: 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_simple_math_parser_test.h"

#include "kis_num_parser.h"
#include <qnumeric.h> // for qIsNaN
#include <QString>
#include <simpletest.h>

KisSimpleMathParserTest::KisSimpleMathParserTest() : QObject()
{

}

void KisSimpleMathParserTest::testDoubleComputation()
{

    QStringList exprs = {"1",
                      "2 + 3.4",
                      "2 + -3.4",
                      "2 - -3.4",
                      "5-2",
                      "7 + 2 - 5",
                      "4.6 * 2 + 13",
                      "4.6 / 2 + 3*3",
                      "4.6 / 0.0 + 3*3",
                      "-4.6 / 0.0 + 3*3",
                      "-4.6 / -0.0 + 3*3",
                      "4.6 / -0.0 + 3*3",
                      "0.0 / 0.0 + 3*3",
                      "2^3 - 4 * 1.5",
                      "2^3.0 - 4 * 1.5",
                      "cos(1)*2",
                      "-cos(1)*2",
                      "cos(1)^3*2",
                      "cos(1)^3.0*2",
                      "cos(1)*2 + sin(3)/2",
                      "cos(acos(-1)+1*3^2.0)*2 + sin(3)/2",
                      "cos(acos(-1)+1*3^2.0)^2 + sin(3)/2",
                      "log10(100)",
                      "exp(10)",
                      "ln(10)"};

    QVector<double> expected = {1,
                             2 + 3.4,
                             2 + -3.4,
                             2 - -3.4,
                             5-2,
                             7 + 2 - 5,
                             4.6 * 2 + 13,
                             4.6 / 2 + 3*3,
                             4.6 / 0.0 + 3*3,
                             -4.6 / 0.0 + 3*3,
                             -4.6 / -0.0 + 3*3,
                             4.6 / -0.0 + 3*3,
                             0.0 / 0.0 + 3*3,
                             qPow(2,3) - 4 * 1.5,
                             qPow(2,3.0) - 4 * 1.5,
                             qCos(1.0/180*qAcos(-1))*2,
                             -qCos(1.0/180*qAcos(-1))*2,
                             qPow(qCos(1.0/180*qAcos(-1)),3)*2,
                             qPow(qCos(1.0/180*qAcos(-1)),3.0)*2,
                             qCos(1.0/180*qAcos(-1))*2 + qSin(3.0/180*qAcos(-1))/2,
                             qCos((qAcos(-1.0)*180/qAcos(-1)+1*qPow(3,2.0))/180*qAcos(-1))*2 + qSin(3.0/180*qAcos(-1))/2,
                             qPow(qCos((qAcos(-1.0)*180/qAcos(-1)+1*qPow(3,2.0))/180*qAcos(-1)),2) + qSin(3.0/180*qAcos(-1))/2,
                             qLn(100)/qLn(10),
                             qExp(10),
                             qLn(10)};

    for (int i = 0; i < expected.size(); i++) {

        double result = KisNumericParser::parseSimpleMathExpr(exprs[i]);

        bool test = result == expected[i] || qAbs(result - expected[i]) < 1e-12 || (qIsNaN(result) && qIsNaN(expected[i]));

        QVERIFY2(test, QString("Failed when %1 should equal %2 but evaluated to %3.").arg(exprs[i]).arg(expected[i]).arg(result).toStdString().c_str());
    }
}

void KisSimpleMathParserTest::testIntComputation()
{

    QStringList exprs = {"1",
                      "2 + 3",
                      "2 + -3",
                      "2 - -3",
                      "5-2",
                      "7 + 2 - 5",
                      "4/3",
                      "12/3",
                      "4*3",
                      "581*2/3"};

    QVector<int> expected = {1,
                             2 + 3,
                             2 + -3,
                             2 - -3,
                             5-2,
                             7 + 2 - 5,
                             qRound(4.0/3.0),
                             qRound(12.0/3.0),
                             4*3,
                             581*2/3};

    for (int i = 0; i < expected.size(); i++) {

        int result = KisNumericParser::parseIntegerMathExpr(exprs[i]);

        QCOMPARE(result, expected[i]);
    }
}

void KisSimpleMathParserTest::testIntFlooring()
{

    QStringList exprs = {"4.5",
                      "-4.5",
                      "3.5 + 4.5",
                      "2.8 - -3.5",
                      "4.5/2.9",
                      "7.6*3.2",
                      "7.6*3.2 + 4.5"
                        };

    QVector<int> expected = {qRound(4.5),
                             qRound(-4.5),
                             qRound(3.5 + 4.5),
                             qRound(2.8 - -3.5),
                             qRound(4.5/2.9),
                             qRound(7.6*3.2),
                             qRound(7.6*3.2 + 4.5)
                            };

    for (int i = 0; i < expected.size(); i++) {

        int result = KisNumericParser::parseIntegerMathExpr(exprs[i]);

        QCOMPARE(result, expected[i]);
    }

}

QTEST_APPLESS_MAIN(KisSimpleMathParserTest)

