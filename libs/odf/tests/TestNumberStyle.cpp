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

#include <KoUnit.h>

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
}

void TestNumberStyle::testBoolean()
{
    QCOMPARE(KoOdfNumberStyles::formatBoolean("0", ""), QString("FALSE"));
    QCOMPARE(KoOdfNumberStyles::formatBoolean("234", ""), QString("TRUE"));
    QCOMPARE(KoOdfNumberStyles::formatBoolean("0", ""), QString("FALSE"));
}

void TestNumberStyle::testPercent()
{
    QCOMPARE(KoOdfNumberStyles::formatPercent("23", ""), QString("23"));
    QCOMPARE(KoOdfNumberStyles::formatPercent("23.4567", "0.00%", 2), QString("2345.67%"));
    QCOMPARE(KoOdfNumberStyles::formatPercent("23.456789", "0.0000%", 4), QString("2345.6789%"));
    QCOMPARE(KoOdfNumberStyles::formatPercent("0", ""), QString("0"));
}

void TestNumberStyle::testScientific()
{
    QCOMPARE(KoOdfNumberStyles::formatScientific(345678, "0.00E+000"), QString("3,456780E+05"));
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
    QCOMPARE(KoOdfNumberStyles::formatCurrency(34.5, "#,###0 CCC", "CCC"), QString("34,50 USD"));
    QCOMPARE(KoOdfNumberStyles::formatCurrency(34.56789, "-#,###0.00 EUR", QString(), 2), QString("34.57 EUR"));
    QCOMPARE(KoOdfNumberStyles::formatCurrency(34.5, "-#,###0.00 EUR", QString(), 2), QString("34.50 EUR"));
}

QTEST_MAIN(TestNumberStyle)
#include <TestNumberStyle.moc>
