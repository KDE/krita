/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <iostream>
#include <QString>
#include <QFontMetrics>

#include <klocale.h>
#include <kmessagebox.h>

//#include <KoUnit.h>

#include "kformulamathmlread.h"
#include "symboltable.h"

KFORMULA_NAMESPACE_BEGIN
using namespace std;

class MathML2KFormulaPrivate
{
    friend class MathML2KFormula;

public:
    MathML2KFormulaPrivate( MathML2KFormula* mml_filter,
                            const ContextStyle& contextStyle,
                            const QDomDocument& formuladoc );
    ~MathML2KFormulaPrivate();

    void math( QDomElement element );

    // Token Elements
    void mi( QDomElement element, QDomNode docnode );
    void mn( QDomElement element, QDomNode docnode );
    void mo( QDomElement element, QDomNode docnode );
    void mtext( QDomElement element, QDomNode docnode );
    void mspace( QDomElement element, QDomNode docnode );
    void ms( QDomElement element, QDomNode docnode );
    // mglyph not supported

    // General Layout Schemata
    void mrow( QDomElement element, QDomNode docnode );
    void mfrac( QDomElement element, QDomNode docnode );
    void msqrt( QDomElement element, QDomNode docnode );
    void mroot( QDomElement element, QDomNode docnode );
    void mstyle( QDomElement element, QDomNode docnode );
    // merror not supported
    // mpadded not supported
    // mphantom not supported
    void mfenced( QDomElement element, QDomNode docnode );
    // menclose not supported

    // Script and Limit Schemata
    void msub_msup( QDomElement element, QDomNode docnode );
    void msubsup( QDomElement element, QDomNode docnode );
    void munder( QDomElement element, QDomNode docnode, bool oasisFormat );
    void mover( QDomElement element, QDomNode docnode, bool oasisFormat );
    void munderover( QDomElement element, QDomNode docnode, bool oasisFormat );
    // mmultiscripts not supported

    // Tables and Matrices
    void mtable( QDomElement element, QDomNode docnode );
    // not much supported

    // Enlivening Expressions
    // maction not supported

protected:
    void createTextElements( QString text, QDomNode docnode );
    double convertToPoint( QString value, bool* ok );
    bool isEmbellishedOperator( QDomNode node, QDomElement* mo, bool oasisFormat );
    bool isSpaceLike( QDomNode node, bool oasisFormat );

    enum MathVariant {
        normal,
        bold,
        italic,
        bold_italic,
        double_struck,
        bold_fraktur,
        script,
        bold_script,
        fraktur,
        sans_serif,
        bold_sans_serif,
        sans_serif_italic,
        sans_serif_bold_italic,
        monospace
    };

    struct MathStyle {
        MathStyle()
            : scriptsizemultiplier( 0.71 ),
              scriptminsize( 8 ),
              veryverythinmathspace( 1.0/18.0 ),
              verythinmathspace( 2.0/18.0 ),
              thinmathspace( 3.0/18.0 ),
              mediummathspace( 4.0/18.0 ),
              thickmathspace( 5.0/18.0 ),
              verythickmathspace( 6.0/18.0 ),
              veryverythickmathspace( 7.0/18.0 ),

              useVariant( false )
            {
            }

        void styleChange()
            {
                kDebug( DEBUGID ) << "Style Change:"
                                   << "\n scriptlevel = " << scriptlevel
                                   << "\n displaystyle = " << displaystyle
                                   << "\n scriptsizemultiplier = "
                                   << scriptsizemultiplier
                                   << "\n scriptminsize = " << scriptminsize
                                   << endl;
            }

        void setStyles( QDomElement element )
            {
                if ( !useVariant )
                    return;

                switch ( mathvariant )
                {
                case normal:
                    element.setAttribute( "STYLE", "normal" );
                    break;
                case bold:
                    element.setAttribute( "STYLE", "bold" );
                    break;

                case bold_italic:
                    element.setAttribute( "STYLE", "bolditalic" );
                    break;
                case italic:
                    element.setAttribute( "STYLE", "italic" );
                    break;

                case double_struck:
                    element.setAttribute( "FAMILY", "doublestruck" );
                    break;

                case bold_fraktur:
                    element.setAttribute( "STYLE", "bold" );
                case fraktur:
                    element.setAttribute( "FAMILY", "fraktur" );
                    break;

                case bold_script:
                    element.setAttribute( "STYLE", "bold" );
                case script:
                    element.setAttribute( "FAMILY", "script" );
                    break;

                case bold_sans_serif:
                    element.setAttribute( "STYLE", "bold" );
                case sans_serif:
                    element.setAttribute( "FAMILY", "normal" );
                    break;
                case sans_serif_bold_italic:
                    element.setAttribute( "STYLE", "bolditalic" );
                    element.setAttribute( "FAMILY", "normal" );
                    break;
                case sans_serif_italic:
                    element.setAttribute( "STYLE", "italic" );
                    element.setAttribute( "FAMILY", "normal" );
                    break;

                //case monospace:
                default:
                    break;
                }
            }

        void readStyles( QDomElement mmlElement )
            {
                if ( mmlElement.hasAttribute( "mathvariant" ) )
                {
                    useVariant = true;

                    if ( mmlElement.attribute( "mathvariant" ) == "normal" )
                        mathvariant = normal;
                    else if ( mmlElement.attribute( "mathvariant" ) == "bold" )
                        mathvariant = bold;
                    else if ( mmlElement.attribute( "mathvariant" ) == "italic" )
                        mathvariant = italic;
                    else if ( mmlElement.attribute( "mathvariant" ) == "bold-italic" )
                        mathvariant = bold_italic;
                    else if ( mmlElement.attribute( "mathvariant" ) == "double-struck" )
                        mathvariant = double_struck;
                    else if ( mmlElement.attribute( "mathvariant" ) == "bold-fraktur" )
                        mathvariant = bold_fraktur;
                    else if ( mmlElement.attribute( "mathvariant" ) == "script" )
                        mathvariant = script;
                    else if ( mmlElement.attribute( "mathvariant" ) == "bold-script" )
                        mathvariant = bold_script;
                    else if ( mmlElement.attribute( "mathvariant" ) == "fraktur" )
                        mathvariant = fraktur;
                    else if ( mmlElement.attribute( "mathvariant" ) == "sans-serif" )
                        mathvariant = sans_serif;
                    else if ( mmlElement.attribute( "mathvariant" ) == "bold-sans-serif" )
                        mathvariant = bold_sans_serif;
                    else if ( mmlElement.attribute( "mathvariant" ) == "sans-serif-italic" )
                        mathvariant = sans_serif_italic;
                    else if ( mmlElement.attribute( "mathvariant" ) == "sans-serif-bold-italic" )
                        mathvariant = sans_serif_bold_italic;
                    else if ( mmlElement.attribute( "mathvariant" ) == "monospace" )
                        mathvariant = monospace;
                }
            }

