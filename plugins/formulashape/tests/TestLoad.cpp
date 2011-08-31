/* This file is part of the KDE project

   Copyright 2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>
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

#include "TestLoad.h"

#include <QtTest/QtTest>
#include <QtCore/QBuffer>

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
#include "UnderOverElement.h"
#include "SubSupElement.h"
#include "TableElement.h"
#include "TableRowElement.h"
#include "TableDataElement.h"
#include "ActionElement.h"

static void load(BasicElement* element, const QString& input)
{
    KoXmlDocument doc;
    doc.setContent( input );
    element->readMathML(doc.documentElement());
}

static int count( const QList<BasicElement*>& list )
{
    BasicElement* element;
    int counter = list.count();
    foreach ( element, list )
        counter += count( element->childElements() );

    return counter;
}

static QString dumpRecurse(const QList<BasicElement*>& list)
{
    BasicElement *element;
    QString       result = "[ ";

    if (list.count() > 0) {
        result.append(QString::number(list.count()));
        result.append(' ');

        foreach ( element, list )
            result.append(dumpRecurse(element->childElements()));
    }
    return result + " ]";
}

//static void dump( const QList<BasicElement*>& list )
//{
//    qDebug() << dumpRecurse(list);
//}

static void addRow( const QString& input, int output )
{
    static int counter = 0;
    QString name = "Load " + QString::number( ++counter );
    QTest::newRow( name.toLatin1() ) << input << output << output;
}

static void addRow( const QString& input, int output, int outputRecursive )
{
    static int counter = 0;
    QString name = "LoadRecursive " + QString::number( ++counter );
    QTest::newRow( name.toLatin1() ) << input << output << outputRecursive;
}

void test( BasicElement* element )
{
    QFETCH(QString, input);
    QFETCH(int, output);
    QFETCH(int, outputRecursive);

    load( element, input );
    //element->writeElementTree();
    int numElements = count( element->childElements() );
#if 0 // Set to 1 if you want to dump the xml tree if the test fails.
    if (numElements != outputRecursive) {
        qDebug() << input;
        //dump(element->childElements());
        element->writeElementTree();
    }
#endif
    QCOMPARE( element->childElements().count() , output );
    QCOMPARE( numElements, outputRecursive );

    delete element;
}


void TestLoad::identifierElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Empty content
    addRow( "<mi></mi>", 0 );

    // Basic content
    addRow( "<mi>x</mi>", 0 );
    addRow( "<mi>sin</mi>", 0 );

    // Glyph element contents
    addRow( "<mi>x<mglyph fontfamily=\"serif\" alt=\"a\" index=\"97\"/></mi>", 1);
    addRow( "<mi> <mglyph fontfamily=\"serif\" alt=\"sin\" index=\"97\"/> </mi>", 1);
    addRow( "<mi> <mglyph fontfamily=\"serif\" alt=\"x\" index=\"97\"/> "
            "     <mglyph fontfamily=\"serif\" alt=\"y\" index=\"97\"/> </mi>", 2);

    // Be sure attributes don't break anything
    addRow( "<mi mathvariant=\"bold\">x</mi>", 0 );
    addRow( "<mi fontsize=\"18pt\">x</mi>", 0 );

    // Be sure content with entity references don't break anything
    addRow( "<mi> &pi; </mi>", 0 );
    addRow( "<mi> &ImaginaryI; </mi>", 0 );
}

void TestLoad::numberElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<mn> 3 </mn>", 0 );
    addRow( "<mn> 1,000,000.11 </mn>", 0 );
    addRow( "<mn> 1.000.000,11 </mn>", 0 );

    // Glyph element contents
    addRow( "<mn>12<mglyph fontfamily=\"serif\" alt=\"8\" index=\"56\"/></mn>", 1);
    addRow( "<mn> <mglyph fontfamily=\"serif\" alt=\"8\" index=\"56\"/> </mn>", 1);
    addRow( "<mn> <mglyph fontfamily=\"serif\" alt=\"8\" index=\"56\"/> "
            "     <mglyph fontfamily=\"serif\" alt=\"7\" index=\"55\"/> </mn>", 2);

    // Be sure attributes don't break anything
    addRow( "<mn mathvariant=\"bold\">1</mn>", 0 );
    addRow( "<mn fontsize=\"18pt\">1</mn>", 0 );
}

void TestLoad::operatorElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<mo>+</mo>", 0 );
    addRow( "<mo> ++ </mo>", 0 );

    // Glyph element contents
    addRow( "<mo>+<mglyph fontfamily=\"serif\" alt=\"+\" index=\"43\"/></mo>", 1);
    addRow( "<mo> <mglyph fontfamily=\"serif\" alt=\"+\" index=\"43\"/> </mo>", 1);
    addRow( "<mo> <mglyph fontfamily=\"serif\" alt=\"+\" index=\"43\"/> "
            "     <mglyph fontfamily=\"serif\" alt=\"=\" index=\"61\"/> </mo>", 2);

    // Be sure attributes don't break anything
    addRow( "<mo mathvariant=\"bold\">+</mo>", 0 );
    addRow( "<mo fontsize=\"18pt\">+</mo>", 0 );

    // Be sure content with entity references don't break anything
    addRow( "<mo> &sum; </mo>", 0 );
    addRow( "<mo> &InvisibleTimes; </mo>", 0 );
}

void TestLoad::textElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<mtext>text</mtext>", 0 );
    addRow( "<mtext> more text </mtext>", 0 );

    // Glyph element contents
    addRow( "<mtext>tex<mglyph fontfamily=\"serif\" alt=\"t\" index=\"116\"/></mtext>", 1);
    addRow( "<mtext> <mglyph fontfamily=\"serif\" alt=\"t\" index=\"116\"/> </mtext>", 1);
    addRow( "<mtext>te <mglyph fontfamily=\"serif\" alt=\"x\" index=\"120\"/> "
            "     <mglyph fontfamily=\"serif\" alt=\"t\" index=\"116\"/> </mtext>", 2);

    // Be sure attributes don't break anything
    addRow( "<mtext mathvariant=\"bold\">text</mtext>", 0 );
    addRow( "<mtext fontsize=\"18pt\">text</mtext>", 0 );

    // Be sure content with entity references don't break anything
    addRow( "<mtext> &ThinSpace; </mtext>", 0 );
    addRow( "<mtext> &ThinSpace;&ThickSpace; </mtext>", 0 );
}

void TestLoad::spaceElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Space element does not have content
    addRow( "<mspace/>", 0 );
    addRow( "<mspace width=\0.5em\"/>", 0 );
    addRow( "<mspace linebreak=\"newline\"/>", 0 );
}

void TestLoad::stringElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<ms>text</ms>", 0 );
    addRow( "<ms> more text </ms>", 0 );

    // Glyph element contents
    addRow( "<ms>tex<mglyph fontfamily=\"serif\" alt=\"t\" index=\"116\"/></ms>", 1);
    addRow( "<ms> <mglyph fontfamily=\"serif\" alt=\"t\" index=\"116\"/> </ms>", 1);
    addRow( "<ms>te <mglyph fontfamily=\"serif\" alt=\"x\" index=\"120\"/> "
            "     <mglyph fontfamily=\"serif\" alt=\"t\" index=\"116\"/> </ms>", 2);

    // Be sure attributes don't break anything
    addRow( "<ms mathvariant=\"bold\">text</ms>", 0 );
    addRow( "<ms fontsize=\"18pt\">text</ms>", 0 );

    // Be sure content with entity references don't break anything
    addRow( "<ms> &amp; </ms>", 0 );
    addRow( "<ms> &amp;amp; </ms>", 0 );
}

void TestLoad::glyphElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Glyph element does not have content
    addRow( "<mglyph fontfamily=\"serif\" index=\"97\" alt=\"a\"/>", 0 );
}

void TestLoad::rowElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<mrow></mrow>", 0 );
    addRow( "<mrow><mi>x</mi></mrow>", 1 );
    addRow( "<mrow><mi>x</mi><mo>=</mo><mn>3</mn></mrow>", 3 );

    // More complex content
    addRow( "<mrow><mrow></mrow></mrow>", 0 );
    addRow( "<mrow><mrow><mi>x</mi></mrow></mrow>", 1 );
    addRow( "<mrow><mrow><mi>x</mi><mn>2</mn></mrow></mrow>", 1, 3 ); // Keep mrow with >1 children

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
            "</mrow>", 5, 8 );

    addRow( "<mrow>"
            " <mo> ( </mo>"
            " <mrow>"
            "  <mi> x </mi>"
            "  <mo> , </mo>"
            "  <mi> y </mi>"
            " </mrow>"
            " <mo> ) </mo>"
            "</mrow>", 3, 6 );


}

void TestLoad::fracElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<mfrac><mi>x</mi><mi>y</mi></mfrac>", 2, 4 ); // +1 <mrow> for both sides

    // More complex content
    addRow( "<mfrac linethickness=\"2\">"
            " <mfrac>"
            "  <mi> a </mi>"
            "  <mi> b </mi>"
            " </mfrac>"
            " <mfrac>"
            "  <mi> c </mi>"
            "  <mi> d </mi>"
            " </mfrac>"
            "</mfrac>", 2, 6 + 6 ); // +2 mrow per mfrac

    addRow( "<mfrac>"
            " <mn> 1 </mn>"
            " <mrow>"
            "  <msup>"
            "   <mi> x </mi>"
            "   <mn> 3 </mn>"
            "  </msup>"
            "  <mo> + </mo>"
            "  <mfrac>"
            "   <mi> x </mi>"
            "   <mn> 3 </mn>"
            "  </mfrac>"
            " </mrow>"
            "</mfrac>", 2, 9 + 5 ); // +2 mrow per mfrac, and msup -1 for the one in the test
}

void TestLoad::rootElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
/*    addRow( "<msqrt></msqrt>", 1 );
    addRow( "<msqrt><mrow></mrow></msqrt>", 1 );
    addRow( "<msqrt><mi>x</mi></msqrt>", 1, 2 );
    addRow( "<msqrt><mrow><mi>x</mi></mrow></msqrt>", 2, 2 );*/
    addRow( "<mroot><mi>x</mi><mn>2</mn></mroot>", 2, 4 );

    // More complex content
