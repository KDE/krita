/*
 *  This file is part of Calligra tests
 *
 *  Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
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
#ifndef TESTBLOCKLAYOUT_H
#define TESTBLOCKLAYOUT_H

#include <QObject>
#include <qtest_kde.h>

#include <KoTextDocumentLayout.h>
#include <KoTextLayoutRootArea.h>

class QPainter;
class KoViewConverter;
class KoStyleManager;
class QTextDocument;
class QTextLayout;

#define ROUNDING 0.126

class TestBlockLayout : public QObject
{
    Q_OBJECT
public:
    TestBlockLayout() {}

private slots:
    void initTestCase();

    /// make sure empty paragraphs are initialized properly
    void testEmptyParag();

    /// Test breaking lines based on the width of the reference rect.
    void testLineBreaking();

    /// Tests incrementing Y pos based on the font size
    void testBasicLineSpacing();
    /// Tests incrementing Y pos based on the font size
    void testBasicLineSpacing2();
    /// Tests fixed linespacing.
    void testFixedLineSpacing();
    /// Tests percentage linespacing.
    void testPercentageLineSpacing();
    /// Tests advanced linespacing options provided in our style.
    void testAdvancedLineSpacing();
    /// Tests that empty lines are given the correct height like in LibreOffice
    void testEmptyLineHeights();

    /// Test distance above and below paragraphs.
    void testBlockSpacing();

    /// Test left and right margins of paragraphs.
    void testLeftRightMargins();

    /// Test first line indent of paragraphs.
    void testTextIndent();

    /// Test tabs.
    void testTabs_data();
    void testTabs();

    void testBasicTextAlignments();
    void testTextAlignments();

// relativeBulletSize

    //etc
    //void testParagOffset();
    void testParagraphBorders();
    void testBorderData();
    void testDropCaps();

private:
    void setupTest(const QString &initText = QString());

private:
    QTextDocument *m_doc;
    KoTextDocumentLayout *m_layout;
    QTextBlock m_block;
    QString m_loremIpsum;
    KoStyleManager *m_styleManager;
    KoTextLayoutRootArea *m_area;
};

#endif