        // Styles, set by <mstyle>     // default

        int scriptlevel;               // inherited
        bool displaystyle;             // inherited
        double scriptsizemultiplier;   // 0.71
        double scriptminsize;          // 8pt
        // color
        // background
        double veryverythinmathspace;  // 1/18em = 0.0555556em
        double verythinmathspace;      // 2/18em = 0.111111em
        double thinmathspace;          // 3/18em = 0.166667em
        double mediummathspace;        // 4/18em = 0.222222em
        double thickmathspace;         // 5/18em = 0.277778em
        double verythickmathspace;     // 6/18em = 0.333333em
        double veryverythickmathspace; // 7/18em = 0.388889em

        // 'Local' styles

        MathVariant mathvariant;
        bool useVariant;
        //int mathsize;
    };

    MathStyle style;
    QDomDocument doc;

private:
    const ContextStyle& context;
    MathML2KFormula* filter;
};

MathML2KFormulaPrivate::MathML2KFormulaPrivate( MathML2KFormula* mml_filter, const ContextStyle& cs, const QDomDocument& formuladoc )
    : doc( formuladoc ), context( cs ), filter( mml_filter )
{
}

MathML2KFormulaPrivate::~MathML2KFormulaPrivate()
{
}

void MathML2KFormulaPrivate::math( QDomElement element )
{
    QDomElement formula = doc.createElement( "FORMULA" );
    QDomNode n = element.firstChild();

    QString display = element.attribute( "display" );

    if ( display == "block" ) {
        style.displaystyle = true;
    }
    else {
        // if display == "inline" (default) or illegal (then use default)
        style.displaystyle = false;
    }

    style.scriptlevel = 0;

    /*kDebug( DEBUGID ) << "<math> element:\n displaystyle = "
                       << style.displaystyle << "\n scriptlevel = "
                       << style.scriptlevel << endl;*/

    while ( !n.isNull() ) {
        filter->processElement( n, doc, formula );
        n = n.nextSibling();
    }

    doc.appendChild( formula );
}

void MathML2KFormulaPrivate::mi( QDomElement element, QDomNode docnode )
{
    MathStyle previousStyle( style );
    //style.mathvariant = italic;
    style.readStyles( element );

    QString text = element.text().trimmed();
    createTextElements( text, docnode );

    style = previousStyle;
}

void MathML2KFormulaPrivate::mo( QDomElement element, QDomNode docnode )
{
    MathStyle previousStyle( style );
    style.readStyles( element );

    QString text = element.text().trimmed();
    createTextElements( text, docnode );

    style = previousStyle;
}

void MathML2KFormulaPrivate::mn( QDomElement element, QDomNode docnode )
{
    MathStyle previousStyle( style );
    style.readStyles( element );

    QString text = element.text().trimmed();
    createTextElements( text, docnode );

    style = previousStyle;
}

void MathML2KFormulaPrivate::mtext( QDomElement element, QDomNode docnode )
{
    MathStyle previousStyle( style );
    style.readStyles( element );

    QDomNode n = element.firstChild();

    while ( !n.isNull() ) {
        if ( n.isText() ) {
            QString text = n.toText().data().trimmed();
            createTextElements( text, docnode );
        }
        else if ( n.isElement() ) {
            filter->processElement( n, doc, docnode );
        }
        else {
            kDebug( DEBUGID ) << "<mtext> child: " << n.nodeName() << endl;
        }

        n = n.nextSibling();
    }

    style = previousStyle;
}

void MathML2KFormulaPrivate::ms( QDomElement element, QDomNode docnode )
{
    QString lquote = element.attribute( "lquote", "\"" );
    QString rquote = element.attribute( "rquote", "\"" );
    QString text;

    text = lquote;
    text += element.text().trimmed();
    text += rquote;

    createTextElements( text, docnode );
}

void MathML2KFormulaPrivate::mspace( QDomElement element, QDomNode docnode )
{
    // we support only horizontal space
    QString width = element.attribute( "width" );

    QDomElement spaceelement = doc.createElement( "SPACE" );

    // check for namedspace. We don't support much...
    if ( width == "veryverythinmathspace" ) {
        spaceelement.setAttribute( "WIDTH", "thin" );
    }
    else if ( width == "verythinmathspace" ) {
        spaceelement.setAttribute( "WIDTH", "thin" );
    }
    else if ( width == "thinmathspace" ) {
        spaceelement.setAttribute( "WIDTH", "thin" );
    }
    else if ( width == "mediummathspace" ) {
        spaceelement.setAttribute( "WIDTH", "medium" );
    }
    else if ( width == "thickmathspace" ) {
        spaceelement.setAttribute( "WIDTH", "thick" );
    }
    else if ( width == "verythickmathspace" ) {
        spaceelement.setAttribute( "WIDTH", "thick" );
    }
    else if ( width == "veryverythickmathspace" ) {
        spaceelement.setAttribute( "WIDTH", "quad" );
    }

    else {
        // units

        double w = 0;
        bool ok;

        if ( width.endsWith( "em" ) ) {
            // See MathML specification, Appendix H
            w = context.getDefaultFont().pointSize();
            if ( w == -1 ) {
                QFontMetrics fm( context.getDefaultFont() );
                w = fm.width( 'm' );
            }
            w = w * width.remove( width.length() - 2, 2 ).toDouble( &ok );
            // w in points?
        }
        else if ( width.endsWith( "px" ) ) {
            w = width.remove( width.length() - 2, 2 ).toDouble( &ok );
            // w in pixels
        }
        else if ( width.endsWith( "in" ) ) {
            w = width.remove( width.length() - 2, 2 ).toDouble( &ok );
            w *= 72; // w in points
        }
        else if ( width.endsWith( "cm" ) ) {
            w = width.remove( width.length() - 2, 2 ).toDouble( &ok );
            w *= 1/2.54 * 72; // w in points
        }
        else if ( width.endsWith( "mm" ) ) {
            w = width.remove( width.length() - 2, 2 ).toDouble( &ok );
            w *= 1/25.4 * 72; // w in points
        }
        else if ( width.endsWith( "pt" ) ) {
            w = width.remove( width.length() - 2, 2 ).toDouble( &ok );
            // w in points
        }
        else if ( width.endsWith( "pc" ) ) {
            w = width.remove( width.length() - 2, 2 ).toDouble( &ok );
            w /= 12; // w in points
        }
        else {
            w = width.toDouble( &ok );
        }

        if ( !ok )
            return;

        if ( w < 20 )
            spaceelement.setAttribute( "WIDTH", "thin" );
        else if ( w < 40 )
            spaceelement.setAttribute( "WIDTH", "medium" );
        else if ( w < 80 )
            spaceelement.setAttribute( "WIDTH", "thick" );
        else
            spaceelement.setAttribute( "WIDTH", "quad" );
    }

    docnode.appendChild( spaceelement );
}

