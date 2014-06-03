/* This file is part of the KDE project

   Copyright 2007-2009 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>
   Copyright 2010      Inge wallin <inge@lysator.liu.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "TestLoadAndSave.h"

#include <QtTest>
#include <QBuffer>
#include <QDebug>

#include <KoXmlWriter.h>
#include <KoXmlReader.h>

#include "BasicElement.h"
#include "IdentifierElement.h"
#include "NumberElement.h"
#include "OperatorElement.h"
#include "TextElement.h"
#include "SpaceElement.h"
#include "StringElement.h"
#include "GlyphElement.h"
#include "RowElement.h"
#include "FractionElement.h"
#include "RootElement.h"
#include "StyleElement.h"
#include "ErrorElement.h"
#include "PaddedElement.h"
#include "PhantomElement.h"
#include "FencedElement.h"
#include "EncloseElement.h"
#include "MultiscriptElement.h"
#include "SubSupElement.h"
#include "UnderOverElement.h"
#include "TableElement.h"
#include "TableRowElement.h"
#include "TableDataElement.h"
#include "ActionElement.h"

static QString loadAndSave( BasicElement* element, const QString& input )
{
    KoXmlDocument doc;
    doc.setContent( input );
    element->readMathML( doc.documentElement() );
    QBuffer device;
    device.open( QBuffer::ReadWrite );
    KoXmlWriter writer( &device );
    element->writeMathML( &writer, "" );
    device.seek( 0 );
    return device.readAll();
}

static void addRow( const char* input, bool expectedFail = false )
{
    QString inputStr(input);
    QTest::newRow("Load and Save") << inputStr << inputStr << expectedFail;
}

static void addRow( const char* input, const char* output, bool expectedFail = false )
{
    QString inputStr(input);
    QString outputStr(output);
    QTest::newRow("Load and Save") << inputStr << outputStr << expectedFail;
}

void test( BasicElement* element )
{
    QFETCH( QString, input );
    QFETCH( QString, output );
    QFETCH( bool, expectedFail );

    //qDebug() << "expected Fail: " << expectedFail;
    if (expectedFail) {
        QEXPECT_FAIL("", "", Continue);
    }
    QCOMPARE( loadAndSave( element, input ), output );
    delete element;
}

void TestLoadAndSave::identifierElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    addRow( "<mi>x</mi>" );
    addRow( "<mi>abc</mi>" );
    addRow( "<MI>x</MI>",
            "<mi>x</mi>" );

    // See section 2.4.6 Collapsing Whitespace in Input
    addRow( "<mi> a b c </mi>",
            "<mi>a b c</mi>" );
    // Since newline is hardcoded in KoXmlWriter and it's sematically equivalent, add it to expected result
    addRow( "<mi> x <mglyph fontfamily=\"testfont\" index=\"99\" alt=\"c\"/> d </mi>",
            "<mi>x \n <mglyph fontfamily=\"testfont\" index=\"99\" alt=\"c\"/> d</mi>" );
    addRow( "<mi> x  y    z   </mi>",
            "<mi>x y z</mi>" );

    // Entities
    addRow( "<mi>&CapitalDifferentialD;</mi>", true );
    addRow( "<mi>&DifferentialD;</mi>", true );
}

void TestLoadAndSave::numberElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    addRow( "<mn>1</mn>" );
    addRow( "<mn>1.2</mn>" );
    addRow( "<mn>1,2</mn>" );
    addRow( "<mn>1 , 2</mn>" );
    addRow( "<mn> 12 </mn>",
            "<mn>12</mn>");

    // Entities
    addRow( "<mn>&ExponentialE;</mn>", true );
    addRow( "<mn>&ImaginaryI;</mn>", true );
}

void TestLoadAndSave::operatorElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    addRow( "<mo>+</mo>" );

    // Check operator attributes. Section 3.2.5.2
    addRow( "<mo form=\"prefix\">+</mo>" );
    addRow( "<mo form=\"infix\">+</mo>" );
    addRow( "<mo form=\"postfix\">+</mo>" );
    addRow( "<mo fence=\"true\">+</mo>" );
    addRow( "<mo fence=\"false\">+</mo>" );
    addRow( "<mo separator=\"true\">+</mo>" );
    addRow( "<mo separator=\"false\">+</mo>" );
    addRow( "<mo lspace=\"10em\">+</mo>" );
    addRow( "<mo lspace=\"10ex\">+</mo>" );
    addRow( "<mo lspace=\"10px\">+</mo>" );
    addRow( "<mo lspace=\"10in\">+</mo>" );
    addRow( "<mo lspace=\"10cm\">+</mo>" );
    addRow( "<mo lspace=\"10mm\">+</mo>" );
    addRow( "<mo lspace=\"10pt\">+</mo>" );
    addRow( "<mo lspace=\"10pc\">+</mo>" );
    addRow( "<mo lspace=\"90%\">+</mo>" );
    addRow( "<mo lspace=\"1.2\">+</mo>" );
    addRow( "<mo lspace=\"veryverythinmathspace\">+</mo>" );
    addRow( "<mo lspace=\"verythinmathspace\">+</mo>" );
    addRow( "<mo lspace=\"thinmathspace\">+</mo>" );
    addRow( "<mo lspace=\"mediummathspace\">+</mo>" );
    addRow( "<mo lspace=\"thickmathspace\">+</mo>" );
    addRow( "<mo lspace=\"verythickmathspace\">+</mo>" );
    addRow( "<mo lspace=\"veryverythickmathspace\">+</mo>" );
    addRow( "<mo lspace=\"negativeveryverythinmathspace\">+</mo>" );
    addRow( "<mo lspace=\"negativeverythinmathspace\">+</mo>" );
    addRow( "<mo lspace=\"negativethinmathspace\">+</mo>" );
    addRow( "<mo lspace=\"negativemediummathspace\">+</mo>" );
    addRow( "<mo lspace=\"negativethickmathspace\">+</mo>" );
    addRow( "<mo lspace=\"negativeverythickmathspace\">+</mo>" );
    addRow( "<mo lspace=\"negativeveryverythickmathspace\">+</mo>" );
    addRow( "<mo rspace=\"10em\">+</mo>" );
    addRow( "<mo rspace=\"10ex\">+</mo>" );
    addRow( "<mo rspace=\"10px\">+</mo>" );
    addRow( "<mo rspace=\"10in\">+</mo>" );
    addRow( "<mo rspace=\"10cm\">+</mo>" );
    addRow( "<mo rspace=\"10mm\">+</mo>" );
    addRow( "<mo rspace=\"10pt\">+</mo>" );
    addRow( "<mo rspace=\"10pc\">+</mo>" );
    addRow( "<mo rspace=\"90%\">+</mo>" );
    addRow( "<mo rspace=\"1.2\">+</mo>" );
    addRow( "<mo rspace=\"veryverythinmathspace\">+</mo>" );
    addRow( "<mo rspace=\"verythinmathspace\">+</mo>" );
    addRow( "<mo rspace=\"thinmathspace\">+</mo>" );
    addRow( "<mo rspace=\"mediummathspace\">+</mo>" );
    addRow( "<mo rspace=\"thickmathspace\">+</mo>" );
    addRow( "<mo rspace=\"verythickmathspace\">+</mo>" );
    addRow( "<mo rspace=\"veryverythickmathspace\">+</mo>" );
    addRow( "<mo rspace=\"negativeveryverythinmathspace\">+</mo>" );
    addRow( "<mo rspace=\"negativeverythinmathspace\">+</mo>" );
    addRow( "<mo rspace=\"negativethinmathspace\">+</mo>" );
    addRow( "<mo rspace=\"negativemediummathspace\">+</mo>" );
    addRow( "<mo rspace=\"negativethickmathspace\">+</mo>" );
    addRow( "<mo rspace=\"negativeverythickmathspace\">+</mo>" );
    addRow( "<mo rspace=\"negativeveryverythickmathspace\">+</mo>" );
    addRow( "<mo stretchy=\"true\">+</mo>" );
    addRow( "<mo stretchy=\"false\">+</mo>" );
    addRow( "<mo symmetric=\"true\">+</mo>" );
    addRow( "<mo symmetric=\"false\">+</mo>" );
    addRow( "<mo maxsize=\"10em\">+</mo>" );
    addRow( "<mo maxsize=\"10ex\">+</mo>" );
    addRow( "<mo maxsize=\"10px\">+</mo>" );
    addRow( "<mo maxsize=\"10in\">+</mo>" );
    addRow( "<mo maxsize=\"10cm\">+</mo>" );
    addRow( "<mo maxsize=\"10mm\">+</mo>" );
    addRow( "<mo maxsize=\"10pt\">+</mo>" );
    addRow( "<mo maxsize=\"10pc\">+</mo>" );
    addRow( "<mo maxsize=\"90%\">+</mo>" );
    addRow( "<mo maxsize=\"1.2\">+</mo>" );
    addRow( "<mo maxsize=\"veryverythinmathspace\">+</mo>" );
    addRow( "<mo maxsize=\"verythinmathspace\">+</mo>" );
    addRow( "<mo maxsize=\"thinmathspace\">+</mo>" );
    addRow( "<mo maxsize=\"mediummathspace\">+</mo>" );
    addRow( "<mo maxsize=\"thickmathspace\">+</mo>" );
    addRow( "<mo maxsize=\"verythickmathspace\">+</mo>" );
    addRow( "<mo maxsize=\"veryverythickmathspace\">+</mo>" );
    addRow( "<mo maxsize=\"negativeveryverythinmathspace\">+</mo>" );
    addRow( "<mo maxsize=\"negativeverythinmathspace\">+</mo>" );
    addRow( "<mo maxsize=\"negativethinmathspace\">+</mo>" );
    addRow( "<mo maxsize=\"negativemediummathspace\">+</mo>" );
    addRow( "<mo maxsize=\"negativethickmathspace\">+</mo>" );
    addRow( "<mo maxsize=\"negativeverythickmathspace\">+</mo>" );
    addRow( "<mo maxsize=\"negativeveryverythickmathspace\">+</mo>" );
    addRow( "<mo maxsize=\"infinity\">+</mo>" );
    addRow( "<mo minsize=\"10em\">+</mo>" );
    addRow( "<mo minsize=\"10ex\">+</mo>" );
    addRow( "<mo minsize=\"10px\">+</mo>" );
    addRow( "<mo minsize=\"10in\">+</mo>" );
    addRow( "<mo minsize=\"10cm\">+</mo>" );
    addRow( "<mo minsize=\"10mm\">+</mo>" );
    addRow( "<mo minsize=\"10pt\">+</mo>" );
    addRow( "<mo minsize=\"10pc\">+</mo>" );
    addRow( "<mo minsize=\"90%\">+</mo>" );
    addRow( "<mo minsize=\"1.2\">+</mo>" );
    addRow( "<mo minsize=\"veryverythinmathspace\">+</mo>" );
    addRow( "<mo minsize=\"verythinmathspace\">+</mo>" );
    addRow( "<mo minsize=\"thinmathspace\">+</mo>" );
    addRow( "<mo minsize=\"mediummathspace\">+</mo>" );
    addRow( "<mo minsize=\"thickmathspace\">+</mo>" );
    addRow( "<mo minsize=\"verythickmathspace\">+</mo>" );
    addRow( "<mo minsize=\"veryverythickmathspace\">+</mo>" );
    addRow( "<mo minsize=\"negativeveryverythinmathspace\">+</mo>" );
    addRow( "<mo minsize=\"negativeverythinmathspace\">+</mo>" );
    addRow( "<mo minsize=\"negativethinmathspace\">+</mo>" );
    addRow( "<mo minsize=\"negativemediummathspace\">+</mo>" );
    addRow( "<mo minsize=\"negativethickmathspace\">+</mo>" );
    addRow( "<mo minsize=\"negativeverythickmathspace\">+</mo>" );
    addRow( "<mo minsize=\"negativeveryverythickmathspace\">+</mo>" );
    addRow( "<mo largeop=\"true\">+</mo>" );
    addRow( "<mo largeop=\"false\">+</mo>" );
    addRow( "<mo movablelimits=\"true\">+</mo>" );
    addRow( "<mo movablelimits=\"false\">+</mo>" );
    addRow( "<mo accent=\"true\">+</mo>" );
    addRow( "<mo accent=\"false\">+</mo>" );

    // Entities
    addRow( "<mo>&InvisibleTimes;</mo>", true );
    addRow( "<mo>&InvisibleComma;</mo>", true );
    addRow( "<mo>&ApplyFunction;</mo>", true );
}

void TestLoadAndSave::textElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    addRow( "<mtext></mtext>" );
    addRow( "<mtext>text</mtext>" );
    addRow( "<mtext> text </mtext>",
            "<mtext>text</mtext>");
    addRow( "<mtext> Theorem 1: </mtext>",
            "<mtext>Theorem 1:</mtext>" );
    addRow( "<mtext> &ThinSpace; </mtext>",
            "<mtext>&ThinSpace;</mtext>", true );
    addRow( "<mtext> &ThickSpace;&ThickSpace; </mtext>",
            "<mtext>&ThickSpace;&ThickSpace;</mtext>", true );
    addRow( "<mtext> /* a comment */ </mtext>",
            "<mtext>/* a comment */</mtext>" );
}