/*    addRow( "<msqrt>"
            " <mi> x </mi>"
            " <mroot>"
            "  <mrow>"
            "   <mn> 2 </mn>"
            "   <mo> &InvisibleTimes </mn>"
            "   <mi> y </mi>"
            "  </mrow>"
            "  <mfrac>"
            "   <mn> 1 </mn>"
            "   <mn> 2 </mn>"
            "  </frac>"
            " </mroot>"
            "</msqrt", 1, 13 );*/
}

void TestLoad::styleElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<mstyle></mstyle>", 0 );
    addRow( "<mstyle><mrow></mrow></mstyle>", 0 );
    addRow( "<mstyle><mi>x</mi></mstyle>", 1 );
    addRow( "<mstyle><mrow><mi>x</mi></mrow></mstyle>", 1 );

    // Be sure attributes don't break anything
    addRow( "<mstyle mathvariant=\"bold\"><mi>x</mi></mstyle>", 1 );
    addRow( "<mstyle thinmathspace=\"0.5em\"><mi>x</mi></mstyle>", 1 );
}

void TestLoad::errorElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<merror></merror>", 0 );
    addRow( "<merror><mrow></mrow></merror>", 0 );
    addRow( "<merror><mi>x</mi></merror>", 1 );
    addRow( "<merror><mrow><mi>x</mi></mrow></merror>", 1 );

    // More complex content
    addRow( "<merror>"
            " <mtext> Unrecognized element: mfraction;"
            "         arguments were:  "
            " </mtext>"
            " <mrow> <mn> 1 </mn> <mo> + </mo> <msqrt> <mn> 5 </mn> </msqrt> </mrow>"
            " <mtext>  and  </mtext>"
            " <mn> 2 </mn>"
            "</merror>", 4, 8 );
}

