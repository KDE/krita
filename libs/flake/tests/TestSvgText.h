/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TESTSVGTEXT_H
#define TESTSVGTEXT_H

#include <simpletest.h>

class TestSvgText : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testTextProperties();
    void testDefaultTextProperties();
    void testTextPropertiesDifference();

    void testParseFontStyles();
    void testParseTextStyles();


    void testSimpleText();
    void testComplexText();
    void testHindiText();
    void testTextBaselineShift();
    void testTextSpacing();
    void testTextTabSpacing();
    void testTextDecorations();
    void testRightToLeft();
    void testRightToLeftAnchoring();
    void testVerticalText();

    void testQtBidi();
    void testQtDxDy();

    void testTextOutlineSolid();
    void testNbspHandling();
    void testMulticolorText();

    void testConvertToStrippedSvg();
    void testConvertToStrippedSvgNullOrigin();
    void testConvertFromIncorrectStrippedSvg();

    void testEmptyTextChunk();
    void testTrailingWhitespace();
    void testWhiteSpaceRules();

    void testConvertHtmlToSvg();
    void testTextWithMultipleRelativeOffsets();
    void testTextWithMultipleAbsoluteOffsetsArabic();
    void testTextWithMultipleRelativeOffsetsArabic();
    void testTextWithMultipleRelativeOffsetsVertical();
    void testTextWithMultipleRotations();

    void testTextOutline();

    void testTextFontSize();

    void testAddingTestFont();

    void testUnicodeGraphemeClusters();

    void testFontSelectionForText();

    void testWWSConverterWeight_data();
    void testWWSConverterWeight();
    void testWWSConverterSlant_data();
    void testWWSConverterSlant();
    void testWWSConverterWidth_data();
    void testWWSConverterWidth();
    void testWWSConverterFamilyNames_data();
    void testWWSConverterFamilyNames();

    void testFontSizeConfiguration();

    void testFontSizeRender();
    void testFontOpenTypeVariationsConfiguration();
    void testFontColorRender();

    void testCssFontVariants();

    void testTextLength();
    void testTextPathBasic();
    void testTextPathComplex();

    void testCssTextTransform();

    void testTextInlineSize();

    void testTextWrap();

    void testTextBaselineAlignment();

    void testFontMetrics_data();
    void testFontMetrics();

    void testFontRelativeUnits_data();
    void testFontRelativeUnits();

    void testCssShapeParsing();
    void testShapeInsideRender();

    void testTextInsertion();

    void testTextDeletion_data();
    void testTextDeletion();
    void testNavigation();

    void testTextRichCopy();
    void testTextRichInsert();

    void testTextSplit();
    void testTextCleanUp();

    void testTextRichTextMerge();

    void testBcp47Parsing_data();
    void testBcp47Parsing();

    void testSearchingTreeIndex();
    void testRangeForTreeIndex();
    void testSetPropertiesOnTreeIndex();

    void testInsertTransforms_data();
    void testInsertTransforms();

    void testRemoveTransforms_data();
    void testRemoveTransforms();

    void testApplyWhiteSpace_data();
    void testApplyWhiteSpace();

    void testInsertNewLinesAtAnchors_data();
    void testInsertNewLinesAtAnchors();
};

#endif // TESTSVGTEXT_H