void TestLoadAndSave::spaceElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    addRow( "<mspace/>" );

    // Check operator attributes. Sefction 3.2.7.2

    addRow( "<mspace width=\"10em\"/>" );
    addRow( "<mspace width=\"10ex\"/>" );
    addRow( "<mspace width=\"10px\"/>" );
    addRow( "<mspace width=\"10in\"/>" );
    addRow( "<mspace width=\"10cm\"/>" );
    addRow( "<mspace width=\"10mm\"/>" );
    addRow( "<mspace width=\"10pt\"/>" );
    addRow( "<mspace width=\"10pc\"/>" );
    addRow( "<mspace width=\"90%\"/>" );
    addRow( "<mspace width=\"1.2\"/>" );
    addRow( "<mspace width=\"veryverythinmathspace\"/>" );
    addRow( "<mspace width=\"verythinmathspace\"/>" );
    addRow( "<mspace width=\"thinmathspace\"/>" );
    addRow( "<mspace width=\"mediummathspace\"/>" );
    addRow( "<mspace width=\"thickmathspace\"/>" );
    addRow( "<mspace width=\"verythickmathspace\"/>" );
    addRow( "<mspace width=\"veryverythickmathspace\"/>" );
    addRow( "<mspace width=\"negativeveryverythinmathspace\"/>" );
    addRow( "<mspace width=\"negativeverythinmathspace\"/>" );
    addRow( "<mspace width=\"negativethinmathspace\"/>" );
    addRow( "<mspace width=\"negativemediummathspace\"/>" );
    addRow( "<mspace width=\"negativethickmathspace\"/>" );
    addRow( "<mspace width=\"negativeverythickmathspace\"/>" );
    addRow( "<mspace width=\"negativeveryverythickmathspace\"/>" );

    addRow( "<mspace height=\"10em\"/>" );
    addRow( "<mspace height=\"10ex\"/>" );
    addRow( "<mspace height=\"10px\"/>" );
    addRow( "<mspace height=\"10in\"/>" );
    addRow( "<mspace height=\"10cm\"/>" );
    addRow( "<mspace height=\"10mm\"/>" );
    addRow( "<mspace height=\"10pt\"/>" );
    addRow( "<mspace height=\"10pc\"/>" );
    addRow( "<mspace height=\"90%\"/>" );
    addRow( "<mspace height=\"1.2\"/>" );

    addRow( "<mspace depth=\"10em\"/>" );
    addRow( "<mspace depth=\"10ex\"/>" );
    addRow( "<mspace depth=\"10px\"/>" );
    addRow( "<mspace depth=\"10in\"/>" );
    addRow( "<mspace depth=\"10cm\"/>" );
    addRow( "<mspace depth=\"10mm\"/>" );
    addRow( "<mspace depth=\"10pt\"/>" );
    addRow( "<mspace depth=\"10pc\"/>" );
    addRow( "<mspace depth=\"90%\"/>" );
    addRow( "<mspace depth=\"1.2\"/>" );

    addRow( "<mspace linebreak=\"auto\"/>" );
    addRow( "<mspace linebreak=\"newline\"/>" );
    addRow( "<mspace linebreak=\"indentingnewline\"/>" );
    addRow( "<mspace linebreak=\"nobreak\"/>" );
    addRow( "<mspace linebreak=\"goodbreak\"/>" );
    addRow( "<mspace linebreak=\"badbreak\"/>" );
}