void TestLoad::paddedElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<mpadded></mpadded>", 0);
    addRow( "<mpadded><mrow></mrow></mpadded>", 0 );
    addRow( "<mpadded><mi>x</mi></mpadded>", 1 );
    addRow( "<mpadded><mrow><mi>x</mi></mrow></mpadded>", 1 );

    // Be sure attributes don't break anything
    addRow( "<mpadded width=\"+0.8em\"><mi>x</mi></mpadded>", 1 );
    addRow( "<mpadded depth=\"1.2\"><mi>x</mi></mpadded>", 1 );
}

void TestLoad::phantomElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<mphantom></mphantom>", 0 );
    addRow( "<mphantom><mrow></mrow></mphantom>", 0 );
    addRow( "<mphantom><mi>x</mi></mphantom>", 1 );
    addRow( "<mphantom><mrow><mi>x</mi></mrow></mphantom>", 1 );

    // Be sure attributes don't break anything
    addRow( "<mphantom width=\"+0.8em\"><mi>x</mi></mphantom>", 1 );
    addRow( "<mphantom depth=\"1.2\"><mi>x</mi></mphantom>", 1 );
}

void TestLoad::fencedElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // This is an inferred mrow element
    addRow( "<mfenced></mfenced>", 0 );
    addRow( "<mfenced><mi>x</mi></mfenced>", 1 );
    addRow( "<mfenced><mi>x</mi><mn>2</mn></mfenced>", 2 ); // Inferred mrow
}