void MathML2KFormulaPrivate::mrow( QDomElement element, QDomNode docnode )
{
    QDomNode n = element.firstChild();
    while ( !n.isNull() ) {
        if ( n.isElement () ) {
            QDomElement e = n.toElement();
            // We do not allow sequence inside sequence
            filter->processElement( e, doc, docnode );
        }
        else {
            kDebug( DEBUGID ) << "<mrow> child: " << n.nodeName() << endl;
        }
        n = n.nextSibling();
    }
}

void MathML2KFormulaPrivate::mfrac( QDomElement element, QDomNode docnode )
{
    QDomNode n = element.firstChild();
    QDomElement fraction = doc.createElement( "FRACTION" );

    MathStyle previousStyle( style );
    style.displaystyle ? style.displaystyle = false : style.scriptlevel += 1;
    style.styleChange();

    int i = 0;
    while ( !n.isNull() && i < 2 ) {
        if ( n.isElement() ) {
            ++i;
            if ( i == 1 ) { //first is numerator
                QDomElement numerator =
                    doc.createElement( "NUMERATOR" );
                QDomElement sequence = doc.createElement( "SEQUENCE" );
                numerator.appendChild( sequence );
                QDomElement e = n.toElement();
                filter->processElement( e, doc, sequence );
                fraction.appendChild( numerator );

            }
            else {
                QDomElement denominator =
                    doc.createElement( "DENOMINATOR" );
                QDomElement sequence = doc.createElement( "SEQUENCE" );
                denominator.appendChild( sequence );
                QDomElement e = n.toElement();
                filter->processElement( e, doc, sequence );
                fraction.appendChild( denominator );

            }
        }
        else {
            kDebug( DEBUGID ) << "<mfrac> child: " << n.nodeName() << endl;
        }
        n = n.nextSibling();
    }

    style = previousStyle;
    docnode.appendChild( fraction );
}

void MathML2KFormulaPrivate::mroot( QDomElement element, QDomNode docnode )
{
    QDomNode n = element.firstChild();
    int i = 0;
    QDomElement root = doc.createElement( "ROOT" );

    while ( !n.isNull() && i < 2 ) {
        if ( n.isElement() ) {
            ++i;
            if ( i == 1 ) { //first is content (base)
                QDomElement content = doc.createElement( "CONTENT" );
                QDomElement sequence = doc.createElement( "SEQUENCE" );
                content.appendChild( sequence );
                QDomElement e = n.toElement();
                filter->processElement( e, doc, sequence );

                root.appendChild(content);
            }
            else { // index
                MathStyle previousStyle( style );
                style.scriptlevel += 2;
                style.displaystyle = false;
                style.styleChange();

                QDomElement index = doc.createElement( "INDEX" );
                QDomElement sequence = doc.createElement( "SEQUENCE" );
                index.appendChild( sequence );
                QDomElement e = n.toElement();
                filter->processElement( e, doc, sequence );
                root.appendChild( index );

                style = previousStyle;
            }
        }
        else {
            kDebug( DEBUGID ) << "<mroot> child: " << n.nodeName() << endl;
        }
        n = n.nextSibling();
    }
    docnode.appendChild( root );
}

void MathML2KFormulaPrivate::msqrt( QDomElement element, QDomNode docnode )
{
    QDomNode n = element.firstChild();
    QDomElement root = doc.createElement( "ROOT" );

    QDomElement content = doc.createElement( "CONTENT" );
    QDomElement sequence = doc.createElement( "SEQUENCE" );
    content.appendChild( sequence );
    root.appendChild( content );

    while ( !n.isNull() ) {
        if ( n.isElement() ) {
            filter->processElement( n.toElement(), doc, sequence );
        }
        else {
            kDebug( DEBUGID ) << "<msqrt> child: " << n.nodeName() << endl;
        }
        n = n.nextSibling();
    }

    docnode.appendChild( root );
}

void MathML2KFormulaPrivate::mstyle( QDomElement element, QDomNode docnode )
{
    bool ok;

    MathStyle previousStyle( style );
    style.readStyles( element );

    if ( element.hasAttribute( "scriptlevel" ) ) {
        QString scriptlevel = element.attribute( "scriptlevel" );
        if ( scriptlevel.startsWith( "+" ) || scriptlevel.startsWith( "-" ) )
            style.scriptlevel += scriptlevel.toInt( &ok );
        else
            style.scriptlevel = scriptlevel.toInt( &ok );
        if ( !ok )
            style.scriptlevel = previousStyle.scriptlevel;
    }
    if ( element.hasAttribute( "displaystyle" ) ) {
        QString displaystyle = element.attribute( "displaystyle" );
        if ( displaystyle == "true" )
            style.displaystyle = true;
        else if ( displaystyle == "false" )
            style.displaystyle = false;
    }
    if ( element.hasAttribute( "scriptsizemultiplier" ) ) {
        style.scriptsizemultiplier =
            element.attribute( "scriptsizemultiplier" ).toDouble( &ok );
        if ( !ok )
            style.scriptsizemultiplier = previousStyle.scriptsizemultiplier;
    }
    if ( element.hasAttribute( "scriptminsize" ) ) {
        QString scriptminsize = element.attribute( "scriptminsize" );
        style.scriptminsize = convertToPoint( scriptminsize, &ok );
        if ( !ok )
            style.scriptminsize = previousStyle.scriptminsize;
    }

    if ( element.hasAttribute( "veryverythinmathspace" ) ) {
        QString vvthinmspace = element.attribute( "veryverythinmathspace" );
        style.veryverythinmathspace = convertToPoint( vvthinmspace, &ok );
        if ( !ok )
            style.veryverythinmathspace = previousStyle.veryverythinmathspace;
    }
    if ( element.hasAttribute( "verythinmathspace" ) ) {
        QString vthinmspace = element.attribute( "verythinmathspace" );
        style.verythinmathspace = convertToPoint( vthinmspace, &ok );
        if ( !ok )
            style.verythinmathspace = previousStyle.verythinmathspace;
    }
    if ( element.hasAttribute( "thinmathspace" ) ) {
        QString thinmathspace = element.attribute( "thinmathspace" );
        style.thinmathspace = convertToPoint( thinmathspace, &ok );
        if ( !ok )
            style.thinmathspace = previousStyle.thinmathspace;
    }
    if ( element.hasAttribute( "mediummathspace" ) ) {
        QString mediummathspace = element.attribute( "mediummathspace" );
        style.mediummathspace = convertToPoint( mediummathspace, &ok );
        if ( !ok )
            style.mediummathspace = previousStyle.mediummathspace;
    }
    if ( element.hasAttribute( "thickmathspace" ) ) {
        QString thickmathspace = element.attribute( "thickmathspace" );
        style.thickmathspace = convertToPoint( thickmathspace, &ok );
        if ( !ok )
            style.thickmathspace = previousStyle.thickmathspace;
    }
    if ( element.hasAttribute( "verythickmathspace" ) ) {
        QString vthickmspace = element.attribute( "verythickmathspace" );
        style.verythickmathspace = convertToPoint( vthickmspace, &ok );
        if ( !ok )
            style.verythickmathspace = previousStyle.verythickmathspace;
    }
    if ( element.hasAttribute( "veryverythickmathspace" ) ) {
        QString vvthickmspace = element.attribute( "veryverythickmathspace" );
        style.veryverythickmathspace = convertToPoint( vvthickmspace, &ok );
        if ( !ok )
            style.veryverythickmathspace =
                previousStyle.veryverythickmathspace;
    }

    style.styleChange();

    QDomNode n = element.firstChild();
    while ( !n.isNull() ) {
        filter->processElement( n, doc, docnode );
        n = n.nextSibling();
    }

    style = previousStyle;
}