void TestLoadAndSave::stringElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<ms>text</ms>",
            "<ms>text</ms>" );
    addRow( "<ms> more text </ms>",
            "<ms>more text</ms>" );

    // Glyph element contents
    //addRow( "<ms> foo </ms>");  // marker just to have a failing test.
#if 0  // FIXME: These tests make the test program crash.  Investigate and fix.
    addRow( "<ms>tex<mglyph fontfamily=\"serif\" alt=\"t\" index=\"116\"/></ms>",
            "<ms>tex<mglyph fontfamily=\"serif\" alt=\"t\" index=\"116\"/></ms>");
    addRow( "<ms> <mglyph fontfamily=\"serif\" alt=\"t\" index=\"116\"/> </ms>",
            "<ms> <mglyph fontfamily=\"serif\" alt=\"t\" index=\"116\"/> </ms>" );
    addRow( "<ms>te <mglyph fontfamily=\"serif\" alt=\"x\" index=\"120\"/> "
            "     <mglyph fontfamily=\"serif\" alt=\"t\" index=\"116\"/> </ms>",
            "<ms>te <mglyph fontfamily=\"serif\" alt=\"x\" index=\"120\"/> " 
            "     <mglyph fontfamily=\"serif\" alt=\"t\" index=\"116\"/> </ms>" );
#endif

    // Attributes
    addRow( "<ms mathvariant=\"bold\">text</ms>",
            "<ms mathvariant=\"bold\">text</ms>" );
    addRow( "<ms fontsize=\"18pt\">text</ms>",
            "<ms fontsize=\"18pt\">text</ms>"  );

    // Entities
    addRow( "<ms> &amp; </ms>",
            "<ms>&amp;</ms>" );
    addRow( "<ms> &amp;amp; </ms>",
            "<ms>&amp;amp;</ms>" );
}

void TestLoadAndSave::glyphElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Glyph element does not have content
    addRow( "<mglyph fontfamily=\"serif\" index=\"97\" alt=\"a\"/>" );
}

void TestLoadAndSave::mathVariant_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    /*
     * Test all possible values of mathvariant attributes
     */
    addRow( "<mi mathvariant=\"normal\">x</mi>" );
    addRow( "<mi mathvariant=\"bold\">x</mi>" );
    addRow( "<mi mathvariant=\"italic\">x</mi>" );
    addRow( "<mi mathvariant=\"bold-italic\">x</mi>" );
    addRow( "<mi mathvariant=\"double-struck\">x</mi>" );
    addRow( "<mi mathvariant=\"bold-fraktur\">x</mi>" );
    addRow( "<mi mathvariant=\"fraktur\">x</mi>" );
    addRow( "<mi mathvariant=\"sans-serif\">x</mi>" );
    addRow( "<mi mathvariant=\"bold-sans-serif\">x</mi>" );
    addRow( "<mi mathvariant=\"sans-serif-italic\">x</mi>" );
    addRow( "<mi mathvariant=\"sans-serif-bold-italic\">x</mi>" );
    addRow( "<mi mathvariant=\"monospace\">x</mi>" );

    /*
     * Unallowed mathvariant values should be removed
     */
    addRow( "<mi mathvariant=\"invalid\">x</mi>",
            "<mi>x</mi>", true );

    /*
     * It's better to store attribute names and values lowercase and avoid
     * having to check whether it's upper or lower case on a per-use case,
     * which is more error prone performance consuming.
     */
    addRow( "<mi mathvariant=\"Bold\">x</mi>",
            "<mi mathvariant=\"bold\">x</mi>" );
    addRow( "<mi mathvariant=\"BOLD\">x</mi>",
            "<mi mathvariant=\"bold\">x</mi>" );
    addRow( "<mi MATHVARIANT=\"bold\">x</mi>",
            "<mi mathvariant=\"bold\">x</mi>" );
    addRow( "<mi MathVariant=\"bold\">x</mi>",
            "<mi mathvariant=\"bold\">x</mi>" );
}

void TestLoadAndSave::mathSize_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    /*
     * Test all possible values of mathsize attributes
     */
    addRow( "<mi mathsize=\"small\">x</mi>" );
    addRow( "<mi mathsize=\"normal\">x</mi>" );
    addRow( "<mi mathsize=\"big\">x</mi>" );
    addRow( "<mi mathsize=\"10em\">x</mi>" );
    addRow( "<mi mathsize=\"10ex\">x</mi>" );
    addRow( "<mi mathsize=\"10px\">x</mi>" );
    addRow( "<mi mathsize=\"10in\">x</mi>" );
    addRow( "<mi mathsize=\"10cm\">x</mi>" );
    addRow( "<mi mathsize=\"10mm\">x</mi>" );
    addRow( "<mi mathsize=\"10pt\">x</mi>" );
    addRow( "<mi mathsize=\"10pc\">x</mi>" );
    addRow( "<mi mathsize=\"90%\">x</mi>" );
    addRow( "<mi mathsize=\"1.2\">x</mi>" );

    /*
     * Unallowed mathsize values should be removed
     */
    addRow( "<mi mathsize=\"invalid\">x</mi>",
            "<mi>x</mi>", true );

    /*
     * It's better to store attribute names and values lowercase and avoid
     * having to check whether it's upper or lower case on a per-use case,
     * which is more error prone performance consuming.
     */
    addRow( "<mi mathsize=\"Normal\">x</mi>",
            "<mi mathsize=\"normal\">x</mi>" );
    addRow( "<mi mathsize=\"NORMAL\">x</mi>",
            "<mi mathsize=\"normal\">x</mi>");
    addRow( "<mi MATHSIZE=\"normal\">x</mi>",
            "<mi mathsize=\"normal\">x</mi>" );
    addRow( "<mi MathSize=\"normal\">x</mi>",
            "<mi mathsize=\"normal\">x</mi>" );
}

