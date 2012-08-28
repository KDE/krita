/* This file is part of the Calligra project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2011 Stuart Dickson <stuart@furkinfantastic.net>
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
#include "TestStyles.h"

#include <styles/KoParagraphStyle.h>
#include <KDebug>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextLength>

void TestStyles::testStyleInheritance()
{
    KoParagraphStyle style1;
    style1.setTopMargin(QTextLength(QTextLength::FixedLength, 10.0));
    QCOMPARE(style1.topMargin(), 10.0);

    KoParagraphStyle style2;
    style2.setParentStyle(&style1);

    QCOMPARE(style2.topMargin(), 10.0);
    style2.setTopMargin(QTextLength(QTextLength::FixedLength, 20.0));
    QCOMPARE(style2.topMargin(), 20.0);
    QCOMPARE(style1.topMargin(), 10.0);

    style1.setTopMargin(QTextLength(QTextLength::FixedLength, 15.0));
    QCOMPARE(style2.topMargin(), 20.0);
    QCOMPARE(style1.topMargin(), 15.0);

    style2.setTopMargin(QTextLength(QTextLength::FixedLength, 15.0)); // the same, resetting the difference.
    QCOMPARE(style2.topMargin(), 15.0);
    QCOMPARE(style1.topMargin(), 15.0);

    style1.setTopMargin(QTextLength(QTextLength::FixedLength, 12.0)); // parent, so both are affected
    QCOMPARE(style2.topMargin(), 12.0);
    QCOMPARE(style1.topMargin(), 12.0);
}

void TestStyles::testChangeParent()
{
    KoParagraphStyle style1;
    style1.setTopMargin(QTextLength(QTextLength::FixedLength, 10.0));

    KoParagraphStyle style2;
    style2.setTopMargin(QTextLength(QTextLength::FixedLength, 20.0));

    style2.setParentStyle(&style1);
    QCOMPARE(style1.topMargin(), 10.0);
    QCOMPARE(style2.topMargin(), 20.0);

    KoParagraphStyle style3;
    style3.setParentStyle(&style1);
    QCOMPARE(style1.topMargin(), 10.0);
    QCOMPARE(style3.topMargin(), 10.0);

    // test that separating will leave the child with exactly the same dataset
    // as it had before the inheritance
    style3.setParentStyle(0);
    QCOMPARE(style1.topMargin(), 10.0);
    QCOMPARE(style3.topMargin(), 0.0); // we hadn't explicitly set the margin on style3

    // test adding it to another will not destroy any data
    style3.setParentStyle(&style1);
    QCOMPARE(style1.topMargin(), 10.0); // from style1
    QCOMPARE(style2.topMargin(), 20.0); // from style2
    QCOMPARE(style3.topMargin(), 10.0); // inherited from style1

    // Check that style3 now starts following the parent since it does not have
    // the property set
    style3.setParentStyle(&style2);
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
    QCOMPARE(format.properties().count(), 2); // the styleId and nextStyleId
    QCOMPARE(format.property(KoParagraphStyle::StyleId).toInt(), 1001);
    QCOMPARE(format.property(KoParagraphStyle::NextStyle).toInt(), 1001);

    style.setName("name");
    style.setAlignment(Qt::AlignRight);
    style.applyStyle(format);
    QCOMPARE(format.properties().count(), 3);
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

    style3.setParentStyle(&style2);
    style2.setParentStyle(&style1);

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

    style3.setLineSpacing(23.45);
    style3.setLineHeightPercent(150);
    style3.setLineHeightAbsolute(8.0);
    QCOMPARE(style3.lineHeightPercent(), 0.0);
    QCOMPARE(style3.lineHeightAbsolute(), 8.0);
    QCOMPARE(style3.lineSpacing(), 23.45);
    QVERIFY(!style3.hasNormalLineHeight());

    style3.setNormalLineHeight();
    QCOMPARE(style3.lineHeightPercent(), 0.0);
    QCOMPARE(style3.lineHeightAbsolute(), 0.0);
    QCOMPARE(style3.lineSpacing(), 0.0);
    QVERIFY(style3.hasNormalLineHeight());

    style3.setLineHeightPercent(150);
    style3.setLineSpacing(56.78);
    QCOMPARE(style3.lineHeightPercent(), 150.0);
    QCOMPARE(style3.lineHeightAbsolute(), 0.0);
    QCOMPARE(style3.lineSpacing(), 56.78);
    QVERIFY(!style3.hasNormalLineHeight());

    QTextLength length0(QTextLength::FixedLength, 0.0);
    QTextLength length1(QTextLength::FixedLength, 10.0);
    QTextLength length2(QTextLength::FixedLength, 20.0);

    style1.setLeftMargin(length1);
    QCOMPARE(style1.leftMargin(), 10.0);
    QCOMPARE(style2.leftMargin(), 10.0);
    QCOMPARE(style3.leftMargin(), 10.0);
    style2.setRightMargin(length2);
    QCOMPARE(style1.rightMargin(), 0.0);
    QCOMPARE(style2.rightMargin(), 20.0);
    QCOMPARE(style3.rightMargin(), 20.0);

    // now actually apply it.
    QTextBlockFormat rawFormat;
    style1.applyStyle(rawFormat);
    KoParagraphStyle format(rawFormat, rawFormat.toCharFormat());
    QCOMPARE(rawFormat.properties().count(), 4);
    QCOMPARE(format.alignment(), Qt::AlignRight);
    QCOMPARE(rawFormat.property(KoParagraphStyle::StyleId).toInt(), 1002);
    //since we have not specified any NextStyle it should be the same as the current style
    QCOMPARE(rawFormat.property(KoParagraphStyle::StyleId).toInt(), rawFormat.property(KoParagraphStyle::NextStyle).toInt());
    QCOMPARE(format.leftMargin(), 10.0);
    QCOMPARE(format.rightMargin(), 0.0);

    style2.applyStyle(rawFormat);
    KoParagraphStyle format2(rawFormat, rawFormat.toCharFormat());
    QCOMPARE(rawFormat.properties().count(), 5);
    QCOMPARE(format2.alignment(), Qt::AlignCenter);
    QCOMPARE(rawFormat.property(KoParagraphStyle::StyleId).toInt(), 1003);
    //since we have not specified any NextStyle it should be the same as the current style
    QCOMPARE(rawFormat.property(KoParagraphStyle::StyleId).toInt(), rawFormat.property(KoParagraphStyle::NextStyle).toInt());
    QCOMPARE(format2.leftMargin(), 10.0);
    QCOMPARE(format2.rightMargin(), 20.0);

    style3.applyStyle(rawFormat);
    KoParagraphStyle format3(rawFormat, rawFormat.toCharFormat());
    QCOMPARE(rawFormat.properties().count(), 9);
    QCOMPARE(rawFormat.property(KoParagraphStyle::LineSpacing).toReal(), 56.78);
    QCOMPARE(format3.alignment(), Qt::AlignLeft | Qt::AlignAbsolute);
    QCOMPARE(rawFormat.property(KoParagraphStyle::StyleId).toInt(), 1004);
    //since we have not specified any NextStyle it should be the same as the current style
    QCOMPARE(rawFormat.property(KoParagraphStyle::StyleId).toInt(), rawFormat.property(KoParagraphStyle::NextStyle).toInt());
    QCOMPARE(format3.leftMargin(), 10.0);
    QCOMPARE(format3.rightMargin(), 20.0);
}

void TestStyles::testCopyParagraphStyle()
{
    QTextLength length1(QTextLength::FixedLength, 10.0);
    QTextLength length2(QTextLength::FixedLength, 20.0);
    QTextLength length3(QTextLength::FixedLength, 30.0);

    KoParagraphStyle style1;
    KoParagraphStyle style2;
    style2.setParentStyle(&style1);

    style1.setLeftMargin(length1);
    style1.setRightMargin(length3);
    style2.setRightMargin(length2);

    KoParagraphStyle newStyle;
    newStyle.copyProperties(&style2);
    QCOMPARE(newStyle.leftMargin(), 10.0);
    QCOMPARE(newStyle.rightMargin(), 20.0);
}

void TestStyles::testUnapplyStyle()
{
    // Used to test OverlineColor style
    QColor testOverlineColor(255, 128, 64);
    KoCharacterStyle::LineWeight testOverlineWeight = KoCharacterStyle::ThickLineWeight;
    qreal testOverlineWidth = 1.5;

    // in this test we should avoid testing any of the hardcodedDefaultProperties; see KoCharacterStyle for details!
    KoParagraphStyle headers;
    headers.setOverlineColor(testOverlineColor);
    headers.setOverlineMode(KoCharacterStyle::ContinuousLineMode);
    headers.setOverlineStyle(KoCharacterStyle::DottedLine);
    headers.setOverlineType(KoCharacterStyle::DoubleLine);
    headers.setOverlineWidth(testOverlineWeight, testOverlineWidth);
    headers.setFontWeight(QFont::Bold);
    headers.setAlignment(Qt::AlignCenter);
    KoParagraphStyle head1;
    head1.setParentStyle(&headers);
    head1.setLeftMargin(QTextLength(QTextLength::FixedLength, 40));

    QTextDocument doc;
    doc.setPlainText("abc");
    QTextBlock block = doc.begin();
    head1.applyStyle(block);

    QTextCursor cursor(block);
    QTextBlockFormat bf = cursor.blockFormat();
    KoParagraphStyle bfStyle (bf, cursor.charFormat());
    QCOMPARE(bf.alignment(), Qt::AlignCenter);
    QCOMPARE(bfStyle.leftMargin(), 40.);
    QTextCharFormat cf = cursor.charFormat();
    QCOMPARE(cf.colorProperty(KoCharacterStyle::OverlineColor), testOverlineColor);
    QCOMPARE(cf.intProperty(KoCharacterStyle::OverlineMode), (int) KoCharacterStyle::ContinuousLineMode);
    QCOMPARE(cf.intProperty(KoCharacterStyle::OverlineStyle), (int) KoCharacterStyle::DottedLine);
    QCOMPARE(cf.intProperty(KoCharacterStyle::OverlineType), (int) KoCharacterStyle::DoubleLine);
    QCOMPARE(cf.intProperty(KoCharacterStyle::OverlineWeight), (int) testOverlineWeight);
    QCOMPARE(cf.doubleProperty(KoCharacterStyle::OverlineWidth), testOverlineWidth);

    head1.unapplyStyle(block);
    bf = cursor.blockFormat();
    QCOMPARE(bf.hasProperty(QTextFormat::BlockAlignment), false);
    QCOMPARE(bf.hasProperty(QTextFormat::BlockLeftMargin), false);
    cf = cursor.charFormat();
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineColor), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineMode), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineStyle), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineType), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineWeight), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineWidth), false);

    doc.clear();
    block = doc.begin();
    head1.applyStyle(block);
    bf = cursor.blockFormat();
    KoParagraphStyle bfStyle2 (bf, cursor.charFormat());
    QCOMPARE(bf.alignment(), Qt::AlignCenter);
    QCOMPARE(bfStyle2.leftMargin(), 40.);
    cf = cursor.charFormat();
    //QCOMPARE(cf.fontOverline(), true);
    QCOMPARE(cf.colorProperty(KoCharacterStyle::OverlineColor), testOverlineColor);
    QCOMPARE(cf.intProperty(KoCharacterStyle::OverlineMode), (int) KoCharacterStyle::ContinuousLineMode);
    QCOMPARE(cf.intProperty(KoCharacterStyle::OverlineStyle), (int) KoCharacterStyle::DottedLine);
    QCOMPARE(cf.intProperty(KoCharacterStyle::OverlineType), (int) KoCharacterStyle::DoubleLine);
    QCOMPARE(cf.intProperty(KoCharacterStyle::OverlineWeight), (int) testOverlineWeight);
    QCOMPARE(cf.doubleProperty(KoCharacterStyle::OverlineWidth), testOverlineWidth);


    head1.unapplyStyle(block);
    bf = cursor.blockFormat();
    QCOMPARE(bf.hasProperty(QTextFormat::BlockAlignment), false);
    QCOMPARE(bf.hasProperty(QTextFormat::BlockLeftMargin), false);
    cf = cursor.charFormat();
    //QCOMPARE(cf.hasProperty(QTextFormat::FontOverline), false);

    doc.setHtml("bla bla<i>italic</i>enzo");

    block = doc.begin();
    head1.applyStyle(block);
    bf = cursor.blockFormat();
    KoParagraphStyle bfStyle3(bf, cursor.charFormat());
    QCOMPARE(bf.alignment(), Qt::AlignCenter);
    QCOMPARE(bfStyle3.leftMargin(), 40.);
    cf = cursor.charFormat();
    //QCOMPARE(cf.fontOverline(), true);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineColor), true);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineMode), true);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineStyle), true);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineType), true);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineWeight), true);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineWidth), true);

    cursor.setPosition(7);
    cursor.setPosition(12, QTextCursor::KeepAnchor);
    QTextCharFormat italic;
    italic.setFontItalic(true);
    cursor.mergeCharFormat(italic);
    cursor.setPosition(8);
    cf = cursor.charFormat();
    QCOMPARE(cf.fontItalic(), true);
    cursor.setPosition(0);

    head1.unapplyStyle(block);
    cursor.setPosition(0);
    bf = cursor.blockFormat();
    QCOMPARE(bf.hasProperty(QTextFormat::BlockAlignment), false);
    QCOMPARE(bf.hasProperty(QTextFormat::BlockLeftMargin), false);
    cf = cursor.charFormat();
    //QCOMPARE(cf.hasProperty(QTextFormat::FontOverline), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineColor), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineMode), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineStyle), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineType), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineWeight), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineWidth), false);


    cursor.setPosition(8);
    cf = cursor.charFormat();
    //QCOMPARE(cf.hasProperty(QTextFormat::FontOverline), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineColor), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineMode), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineStyle), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineType), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineWeight), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineWidth), false);
    QCOMPARE(cf.fontItalic(), true);
    cursor.setPosition(12);
    cf = cursor.charFormat();
    //QCOMPARE(cf.hasProperty(QTextFormat::FontOverline), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineColor), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineMode), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineStyle), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineType), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineWeight), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineWidth), false);
    QCOMPARE(cf.fontItalic(), true);

    cursor.setPosition(13);
    cf = cursor.charFormat();
    //QCOMPARE(cf.hasProperty(QTextFormat::FontOverline), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineColor), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineMode), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineStyle), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineType), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineWeight), false);
    QCOMPARE(cf.hasProperty(KoCharacterStyle::OverlineWidth), false);
    QCOMPARE(cf.hasProperty(QTextFormat::FontWeight), false);
    QCOMPARE(cf.hasProperty(QTextFormat::FontItalic), false);
}

QTEST_KDEMAIN(TestStyles, GUI)
#include <TestStyles.moc>