void MathML2KFormulaPrivate::mfenced( QDomElement element, QDomNode docnode )
{
    QDomElement bracket = doc.createElement( "BRACKET" );
    QString value = element.attribute( "open", "(" );
    bracket.setAttribute( "LEFT", QString::number( value.at( 0 ).toLatin1() ) );
    value = element.attribute( "close", ")" );
    bracket.setAttribute( "RIGHT", QString::number( value.at( 0 ).toLatin1() ) );

    QDomElement content = doc.createElement( "CONTENT" );
    QDomElement sequence = doc.createElement( "SEQUENCE" );
    content.appendChild( sequence );

    QString separators = element.attribute( "separators", "," );

    QDomNode n = element.firstChild();
    int i = 0;
    while ( !n.isNull() ) {
        if ( n.isElement() ) {
            if ( i != 0 && !separators.isEmpty() ) {
                QDomElement textelement = doc.createElement( "TEXT" );
                if ( i > separators.length() )
                    i = separators.length();
                textelement.setAttribute( "CHAR", QString( separators.at( i - 1 ) ) );
                //style.setStyles( textelement );
                sequence.appendChild( textelement );
            }
            ++i;
            QDomElement e = n.toElement();
            filter->processElement( e, doc, sequence );
        }
        else {
            kDebug( DEBUGID ) << "<mfenced> child: " << n.nodeName() << endl;
        }
        n = n.nextSibling();
    }
    bracket.appendChild( content );
    docnode.appendChild( bracket );
}

void MathML2KFormulaPrivate::mtable( QDomElement element, QDomNode docnode )
{
    MathStyle previousStyle( style );
    QString displaystyle = element.attribute( "displaystyle", "false" );
    if ( displaystyle == "true" ) {
        style.displaystyle = true;
    }
    else {
        // false is default and also used for illegal values
        style.displaystyle = false;
    }
    style.styleChange();

    QString subtag;
    int rows = 0; int cols = 0;
    QDomNode n = element.firstChild();

    while ( !n.isNull() ) {
        if ( n.isElement() ) {
            QDomElement e = n.toElement();
            subtag = e.tagName();
            if (subtag == "mtr")
            {
                ++rows;

                /* Determins the number of columns */

                QDomNode cellnode = e.firstChild();
                int cc = 0;

                while ( !cellnode.isNull() ) {
                    if ( cellnode.isElement() )
                        cc++;
                    cellnode = cellnode.nextSibling();
                }

                if ( cc > cols )
                    cols = cc;

            }
        }
        else {
            kDebug( DEBUGID ) << "<mtable> child: " << n.nodeName() << endl;
        }
        n = n.nextSibling();
    }

    /* Again createing elements, I need to know the number
       of rows and cols to leave empty spaces */

    n = element.firstChild();
    QDomElement matrix = doc.createElement( "MATRIX" );
    matrix.setAttribute( "COLUMNS", cols );
    matrix.setAttribute( "ROWS", rows );

    while ( !n.isNull() ) {
        if ( n.isElement() ) {
            QDomElement e = n.toElement();
            subtag = e.tagName();
            if ( subtag == "mtr" ) {
                QDomNode cellnode = e.firstChild();
                int cc = 0;
                while ( !cellnode.isNull() ) {
                    if ( cellnode.isElement() ) {
                        ++cc;
                        QDomElement cell = doc.createElement( "SEQUENCE" );
                        QDomElement cellelement = cellnode.toElement();
                        filter->processElement( cellelement, doc, cell );
                        matrix.appendChild( cell );
                    }
                    cellnode = cellnode.nextSibling();
                }

                /* Add empty elements */
                for(; cc < cols; cc++ ) {
                    QDomElement cell = doc.createElement( "SEQUENCE" );
                    matrix.appendChild( cell );
                }
            }
        }
        n = n.nextSibling();
    }

    style = previousStyle;
    docnode.appendChild(matrix);
}

void MathML2KFormulaPrivate::msub_msup( QDomElement element, QDomNode docnode )
{
    QDomNode n = element.firstChild();
    int i = 0;
    QDomElement root = doc.createElement( "INDEX" );

    while ( !n.isNull() && i < 2 ) {
        if ( n.isElement() ) {
            ++i;
            if ( i == 1 ) { // first is content (base)
                QDomElement content = doc.createElement( "CONTENT" );
                QDomElement sequence = doc.createElement( "SEQUENCE" );
                content.appendChild( sequence );
                QDomElement e = n.toElement();
                filter->processElement( e, doc, sequence );

                root.appendChild( content );
            }
            else {
                QDomElement index;
                if ( element.tagName() == "msup" )
                    index = doc.createElement( "UPPERRIGHT" );
                else
                    index = doc.createElement( "LOWERRIGHT" );

                MathStyle previousStyle( style );
                style.scriptlevel += 1;
                style.displaystyle = false;
                style.styleChange();

                QDomElement sequence = doc.createElement( "SEQUENCE" );
                index.appendChild( sequence );
                QDomElement e = n.toElement();
                filter->processElement( e, doc, sequence );
                root.appendChild( index );

                style = previousStyle;
            }
        }
        else {
            kDebug( DEBUGID ) << "<" << element.tagName() << "> child: "
                               << n.nodeName() << endl;
        }
        n = n.nextSibling();
    }
    docnode.appendChild( root );
}