void TestLoad::encloseElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<menclose></menclose>", 0 );
    addRow( "<menclose><mrow></mrow></menclose>", 0 );
    addRow( "<menclose><mi>x</mi></menclose>", 1 );
    addRow( "<menclose><mrow><mi>x</mi></mrow></menclose>", 1 );

    // Be sure attributes don't break anything
    addRow( "<menclose notation=\"longdiv\"><mi>x</mi></menclose>", 1 );
    addRow( "<menclose notation=\"downdiagonalstrike\"><mi>x</mi></menclose>", 1 );
}

void TestLoad::subElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<msub><mrow></mrow><mrow></mrow></msub>", 2 );
    addRow( "<msub><mi>x</mi><mi>y</mi></msub>", 2, 4 );
    addRow( "<msub><mrow><mi>x</mi></mrow><mi>y</mi></msub>", 2, 4 );
    addRow( "<msub><mi>x</mi><mrow><mi>y</mi></mrow></msub>", 2, 4 );
    addRow( "<msub><mrow><mi>x</mi></mrow><mrow><mi>y</mi></mrow></msub>", 2, 4 );

    // More complex content
    addRow( "<msub>"
            " <mrow>"
            "  <mo> ( </mo>"
            "  <mrow>"
            "   <mi> x </mi>"
            "   <mo> + </mo>"
            "   <mi> y </mi>"
            "  </mrow>"
            "  <mo> ) </mo>"
            " </mrow>"
            " <mn> 2 </mn>" // An mrow is added here
            "</msub>", 2, 9 );

    // Be sure attributes don't break anything
    addRow( "<msub subscriptshift=\"1.5ex\"><mi>x</mi><mi>y</mi></msub>", 2, 4 );
    addRow( "<msub subscriptshift=\"1.5\"><mi>x</mi><mi>y</mi></msub>", 2, 4 );
}

void TestLoad::supElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<msup><mrow></mrow><mrow></mrow></msup>", 2 );
    addRow( "<msup><mi>x</mi><mi>y</mi></msup>", 2, 4 );
    addRow( "<msup><mrow><mi>x</mi></mrow><mi>y</mi></msup>", 2, 4 );
    addRow( "<msup><mi>x</mi><mrow><mi>y</mi></mrow></msup>", 2, 4 );
    addRow( "<msup><mrow><mi>x</mi></mrow><mrow><mi>y</mi></mrow></msup>", 2, 4 );

    // More complex content
    addRow( "<msup>"
            " <mrow>"
            "  <mo> ( </mo>"
            "  <mrow>"
            "   <mi> x </mi>"
            "   <mo> + </mo>"
            "   <mi> y </mi>"
            "  </mrow>"
            "  <mo> ) </mo>"
            " </mrow>"
            " <mn> 2 </mn>"  // An mrow is added here
            "</msup>", 2, 9 );

    // Be sure attributes don't break anything
    addRow( "<msup superscriptshift=\"1.5ex\"><mi>x</mi><mi>y</mi></msup>", 2, 4 );
    addRow( "<msup superscriptshift=\"1.5\"><mi>x</mi><mi>y</mi></msup>", 2, 4 );
}