void TestLoadAndSave::mathColor_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    /*
     * Test all possible values of mathcolor attributes
     */
    addRow( "<mi mathcolor=\"white\">x</mi>" );
    addRow( "<mi mathcolor=\"black\">x</mi>" );
    addRow( "<mi mathcolor=\"green\">x</mi>" );
    addRow( "<mi mathcolor=\"#abc\">x</mi>" );
    addRow( "<mi mathcolor=\"#abcdef\">x</mi>" );

    /*
     * Unallowed mathcolor values should be removed
     */
    addRow( "<mi mathcolor=\"invalid\">x</mi>",
            "<mi>x</mi>", true );
    addRow( "<mi mathcolor=\"#abcdefg\">x</mi>",
            "<mi>x</mi>", true );

    /*
     * It's better to store attribute names and values lowercase and avoid
     * having to check whether it's upper or lower case on a per-use case,
     * which is more error prone and performance consuming.
     */
    addRow( "<mi mathcolor=\"Black\">x</mi>",
            "<mi mathcolor=\"black\">x</mi>" );
    addRow( "<mi mathcolor=\"BLACK\">x</mi>",
            "<mi mathcolor=\"black\">x</mi>");
    addRow( "<mi MATHCOLOR=\"black\">x</mi>",
            "<mi mathcolor=\"black\">x</mi>" );
    addRow( "<mi MathColor=\"black\">x</mi>",
            "<mi mathcolor=\"black\">x</mi>" );
    addRow( "<mi MathColor=\"#ABC\">x</mi>",
            "<mi mathcolor=\"#abc\">x</mi>" );
}

void TestLoadAndSave::mathBackground_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    /*
     * Test all possible values of mathbackground attributes
     */
    addRow( "<mi mathbackground=\"white\">x</mi>" );
    addRow( "<mi mathbackground=\"black\">x</mi>" );
    addRow( "<mi mathbackground=\"green\">x</mi>" );
    addRow( "<mi mathbackground=\"#abc\">x</mi>" );
    addRow( "<mi mathbackground=\"#abcdef\">x</mi>" );

    /*
     * Unallowed mathbackground values should be removed
     */
    addRow( "<mi mathbackground=\"invalid\">x</mi>",
            "<mi>x</mi>", true );
    addRow( "<mi mathbackground=\"#abcdefg\">x</mi>",
            "<mi>x</mi>", true);

    /*
     * It's better to store attribute names and values lowercase and avoid
     * having to check whether it's upper or lower case on a per-use case,
     * which is more error prone performance consuming.
     */
    addRow( "<mi mathbackground=\"Black\">x</mi>",
            "<mi mathbackground=\"black\">x</mi>" );
    addRow( "<mi mathbackground=\"BLACK\">x</mi>",
            "<mi mathbackground=\"black\">x</mi>");
    addRow( "<mi MATHBACKGROUND=\"black\">x</mi>",
            "<mi mathbackground=\"black\">x</mi>" );
    addRow( "<mi MathBackground=\"black\">x</mi>",
            "<mi mathbackground=\"black\">x</mi>" );
    addRow( "<mi MathBackground=\"#ABC\">x</mi>",
            "<mi mathbackground=\"#abc\">x</mi>" );
}

void TestLoadAndSave::fontSize_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    /*
     * Test all possible values of fontsize attributes
     */
    addRow( "<mi fontsize=\"10em\">x</mi>" );
    addRow( "<mi fontsize=\"10ex\">x</mi>" );
    addRow( "<mi fontsize=\"10px\">x</mi>" );
    addRow( "<mi fontsize=\"10in\">x</mi>" );
    addRow( "<mi fontsize=\"10cm\">x</mi>" );
    addRow( "<mi fontsize=\"10mm\">x</mi>" );
    addRow( "<mi fontsize=\"10pt\">x</mi>" );
    addRow( "<mi fontsize=\"10pc\">x</mi>" );
    addRow( "<mi fontsize=\"90%\">x</mi>" );
    addRow( "<mi fontsize=\"1.2\">x</mi>" );

    /*
     * Unallowed fontsize values should be removed
     */
    addRow( "<mi fontsize=\"invalid\">x</mi>",
            "<mi>x</mi>", true );

    /*
     * It's better to store attribute names and values lowercase and avoid
     * having to check whether it's upper or lower case on a per-use case,
     * which is more error prone performance consuming.
     */
    addRow( "<mi fontsize=\"10Em\">x</mi>",
            "<mi fontsize=\"10em\">x</mi>" );
    addRow( "<mi fontsize=\"10EM\">x</mi>",
            "<mi fontsize=\"10em\">x</mi>");
    addRow( "<mi FONTSIZE=\"10em\">x</mi>",
            "<mi fontsize=\"10em\">x</mi>" );
    addRow( "<mi FontSize=\"10em\">x</mi>",
            "<mi fontsize=\"10em\">x</mi>" );
}

void TestLoadAndSave::fontWeight_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    /*
     * Test all possible values of fontweight attributes
     */
    addRow( "<mi fontweight=\"bold\">x</mi>" );
    addRow( "<mi fontweight=\"normal\">x</mi>" );

    /*
     * Unallowed fontweight values should be removed
     */
    addRow( "<mi fontweight=\"invalid\">x</mi>",
            "<mi>x</mi>", true );

    /*
     * It's better to store attribute names and values lowercase and avoid
     * having to check whether it's upper or lower case on a per-use case,
     * which is more error prone performance consuming.
     */
    addRow( "<mi fontweight=\"Bold\">x</mi>",
            "<mi fontweight=\"bold\">x</mi>" );
    addRow( "<mi fontweight=\"BOLD\">x</mi>",
            "<mi fontweight=\"bold\">x</mi>");
    addRow( "<mi FONTWEIGHT=\"bold\">x</mi>",
            "<mi fontweight=\"bold\">x</mi>" );
    addRow( "<mi FontWeight=\"bold\">x</mi>",
            "<mi fontweight=\"bold\">x</mi>" );
}

void TestLoadAndSave::fontStyle_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    /*
     * Test all possible values of fontstyle attributes
     */
    addRow( "<mi fontstyle=\"italic\">x</mi>" );
    addRow( "<mi fontstyle=\"normal\">x</mi>" );

    /*
     * Unallowed fontstyle values should be removed
     */
    addRow( "<mi fontstyle=\"invalid\">x</mi>",
            "<mi>x</mi>", true );

    /*
     * It's better to store attribute names and values lowercase and avoid
     * having to check whether it's upper or lower case on a per-use case,
     * which is more error prone performance consuming.
     */
    addRow( "<mi fontstyle=\"Italic\">x</mi>",
            "<mi fontstyle=\"italic\">x</mi>" );
    addRow( "<mi fontstyle=\"ITALIC\">x</mi>",
            "<mi fontstyle=\"italic\">x</mi>");
    addRow( "<mi FONTSTYLE=\"italic\">x</mi>",
            "<mi fontstyle=\"italic\">x</mi>" );
    addRow( "<mi FontStyle=\"italic\">x</mi>",
            "<mi fontstyle=\"italic\">x</mi>" );
}

