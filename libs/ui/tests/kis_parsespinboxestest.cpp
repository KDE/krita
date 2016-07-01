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

#include "kis_parsespinboxestest.h"

#include "kis_numparser.h"
#include "kis_doubleparsespinbox.h"
#include "kis_intparsespinbox.h"

#include <QString>
#include <QtTest>

const QStringList KisParseSpinBoxesTest::doubleExprs = {"1",
														"-12",
														"7.9 - 12",
														"cos(90)*2",
														"cos(acos(-1)+1*3^2.0)*2 + sin(3)/2"};
const QStringList KisParseSpinBoxesTest::intExprs = {"12",
													 "-12",
													 "-12.3",
													 "12.7 - 25",
													 "12.7",
													 "12*1.5",
													 "12/2.5",
													 "518*2/3"};

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

	QStringList exprs = {"abc",
						 "1/",
						 "7.9 + 12*",
						 "cos(90)*2 + ",
						 "23.0/0",
						 "0.0/0.0"};

	//error can happen with incomplete or incorrect expressions, inf or nan values.

	KisDoubleParseSpinBox spinBox;
	spinBox.setMaximum(9999.0);
	spinBox.setMinimum(-9999.0);

	for (int i = 0; i < exprs.size(); i++) {

		spinBox.clearFocus();
		spinBox.clear(); //clear all
		QTest::keyClicks(&spinBox, exprs[i]);
		spinBox.clearFocus();

		QVERIFY2(!spinBox.isLastValid(), QString("SpinBox is in a valid state with expression %1, but shouldn't.")
				 .arg(exprs[i]).toStdString().c_str());

		spinBox.setValue(0.0);

		QVERIFY2(spinBox.isLastValid(), QString("SpinBox unsable to recover error free state after a value reset.")
				 .toStdString().c_str());

		spinBox.setValue(0);

	}

}
void KisParseSpinBoxesTest::testDoubleParseWithSuffix(){

	KisDoubleParseSpinBox spinBox;
	spinBox.setDecimals(3);
	spinBox.setMaximum(9999.0);
	spinBox.setMinimum(-9999.0);
	spinBox.setSuffix("px");

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
void KisParseSpinBoxesTest::testDoubleParseWithPrefix()
{

	KisDoubleParseSpinBox spinBox;
	spinBox.setDecimals(3);
	spinBox.setMaximum(9999.0);
	spinBox.setMinimum(-9999.0);
	spinBox.setPrefix("px");

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

	QStringList exprs = {"abc",
						 "12.5/2 +",
						 "12*",
						 "12/0"};
	//errors can happen with incorrect or incomplete expressions, or division by 0.

	KisIntParseSpinBox spinBox;
	spinBox.setMaximum(999);
	spinBox.setMinimum(-999);

	for (int i = 0; i < exprs.size(); i++) {

		spinBox.clearFocus();
		spinBox.clear(); //clear all
		QTest::keyClicks(&spinBox, exprs[i]);
		spinBox.clearFocus();

		QVERIFY2(!spinBox.isLastValid(), QString("SpinBox is in a valid state with expression %1, but shouldn't.")
				 .arg(exprs[i]).toStdString().c_str());

		spinBox.setValue(0);

		QVERIFY2(spinBox.isLastValid(), QString("SpinBox unsable to recover error free state after a value reset.")
				 .toStdString().c_str());

		spinBox.setValue(0);

	}

}

void KisParseSpinBoxesTest::testIntParseWithSuffix()
{

	KisIntParseSpinBox spinBox;
	spinBox.setMaximum(999);
	spinBox.setMinimum(-999);
	spinBox.setSuffix("px");

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
void KisParseSpinBoxesTest::testIntParseWithPrefix()
{

	KisIntParseSpinBox spinBox;
	spinBox.setMaximum(999);
	spinBox.setMinimum(-999);
	spinBox.setPrefix("px");

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

QTEST_MAIN(KisParseSpinBoxesTest)