void MathML2KFormulaPrivate::munder( QDomElement element, QDomNode docnode, bool oasisFormat )
{
    bool accentunder;

    QString au = element.attribute( "accentunder" );
    if ( au == "true" )
        accentunder = true;
    else if ( au == "false" )
        accentunder = false;
    else {
        // use default

        QDomElement mo;
        // is underscript an embellished operator?
        if ( isEmbellishedOperator( element.childNodes().item( 1 ), &mo, oasisFormat ) ) {
            if ( mo.attribute( "accent" ) == "true" )
                accentunder = true;
            else
                accentunder = false;
        }
        else
            accentunder = false;
    }

    QDomNode n = element.firstChild();
    int i = 0;
    QDomElement root = doc.createElement( "INDEX" );

    while ( !n.isNull() && i < 2 ) {
        if ( n.isElement() ) {
            ++i;
            if ( i == 1 ) { // first is content (base)
                QDomElement content = doc.createElement( "CONTENT" );
                QDomElement sequence = doc.createElement( "SEQUENCE" );
                content.appendChild( sequence );
                QDomElement e = n.toElement();
                filter->processElement( e, doc, sequence );

                root.appendChild( content );
            }
            else { // underscript
                MathStyle previousStyle( style );
                style.displaystyle = false;
                if ( !accentunder ) {
                    style.scriptlevel += 1;
                    style.styleChange();
                }

                QDomElement mo; QDomElement index;
                if ( isEmbellishedOperator( n.previousSibling(), &mo, oasisFormat ) &&
                     !previousStyle.displaystyle &&
                     mo.attribute( "movablelimits" ) == "true" )
                {
                    index = doc.createElement( "LOWERRIGHT" );
                }
                else {
                    index = doc.createElement( "LOWERMIDDLE" );
                }

                QDomElement sequence = doc.createElement( "SEQUENCE" );
                index.appendChild( sequence );
                QDomElement e = n.toElement();
                filter->processElement( e, doc, sequence );
                root.appendChild( index );

                style = previousStyle;
            }
        }
        else {
            kDebug( DEBUGID ) << "<" << element.tagName() << "> child: "
                               << n.nodeName() << endl;
        }
        n = n.nextSibling();
    }

    docnode.appendChild( root );
}

void MathML2KFormulaPrivate::mover( QDomElement element, QDomNode docnode, bool oasisFormat )
{
    bool accent;

    QString ac = element.attribute( "accent" );
    if ( ac == "true" )
        accent = true;
    else if ( ac == "false" )
        accent = false;
    else {
        // use default

        QDomElement mo;
        // is overscript an embellished operator?
        if ( isEmbellishedOperator( element.childNodes().item( 1 ), &mo, oasisFormat ) ) {
            if ( mo.attribute( "accent" ) == "true" )
                accent = true;
            else
                accent = false;
        }
        else
            accent = false;
    }

    QDomNode n = element.firstChild();
    int i = 0;
    QDomElement root = doc.createElement( "INDEX" );

    while ( !n.isNull() && i < 2 ) {
        if ( n.isElement() ) {
            ++i;
            if ( i == 1 ) { // first is content (base)
                QDomElement content = doc.createElement( "CONTENT" );
                QDomElement sequence = doc.createElement( "SEQUENCE" );
                content.appendChild( sequence );
                QDomElement e = n.toElement();
                filter->processElement( e, doc, sequence );

                root.appendChild( content );
            }
            else { // overscript
                MathStyle previousStyle( style );
                style.displaystyle = false;
                if ( !accent ) {
                    style.scriptlevel += 1;
                    style.styleChange();
                }

                QDomElement mo; QDomElement index;
                if ( isEmbellishedOperator( n.previousSibling(), &mo, oasisFormat ) &&
                     !previousStyle.displaystyle &&
                     mo.attribute( "movablelimits" ) == "true" )
                {
                    index = doc.createElement( "UPPERRIGHT" );
                }
                else {
                    index = doc.createElement( "UPPERMIDDLE" );
                }

                QDomElement sequence = doc.createElement( "SEQUENCE" );
                index.appendChild( sequence );
                QDomElement e = n.toElement();
                filter->processElement( e, doc, sequence );
                root.appendChild( index );

                style = previousStyle;
            }
        }
        else {
            kDebug( DEBUGID ) << "<" << element.tagName() << "> child: "
                               << n.nodeName() << endl;
        }
        n = n.nextSibling();
    }

    docnode.appendChild( root );
}

void MathML2KFormulaPrivate::munderover( QDomElement element, QDomNode docnode, bool oasisFormat )
{
    bool accent;
    bool accentunder;

    QString value = element.attribute( "accentunder" );
    if ( value == "true" )
        accentunder = true;
    else if ( value == "false" )
        accentunder = false;
    else {
        // use default

        QDomElement mo;
        // is underscript an embellished operator?
        if ( isEmbellishedOperator( element.childNodes().item( 1 ), &mo, oasisFormat ) ) {
            if ( mo.attribute( "accent" ) == "true" )
                accentunder = true;
            else
                accentunder = false;
        }
        else
            accentunder = false;
    }
    value = element.attribute( "accent" );
    if ( value == "true" )
        accent = true;
    else if ( value == "false" )
        accent = false;
    else {
        // use default

        QDomElement mo;
        // is overscript an embellished operator?
        if ( isEmbellishedOperator( element.childNodes().item( 2 ), &mo,oasisFormat ) ) {
            kDebug( DEBUGID ) << "embellished operator" << endl;
            if ( mo.attribute( "accent" ) == "true" )
                accent = true;
            else
                accent = false;
        }
        else
            accent = false;
    }
    kDebug( DEBUGID ) << "munderover:\n accentunder = " << accentunder
                       << "\n accent = " << accent << endl;

    QDomNode n = element.firstChild();
    int i = 0;
    QDomElement root = doc.createElement( "INDEX" );

    while ( !n.isNull() && i < 3 ) {
        if ( n.isElement() ) {
            ++i;
            if ( i == 1 ) { // base
                QDomElement content = doc.createElement( "CONTENT" );
                QDomElement sequence = doc.createElement( "SEQUENCE" );
                content.appendChild( sequence );
                QDomElement e = n.toElement();
                filter->processElement( e, doc, sequence );

                root.appendChild( content );
            }
            else if ( i == 2 ) { // underscript
                MathStyle previousStyle( style );
                style.displaystyle = false;
                if ( !accentunder ) {
                    style.scriptlevel += 1;
                    style.styleChange();
                }

                QDomElement mo; QDomElement index;
                // is the base an embellished operator?
                if ( isEmbellishedOperator( element.firstChild(), &mo, oasisFormat ) &&
                     !previousStyle.displaystyle &&
                     mo.attribute( "movablelimits" ) == "true" )
                {
                    index = doc.createElement( "LOWERRIGHT" );
                }
                else {
                    index = doc.createElement( "LOWERMIDDLE" );
                }

                QDomElement sequence = doc.createElement( "SEQUENCE" );
                index.appendChild( sequence );
                QDomElement e = n.toElement();
                filter->processElement( e, doc, sequence );
                root.appendChild( index );

                style = previousStyle;
            }
            else { // overscript
                MathStyle previousStyle( style );
                style.displaystyle = false;
                if ( !accent ) {
                    style.scriptlevel += 1;
                    style.styleChange();
                }

                QDomElement mo; QDomElement index;
                if ( isEmbellishedOperator( element.firstChild(), &mo, oasisFormat ) &&
                     !previousStyle.displaystyle &&
                     mo.attribute( "movablelimits" ) == "true" )
                {
                    index = doc.createElement( "UPPERRIGHT" );
                }
                else {
                    index = doc.createElement( "UPPERMIDDLE" );
                }

                QDomElement sequence = doc.createElement( "SEQUENCE" );
                index.appendChild( sequence );
                QDomElement e = n.toElement();
                filter->processElement( e, doc, sequence );
                root.appendChild( index );

                style = previousStyle;
            }
        }
        else {
            kDebug( DEBUGID ) << "<" << element.tagName() << "> child: "
                               << n.nodeName() << endl;
        }
        n = n.nextSibling();
    }

    docnode.appendChild( root );
}