void TestLoadAndSave::color_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    /*
     * Test all possible values of color attributes
     */
    addRow( "<mi color=\"white\">x</mi>" );
    addRow( "<mi color=\"black\">x</mi>" );
    addRow( "<mi color=\"green\">x</mi>" );
    addRow( "<mi color=\"#abc\">x</mi>" );
    addRow( "<mi color=\"#abcdef\">x</mi>" );

    /*
     * Unallowed color values should be removed
     */
    addRow( "<mi color=\"invalid\">x</mi>",
            "<mi>x</mi>", true );
    addRow( "<mi color=\"#abcdefg\">x</mi>",
            "<mi>x</mi>", true );

    /*
     * It's better to store attribute names and values lowercase and avoid
     * having to check whether it's upper or lower case on a per-use case,
     * which is more error prone performance consuming.
     */
    addRow( "<mi color=\"Black\">x</mi>",
            "<mi color=\"black\">x</mi>" );
    addRow( "<mi color=\"BLACK\">x</mi>",
            "<mi color=\"black\">x</mi>");
    addRow( "<mi COLOR=\"black\">x</mi>",
            "<mi color=\"black\">x</mi>" );
    addRow( "<mi Color=\"black\">x</mi>",
            "<mi color=\"black\">x</mi>" );
    addRow( "<mi Color=\"#ABC\">x</mi>",
            "<mi color=\"#abc\">x</mi>" );
}

void TestLoadAndSave::rowElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<mrow></mrow>",
            "<mrow/>" );
    addRow( "<mrow><mi>x</mi></mrow>", // Collapse mrow with only one child to the child only.
            "<mi>x</mi>" );
    addRow( "<mrow><mi>x</mi><mi>y</mi></mrow>",
            "<mrow>\n <mi>x</mi>\n <mi>y</mi>\n</mrow>" );

    // More complex content
    addRow( "<mrow><mrow></mrow></mrow>", // Collapse row with no children to nothing
            "<mrow/>");
    addRow( "<mrow><mrow><mi>x</mi></mrow></mrow>",
            //"<mrow>\n <mi>x</mi>\n</mrow>");
            "<mi>x</mi>");
    addRow( "<mrow><mrow><mi>x</mi><mn>2</mn></mrow></mrow>", 
            "<mrow>\n <mi>x</mi>\n <mn>2</mn>\n</mrow>");
    addRow( "<mrow><mrow><mi>x</mi><mn>2</mn></mrow><mi>y</mi></mrow>", 
            "<mrow>\n <mrow>\n  <mi>x</mi>\n  <mn>2</mn>\n </mrow>\n <mi>y</mi>\n</mrow>");

#if 0  // Enable this when entities work (see &InvisibleTimes; below).
    addRow( "<mrow>"
            " <mrow>"
            "  <mn> 2 </mn>"
            "  <mo> &InvisibleTimes; </mo>"
            "  <mi> x </mi>"
            " </mrow>"
            " <mo> + </mo>"
            " <mi> y </mi>"
            " <mo> - </mo>"
            " <mi> z </mi>"
            "</mrow>",

            "<mrow>\n"
            " <mrow>\n"
            "  <mn>2</mn>\n"
            "  <mo>&InvisibleTimes;</mo>\n"
            "  <mi>x</mi>\n"
            " </mrow>\n"
            " <mo>+</mo>\n"
            " <mi>y</mi>\n"
            " <mo>-</mo>\n"
            " <mi>z</mi>\n"
            "</mrow>" );
#endif
}

void TestLoadAndSave::fractionElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<mfrac><mi>x</mi><mi>y</mi></mfrac>",
            "<mfrac>\n"
            " <mi>x</mi>\n"
            " <mi>y</mi>\n"
            "</mfrac>");
}

void TestLoadAndSave::rootElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    addRow( "<mroot><mi>x</mi><mn>2</mn></mroot>",
            "<mroot>\n <mi>x</mi>\n <mn>2</mn>\n</mroot>");
}

void TestLoadAndSave::styleElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<mstyle></mstyle>",
            "<mstyle/>" );
    addRow( "<mstyle><mrow></mrow></mstyle>",
            "<mstyle/>");
    addRow( "<mstyle>\n <mi>x</mi>\n</mstyle>" );
    addRow( "<mstyle><mrow><mi>x</mi></mrow></mstyle>",
            "<mstyle>\n <mi>x</mi>\n</mstyle>");

    // Be sure attributes don't break anything
    addRow( "<mstyle mathvariant=\"bold\">\n <mi>x</mi>\n</mstyle>" );
    addRow( "<mstyle thinmathspace=\"0.5em\">\n <mi>x</mi>\n</mstyle>" );
}

void TestLoadAndSave::errorElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<merror></merror>",
            "<merror/>");
    addRow( "<merror><mrow></mrow></merror>",
            "<merror/>" );
    addRow( "<merror>\n <mi>x</mi>\n</merror>" );
    addRow( "<merror><mrow><mi>x</mi></mrow></merror>",
            "<merror>\n <mi>x</mi>\n</merror>" );

    // More complex content
    addRow( "<merror>\n"
            " <mtext>Unrecognized element: mfraction; arguments were:</mtext>\n"
            " <mrow>\n  <mn>1</mn>\n  <mo>+</mo>\n  <msqrt>\n   <mn>5</mn>\n  </msqrt>\n </mrow>\n"
            " <mtext>and</mtext>\n"
            " <mn>2</mn>\n"
            "</merror>" );
}

void TestLoadAndSave::paddedElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<mpadded></mpadded>",
            "<mpadded/>"  );
    addRow( "<mpadded><mrow></mrow></mpadded>",
            "<mpadded/>" );
    addRow( "<mpadded>\n <mi>x</mi>\n</mpadded>" );
    addRow( "<mpadded><mrow><mi>x</mi></mrow></mpadded>",
            "<mpadded>\n <mi>x</mi>\n</mpadded>" );

    // Be sure attributes don't break anything
    addRow( "<mpadded width=\"+0.8em\">\n <mi>x</mi>\n</mpadded>" );
    addRow( "<mpadded depth=\"1.2\">\n <mi>x</mi>\n</mpadded>" );
}

void TestLoadAndSave::phantomElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<mphantom></mphantom>",
            "<mphantom/>" );
    addRow( "<mphantom><mrow></mrow></mphantom>",
            "<mphantom/>" );
    addRow( "<mphantom>\n <mi>x</mi>\n</mphantom>" );
    addRow( "<mphantom><mrow><mi>x</mi></mrow></mphantom>",
            "<mphantom>\n <mi>x</mi>\n</mphantom>" );

    // Attributes
    addRow( "<mphantom width=\"+0.8em\">\n <mi>x</mi>\n</mphantom>" );
    addRow( "<mphantom depth=\"1.2\">\n <mi>x</mi>\n</mphantom>" );
}

void TestLoadAndSave::fencedElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // This is an inferred mrow element
    addRow( "<mfenced></mfenced>",
            "<mfenced/>" );
    addRow( "<mfenced>\n <mi>x</mi>\n</mfenced>" );
    addRow( "<mfenced>\n <mi>x</mi>\n <mn>2</mn>\n</mfenced>" );
}

void TestLoadAndSave::encloseElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<menclose></menclose>",
            "<menclose/>" );
    addRow( "<menclose><mrow></mrow></menclose>",
            "<menclose/>");
    addRow( "<menclose>\n <mi>x</mi>\n</menclose>" );
    addRow( "<menclose><mrow><mi>x</mi></mrow></menclose>",
            "<menclose>\n <mi>x</mi>\n</menclose>");

    // Attributes
    addRow( "<menclose notation=\"longdiv\">\n <mi>x</mi>\n</menclose>" );
    addRow( "<menclose notation=\"downdiagonalstrike\">\n <mi>x</mi>\n</menclose>" );
}

