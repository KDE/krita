/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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