void TestLoad::subsupElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<msubsup><mrow></mrow><mrow></mrow><mrow></mrow></msubsup>", 3 );
    addRow( "<msubsup><mi>x</mi><mi>y</mi><mi>z</mi></msubsup>", 3, 6 );
    addRow( "<msubsup><mrow><mi>x</mi></mrow><mi>y</mi><mi>z</mi></msubsup>", 3, 6 );
    addRow( "<msubsup><mi>x</mi><mrow><mi>y</mi></mrow><mi>z</mi></msubsup>", 3, 6 );
    addRow( "<msubsup><mrow><mi>x</mi></mrow><mrow><mi>y</mi></mrow><mi>z</mi></msubsup>", 3, 6 );
    addRow( "<msubsup><mrow><mi>x</mi></mrow><mi>y</mi><mrow><mi>z</mi></mrow></msubsup>", 3, 6 );
    addRow( "<msubsup><mi>x</mi><mrow><mi>y</mi></mrow><mrow><mi>z</mi></mrow></msubsup>", 3, 6 );
    addRow( "<msubsup><mrow><mi>x</mi></mrow><mrow><mi>y</mi></mrow><mrow><mi>z</mi></mrow></msubsup>", 3, 6 );

    // Be sure attributes don't break anything
    addRow( "<msubsup subscriptshift=\"1.5ex\"><mi>x</mi><mi>y</mi><mi>z</mi></msubsup>", 3, 6 );
    addRow( "<msubsup superscriptshift=\"1.5ex\"><mi>x</mi><mi>y</mi><mi>z</mi></msubsup>", 3, 6 );
}

void TestLoad::underElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<munder><mrow></mrow><mrow></mrow></munder>", 2 );
    addRow( "<munder><mi>x</mi><mi>y</mi></munder>", 2, 4 );
    addRow( "<munder><mrow><mi>x</mi></mrow><mi>y</mi></munder>", 2, 4 );
    addRow( "<munder><mi>x</mi><mrow><mi>y</mi></mrow></munder>", 2, 4 );
    addRow( "<munder><mrow><mi>x</mi></mrow><mrow><mi>y</mi></mrow></munder>", 2, 4 );

    // More complex content
    addRow( "<munder accentunder=\"true\">"
            " <mrow>"
            "  <mi> x </mi>"
            "  <mo> + </mo>"
            "  <mi> y </mi>"
            "  <mo> + </mo>"
            "  <mi> z </mi>"
            " </mrow>"
            " <mo> &UnderBrace; </mo>"
            "</munder>", 2, 8);

    addRow( "<munder accentunder=\"false\">"
            " <mrow>"
            "  <mi> x </mi>"
            "  <mo> + </mo>"
            "  <mi> y </mi>"
            "  <mo> + </mo>"
            "  <mi> z </mi>"
            " </mrow>"
            " <mo> &UnderBrace; </mo>"
            "</munder>", 2, 8 );

    // Be sure attributes don't break anything
    addRow( "<munder accentunder=\"true\"><mi>x</mi><mi>y</mi></munder>", 2, 4 );
    addRow( "<munder accentunder=\"false\"><mi>x</mi><mi>y</mi></munder>", 2, 4 );
}

void TestLoad::overElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<mover><mrow></mrow><mrow></mrow></mover>", 2 );
    addRow( "<mover><mi>x</mi><mi>y</mi></mover>", 2, 4 );
    addRow( "<mover><mrow><mi>x</mi></mrow><mi>y</mi></mover>", 2, 4 );
    addRow( "<mover><mi>x</mi><mrow><mi>y</mi></mrow></mover>", 2, 4 );
    addRow( "<mover><mrow><mi>x</mi></mrow><mrow><mi>y</mi></mrow></mover>", 2, 4 );

    // More complex content
    addRow( "<mover accent=\"true\">"
            " <mrow>"
            "  <mi> x </mi>"
            "  <mo> + </mo>"
            "  <mi> y </mi>"
            "  <mo> + </mo>"
            "  <mi> z </mi>"
            " </mrow>"
            " <mo> &OverBrace; </mo>"
            "</mover>", 2, 8);

    addRow( "<mover accent=\"false\">"
            " <mrow>"
            "  <mi> x </mi>"
            "  <mo> + </mo>"
            "  <mi> y </mi>"
            "  <mo> + </mo>"
            "  <mi> z </mi>"
            " </mrow>"
            " <mo> &OverBrace; </mo>"
            "</mover>", 2, 8 );

    // Be sure attributes don't break anything
    addRow( "<mover accent=\"true\"><mi>x</mi><mi>y</mi></mover>", 2, 4 );
    addRow( "<mover accent=\"false\"><mi>x</mi><mi>y</mi></mover>", 2, 4 );
}