void TestLoadAndSave::subElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<msub>\n <mrow></mrow>\n <mrow></mrow>\n</msub>",
            "<msub>\n <mrow/>\n <mrow/>\n</msub>");
    addRow( "<msub>\n <mi>x</mi>\n <mi>y</mi>\n</msub>" );
    addRow( "<msub>\n <mi>x</mi>\n <mi>y</mi>\n</msub>" );
    addRow( "<msub>\n <mi>x</mi>\n <mi>y</mi>\n</msub>" );

    // More complex content
    addRow( "<msub>\n"
            " <mrow>\n"
            "  <mo>(</mo>\n"
            "  <mrow>\n"
            "   <mi>x</mi>\n"
            "   <mo>+</mo>\n"
            "   <mi>y</mi>\n"
            "  </mrow>\n"
            "  <mo>)</mo>\n"
            " </mrow>\n"
            " <mn>2</mn>\n"
            "</msub>" );

    // Attributes
    addRow( "<msub subscriptshift=\"1.5ex\">\n <mi>x</mi>\n <mi>y</mi>\n</msub>" );
    addRow( "<msub subscriptshift=\"1.5\">\n <mi>x</mi>\n <mi>y</mi>\n</msub>" );
}

void TestLoadAndSave::supElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<msup>\n <mrow></mrow>\n <mrow></mrow>\n</msup>",
            "<msup>\n <mrow/>\n <mrow/>\n</msup>");
    addRow( "<msup>\n <mi>x</mi>\n <mi>y</mi>\n</msup>" );
    addRow( "<msup>\n <mi>x</mi>\n <mi>y</mi>\n</msup>" );
    addRow( "<msup>\n <mi>x</mi>\n <mi>y</mi>\n</msup>" );

    // More complex content
    addRow( "<msup>\n"
            " <mrow>\n"
            "  <mo>(</mo>\n"
            "  <mrow>\n"
            "   <mi>x</mi>\n"
            "   <mo>+</mo>\n"
            "   <mi>y</mi>\n"
            "  </mrow>\n"
            "  <mo>)</mo>\n"
            " </mrow>\n"
            " <mn>2</mn>\n"
            "</msup>" );

    // Attributes
    addRow( "<msup subscriptshift=\"1.5ex\">\n <mi>x</mi>\n <mi>y</mi>\n</msup>" );
    addRow( "<msup subscriptshift=\"1.5\">\n <mi>x</mi>\n <mi>y</mi>\n</msup>" );
}

void TestLoadAndSave::subsupElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<msubsup><mrow></mrow><mrow></mrow><mrow></mrow></msubsup>",
            "<msubsup>\n <mrow/>\n <mrow/>\n <mrow/>\n</msubsup>");
    addRow( "<msubsup>\n <mi>x</mi>\n <mi>y</mi>\n <mi>z</mi>\n</msubsup>" );
    addRow( "<msubsup><mrow><mi>x</mi></mrow><mi>y</mi><mi>z</mi></msubsup>",
            "<msubsup>\n <mi>x</mi>\n <mi>y</mi>\n <mi>z</mi>\n</msubsup>");
    addRow( "<msubsup><mi>x</mi><mi>y</mi><mrow><mi>z</mi></mrow></msubsup>",
            "<msubsup>\n <mi>x</mi>\n <mi>y</mi>\n <mi>z</mi>\n</msubsup>");

    addRow( "<msubsup>\n"
            " <mrow>\n"
            "  <mo>(</mo>\n"
            "  <mrow>\n"
            "   <mi>x</mi>\n"
            "   <mo>+</mo>\n"
            "   <mi>y</mi>\n"
            "  </mrow>\n"
            "  <mo>)</mo>\n"
            " </mrow>\n"
            " <mi>i</mi>\n"
            " <mn>2</mn>\n"
            "</msubsup>" );

    // Attributes
    addRow( "<msubsup subscriptshift=\"1.5ex\">\n <mi>x</mi>\n <mi>y</mi>\n <mi>z</mi>\n</msubsup>" );
    addRow( "<msubsup superscriptshift=\"1.5ex\">\n <mi>x</mi>\n <mi>y</mi>\n <mi>z</mi>\n</msubsup>" );
}

void TestLoadAndSave::underElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<munder>\n <mrow></mrow>\n <mrow></mrow>\n</munder>",
            "<munder>\n <mrow/>\n <mrow/>\n</munder>");
    addRow( "<munder>\n <mi>x</mi>\n <mi>y</mi>\n</munder>" );
    addRow( "<munder>\n <mi>x</mi>\n <mi>y</mi>\n</munder>" );
    addRow( "<munder>\n <mi>x</mi>\n <mi>y</mi>\n</munder>" );

    // More complex content
    addRow( "<munder>\n"
            " <mrow>\n"
            "  <mo>(</mo>\n"
            "  <mrow>\n"
            "   <mi>x</mi>\n"
            "   <mo>+</mo>\n"
            "   <mi>y</mi>\n"
            "  </mrow>\n"
            "  <mo>)</mo>\n"
            " </mrow>\n"
            " <mn>2</mn>\n"
            "</munder>" );

    // Attributes
    addRow( "<munder accentunder=\"true\">\n <mi>x</mi>\n <mi>y</mi>\n</munder>" );
}

void TestLoadAndSave::overElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<mover>\n <mrow></mrow>\n <mrow></mrow>\n</mover>",
            "<mover>\n <mrow/>\n <mrow/>\n</mover>");
    addRow( "<mover>\n <mi>x</mi>\n <mi>y</mi>\n</mover>" );
    addRow( "<mover>\n <mi>x</mi>\n <mi>y</mi>\n</mover>" );
    addRow( "<mover>\n <mi>x</mi>\n <mi>y</mi>\n</mover>" );

    // More complex content
    addRow( "<mover>\n"
            " <mrow>\n"
            "  <mo>(</mo>\n"
            "  <mrow>\n"
            "   <mi>x</mi>\n"
            "   <mo>+</mo>\n"
            "   <mi>y</mi>\n"
            "  </mrow>\n"
            "  <mo>)</mo>\n"
            " </mrow>\n"
            " <mn>2</mn>\n"
            "</mover>" );

    // Attributes
    addRow( "<mover accent=\"true\">\n <mi>x</mi>\n <mi>y</mi>\n</mover>" );
}

void TestLoadAndSave::underoverElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<munderover><mrow></mrow><mrow></mrow><mrow></mrow></munderover>",
            "<munderover>\n <mrow/>\n <mrow/>\n <mrow/>\n</munderover>");
    addRow( "<munderover>\n <mi>x</mi>\n <mi>y</mi>\n <mi>z</mi>\n</munderover>" );
    addRow( "<munderover><mrow><mi>x</mi></mrow><mi>y</mi><mi>z</mi></munderover>",
            "<munderover>\n <mi>x</mi>\n <mi>y</mi>\n <mi>z</mi>\n</munderover>");
    addRow( "<munderover><mi>x</mi><mi>y</mi><mrow><mi>z</mi></mrow></munderover>",
            "<munderover>\n <mi>x</mi>\n <mi>y</mi>\n <mi>z</mi>\n</munderover>");

    addRow( "<munderover>\n"
            " <mrow>\n"
            "  <mo>(</mo>\n"
            "  <mrow>\n"
            "   <mi>x</mi>\n"
            "   <mo>+</mo>\n"
            "   <mi>y</mi>\n"
            "  </mrow>\n"
            "  <mo>)</mo>\n"
            " </mrow>\n"
            " <mi>i</mi>\n"
            " <mn>2</mn>\n"
            "</munderover>" );

    // Attributes
    addRow( "<munderover accent=\"true\">\n <mi>x</mi>\n <mi>y</mi>\n <mi>z</mi>\n</munderover>" );
    addRow( "<munderover accentunder=\"true\">\n <mi>x</mi>\n <mi>y</mi>\n <mi>z</mi>\n</munderover>" );
}

