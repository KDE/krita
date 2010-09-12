/*
 *  This file is part of KOffice tests
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
#ifndef TESTDOCUMENTAYOUT_H
#define TESTDOCUMENTAYOUT_H

#include <QObject>
#include <qtest_kde.h>

#include "../Layout.h"
#include "MockTextShape.h"

#include <KoTextShapeData.h>
#include <KoTextDocumentLayout.h>
#include <KoShape.h>

class QPainter;
class KoViewConverter;
class KoStyleManager;
class QTextDocument;
class QTextLayout;

#define ROUNDING 0.126

class TestDocumentLayout : public QObject
{
    Q_OBJECT
public:
    TestDocumentLayout() {}

private slots:
    void initTestCase();

    /// make sure empty paragraphs are initialized properly
    void testEmptyParag();

    /// Test breaking lines based on the width of the shape.
    void testLineBreaking();
    /// Test breaking lines for frames with different widths.
    void testMultiFrameLineBreaking();
    /// Tests incrementing Y pos based on the font size
    void testBasicLineSpacing();
    /// Tests incrementing Y pos based on the font size
    void testBasicLineSpacing2();
    /// Tests advanced linespacing options provided in our style.
    void testAdvancedLineSpacing();
    /// test data integrety for multiple shapes.
    void testShapePosition();
    void testShapePosition2();

// Block styles
    /// Test top, left, right and bottom margins of paragraphs.
    void testMargins();
    void testMultipageMargins();
    void testTextIndent();
    void testBasicTextAlignments();
    void testTextAlignments();
    void testPageBreak();
    void testPageBreak2();
    void testNonBreakableLines();

// Lists
    void testBasicList();
    void testNumberedList();
    void testInterruptedLists(); // consecutiveNumbering
    void testNestedLists();
    void testNestedPrefixedLists();
    void testAutoRestartList();
    void testListParagraphIndent();
    void testRomanNumbering();
    void testUpperAlphaNumbering();
    void testRestartNumbering();
    void testRightToLeftList();
    void testLetterSynchronization();
    void testInvalidateLists();
    void testCenteredItems();
    void testMultiLevel();

// relativeBulletSize

    //etc
    void testParagOffset();
    void testParagraphBorders();
    void testBorderData();
    void testDropCaps();

private:
    void initForNewTest(const QString &initText = QString());

private:
    MockTextShape *m_shape1;
    QTextDocument *m_doc;
    KoTextDocumentLayout *m_layout;
    QTextLayout *m_blockLayout;
    QString m_loremIpsum;
    KoStyleManager *m_styleManager;
    Layout *m_textLayout;
};

#endif