void TestLoad::underOverElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<munderover><mrow></mrow><mrow></mrow><mrow></mrow></munderover>", 3 );
    addRow( "<munderover><mi>x</mi><mi>y</mi><mi>z</mi></munderover>", 3, 6 );
    addRow( "<munderover><mrow><mi>x</mi></mrow><mi>y</mi><mi>z</mi></munderover>", 3, 6 );
    addRow( "<munderover><mi>x</mi><mrow><mi>y</mi></mrow><mi>z</mi></munderover>", 3, 6 );
    addRow( "<munderover><mrow><mi>x</mi></mrow><mrow><mi>y</mi></mrow><mi>z</mi></munderover>", 3, 6 );
    addRow( "<munderover><mrow><mi>x</mi></mrow><mi>y</mi><mrow><mi>z</mi></mrow></munderover>", 3, 6 );
    addRow( "<munderover><mi>x</mi><mrow><mi>y</mi></mrow><mrow><mi>z</mi></mrow></munderover>", 3, 6 );
    addRow( "<munderover><mrow><mi>x</mi></mrow><mrow><mi>y</mi></mrow><mrow><mi>z</mi></mrow></munderover>", 3, 6 );

    // Be sure attributes don't break anything
    addRow( "<munderover accent=\"true\"><mi>x</mi><mi>y</mi><mi>z</mi></munderover>", 3, 6 );
    addRow( "<munderover accentunder=\"false\"><mi>x</mi><mi>y</mi><mi>z</mi></munderover>", 3, 6 );
}

void TestLoad::multiscriptsElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<mmultiscripts><mi>x</mi><mi>i</mi><mi>j</mi></mmultiscripts>", 3, 3 );
    addRow( "<mmultiscripts><mi>x</mi><mprescripts/><mi>i</mi><mi>j</mi></mmultiscripts>", 3, 3 );
    addRow( "<mmultiscripts><mi>x</mi><mi>i</mi><none/></mmultiscripts>", 2 );
    addRow( "<mmultiscripts><mi>x</mi><none/><none/></mmultiscripts>", 1 );
    addRow( "<mmultiscripts><mi>x</mi><none/><none/></mmultiscripts>", 1 ); // ?? Same as above
    addRow( "<mmultiscripts><mi>x</mi><mprescripts/><none/><none/></mmultiscripts>", 1 );
    addRow( "<mmultiscripts><mi>x</mi><none/><none/><mprescripts/><none/><none/></mmultiscripts>", 1 );
    addRow( "<mmultiscripts><mi>x</mi><mi>x</mi><none/><mprescripts/><mi>y</mi><none/></mmultiscripts>", 3 );

    // More complex content
    addRow( "<mmultiscripts>"
            " <mi> R </mi>"
            " <mi> i </mi>"
            " <none/>"
            " <none/>"
            " <mi> j </mi>"
            " <mi> k </mi>"
            " <none/>"
            " <mi> l </mi>"
            " <none/>"
            "</mmultiscripts>", 5 );
}

void TestLoad::tableElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<mtable></mtable>", 0 );
    addRow( "<mtable><mtr></mtr></mtable>", 1 );
    addRow( "<mtable><mtr><mtd></mtd></mtr></mtable>", 1, 2 );
    addRow( "<mtable><mtr><mtd><mrow></mrow></mtd></mtr></mtable>", 1, 2 ); // mtd is an inferred mrow
    addRow( "<mtable><mtr><mtd><mrow><mi>x</mi></mrow></mtd></mtr></mtable>", 1, 3 );