void TestLoadAndSave::multiscriptsElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<mmultiscripts><mi>x</mi><mi>i</mi><mi>j</mi></mmultiscripts>",
            "<mmultiscripts>\n <mi>x</mi>\n <mi>i</mi>\n <mi>j</mi>\n</mmultiscripts>" );
    addRow( "<mmultiscripts><mi>x</mi><mprescripts/><mi>i</mi><mi>j</mi></mmultiscripts>",
            "<mmultiscripts>\n <mi>x</mi>\n <mprescripts/>\n <mi>i</mi>\n <mi>j</mi>\n</mmultiscripts>" );
    addRow( "<mmultiscripts><mi>x</mi><mi>i</mi><none/></mmultiscripts>",
            "<mmultiscripts>\n <mi>x</mi>\n <mi>i</mi>\n <none/>\n</mmultiscripts>" );
    addRow( "<mmultiscripts><mi>x</mi><none/><none/></mmultiscripts>",
            "<mmultiscripts>\n <mi>x</mi>\n <none/>\n <none/>\n</mmultiscripts>" );
    addRow( "<mmultiscripts><mi>x</mi><mprescripts/><none/><none/></mmultiscripts>",
            "<mmultiscripts>\n <mi>x</mi>\n <mprescripts/>\n <none/>\n <none/>\n</mmultiscripts>" );
    addRow( "<mmultiscripts><mi>x</mi><none/><none/><mprescripts/><none/><none/></mmultiscripts>",
            "<mmultiscripts>\n <mi>x</mi>\n <none/>\n <none/>\n <mprescripts/>\n <none/>\n <none/>\n</mmultiscripts>" );
    addRow( "<mmultiscripts><mi>x</mi><mi>x</mi><none/><mprescripts/><mi>y</mi><none/></mmultiscripts>",
            "<mmultiscripts>\n <mi>x</mi>\n <mi>x</mi>\n <none/>\n <mprescripts/>\n <mi>y</mi>\n <none/>\n</mmultiscripts>" );

    // More complex content
    addRow( "<mmultiscripts>\n"
            " <mi>R</mi>\n"
            " <mi>i</mi>\n"
            " <none/>\n"
            " <none/>\n"
            " <mi>j</mi>\n"
            " <mi>k</mi>\n"
            " <none/>\n"
            " <mi>l</mi>\n"
            " <none/>\n"
            "</mmultiscripts>" );
}

void TestLoadAndSave::tableElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<mtable></mtable>",
            "<mtable/>" );
    addRow( "<mtable><mtr></mtr></mtable>",
            "<mtable>\n <mtr/>\n</mtable>" );
    addRow( "<mtable><mtr><mtd></mtd></mtr></mtable>",
            "<mtable>\n <mtr>\n  <mtd/>\n </mtr>\n</mtable>" );
    addRow( "<mtable><mtr><mtd><mrow></mrow></mtd></mtr></mtable>",
            "<mtable>\n <mtr>\n  <mtd/>\n </mtr>\n</mtable>" ); // mtd is an inferred mrow
    addRow( "<mtable><mtr><mtd><mrow><mi>x</mi></mrow></mtd></mtr></mtable>",
            "<mtable>\n <mtr>\n  <mtd>\n   <mi>x</mi>\n  </mtd>\n </mtr>\n</mtable>" );
//   addRow( "<mtable><mlabeledtr><mrow></mrow></mlabeledtr></mtable>",
//           "<mtable><mlabeledtr><mrow></mrow></mlabeledtr></mtable>" );
//   addRow( "<mtable><mlabeledtr><mrow></mrow><mtd></mtd></mlabeledtr></mtable>",
//           "<mtable><mlabeledtr><mrow></mrow><mtd></mtd></mlabeledtr></mtable>" );

    // More complex content (unity matrix)
    addRow( "<mtable>\n"
            " <mtr>\n"
            "  <mtd>\n   <mn>1</mn>\n  </mtd>\n"
            "  <mtd>\n   <mn>0</mn>\n  </mtd>\n"
            "  <mtd>\n   <mn>0</mn>\n  </mtd>\n"
            " </mtr>\n"
            " <mtr>\n"
            "  <mtd>\n   <mn>0</mn>\n  </mtd>\n"
            "  <mtd>\n   <mn>1</mn>\n  </mtd>\n"
            "  <mtd>\n   <mn>0</mn>\n  </mtd>\n"
            " </mtr>\n"
            " <mtr>\n"
            "  <mtd>\n   <mn>0</mn>\n  </mtd>\n"
            "  <mtd>\n   <mn>0</mn>\n  </mtd>\n"
            "  <mtd>\n   <mn>1</mn>\n  </mtd>\n"
            " </mtr>\n"
            "</mtable>" );

    // Attributes
    addRow( "<mtable align=\"top\">\n <mtr>\n  <mtd>\n   <mi>x</mi>\n  </mtd>\n </mtr>\n</mtable>" );
    addRow( "<mtable rowalign=\"center\">\n <mtr>\n  <mtd>\n   <mi>x</mi>\n  </mtd>\n </mtr>\n</mtable>" );

    // Content with alignment elements
/*    addRow( "<mtable groupalign=\"{decimalpoint left left decimalpoint left left decimalpoint}\">"
            " <mtr>"
            "  <mtd>"
            "   <mrow>"
            "    <mrow>"
            "     <mrow>"
            "      <maligngroup/>"
            "      <mn> 8.44 </mn>"
            "      <mo> &InvisibleTimes; </mo>"
            "      <maligngroup/>"
            "      <mi> x </mi>"
            "     </mrow>"
            "     <maligngroup/>"
            "     <mo> + </mo>"
            "     <mrow>"
            "      <maligngroup/>"
            "      <mn> 55 </mn>"
            "      <mo> &InvisibleTimes; </mo>"
            "      <maligngroup/>"
            "      <mi> y </mi>"
            "     </mrow>"
            "    </mrow>"
            "    <maligngroup/>"
            "    <mo> = </mo>"
            "    <maligngroup/>"
            "    <mn> 0 </mn>"
            "   </mrow>"
            "  </mtd>"
            " </mtr>"
            " <mtr>"
            "  <mtd>"
            "   <mrow>"
            "    <mrow>"
            "     <mrow>"
            "      <maligngroup/>"
            "      <mn> 3.1 </mn>"
            "      <mo> &InvisibleTimes; </mo>"
            "      <maligngroup/>"
            "      <mi> x </mi>"
            "     </mrow>"
            "     <maligngroup/>"
            "     <mo> - </mo>"
            "     <mrow>"
            "      <maligngroup/>"
            "      <mn> 0.7 </mn>"
            "      <mo> &InvisibleTimes; </mo>"
            "      <maligngroup/>"
            "      <mi> y </mi>"
            "     </mrow>"
            "    </mrow>"
            "    <maligngroup/>"
            "    <mo> = </mo>"
            "    <maligngroup/>"
            "    <mrow>"
            "     <mo> - </mo>"
            "     <mn> 1.1 </mn>"
            "    </mrow>"
            "   </mrow>"
            "  </mtd>"
            " </mtr>"
            "</mtable>" );*/
}

