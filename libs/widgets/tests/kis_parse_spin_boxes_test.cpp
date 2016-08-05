/*
 *  Copyright (c) 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
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

#include "kis_parse_spin_boxes_test.h"

#include "kis_num_parser.h"
#include "kis_double_parse_spin_box.h"
#include "kis_int_parse_spin_box.h"

#include <QString>
#include <QtTest>

const QStringList KisParseSpinBoxesTest::doubleExprs = {"1",
                                                        "-12",
                                                        "7.9 - 12",
                                                        "cos(90)*2",
                                                        "cos(acos(-1)+1*3^2.0)*2 + sin(3)/2"};

const QStringList KisParseSpinBoxesTest::doubleWrongExprs = {"abc",
                                                             "1/",
                                                             "7.9 + 12*",
                                                             "cos(90)*2 + ",
                                                             "23.0/0",
                                                             "0.0/0.0"};

const QStringList KisParseSpinBoxesTest::intExprs = {"12",
                                                     "-12",
                                                     "-12.3",
                                                     "12.7 - 25",
                                                     "12.7",
                                                     "12*1.5",
                                                     "12/2.5",
                                                     "518*2/3"};

const QStringList KisParseSpinBoxesTest::intWrongExprs = {"abc",
                                                          "12.5/2 +",
                                                          "12*",
                                                          "12/0"};

KisParseSpinBoxesTest::KisParseSpinBoxesTest() : QObject()
{

}

void KisParseSpinBoxesTest::testDoubleParseNormal()
{

    KisDoubleParseSpinBox spinBox;
    spinBox.setDecimals(3);
    spinBox.setMaximum(9999.0);
    spinBox.setMinimum(-9999.0);

    for (int i = 0; i < doubleExprs.size(); i++) {

        spinBox.clearFocus();
        spinBox.clear(); //clear all
        QTest::keyClicks(&spinBox, doubleExprs[i]);
        spinBox.clearFocus();

        double resultParser = KisNumericParser::parseSimpleMathExpr(doubleExprs[i]);
        double valueSpinBox = spinBox.value();

        bool test = resultParser == valueSpinBox || qAbs(resultParser - valueSpinBox) < 1e-2;

        QVERIFY2(test, QString("Failed with expression %1, result is %2, value is %3")
                 .arg(doubleExprs[i]).arg(resultParser).arg(valueSpinBox).toStdString().c_str());

        spinBox.setValue(0);

    }


}

void KisParseSpinBoxesTest::testDoubleParseProblem()
{

    //error can happen with incomplete or incorrect expressions, inf or nan values.

    KisDoubleParseSpinBox spinBox;
    spinBox.setMaximum(9999.0);
    spinBox.setMinimum(-9999.0);

    for (int i = 0; i < doubleWrongExprs.size(); i++) {

        spinBox.clearFocus();
        spinBox.clear(); //clear all
        QTest::keyClicks(&spinBox, doubleWrongExprs[i]);
        spinBox.clearFocus();

        QVERIFY2(!spinBox.isLastValid(), QString("SpinBox is in a valid state with expression %1, but shouldn't.")
                 .arg(doubleWrongExprs[i]).toStdString().c_str());

        spinBox.setValue(0.0);

        QVERIFY2(spinBox.isLastValid(), QString("SpinBox unsable to recover error free state after a value reset.")
                 .toStdString().c_str());

        spinBox.setValue(0);

    }

}
void KisParseSpinBoxesTest::testDoubleParseWithSuffix(){

    QString suff = "px";

    KisDoubleParseSpinBox spinBox;
    spinBox.setDecimals(3);
    spinBox.setMaximum(9999.0);
    spinBox.setMinimum(-9999.0);
    spinBox.setSuffix(suff);

    for (int i = 0; i < doubleExprs.size(); i++) {

        spinBox.clearFocus();
        spinBox.clear(); //clear all
        QTest::keyClicks(&spinBox, doubleExprs[i]);
        spinBox.clearFocus();

        double resultParser = KisNumericParser::parseSimpleMathExpr(doubleExprs[i]);
        double valueSpinBox = spinBox.value();

        bool test = resultParser == valueSpinBox || qAbs(resultParser - valueSpinBox) < 1e-2;

        QVERIFY2(test, QString("Failed with expression %1, result is %2, value is %3")
                 .arg(doubleExprs[i]).arg(resultParser).arg(valueSpinBox).toStdString().c_str());

        spinBox.setValue(0);

    }

    //verify that the suffix don't appear in the clean text in case of error.
    for (int i = 0; i < doubleWrongExprs.size(); i++) {

        if (doubleWrongExprs[i].endsWith(suff)) {
            continue;
        }

        spinBox.clearFocus();
        spinBox.clear(); //clear all
        QTest::keyClicks(&spinBox, doubleWrongExprs[i]);
        spinBox.clearFocus();

        QVERIFY2(!spinBox.cleanText().endsWith(suff), "SpinBox failed to remove suffix from clean text in error state.");

        spinBox.setValue(0.0);

    }
}
void KisParseSpinBoxesTest::testDoubleParseWithPrefix()
{

    QString preff = "px";

    KisDoubleParseSpinBox spinBox;
    spinBox.setDecimals(3);
    spinBox.setMaximum(9999.0);
    spinBox.setMinimum(-9999.0);
    spinBox.setPrefix(preff);

    for (int i = 0; i < doubleExprs.size(); i++) {

        spinBox.clearFocus();
        spinBox.clear(); //clear all
        QTest::keyClicks(&spinBox, doubleExprs[i]);
        spinBox.clearFocus();

        double resultParser = KisNumericParser::parseSimpleMathExpr(doubleExprs[i]);
        double valueSpinBox = spinBox.value();

        bool test = resultParser == valueSpinBox || qAbs(resultParser - valueSpinBox) < 1e-2;

        QVERIFY2(test, QString("Failed with expression %1, result is %2, value is %3")
                 .arg(doubleExprs[i]).arg(resultParser).arg(valueSpinBox).toStdString().c_str());

        spinBox.setValue(0);

    }

    //verify that the prefix don't appear in the clean text in case of error.
    for (int i = 0; i < doubleWrongExprs.size(); i++) {

        if (doubleWrongExprs[i].endsWith(preff)) {
            continue;
        }

        spinBox.clearFocus();
        spinBox.clear(); //clear all
        QTest::keyClicks(&spinBox, doubleWrongExprs[i]);
        spinBox.clearFocus();

        QVERIFY2(!spinBox.cleanText().startsWith(preff), "SpinBox failed to remove prefix from clean text in error state.");

        spinBox.setValue(0.0);

    }
}

void KisParseSpinBoxesTest::testIntParseNormal()
{

    KisIntParseSpinBox spinBox;
    spinBox.setMaximum(999);
    spinBox.setMinimum(-999);

    for (int i = 0; i < intExprs.size(); i++) {

        spinBox.clearFocus();
        spinBox.clear(); //clear all
        QTest::keyClicks(&spinBox, intExprs[i]);
        spinBox.clearFocus();

        int resultParser = KisNumericParser::parseIntegerMathExpr(intExprs[i]);
        int valueSpinBox = spinBox.value();

        bool test = resultParser == valueSpinBox;

        QVERIFY2(test, QString("Failed with expression %1, result is %2, value is %3")
                 .arg(intExprs[i]).arg(resultParser).arg(valueSpinBox).toStdString().c_str());

        spinBox.setValue(0);
    }

}

void KisParseSpinBoxesTest::testIntParseProblem()
{

    //errors can happen with incorrect or incomplete expressions, or division by 0.

    KisIntParseSpinBox spinBox;
    spinBox.setMaximum(999);
    spinBox.setMinimum(-999);

    for (int i = 0; i < intWrongExprs.size(); i++) {

        spinBox.clearFocus();
        spinBox.clear(); //clear all
        QTest::keyClicks(&spinBox, intWrongExprs[i]);
        spinBox.clearFocus();

        QVERIFY2(!spinBox.isLastValid(), QString("SpinBox is in a valid state with expression %1, but shouldn't.")
                 .arg(intWrongExprs[i]).toStdString().c_str());

        spinBox.setValue(0);

        QVERIFY2(spinBox.isLastValid(), QString("SpinBox unsable to recover error free state after a value reset.")
                 .toStdString().c_str());

        spinBox.setValue(0);

    }

}

void KisParseSpinBoxesTest::testIntParseWithSuffix()
{
    QString suff = "px";

    KisIntParseSpinBox spinBox;
    spinBox.setMaximum(999);
    spinBox.setMinimum(-999);
    spinBox.setSuffix(suff);

    for (int i = 0; i < intExprs.size(); i++) {

        spinBox.clearFocus();
        spinBox.clear(); //clear all
        QTest::keyClicks(&spinBox, intExprs[i]);
        spinBox.clearFocus();

        int resultParser = KisNumericParser::parseIntegerMathExpr(intExprs[i]);
        int valueSpinBox = spinBox.value();

        bool test = resultParser == valueSpinBox;

        QVERIFY2(test, QString("Failed with expression %1, result is %2, value is %3")
                 .arg(intExprs[i]).arg(resultParser).arg(valueSpinBox).toStdString().c_str());

        spinBox.setValue(0);
    }

    //verify that the suffix don't appear in the clean text in case of error.
    for (int i = 0; i < intWrongExprs.size(); i++) {

        if (intWrongExprs[i].endsWith(suff)) {
            continue;
        }

        spinBox.clearFocus();
        spinBox.clear(); //clear all
        QTest::keyClicks(&spinBox, intWrongExprs[i]);
        spinBox.clearFocus();

        QVERIFY2(!spinBox.cleanText().endsWith(suff), "SpinBox failed to remove suffix from clean text in error state.");

        spinBox.setValue(0.0);

    }

}
void KisParseSpinBoxesTest::testIntParseWithPrefix()
{
    QString preff = "px";

    KisIntParseSpinBox spinBox;
    spinBox.setMaximum(999);
    spinBox.setMinimum(-999);
    spinBox.setPrefix(preff);

    for (int i = 0; i < intExprs.size(); i++) {

        spinBox.clearFocus();
        spinBox.clear(); //clear all
        QTest::keyClicks(&spinBox, intExprs[i]);
        spinBox.clearFocus();

        int resultParser = KisNumericParser::parseIntegerMathExpr(intExprs[i]);
        int valueSpinBox = spinBox.value();

        bool test = resultParser == valueSpinBox;

        QVERIFY2(test, QString("Failed with expression %1, result is %2, value is %3")
                 .arg(intExprs[i]).arg(resultParser).arg(valueSpinBox).toStdString().c_str());

        spinBox.setValue(0);
    }

    //verify that the prefix don't appear in the clean text in case of error.
    for (int i = 0; i < intWrongExprs.size(); i++) {

        if (intWrongExprs[i].startsWith(preff)) {
            continue;
        }

        spinBox.clearFocus();
        spinBox.clear(); //clear all
        QTest::keyClicks(&spinBox, intWrongExprs[i]);
        spinBox.clearFocus();

        QVERIFY2(!spinBox.cleanText().startsWith(preff), "SpinBox failed to remove prefix from clean text in error state.");

        spinBox.setValue(0.0);

    }

}

QTEST_MAIN(KisParseSpinBoxesTest)