//   addRow( "<mtable><mlabeledtr><mrow></mrow></mlabeledtr></mtable>", 1, 2 );
//   addRow( "<mtable><mlabeledtr><mrow></mrow><mtd></mtd></mlabeledtr></mtable>", 1, 4 );

    // More complex content
    addRow( "<mtable>"
            " <mtr>"
            "  <mtd> <mn>1</mn> </mtd>"
            "  <mtd> <mn>0</mn> </mtd>"
            "  <mtd> <mn>0</mn> </mtd>"
            " </mtr>"
            " <mtr>"
            "  <mtd> <mn>0</mn> </mtd>"
            "  <mtd> <mn>1</mn> </mtd>"
            "  <mtd> <mn>0</mn> </mtd>"
            " </mtr>"
            " <mtr>"
            "  <mtd> <mn>0</mn> </mtd>"
            "  <mtd> <mn>0</mn> </mtd>"
            "  <mtd> <mn>1</mn> </mtd>"
            " </mtr>"
            "</mtable>", 3, 21 );

    // Be sure attributes don't break anything
    addRow( "<mtable align=\"top\"><mtr><mtd><mi>x</mi></mtd></mtr></mtable>", 1, 3 );
    addRow( "<mtable rowalign=\"center\"><mtr><mtd><mi>x</mi></mtd></mtr></mtable>", 1, 3 );

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
            "</mtable>", 2, 32 );*/
}

void TestLoad::trElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<mtr></mtr>", 0 );
    addRow( "<mtr><mtd></mtd></mtr>", 1, 1 );
    addRow( "<mtr><mtd><mrow></mrow></mtd></mtr>", 1, 1 ); // <mtd> is an inferred <mrow>
    addRow( "<mtr><mtd><mi>x</mi></mtd></mtr>", 1, 2 );
    addRow( "<mtr><mtd><mrow><mi>x</mi></mrow></mtd></mtr>", 1, 2 );

    // More complex content
    addRow( "<mtr id='e-is-m-c-square'>"
            " <mtd>"
            "  <mrow>"
            "   <mi>E</mi>"
            "   <mo>=</mo>"
            "   <mrow>"
            "    <mi>m</mi>"
            "    <mo>&it;</mo>"
            "    <msup>"
            "     <mi>c</mi>"
            "     <mn>2</mn>"
            "    </msup>"
            "   </mrow>"
            "  </mrow>"
            " </mtd>"
            " <mtd>"
            "  <mtext> (2.1) </mtext>"
            " </mtd>"
            "</mtr>", 2, 14 );

    // Be sure attributes don't break anything
    addRow( "<mtr rowalign=\"top\"><mtd><mi>x</mi></mtd></mtr>", 1, 2 );
    addRow( "<mtr groupalign=\"left\"><mtd><mi>x</mi></mtd></mtr>", 1, 2 );
}
/*
void TestLoad::labeledtrElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<mlabeledtr><mrow></mrow></mlabeledtr>", 1 );
    addRow( "<mlabeledtr><mi>x</mi></mlabeledtr>", 1, 2 );
    addRow( "<mlabeledtr><mrow></mrow><mtd></mtd></mlabeledtr>", 2, 3 );
    addRow( "<mlabeledtr><mi>x</mi><mtd><mtd></mlabeledtr>", 2, 4 );
    addRow( "<mlabeledtr><mrow><mi>x</mi></mrow><mtd><mrow></mrow></mtd></mlabeledtr>", 2, 4 );
    addRow( "<mlabeledtr><mrow><mi>x</mi></mrow><mtd><mi>x</mi></mtd></mlabeledtr>", 2, 5 );
    addRow( "<mlabeledtr><mrow><mi>x</mi></mrow><mtd><mrow><mi>x</mi></mrow></mtd></mlabeledtr>", 2, 5 );

    // More complex ccontent
    addRow( "<mlabeledtr id='e-is-m-c-square'>"
            " <mtd>"
            "  <mtext> (2.1) </mtext>"
            " </mtd>"
            " <mtd>"
            "  <mrow>"
            "   <mi>E</mi>"
            "   <mo>=</mo>"
            "   <mrow>"
            "    <mi>m</mi>"
            "    <mo>&it;</mo>"
            "    <msup>"
            "     <mi>c</mi>"
            "     <mn>2</mn>"
            "    </msup>"
            "   </mrow>"
            "  </mrow>"
            " </mtd>"
            "</mlabeledtr>", 2, 15 );

    // Be sure attributes don't break anything
    addRow( "<mlabeledtr rowalign=\"top\"><mi>x</mi></mlabeledtr>", 1, 2 );
    addRow( "<mlabeledtr groupalign=\"left\"><mi>x</mi></mlabeledtr>", 1, 2 );
}
*/
void TestLoad::tdElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<mtd></mtd>", 0 );
    addRow( "<mtd><mrow></mrow></mtd>", 0, 0 ); // Empty mrow is deleted
    addRow( "<mtd><mi>x</mi></mtd>", 1, 1 );
    addRow( "<mtd><mrow><mi>x</mi></mrow></mtd>", 1, 1 ); // mrow with one element is deleted

    // Be sure attributes don't break anything
    addRow( "<mtd rowspan=\"3\"><mi>x</mi></mtd>", 1, 1 );
    addRow( "<mtd groupalign=\"left\"><mi>x</mi></mtd>", 1, 1 );
}