void TestLoadAndSave::trElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<mtr></mtr>",
            "<mtr/>" );
    addRow( "<mtr><mtd></mtd></mtr>",
            "<mtr>\n <mtd/>\n</mtr>" );
    addRow( "<mtr><mtd><mrow></mrow></mtd></mtr>",
            "<mtr>\n <mtd/>\n</mtr>" ); // <mtd> is an inferred <mrow>
    addRow( "<mtr><mtd><mi>x</mi></mtd></mtr>",
            "<mtr>\n <mtd>\n  <mi>x</mi>\n </mtd>\n</mtr>" );
    addRow( "<mtr><mtd><mrow><mi>x</mi></mrow></mtd></mtr>",
            "<mtr>\n <mtd>\n  <mi>x</mi>\n </mtd>\n</mtr>" );

    // More complex content
    addRow( "<mtr id=\"e-is-m-c-square\">\n"
            " <mtd>\n"
            "  <mrow>\n"
            "   <mi>E</mi>\n"
            "   <mo>=</mo>\n"
            "   <mrow>\n"
            "    <mi>m</mi>\n"
            //"    <mo>&it;</mo>\n"  FIXME: When entities work again, switch the next line for this one.
            "    <mo>*</mo>\n"
            "    <msup>\n"
            "     <mi>c</mi>\n"
            "     <mn>2</mn>\n"
            "    </msup>\n"
            "   </mrow>\n"
            "  </mrow>\n"
            " </mtd>\n"
            " <mtd>\n"
            "  <mtext>(2.1)</mtext>\n"
            " </mtd>\n"
            "</mtr>" );

    // Be sure attributes don't break anything
    addRow( "<mtr rowalign=\"top\"><mtd><mi>x</mi></mtd></mtr>",
            "<mtr rowalign=\"top\">\n <mtd>\n  <mi>x</mi>\n </mtd>\n</mtr>" );
    addRow( "<mtr groupalign=\"left\"><mtd><mi>x</mi></mtd></mtr>",
            "<mtr groupalign=\"left\">\n <mtd>\n  <mi>x</mi>\n </mtd>\n</mtr>" );
}

// labeledtr is not yet implemented
#if 0
void TestLoadAndSave::labeledtrElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // TODO
    //addRow( "<labeledtr/>" );
}
#endif

void TestLoadAndSave::tdElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<mtd></mtd>",
            "<mtd/>" );
    addRow( "<mtd/><mrow></mrow></mtd>",
            "<mtd/>" );
    addRow( "<mtd>\n <mi>x</mi>\n</mtd>" );
    addRow( "<mtd><mrow><mi>x</mi></mrow></mtd>", // mrow with one element is deleted
            "<mtd>\n <mi>x</mi>\n</mtd>");

    // Be sure attributes don't break anything
    addRow( "<mtd rowspan=\"3\">\n <mi>x</mi>\n</mtd>" );
    addRow( "<mtd groupalign=\"left\">\n <mi>x</mi>\n</mtd>" );
}

void TestLoadAndSave::actionElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<bool>("expectedFail");

    // Basic content
    addRow( "<maction actiontype=\"toggle\" selection=\"positive-integer\"><mrow></mrow><mrow></mrow></maction>",
            "<maction actiontype=\"toggle\" selection=\"positive-integer\"/>" ); // 
    addRow( "<maction actiontype=\"statusline\"><mrow></mrow><mrow></mrow></maction>",
            "<maction actiontype=\"statusline\"/>" );
    addRow( "<maction actiontype=\"tooltip\"><mrow></mrow><mrow></mrow></maction>",
            "<maction actiontype=\"tooltip\"/>" );
    // This is expected to fail since we are only comparing attributes in a certain order.
    // In reality it works, but we have to improve the test.
    addRow( "<maction actiontype=\"highlight\" my:color=\"red\" my:background=\"yellow\"><mrow></mrow></maction>",
            "<maction actiontype=\"highlight\" my:color=\"red\" my:background=\"yellow\"/>",
            true );
}

void TestLoadAndSave::identifierElement()
{
    test( new IdentifierElement );
}

void TestLoadAndSave::numberElement()
{
    test( new NumberElement );
}

void TestLoadAndSave::operatorElement()
{
    test( new OperatorElement );
}

void TestLoadAndSave::textElement()
{
    test( new TextElement );
}

void TestLoadAndSave::spaceElement()
{
    test( new SpaceElement );
}

void TestLoadAndSave::stringElement()
{
    test( new StringElement );
}

void TestLoadAndSave::glyphElement()
{
    test( new GlyphElement );
}

void TestLoadAndSave::mathVariant()
{
    identifierElement();
}

void TestLoadAndSave::mathSize()
{
    identifierElement();
}

void TestLoadAndSave::mathColor()
{
    identifierElement();
}

void TestLoadAndSave::mathBackground()
{
    identifierElement();
}

void TestLoadAndSave::fontSize()
{
    identifierElement();
}

void TestLoadAndSave::fontWeight()
{
    identifierElement();
}

void TestLoadAndSave::fontStyle()
{
    identifierElement();
}

void TestLoadAndSave::color()
{
    identifierElement();
}

void TestLoadAndSave::rowElement()
{
    test( new RowElement );
}

void TestLoadAndSave::fractionElement()
{
    test( new FractionElement );
}

void TestLoadAndSave::rootElement()
{
    test( new RootElement );
}

void TestLoadAndSave::styleElement()
{
    test( new StyleElement );
}

void TestLoadAndSave::errorElement()
{
    test( new ErrorElement );
}

void TestLoadAndSave::paddedElement()
{
    test( new PaddedElement );
}

void TestLoadAndSave::phantomElement()
{
    test( new PhantomElement );
}

void TestLoadAndSave::fencedElement()
{
    test( new FencedElement );
}

void TestLoadAndSave::encloseElement()
{
    test( new EncloseElement );
}

void TestLoadAndSave::subElement()
{
    test( new SubSupElement(0, SubScript) );
}

void TestLoadAndSave::supElement()
{
    test( new SubSupElement(0, SupScript) );
}

void TestLoadAndSave::subsupElement()
{
    test( new SubSupElement(0, SubSupScript) );
}

void TestLoadAndSave::underElement()
{
    test( new UnderOverElement(0, Under) );
}

void TestLoadAndSave::overElement()
{
    test( new UnderOverElement(0, Over) );
}

void TestLoadAndSave::underoverElement()
{
    test( new UnderOverElement(0, UnderOver) );
}

void TestLoadAndSave::multiscriptsElement()
{
    test( new MultiscriptElement );
}

void TestLoadAndSave::tableElement()
{
    test( new TableElement );
}

void TestLoadAndSave::trElement()
{
    test( new TableRowElement );
}

#if 0    // NYI
void TestLoadAndSave::labeledtrElement()
{
    test( new TableRowElement );
}
#endif
void TestLoadAndSave::tdElement()
{
    test( new TableDataElement );
}

void TestLoadAndSave::actionElement()
{
    test( new ActionElement );
}

QTEST_MAIN(TestLoadAndSave)
#include "TestLoadAndSave.moc"
