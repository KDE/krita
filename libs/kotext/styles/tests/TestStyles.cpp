/* This file is part of the KOffice project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

void TestStyles::testStyleInheritance()
{
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

void TestStyles::testChangeParent()
{
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
    // as it had before the inheritance
    style3.setParent(0);
    QCOMPARE(style1.topMargin(), 10.0);
    QCOMPARE(style3.topMargin(), 0.0); // we hadn't explicitly set the margin on style3

    // test adding it to another will not destroy any data
    style3.setParent(&style1);
    QCOMPARE(style1.topMargin(), 10.0); // from style1
    QCOMPARE(style2.topMargin(), 20.0); // from style2
    QCOMPARE(style3.topMargin(), 10.0); // inherited from style1

    // Check that style3 now starts following the parent since it does not have
    // the property set
    style3.setParent(&style2);
    QCOMPARE(style3.topMargin(), 20.0); // inherited from style2
}

void TestStyles::testTabsStorage()
{
    KoParagraphStyle paragStyle;

    QList<KoText::Tab> tabs;
    paragStyle.setTabPositions(tabs);
    QCOMPARE(paragStyle.tabPositions().count(), 0);

    KoText::Tab tab;
    tabs.append(tab);
    KoText::Tab tab2;
    tab2.position = 10;
    tabs.append(tab2);

    paragStyle.setTabPositions(tabs);
    QCOMPARE(paragStyle.tabPositions().count(), 2);
    QCOMPARE(paragStyle.tabPositions()[0], tab);
    QCOMPARE(paragStyle.tabPositions()[1], tab2);
}

void TestStyles::testApplyParagraphStyle()
{
    KoParagraphStyle style;
    style.setStyleId(1001);

    QTextBlockFormat format;
    QCOMPARE(format.properties().count(), 0);
    style.applyStyle(format);
    QCOMPARE(format.properties().count(), 1); // the styleId

    style.setName("name");
    style.setAlignment(Qt::AlignRight);
    style.applyStyle(format);
    QCOMPARE(format.properties().count(), 2);
    QCOMPARE(format.alignment(), Qt::AlignRight);
}

void TestStyles::testApplyParagraphStyleWithParent()
{
    KoParagraphStyle style1;
    style1.setStyleId(1002);
    KoParagraphStyle style2;
    style2.setStyleId(1003);
    KoParagraphStyle style3;
    style3.setStyleId(1004);

    style3.setParent(&style2);
    style2.setParent(&style1);

    style1.setAlignment(Qt::AlignRight);
    QCOMPARE(style1.alignment(), Qt::AlignRight);
    QCOMPARE(style2.alignment(), Qt::AlignRight);
    QCOMPARE(style3.alignment(), Qt::AlignRight);

    style2.setAlignment(Qt::AlignCenter);
    QCOMPARE(style1.alignment(), Qt::AlignRight);
    QCOMPARE(style2.alignment(), Qt::AlignCenter);
    QCOMPARE(style3.alignment(), Qt::AlignCenter);

    style3.setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    QCOMPARE(style1.alignment(), Qt::AlignRight);
    QCOMPARE(style2.alignment(), Qt::AlignCenter);
    QCOMPARE(style3.alignment(), Qt::AlignLeft | Qt::AlignAbsolute);

    style1.setLeftMargin(10.);
    QCOMPARE(style1.leftMargin(), 10.);
    QCOMPARE(style2.leftMargin(), 10.);
    QCOMPARE(style3.leftMargin(), 10.);
    style2.setRightMargin(20.);
    QCOMPARE(style1.rightMargin(), 0.);
    QCOMPARE(style2.rightMargin(), 20.);
    QCOMPARE(style3.rightMargin(), 20.);

    // now actually apply it.
    QTextBlockFormat format;
    style1.applyStyle(format);
    QCOMPARE(format.properties().count(), 3);
    QCOMPARE(format.alignment(), Qt::AlignRight);
    QCOMPARE(format.property(KoParagraphStyle::StyleId).toInt(), 1002);
    QCOMPARE(format.leftMargin(), 10.);
    QCOMPARE(format.rightMargin(), 0.);

    style2.applyStyle(format);
    QCOMPARE(format.properties().count(), 4);
    QCOMPARE(format.alignment(), Qt::AlignCenter);
    QCOMPARE(format.property(KoParagraphStyle::StyleId).toInt(), 1003);
    QCOMPARE(format.leftMargin(), 10.);
    QCOMPARE(format.rightMargin(), 20.);

    style3.applyStyle(format);
    QCOMPARE(format.properties().count(), 4);
    QCOMPARE(format.alignment(), Qt::AlignLeft | Qt::AlignAbsolute);
    QCOMPARE(format.property(KoParagraphStyle::StyleId).toInt(), 1004);
    QCOMPARE(format.leftMargin(), 10.);
    QCOMPARE(format.rightMargin(), 20.);
}

void TestStyles::testCopyParagraphStyle()
{
    KoParagraphStyle style1;
    KoParagraphStyle style2;
    style2.setParent(&style1);

    style1.setLeftMargin(10.);
    style1.setRightMargin(30.);
    style2.setRightMargin(20.);

    KoParagraphStyle newStyle;
    newStyle.copyProperties(&style2);
    QCOMPARE(newStyle.leftMargin(), 10.);
    QCOMPARE(newStyle.rightMargin(), 20.);
}

QTEST_KDEMAIN(TestStyles, GUI)
#include "TestStyles.moc"