void TestLoad::actionElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");
    QTest::addColumn<int>("outputRecursive");

    // Basic content
    addRow( "<maction actiontype=\"toggle\" selection=\"positive-integer\"><mrow></mrow><mrow></mrow></maction>", 0 ); // 
    addRow( "<maction actiontype=\"statusline\"><mrow></mrow><mrow></mrow></maction>", 0 );
    addRow( "<maction actiontype=\"tooltip\"><mrow></mrow><mrow></mrow></maction>", 0 );
    addRow( "<maction actiontype=\"highlight\" my:color=\"red\" my:background=\"yellow\"><mrow></mrow></maction>", 0 );
}

void TestLoad::identifierElement()
{
    test( new IdentifierElement );
}

void TestLoad::numberElement()
{
    test( new NumberElement );
}

void TestLoad::operatorElement()
{
    test( new OperatorElement );
}

void TestLoad::textElement()
{
    test( new TextElement );
}

void TestLoad::spaceElement()
{
    test( new SpaceElement );
}

void TestLoad::stringElement()
{
    test( new StringElement );
}

void TestLoad::glyphElement()
{
    test( new GlyphElement );
}

void TestLoad::rowElement()
{
    test( new RowElement );
}

void TestLoad::fracElement()
{
    test( new FractionElement );
}

void TestLoad::rootElement()
{
    test( new RootElement );
}

void TestLoad::styleElement()
{
    test( new StyleElement );
}

void TestLoad::errorElement()
{
    test( new ErrorElement );
}

void TestLoad::paddedElement()
{
    test( new PaddedElement );
}

void TestLoad::phantomElement()
{
    test( new PhantomElement );
}

void TestLoad::fencedElement()
{
    test( new FencedElement );
}

void TestLoad::encloseElement()
{
    test( new EncloseElement );
}

void TestLoad::subElement()
{
    test( new SubSupElement(0, SubScript) );
}

void TestLoad::supElement()
{
    test( new SubSupElement(0, SupScript) );
}

void TestLoad::subsupElement()
{
    test( new SubSupElement(0, SubSupScript) );
}

void TestLoad::underElement()
{
    test( new UnderOverElement(0, Under) );
}

void TestLoad::overElement()
{
    test( new UnderOverElement(0, Over) );
}

void TestLoad::underOverElement()
{
    test( new UnderOverElement(0, UnderOver) );
}

void TestLoad::multiscriptsElement()
{
    test( new MultiscriptElement );
}

void TestLoad::tableElement()
{
    test( new TableElement );
}

void TestLoad::trElement()
{
    test( new TableRowElement );
}
/*
void TestLoad::labeledtrElement()
{
    test( new TableRowElement );
}
*/
void TestLoad::tdElement()
{
    test( new TableDataElement );
}

void TestLoad::actionElement()
{
    test( new ActionElement );
}

QTEST_MAIN(TestLoad)
#include "TestLoad.moc"
