/* This file is part of the KDE project
 * Copyright (C) 2011 Sebastian Sauer <sebsauer@kdab.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "TestNumberStyle.h"

#include <KoOdfNumberStyles.h>
#include <QTest>

QString escapeLocals(const QString &text)
{
    QString t(text);
    t.replace(',','.');
    return t;
}

void TestNumberStyle::testEmpty()
{
    KoOdfNumberStyles::NumericStyleFormat f;
    QCOMPARE(f.formatStr, QString());
    QCOMPARE(f.prefix, QString());
    QCOMPARE(f.suffix, QString());
    QCOMPARE(f.type, KoOdfNumberStyles::Text);
    QCOMPARE(f.precision, -1);
    QCOMPARE(f.currencySymbol, QString());
    QCOMPARE(f.thousandsSep, false);
    QCOMPARE(f.styleMaps.count(), 0);
}

void TestNumberStyle::testText()
{
    KoOdfNumberStyles::NumericStyleFormat f;
    f.type = KoOdfNumberStyles::Text;
    QCOMPARE(KoOdfNumberStyles::format("Some text", f), QString("Some text"));
}

void TestNumberStyle::testNumber()
{
    QCOMPARE(KoOdfNumberStyles::formatNumber(23, "0."), QString("23"));
    QCOMPARE(KoOdfNumberStyles::formatNumber(0, "0."), QString("0"));

    KoOdfNumberStyles::NumericStyleFormat f;
    f.type = KoOdfNumberStyles::Number;
    f.precision = 3;
    f.thousandsSep = true;
    f.formatStr = "00.00 test";
    QCOMPARE(KoOdfNumberStyles::format("12345.6789", f), QString("12345.679 test"));
    f.precision = 1;
    f.formatStr = "test1 00.00 test2";
    QCOMPARE(KoOdfNumberStyles::format("12345.6789", f), QString("test1 12345.70 test2"));
}

void TestNumberStyle::testDate()
{
    QCOMPARE(KoOdfNumberStyles::formatDate(4567, "MM/dd/yyyy"), QString("07/02/1912"));
    QCOMPARE(KoOdfNumberStyles::formatDate(0, "MM/dd/yy"), QString("12/30/99"));
}

void TestNumberStyle::testTime()
{
    QCOMPARE(KoOdfNumberStyles::formatTime(0.524259259259, "hh:mm:ss"), QString("12:34:56"));
    QCOMPARE(KoOdfNumberStyles::formatTime(0.524259259259, "hh:mm"), QString("12:34"));
    QCOMPARE(KoOdfNumberStyles::formatTime(0, "hh:mm:ss"), QString("00:00:00"));

    KoOdfNumberStyles::NumericStyleFormat f;
    f.type = KoOdfNumberStyles::Time;
    f.formatStr = "hh:mm:ss";
    QCOMPARE(KoOdfNumberStyles::format("0.524259259259", f), QString("12:34:56"));
    QCOMPARE(KoOdfNumberStyles::format("test", f), QString("test"));
    QCOMPARE(KoOdfNumberStyles::format("123", f), QString("00:00:00"));
    QCOMPARE(KoOdfNumberStyles::format("1.23", f), QString("05:31:12"));
}

void TestNumberStyle::testBoolean()
{
    QCOMPARE(KoOdfNumberStyles::formatBoolean("0", ""), QString("FALSE"));
    QCOMPARE(KoOdfNumberStyles::formatBoolean("234", ""), QString("TRUE"));
    QCOMPARE(KoOdfNumberStyles::formatBoolean("0", ""), QString("FALSE"));

    KoOdfNumberStyles::NumericStyleFormat f;
    f.type = KoOdfNumberStyles::Boolean;
    QCOMPARE(KoOdfNumberStyles::format("0", f), QString("FALSE"));
    QCOMPARE(KoOdfNumberStyles::format("1", f), QString("TRUE"));
    QCOMPARE(KoOdfNumberStyles::format("123", f), QString("TRUE"));
    QCOMPARE(KoOdfNumberStyles::format("test", f), QString("FALSE"));
}

void TestNumberStyle::testPercent()
{
    QCOMPARE(KoOdfNumberStyles::formatPercent("23", ""), QString("23"));
    QCOMPARE(KoOdfNumberStyles::formatPercent("23.4567", "0.00%", 2), QString("2345.67%"));
    QCOMPARE(KoOdfNumberStyles::formatPercent("23.456789", "0.0000%", 4), QString("2345.6789%"));
    QCOMPARE(KoOdfNumberStyles::formatPercent("0", ""), QString("0"));

    KoOdfNumberStyles::NumericStyleFormat f;
    f.type = KoOdfNumberStyles::Percentage;
    f.precision = 2;
    f.formatStr = "0%";
    QCOMPARE(KoOdfNumberStyles::format("0.2", f), QString("20.00%"));
    QCOMPARE(KoOdfNumberStyles::format("0.02", f), QString("2.00%"));
    QCOMPARE(KoOdfNumberStyles::format("0.02228", f), QString("2.23%"));
    QCOMPARE(KoOdfNumberStyles::format("test", f), QString("test"));
    QCOMPARE(KoOdfNumberStyles::format("123", f), QString("123"));
    QCOMPARE(KoOdfNumberStyles::format("1.23", f), QString("123.00%"));
}

void TestNumberStyle::testScientific()
{
    //QEXPECT_FAIL("", "min-exponent-digits not handled", Continue);
    QCOMPARE(escapeLocals(KoOdfNumberStyles::formatScientific(345678, "0.00E+000")), QString("3.456780E+05"));

    KoOdfNumberStyles::NumericStyleFormat f;
    f.type = KoOdfNumberStyles::Scientific;
    f.precision = 3;
    //QEXPECT_FAIL("", "min-exponent-digits not handled", Continue);
    QCOMPARE(escapeLocals(KoOdfNumberStyles::format("0.2", f)), QString("2.000E-01"));
    //QEXPECT_FAIL("", "min-exponent-digits not handled", Continue);
    QCOMPARE(escapeLocals(KoOdfNumberStyles::format("1.23", f)), QString("1.230E+00"));
    QCOMPARE(escapeLocals(KoOdfNumberStyles::format("test", f)), QString("test"));
}

void TestNumberStyle::testFraction()
{
    QCOMPARE(KoOdfNumberStyles::formatFraction(34.5678, " ?/?"), QString("34 4/7"));
}

void TestNumberStyle::testCurrency()
{
    QCOMPARE(KoOdfNumberStyles::formatCurrency(34.56, "-$#,###0.00", QString(), 2), QString("$34.56"));
    QCOMPARE(KoOdfNumberStyles::formatCurrency(34.56, "-#,###0.00 EUR", QString(), 2), QString("34.56 EUR"));
    QCOMPARE(KoOdfNumberStyles::formatCurrency(34.56, "-$#,###0.", QString(), 0), QString("$35"));
    QString localDependentDollar = escapeLocals(KoOdfNumberStyles::formatCurrency(34.5, "#,###0 CCC", "CCC"));
    QVERIFY(localDependentDollar.startsWith("34.50") || localDependentDollar.endsWith("34.50"));
    QVERIFY(localDependentDollar.startsWith("USD") || localDependentDollar.endsWith("USD"));
    QCOMPARE(KoOdfNumberStyles::formatCurrency(34.56789, "-#,###0.00 EUR", QString(), 2), QString("34.57 EUR"));
    QCOMPARE(KoOdfNumberStyles::formatCurrency(34.5, "-#,###0.00 EUR", QString(), 2), QString("34.50 EUR"));

    KoOdfNumberStyles::NumericStyleFormat f;
    f.type = KoOdfNumberStyles::Currency;
    f.currencySymbol = "";
    f.precision = 2;
    f.formatStr = "-#,###0.00 EUR";
    QCOMPARE(escapeLocals(KoOdfNumberStyles::format("0.2", f)), QString("0.20 EUR"));
    QCOMPARE(escapeLocals(KoOdfNumberStyles::format("$ 1.23", f)), QString("$ 1.23"));
    QCOMPARE(escapeLocals(KoOdfNumberStyles::format("test", f)), QString("test"));
    f.currencySymbol = "$";
    QCOMPARE(escapeLocals(KoOdfNumberStyles::format("0.2", f)), QString("0.20 EUR"));
    f.formatStr = "-#,###0.00";
    QCOMPARE(escapeLocals(KoOdfNumberStyles::format("0.2", f)), QString("0.20"));
    f.formatStr = "";
    localDependentDollar = escapeLocals(KoOdfNumberStyles::format("0.2", f));
    QVERIFY(localDependentDollar.startsWith("0.20") || localDependentDollar.endsWith("0.20"));
    QVERIFY(localDependentDollar.startsWith("$") || localDependentDollar.endsWith("$"));
}

QTEST_MAIN(TestNumberStyle)