void MathML2KFormulaPrivate::msubsup( QDomElement element, QDomNode docnode )
{
    QDomNode n = element.firstChild();
    int i = 0;
    QDomElement root = doc.createElement("INDEX");
    MathStyle previousStyle( style );

    while ( !n.isNull() && i < 2 ) {
        if ( n.isElement() ) {
            ++i;
            if ( i == 1 ) { // base
                QDomElement content = doc.createElement( "CONTENT" );
                QDomElement sequence = doc.createElement( "SEQUENCE" );
                content.appendChild( sequence );
                QDomElement e = n.toElement();
                filter->processElement( e, doc, sequence );

                root.appendChild( content );
            }
            else if ( i == 2 ) { // subscript
                style.scriptlevel += 1;
                style.displaystyle = false;
                style.styleChange();

                QDomElement index;
                index = doc.createElement( "LOWERRIGHT" );

                QDomElement sequence = doc.createElement( "SEQUENCE" );
                index.appendChild( sequence );
                QDomElement e = n.toElement();
                filter->processElement( e, doc, sequence );
                root.appendChild( index );
            }
            else { // superscript
                QDomElement index;
                index = doc.createElement( "UPPERRIGHT" );

                QDomElement sequence = doc.createElement( "SEQUENCE" );
                index.appendChild( sequence );
                QDomElement e = n.toElement();
                filter->processElement( e, doc, sequence );
                root.appendChild( index );

                style = previousStyle;

            }
        }
        else {
            kDebug( DEBUGID ) << "<msubsup> child: " << n.nodeName() << endl;
        }
        n = n.nextSibling();
    }
    docnode.appendChild( root );
}

void MathML2KFormulaPrivate::createTextElements( QString text, QDomNode docnode )
{
    for ( int i = 0; i < text.length(); ++i ) {
        QDomElement textelement = doc.createElement( "TEXT" );
        textelement.setAttribute( "CHAR", QString( text.at( i ) ) );
        style.setStyles( textelement );
        if ( context.symbolTable().inTable( text.at( i ) ) ) {
            // The element is a symbol.
            textelement.setAttribute( "SYMBOL", "3" );
        }
        docnode.appendChild( textelement );
    }
}

double MathML2KFormulaPrivate::convertToPoint( QString value, bool* ok )
{
    double pt = 0;

    if ( value.endsWith( "em" ) ) {
        // See MathML specification, Appendix H
        pt = context.getDefaultFont().pointSize();
        if ( pt == -1 ) {
            QFontMetrics fm( context.getDefaultFont() );
            pt = fm.width( 'M' );
            // PIXELS!
        }
        pt = pt * value.remove( value.length() - 2, 2 ).toDouble( ok );
    }
    else if ( value.endsWith( "ex" ) ) {
        QFontMetrics fm( context.getDefaultFont() );
        pt = fm.height();
        // PIXELS, and totally wrong!
    }
    else if ( value.endsWith( "px" ) ) {
        pt = value.remove( value.length() - 2, 2 ).toDouble( ok );
        // PIXELS!
    }
    else if ( value.endsWith( "in" ) ) {
        pt = value.remove( value.length() - 2, 2 ).toDouble( ok );
        pt *= 72;
    }
    else if ( value.endsWith( "cm" ) ) {
        pt = value.remove( value.length() - 2, 2 ).toDouble( ok );
        pt *= 1/2.54 * 72;
    }
    else if ( value.endsWith( "mm" ) ) {
        pt = value.remove( value.length() - 2, 2 ).toDouble( ok );
        pt *= 1/25.4 * 72;
    }
    else if ( value.endsWith( "pt" ) ) {
        pt = value.remove( value.length() - 2, 2 ).toDouble( ok );
    }
    else if ( value.endsWith( "pc" ) ) {
        pt = value.remove( value.length() - 2, 2 ).toDouble( ok );
        pt /= 12;
    }
    else {
        pt = value.toDouble( ok );
    }

    return pt;
}

bool MathML2KFormulaPrivate::isEmbellishedOperator( QDomNode node,
                                                    QDomElement* mo, bool oasisFormat )
{
    // See MathML 2.0 specification: 3.2.5.7

    if ( !node.isElement() )
        return false;

    QDomElement element = node.toElement();
    QString tag = element.tagName();

    if ( tag == "mo" )
    {
        *mo = element;
        return true;
    }
    if ( tag == "msub" || tag == "msup" || tag == "msubsup" ||
         tag == "munder" || tag == "mover" || tag == "munderover" ||
         tag == "mmultiscripts" || tag == "mfrac" || tag == "semantics" )
    {
        return isEmbellishedOperator( element.firstChild(), mo,oasisFormat );
    }
    if ( tag == "maction" )
    {
        return false; // not supported
    }
    if ( tag == "mrow"  || tag == "mstyle" || tag == "mphantom" || tag == "mpadded" ) {
        QDomNode n = element.firstChild();
        int i = 0;

        while ( !n.isNull() ) {
            if ( isEmbellishedOperator( n, mo,oasisFormat ) ) {
                if ( ++i > 1 ) // one (only one) embellished operator
                    return false;
            }
            else if ( !isSpaceLike( n, oasisFormat ) ) { // zero or more space-like elements
                return false;
            }
            n = n.nextSibling();
        }
        return ( i == 1 );
    }
    return false;
}

bool MathML2KFormulaPrivate::isSpaceLike( QDomNode node, bool oasisFormat )
{
    // See MathML 2.0 specification: 3.2.7.3

    if ( !node.isElement() )
        return false;

    QDomElement element = node.toElement();
    QString tag = element.tagName();

    if ( tag == "mtext" || tag == "mspace" ||
         tag == "maligngroup" || tag == "malignmark" ) {
        return true;
    }
    if ( tag == "mstyle" || tag == "mphantom" || tag == "mpadded" || tag == "mrow" ) {
        QDomNode n = element.firstChild();
        while ( !n.isNull() ) {
            if ( isSpaceLike( n,oasisFormat ) )
                n = n.nextSibling();
            else
                return false;
        }
        return true;
    }
    if ( tag == "maction" ) {
        return false; // not supported
    }

    return false;
}


