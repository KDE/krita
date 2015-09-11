/* This file is part of the KDE project
   Copyright 2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#include "TestLayout.h"
#include <QTest>

#include <QFont>
#include <QFontMetrics>

#include "AttributeManager.h"
#include "IdentifierElement.h"
#include "FencedElement.h"

#include <KoXmlReader.h>

static QRectF layout(BasicElement* element, const QString& input)
{
    KoXmlDocument doc;
    doc.setContent( input );
    element->readMathML(doc.documentElement());
    AttributeManager am;
    element->layout( &am );
    return element->boundingRect();
}

static void addRowInternal( const QString& input, const QString& text, const QFont& font )
{
    QFontMetrics fm( font );
    QTest::newRow( "Layout" ) << input << QRectF( fm.boundingRect( text ) );
}

static void addRow( const QString& input, const QString& text )
{
    QFont font;
    addRowInternal( input, text, font );
}

static void addRow( const QString& input, const QString& text, double size )
{
    QFont font;
    font.setPointSizeF( size );
    addRowInternal( input, text, font);
}

void TestLayout::identifierElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QRectF>("output");

    addRow( "<mi>x</mi>",
            "x");
    addRow( "<mi fontsize=\"12pt\">x</mi>",
            "x", 12);
}

void TestLayout::identifierElement()
{
    QFETCH(QString, input);
    QFETCH(QRectF, output);

    IdentifierElement* element = new IdentifierElement;
    QCOMPARE(layout(element, input), output);
    delete element;
}

void TestLayout::fencedElement_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QRectF>("output");

    addRow( "<mfenced></mfenced>",
            "()");
    addRow( "<mfenced><mi>x</mi></mfenced>",
            "(x)");
    addRow( "<mfenced><mi>x</mi><mi>y</mi></mfenced>",
            "(x,y)");
    addRow( "<mfenced open=\"[\"></mfenced>",
            "[)");
    addRow( "<mfenced open=\"[\" close=\"}\"></mfenced>",
            "[}");
    addRow( "<mfenced open=\"[\" close=\"}\"></mfenced>",
            "[}");
    addRow( "<mfenced separators=\";.\"><mi>x</mi><mi>y</mi><mi>z</mi></mfenced>",
            "(x;y.z)");
}

void TestLayout::fencedElement()
{
    QFETCH(QString, input);
    QFETCH(QRectF, output);

    FencedElement* element = new FencedElement;
    QCOMPARE(layout(element, input), output);
    delete element;
}

QTEST_MAIN(TestLayout)
