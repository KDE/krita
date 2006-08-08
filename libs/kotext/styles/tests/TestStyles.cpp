/* This file is part of the KOffice project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.

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
#include "TestStyles.h"

#include <styles/KoParagraphStyle.h>
#include <QDebug>

void TestStyles::testStyleInheritance() {
    KoParagraphStyle style1;
    style1.setTopMargin(10.0);
    QCOMPARE(style1.topMargin(), 10.0);

    KoParagraphStyle style2;
    style2.setParent(&style1);

    QCOMPARE(style2.topMargin(), 10.0);
    style2.setTopMargin(20.0);
    QCOMPARE(style2.topMargin(), 20.0);
    QCOMPARE(style1.topMargin(), 10.0);

    style1.setTopMargin(15.0);
    QCOMPARE(style2.topMargin(), 20.0);
    QCOMPARE(style1.topMargin(), 15.0);

    style2.setTopMargin(15.0); // the same, resetting the difference.
    QCOMPARE(style2.topMargin(), 15.0);
    QCOMPARE(style1.topMargin(), 15.0);

    style1.setTopMargin(12.0); // parent, so both are affected
    QCOMPARE(style2.topMargin(), 12.0);
    QCOMPARE(style1.topMargin(), 12.0);
}

void TestStyles::testChangeParent() {
    KoParagraphStyle style1;
    style1.setTopMargin(10);

    KoParagraphStyle style2;
    style2.setTopMargin(20);

    style2.setParent(&style1);
    QCOMPARE(style1.topMargin(), 10.0);
    QCOMPARE(style2.topMargin(), 20.0);

    KoParagraphStyle style3;
    style3.setParent(&style1);
    QCOMPARE(style1.topMargin(), 10.0);
    QCOMPARE(style3.topMargin(), 10.0);

    // test that separating will leave the child with exactly the same dataset
    style3.setParent(0);
    QCOMPARE(style1.topMargin(), 10.0);
    QCOMPARE(style3.topMargin(), 10.0);

    // test adding it to another will not destroy any data
    style3.setParent(&style1);
    QCOMPARE(style1.topMargin(), 10.0);
    QCOMPARE(style2.topMargin(), 20.0);
    QCOMPARE(style3.topMargin(), 10.0);

    // Due to the "don't destroy data" above style3 will no longer follow any parent.
    style3.setParent(&style2);
    QCOMPARE(style3.topMargin(), 10.0);
}

QTEST_MAIN(TestStyles)
#include "TestStyles.moc"