MathML2KFormula::MathML2KFormula( const QDomDocument& mmldoc, const ContextStyle &contextStyle, bool _oasisFormat )
    : m_error( false ), origdoc( mmldoc ), oasisFormat( _oasisFormat ), context( contextStyle )
{
    done = false;
}

QDomDocument MathML2KFormula::getKFormulaDom()
{
    return formuladoc;
}



void MathML2KFormula::startConversion()
{
    //TODO:let it be async
    //kDebug() << origdoc.toString() << endl;
    done = false;
    formuladoc = QDomDocument( "KFORMULA" );
    impl = new MathML2KFormulaPrivate( this, context, formuladoc );
    QDomElement element = origdoc.documentElement();
    if ( element.tagName() == "math" ) {
        impl->math( element );
        m_error = false;
    }
    else {
        kError() << "Not a MathML document!" << endl;
        KMessageBox::error( 0, i18n( "The document does not seem to be MathML." ), i18n( "MathML Import Error" ) );
        m_error = true;
    }
    done = true;
}

bool MathML2KFormula::processElement( QDomNode node, QDomDocument& doc, QDomNode docnode )
{

    //QDomElement *element;
    Type type = UNKNOWN;

    if ( node.isElement() ) {
	QDomElement element = node.toElement();
	QString tag = element.tagName();

	if ( tag == "mi" ) {
	    type = TOKEN;
            impl->mi( element, docnode );
	}
	else if ( tag == "mo" ) {
	    type = TOKEN;
            impl->mo( element, docnode );
	}
	else if ( tag == "mn" ) {
	    type = TOKEN;
            impl->mn( element, docnode );
	}
	else if ( tag == "mtext" ) {
	    type = TOKEN;
            impl->mtext( element, docnode );
	}
	else if ( tag == "ms" ) {
	    type = TOKEN;
            impl->ms( element, docnode );
	}
        else if ( tag == "mspace" ) {
            type = TOKEN;
            impl->mspace( element, docnode );
        }
	else if ( tag == "mrow" ) {
	    type = LAYOUT;
            impl->mrow( element, docnode );
	}
	else if ( tag == "mfrac" ) {
	    type = LAYOUT;
            impl->mfrac( element, docnode );
	}
	else if ( tag == "mroot" ) {
	    type = LAYOUT;
            impl->mroot( element, docnode );
	}
	else if ( tag == "msqrt" ) {
	    type = LAYOUT;
            impl->msqrt( element, docnode );
	}
        else if ( tag == "mstyle" ) {
            type = LAYOUT;
            impl->mstyle( element, docnode );
        }

        else if ( tag == "mfenced" ) {
	    type = LAYOUT;
            impl->mfenced( element, docnode );
        }

	else if ( tag == "mtable" ) {
	    type = TABLE;
            impl->mtable( element, docnode );
	}

	else if ( tag == "msub"  || tag == "msup" ) {
	    type = SCRIPT;
            impl->msub_msup( element, docnode );
	}

	else if ( tag == "munder" ) {
	    type = SCRIPT;
            impl->munder( element, docnode,oasisFormat );
	}
        else if ( tag == "mover" ) {
	    type = SCRIPT;
            impl->mover( element, docnode,oasisFormat );
	}
        else if ( tag == "munderover" ) {
            type = SCRIPT;
            impl->munderover( element, docnode, oasisFormat );
        }
	else if ( tag == "msubsup" ) {
	    type = SCRIPT;
            impl->msubsup( element, docnode );
	}

        // content markup (not yet complete)
        else if ( tag == "apply" ) {
            type = CONTENT;
            QDomNode n = element.firstChild();
            QDomElement op = n.toElement();
                uint count = element.childNodes().count();
		//adding explicit brackets to replace "apply"s implicit ones
		QDomElement brackets = doc.createElement("BRACKET");
		brackets.setAttribute("RIGHT", "41");
		brackets.setAttribute("LEFT", "40");
		QDomElement content = doc.createElement("CONTENT");
		brackets.appendChild(content);
		QDomElement base = doc.createElement("SEQUENCE");
		content.appendChild(base);
		docnode.appendChild(brackets);
		//Arithmetic, Algebra and Logic operators status
		// quotient	X
		// factorial	O
		// divide	O
		// max, min	X
		// minus	O
		// plus		O
		// power	O
		// rem		X
		// times	O
		// root		X
		// gcd		X
		// and		O
		// or		O
		// xor		O
		// not		O
		// implies	O
		// forall	X
		// exists	X
		// abs		O
		// conjugate	X
		// arg		X
		// real		X
		// imaginary	X
		// lcm		X
		// floor	X
		// ceiling	X

            // n-ary
            if ( op.tagName() == "plus" || op.tagName() == "times" ||
                 op.tagName() == "and" || op.tagName() == "or" ||
                 op.tagName() == "xor" ) {

                n = n.nextSibling();
                bool first = true;

                while ( !n.isNull() ) {
                    if ( n.isElement() ) {
                        if ( !first ) {
                            QDomElement text = doc.createElement( "TEXT" );
                            QString value;

                            if ( op.tagName() == "plus" )
                                value = "+";
                            else if ( op.tagName() == "times" )
                                value = "*";
                            else if ( op.tagName() == "and" )
                                value = "&";
                            else if ( op.tagName() == "or" )
                                value = "|";
                            else if ( op.tagName() == "xor" )
                                value = "^"; // ???

                            text.setAttribute( "CHAR", value );	//switch to createTextElements?
                            base.appendChild( text );
                        }
                        first = false;
                        QDomElement e = n.toElement();
                        processElement( e, doc, base );
                    }
                    n = n.nextSibling();
                }
            }

            else if ( op.tagName() == "factorial" ) {
                QDomElement e = n.nextSibling().toElement();
                processElement( e, doc, docnode );
                impl->createTextElements( "!", base );
            }
            else if ( op.tagName() == "minus" ) {
                n = n.nextSibling();
                if ( count == 2 ) { // unary
                    impl->createTextElements( "-", base );
                    QDomElement e = n.toElement();
                    processElement( e, doc, base );
                }
                else if ( count == 3 ) { // binary
                    QDomElement e = n.toElement();
                    processElement( e, doc, base );
                    impl->createTextElements( "-", base );
                    n = n.nextSibling();
                    e = n.toElement();
                    processElement( e, doc, base );
                }
            }

		else if ( op.tagName() == "divide" && count == 3 ) {
			n = n.nextSibling();
        	        QDomElement e = n.toElement();
			processElement( e, doc, base );
			impl->createTextElements("/", base);
			n = n.nextSibling();
			e = n.toElement();
			processElement( e, doc, base );
		}
		else if ( op.tagName() == "power" && count == 3 ) {
			//code duplication of msub_sup(), but I can't find a way to cleanly call it
			n = n.nextSibling();
        	        QDomElement e = n.toElement();
			QDomElement index = doc.createElement("INDEX");
			base.appendChild(index);
			QDomElement content = doc.createElement("CONTENT");
			index.appendChild(content);
			QDomElement sequence = doc.createElement("SEQUENCE");
			content.appendChild(sequence);
			processElement(e, doc, sequence);
			QDomElement upper = doc.createElement("UPPERRIGHT");
			index.appendChild(upper);
			sequence = doc.createElement("SEQUENCE");
			upper.appendChild(sequence);
			n = n.nextSibling();
        	        e = n.toElement();
			processElement(e, doc, sequence);
		}
		else if ( op.tagName() == "abs" && count == 2) {
			n = n.nextSibling();
        	        QDomElement e = n.toElement();
			QDomElement bracket = doc.createElement("BRACKET");
			bracket.setAttribute("RIGHT", "257");
			bracket.setAttribute("LEFT", "256");
			base.appendChild(bracket);
			QDomElement content = doc.createElement("CONTENT");
			bracket.appendChild(content);
			QDomElement sequence = doc.createElement("SEQUENCE");
			content.appendChild(sequence);
			processElement(e, doc, sequence);
		}
		else if ( op.tagName() == "not" && count == 2) {
			n = n.nextSibling();
        	        QDomElement e = n.toElement();
			impl->createTextElements(QString(QChar(0xAC)), base);
			processElement(e, doc, base);
		}
		else if ( op.tagName() == "implies" && count == 3 ) {
			n = n.nextSibling();
        	        QDomElement e = n.toElement();
			processElement( e, doc, base );
			impl->createTextElements(QString(QChar(0x21D2)), base);
			n = n.nextSibling();
			e = n.toElement();
			processElement( e, doc, base );
		}
            // many, many more...

        }

        else if ( tag == "cn" ) {
            type = CONTENT;
            QString type = element.attribute( "type", "real" );

            if ( type == "real" || type == "constant" ) {
                impl->createTextElements( element.text().trimmed(),
                                          docnode );
            }
            else if ( type == "integer" ) {
                QString base = element.attribute( "base" );
                if ( !base.isEmpty() ) {
                    impl->createTextElements( element.text().trimmed(),
                                              docnode );
                }
                else {
                    QDomElement index = doc.createElement( "INDEX" );
                    QDomElement content = doc.createElement( "CONTENT" );
                    QDomElement sequence = doc.createElement( "SEQUENCE" );
                    impl->createTextElements( element.text().trimmed(),
                                              sequence );
                    content.appendChild( sequence );
                    index.appendChild( content );

                    QDomElement lowerright = doc.createElement( "LOWERRIGHT" );
                    sequence = doc.createElement( "SEQUENCE" );

                    impl->createTextElements( base, sequence );

                    lowerright.appendChild( sequence );
                    index.appendChild( lowerright );

                    docnode.appendChild( index );
                }
            }
            else if ( type == "rational" ) {
                QDomNode n = element.firstChild();
                impl->createTextElements( n.toText().data().trimmed(),
                                          docnode );

                n = n.nextSibling(); // <sep/>
                impl->createTextElements( "/", docnode );

                n = n.nextSibling();
                impl->createTextElements( n.toText().data().trimmed(),
                                          docnode );
            }
            else if ( type == "complex-cartesian" ) {
                QDomNode n = element.firstChild();
                impl->createTextElements( n.toText().data().trimmed(),
                                          docnode );

                n = n.nextSibling(); // <sep/>
                impl->createTextElements( "+", docnode );

                n = n.nextSibling();
                impl->createTextElements( n.toText().data().trimmed(),
                                          docnode );

                impl->createTextElements( "i", docnode );
            }

            else if ( type == "complex-polar" ) {
                QDomNode n = element.firstChild();
                impl->createTextElements( n.toText().data().trimmed(),
                                          docnode );

                n = n.nextSibling(); // <sep/>
                QDomElement index = doc.createElement( "INDEX" );
                QDomElement content = doc.createElement( "CONTENT" );
                QDomElement sequence = doc.createElement( "SEQUENCE" );
                QDomElement textelement = doc.createElement( "TEXT" );
                textelement.setAttribute( "CHAR", "e" );
                sequence.appendChild( textelement );
                content.appendChild( sequence );
                index.appendChild( content );

                QDomElement upperright = doc.createElement( "UPPERRIGHT" );
                sequence = doc.createElement( "SEQUENCE" );
                textelement = doc.createElement( "TEXT" );
                textelement.setAttribute( "CHAR", "i" );
                sequence.appendChild( textelement );

                n = n.nextSibling();
                impl->createTextElements( n.toText().data().trimmed(),
                                          sequence );

                upperright.appendChild( sequence );
                index.appendChild( upperright );

                docnode.appendChild( index );
            }
        }

        else if ( tag == "ci" ) {
            type = CONTENT;
            QDomNode n = element.firstChild();

            if ( n.isText() ) {
                impl->createTextElements( n.toText().data().trimmed(),
                                          docnode );
            }
            else if ( n.isElement() ) {
                QDomElement e = n.toElement();
                processElement( e, doc, docnode );
            }
            else if ( n.isEntityReference() ) {
                kDebug( DEBUGID ) << "isEntityReference: "
                                   << n.toEntityReference().nodeName().toLatin1()
                                   << endl;
            }
            else
                kDebug( DEBUGID ) << "ci: " << n.nodeName().toLatin1() << endl;
        }

        else if ( tag == "list" ) {
            type = CONTENT;
            QDomNode n = element.firstChild();

            QDomElement bracket = doc.createElement( "BRACKET" );
            bracket.setAttribute( "LEFT", 91 );  // [
            bracket.setAttribute( "RIGHT", 93 ); // ]
            QDomElement content = doc.createElement( "CONTENT" );
            QDomElement sequence = doc.createElement( "SEQUENCE" );

            bool first = true;

            while ( !n.isNull() ) {
                if ( n.isElement() ) {
                    if ( !first ) {
                        QDomElement textelement = doc.createElement( "TEXT" );
                        textelement.setAttribute( "CHAR", "," );
                        sequence.appendChild( textelement );
                    }
                    first = false;
                    QDomElement e = n.toElement();
                    processElement( e, doc, sequence );
                }
                n = n.nextSibling();
            }

            content.appendChild( sequence );
            bracket.appendChild( content );
            docnode.appendChild( bracket );
        }
    }

    if ( type == UNKNOWN && node.nodeType() != QDomNode::AttributeNode ) {
        kDebug() << "Not an element: " << node.nodeName() << endl;
	QDomNode n = node.firstChild();
	while ( !n.isNull() ) {
	    processElement( n, doc, docnode );
	    n = n.nextSibling();
	}
    }

    return true;
}

KFORMULA_NAMESPACE_END

using namespace KFormula;
#include "kformulamathmlread.moc"
