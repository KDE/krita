/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TESTSVGTEXT_H
#define TESTSVGTEXT_H

#include <QtTest>

class TestSvgText : public QObject
{
    Q_OBJECT
private Q_SLOTS:
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

    void testConvertHtmlToSvg();
    void testTextWithMultipleRelativeOffsets();
    void testTextWithMultipleAbsoluteOffsetsArabic();
    void testTextWithMultipleRelativeOffsetsArabic();

    void testTextOutline();

};

#endif // TESTSVGTEXT_H
