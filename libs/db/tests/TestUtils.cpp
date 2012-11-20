/* This file is part of the KDE project
   Copyright (C) 2012 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "TestUtils.h"
#include <db/utils.h>
#include <QTest>

void TestUtils::initTestCase()
{
}

void TestUtils::testIsIdentifier()
{
    QVERIFY(!KexiDB::isIdentifier(""));
    QVERIFY(!KexiDB::isIdentifier(QString()));
    QVERIFY(!KexiDB::isIdentifier("\0"));
    QVERIFY(!KexiDB::isIdentifier(" "));
    QVERIFY(!KexiDB::isIdentifier("7"));
    QVERIFY(KexiDB::isIdentifier("_"));
    QVERIFY(KexiDB::isIdentifier("abc_2"));
    QVERIFY(KexiDB::isIdentifier("Abc_2"));
    QVERIFY(KexiDB::isIdentifier("_7"));
}

void TestUtils::cleanupTestCase()
{
}

QTEST_MAIN(TestUtils)
