/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoOasisStyles.h"
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <kdebug.h>
#include "KoDom.h"
#include "KoXmlNS.h"
#include "KoGenStyles.h"
#include <QBuffer>
#include <kglobal.h>
#include <klocale.h>
#include <QBrush>
#include <KoStyleStack.h>

class KoOasisStyles::Private
{
public:
    // The key of the map is the family
    QMap<QString, Q3Dict<KoXmlElement> > m_styles;
    QMap<QString, Q3Dict<KoXmlElement> > m_stylesAutoStyles;
};

KoOasisStyles::KoOasisStyles()
    : d( new Private )
{
    m_defaultStyle.setAutoDelete( true );
    m_masterPages.setAutoDelete( true );
    m_listStyles.setAutoDelete( true );
    m_drawStyles.setAutoDelete( true );
}

KoOasisStyles::~KoOasisStyles()
{
    delete d;
}

void KoOasisStyles::createStyleMap( const KoXmlDocument& doc, bool stylesDotXml )
{
   const KoXmlElement docElement  = doc.documentElement();
    // We used to have the office:version check here, but better let the apps do that
    KoXmlElement fontStyles = KoDom::namedItemNS( docElement, KoXmlNS::office, "font-decls" );

    if ( !fontStyles.isNull() ) {
        //kDebug(30003) << "Starting reading in font-decl..." << endl;
        insertStyles( fontStyles, false );
    }// else
    //   kDebug(30003) << "No items found" << endl;

    //kDebug(30003) << "Starting reading in office:automatic-styles. stylesDotXml=" << stylesDotXml << endl;

    KoXmlElement autoStyles = KoDom::namedItemNS( docElement, KoXmlNS::office, "automatic-styles" );
    if ( !autoStyles.isNull() ) {
        insertStyles( autoStyles, stylesDotXml );
    }// else
    //    kDebug(30003) << "No items found" << endl;


    //kDebug(30003) << "Reading in master styles" << endl;

    KoXmlNode masterStyles = KoDom::namedItemNS( docElement, KoXmlNS::office, "master-styles" );

    if ( !masterStyles.isNull() ) {
        KoXmlElement master;
        forEachElement( master, masterStyles )
        {
            if ( master.localName() == "master-page" &&
                 master.namespaceURI() == KoXmlNS::style ) {
                const QString name = master.attributeNS( KoXmlNS::style, "name", QString::null );
                kDebug(30003) << "Master style: '" << name << "' loaded " << endl;
                m_masterPages.insert( name, new KoXmlElement( master ) );
            } else
                // OASIS docu mentions style:handout-master and draw:layer-set here
                kWarning(30003) << "Unknown tag " << master.tagName() << " in office:master-styles" << endl;
        }
    }


    kDebug(30003) << "Starting reading in office:styles" << endl;

    const KoXmlElement officeStyle = KoDom::namedItemNS( docElement, KoXmlNS::office, "styles" );

    if ( !officeStyle.isNull() ) {
        m_officeStyle = officeStyle;
        insertOfficeStyles( m_officeStyle );

    }

    //kDebug(30003) << "Styles read in." << endl;
}

Q3ValueVector<KoXmlElement> KoOasisStyles::userStyles() const
{
    Q3ValueVector<KoXmlElement> vec;
    // Collect user styles
    unsigned int i = 0;
    KoXmlElement e;
    forEachElement( e, m_officeStyle )
    {
        if ( e.localName() == "style" &&
             e.namespaceURI() == KoXmlNS::style )
        {
            vec.resize( i+1 );
            vec[i++] = e;
        }
    }
    return vec;
}

const Q3Dict<KoXmlElement>& KoOasisStyles::styles(const QString& family) const
{
    // hmm this can create an empty item in the map, but otherwise we couldn't
    // return a const reference.
    return d->m_styles[family];
}

void KoOasisStyles::insertOfficeStyles( const KoXmlElement& styles )
{
    KoXmlElement e;
    forEachElement( e, styles )
    {
        const QString localName = e.localName();
        const QString ns = e.namespaceURI();
        if ( ( ns == KoXmlNS::svg && (
                   localName == "linearGradient"
                   || localName == "radialGradient" ) )
             || ( ns == KoXmlNS::draw && (
                      localName == "gradient"
                      || localName == "hatch"
                      || localName == "fill-image"
                      || localName == "marker"
                      || localName == "stroke-dash"
                      || localName == "opacity" ) )
             )
        {
            const QString name = e.attributeNS( KoXmlNS::draw, "name", QString::null );
            Q_ASSERT( !name.isEmpty() );
            KoXmlElement* ep = new KoXmlElement( e );
            m_drawStyles.insert( name, ep );
        }
        else
            insertStyle( e, false );
    }
}


void KoOasisStyles::insertStyles( const KoXmlElement& styles, bool styleAutoStyles )
{
    //kDebug(30003) << "Inserting styles from " << styles.tagName() << endl;
    KoXmlElement e;
    forEachElement( e, styles )
        insertStyle( e, styleAutoStyles );
}

void KoOasisStyles::insertStyle( const KoXmlElement& e, bool styleAutoStyles )
{
    const QString localName = e.localName();
    const QString ns = e.namespaceURI();

    const QString name = e.attributeNS( KoXmlNS::style, "name", QString::null );
    if ( ns == KoXmlNS::style && localName == "style" ) {
        const QString family = e.attributeNS( KoXmlNS::style, "family", QString::null );

        if ( styleAutoStyles ) {
            Q3Dict<KoXmlElement>& dict = d->m_stylesAutoStyles[ family ];
            dict.setAutoDelete( true );
            if ( dict.find( name ) != 0 )
                kDebug(30003) << "Auto-style: '" << name << "' already exists" << endl;
            dict.insert( name, new KoXmlElement( e ) );
            //kDebug(30003) << "Style: '" << name << "' loaded as a style auto style" << endl;
        } else {
            Q3Dict<KoXmlElement>& dict = d->m_styles[ family ];
            dict.setAutoDelete( true );

            if ( dict.find( name ) != 0 )
                kDebug(30003) << "Style: '" << name << "' already exists" << endl;
            dict.insert( name, new KoXmlElement( e ) );
            //kDebug(30003) << "Style: '" << name << "' loaded " << endl;
        }
    } else if ( ns == KoXmlNS::style && (
                localName == "page-layout"
             || localName == "font-decl"
             || localName == "presentation-page-layout" ) )
    {
        if ( m_styles.find( name ) != 0 )
            kDebug(30003) << "Style: '" << name << "' already exists" << endl;
        m_styles.insert( name, new KoXmlElement( e ) );
    } else if ( localName == "default-style" && ns == KoXmlNS::style ) {
        const QString family = e.attributeNS( KoXmlNS::style, "family", QString::null );
        if ( !family.isEmpty() )
            m_defaultStyle.insert( family, new KoXmlElement( e ) );
    } else if ( localName == "list-style" && ns == KoXmlNS::text ) {
        m_listStyles.insert( name, new KoXmlElement( e ) );
        //kDebug(30003) << "List style: '" << name << "' loaded " << endl;
    } else if ( ns == KoXmlNS::number && (
                   localName == "number-style"
                || localName == "currency-style"
                || localName == "percentage-style"
                || localName == "boolean-style"
                || localName == "text-style"
                || localName == "date-style"
                || localName == "time-style" ) ) {
        importDataStyle( e );
    }
    // The rest (text:*-configuration and text:outline-style) is to be done by the apps.
}

// OO spec 2.5.4. p68. Conversion to Qt format: see qdate.html
// OpenCalcImport::loadFormat has similar code, but slower, intermixed with other stuff,
// lacking long-textual forms.
void KoOasisStyles::importDataStyle( const KoXmlElement& parent )
{
    NumericStyleFormat dataStyle;

    const QString localName = parent.localName();
    if (localName == "number-style")
      dataStyle.type = NumericStyleFormat::Number;
    else if (localName == "currency-style")
      dataStyle.type = NumericStyleFormat::Currency;
    else if (localName == "percentage-style")
      dataStyle.type = NumericStyleFormat::Percentage;
    else if (localName == "boolean-style")
      dataStyle.type = NumericStyleFormat::Boolean;
    else if (localName == "text-style")
      dataStyle.type = NumericStyleFormat::Text;
    else if (localName == "date-style")
      dataStyle.type = NumericStyleFormat::Date;
    else if (localName == "time-style")
      dataStyle.type = NumericStyleFormat::Time;

    QString format;
    int precision = -1;
    int leadingZ  = 1;
    bool thousandsSep = false;
    //todo negred
    //bool negRed = false;
    bool ok = false;
    int i = 0;
    KoXmlElement e;
    QString prefix;
    QString suffix;
    forEachElement( e, parent )
    {
        if ( e.namespaceURI() != KoXmlNS::number )
            continue;
        QString localName = e.localName();
        const QString numberStyle = e.attributeNS( KoXmlNS::number, "style", QString::null );
        const bool shortForm = numberStyle == "short" || numberStyle.isEmpty();
        if ( localName == "day" ) {
            format += shortForm ? "d" : "dd";
        } else if ( localName == "day-of-week" ) {
            format += shortForm ? "ddd" : "dddd";
        } else if ( localName == "month" ) {
            if ( e.attributeNS( KoXmlNS::number, "possessive-form", QString::null ) == "true" ) {
                format += shortForm ? "PPP" : "PPPP";
            }
            // TODO the spec has a strange mention of number:format-source
            else if ( e.attributeNS( KoXmlNS::number, "textual", QString::null ) == "true" ) {
                format += shortForm ? "MMM" : "MMMM";
            } else { // month number
                format += shortForm ? "M" : "MM";
            }
        } else if ( localName == "year" ) {
            format += shortForm ? "yy" : "yyyy";
        } else if ( localName == "era" ) {
            //TODO I don't know what is it... (define into oo spec)
        } else if ( localName == "week-of-year" || localName == "quarter") {
            // ### not supported in Qt
        } else if ( localName == "hours" ) {
            format += shortForm ? "h" : "hh";
        } else if ( localName == "minutes" ) {
            format += shortForm ? "m" : "mm";
        } else if ( localName == "seconds" ) {
            format += shortForm ? "s" : "ss";
        } else if ( localName == "am-pm" ) {
            format += "ap";
        } else if ( localName == "text" ) { // litteral
            format += e.text();
        } else if ( localName == "suffix" ) {
            suffix = e.text();
            kDebug()<<" suffix :"<<suffix<<endl;
        } else if ( localName == "prefix" ) {
            prefix = e.text();
            kDebug()<<" prefix :"<<prefix<<endl;
        } else if ( localName == "currency-symbol" ) {
            dataStyle.currencySymbol = e.text();
            kDebug()<<" currency-symbol: "<<dataStyle.currencySymbol<<endl;
            format += e.text();
            //TODO
            // number:language="de" number:country="DE">€</number:currency-symbol>
            // Stefan: localization of the symbol?
        } else if ( localName == "number" ) {
            // TODO: number:grouping="true"
            if ( e.hasAttributeNS( KoXmlNS::number, "decimal-places" ) )
            {
                int d = e.attributeNS( KoXmlNS::number, "decimal-places", QString::null ).toInt( &ok );
                if ( ok )
                    precision = d;
            }
            if ( e.hasAttributeNS( KoXmlNS::number, "min-integer-digits" ) )
            {
                int d = e.attributeNS( KoXmlNS::number, "min-integer-digits", QString::null ).toInt( &ok );
                if ( ok )
                    leadingZ = d;
            }
            if ( thousandsSep && leadingZ <= 3 )
            {
                format += "#,";
                for ( i = leadingZ; i <= 3; ++i )
                    format += '#';
            }
            for ( i = 1; i <= leadingZ; ++i )
            {
                format +=  '0';
                if ( ( i % 3 == 0 ) && thousandsSep )
                    format =+ ',' ;
            }
            if (precision > -1)
            {
                format += '.';
                for ( i = 0; i < precision; ++i )
                    format += '0';
            }
        }
        else if ( localName == "scientific-number" ) {
            if (dataStyle.type == NumericStyleFormat::Number)
                dataStyle.type = NumericStyleFormat::Scientific;
            int exp = 2;

            if ( e.hasAttributeNS( KoXmlNS::number, "decimal-places" ) )
            {
                int d = e.attributeNS( KoXmlNS::number, "decimal-places", QString::null ).toInt( &ok );
                if ( ok )
                    precision = d;
            }

            if ( e.hasAttributeNS( KoXmlNS::number, "min-integer-digits" ) )
            {
                int d = e.attributeNS( KoXmlNS::number, "min-integer-digits", QString::null ).toInt( &ok );
                if ( ok )
                    leadingZ = d;
            }

            if ( e.hasAttributeNS( KoXmlNS::number, "min-exponent-digits" ) )
            {
                int d = e.attributeNS( KoXmlNS::number, "min-exponent-digits", QString::null ).toInt( &ok );
                if ( ok )
                    exp = d;
                if ( exp <= 0 )
                    exp = 1;
            }

            if ( thousandsSep && leadingZ <= 3 )
            {
                format += "#,";
                for ( i = leadingZ; i <= 3; ++i )
                    format += '#';
            }

            for ( i = 1; i <= leadingZ; ++i )
            {
                format+='0';
                if ( ( i % 3 == 0 ) && thousandsSep )
                    format+=',';
            }

            if (precision > -1)
            {
                format += '.';
                for ( i = 0; i < precision; ++i )
                    format += '0';
            }

            format+="E+";
            for ( i = 0; i < exp; ++i )
                format+='0';
        } else if ( localName == "fraction" ) {
                if (dataStyle.type == NumericStyleFormat::Number)
                    dataStyle.type = NumericStyleFormat::Fraction;
                int integer = 0;
                int numerator = 1;
                int denominator = 1;
                int denominatorValue=0;
                if ( e.hasAttributeNS( KoXmlNS::number, "min-integer-digits" ) )
                {
                    int d = e.attributeNS( KoXmlNS::number, "min-integer-digits", QString::null ).toInt( &ok );
                    if ( ok )
                        integer = d;
                }
                if ( e.hasAttributeNS( KoXmlNS::number, "min-numerator-digits" ) )
                {
                    int d = e.attributeNS( KoXmlNS::number, "min-numerator-digits", QString::null ).toInt( &ok );
                    if ( ok )
                        numerator = d;
                }
                if ( e.hasAttributeNS( KoXmlNS::number, "min-denominator-digits" ) )
                {
                    int d = e.attributeNS( KoXmlNS::number, "min-denominator-digits", QString::null ).toInt( &ok );
                    if ( ok )
                        denominator = d;
                }
                if ( e.hasAttributeNS( KoXmlNS::number, "denominator-value" ) )
                {
                    int d = e.attributeNS( KoXmlNS::number, "denominator-value", QString::null ).toInt( &ok );
                    if ( ok )
                        denominatorValue = d;
                }

                for ( i = 0; i < integer; ++i )
                    format+='#';

                format+=' ';

                for ( i = 0; i < numerator; ++i )
                    format+='?';

                format+='/';

                if ( denominatorValue != 0 )
                    format+=QString::number( denominatorValue );
                else
                {
                    for ( i = 0; i < denominator; ++i )
                        format+='?';
                }
            }
        // Not needed:
        //  <style:map style:condition="value()&gt;=0" style:apply-style-name="N106P0"/>
        // we handle painting negative numbers in red differently

    }

    const QString styleName = parent.attributeNS( KoXmlNS::style, "name", QString::null );
    kDebug(30003) << "data style: " << styleName << " qt format=" << format << endl;
    if ( !prefix.isEmpty() )
    {
        kDebug(30003)<<" format.left( prefix.length() ) :"<<format.left( prefix.length() )<<" prefix :"<<prefix<<endl;
        if ( format.left( prefix.length() )==prefix )
        {
            format = format.right( format.length()-prefix.length() );
        }
        else
            prefix = QString::null;
    }
    if ( !suffix.isEmpty() )
    {
        kDebug(30003)<<"format.right( suffix.length() ) :"<<format.right( suffix.length() )<<" suffix :"<<suffix<<endl;
        if ( format.right( suffix.length() )==suffix )
        {
            format = format.left( format.length()-suffix.length() );
        }
        else
            suffix = QString::null;
    }

    dataStyle.formatStr=format;
    dataStyle.prefix=prefix;
    dataStyle.suffix=suffix;
    dataStyle.precision = precision;
    kDebug()<<" finish insert format :"<<format<<" prefix :"<<prefix<<" suffix :"<<suffix<<endl;
    m_dataFormats.insert( styleName, dataStyle );
}

#define addTextNumber( text, elementWriter ) { \
        if ( !text.isEmpty() ) \
        { \
            elementWriter.startElement( "number:text" ); \
            elementWriter.addTextNode( text ); \
            elementWriter.endElement(); \
            text=""; \
        } \
}

void KoOasisStyles::parseOasisTimeKlocale(KoXmlWriter &elementWriter, QString & format, QString & text )
{
    kDebug(30003)<<"parseOasisTimeKlocale(KoXmlWriter &elementWriter, QString & format, QString & text ) :"<<format<<endl;
    do
    {
        if ( !saveOasisKlocaleTimeFormat( elementWriter, format, text ) )
        {
            text += format[0];
            format = format.remove( 0, 1 );
        }
    }
    while ( format.length() > 0 );
    addTextNumber( text, elementWriter );
}

bool KoOasisStyles::saveOasisKlocaleTimeFormat( KoXmlWriter &elementWriter, QString & format, QString & text )
{
    bool changed = false;
    if ( format.startsWith( "%H" ) ) //hh
    {
        //hour in 24h
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:hours" );
        elementWriter.addAttribute( "number:style", "long" );
        elementWriter.endElement();
        format = format.remove( 0, 2 );
        changed = true;
    }
    else if ( format.startsWith( "%k" ) )//h
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:hours" );
        elementWriter.addAttribute( "number:style", "short" );
        elementWriter.endElement();
        format = format.remove( 0, 2 );
        changed = true;
    }
    else if ( format.startsWith( "%I" ) )// ?????
    {
        //TODO hour in 12h
        changed = true;
    }
    else if ( format.startsWith( "%l" ) )
    {
        //TODO hour in 12h with 1 digit
        changed = true;
    }
    else if ( format.startsWith( "%M" ) )// mm
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:minutes" );
        elementWriter.addAttribute( "number:style", "long" );
        elementWriter.endElement();
        format = format.remove( 0, 2 );
        changed = true;

    }
    else if ( format.startsWith( "%S" ) ) //ss
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:seconds" );
        elementWriter.addAttribute( "number:style", "long" );
        elementWriter.endElement();
        format = format.remove( 0, 2 );
        changed = true;
    }
    else if ( format.startsWith( "%p" ) )
    {
        //TODO am or pm
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:am-pm" );
        elementWriter.endElement();
        format = format.remove( 0, 2 );
        changed = true;
    }
    return changed;
}


bool KoOasisStyles::saveOasisTimeFormat( KoXmlWriter &elementWriter, QString & format, QString & text, bool &antislash )
{
    bool changed = false;
    //we can also add time to date.
    if ( antislash )
    {
        text+=format[0];
        format = format.remove( 0, 1 );
        antislash = false;
        changed = true;
    }
    else if ( format.startsWith( "hh" ) )
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:hours" );
        elementWriter.addAttribute( "number:style", "long" );
        elementWriter.endElement();
        format = format.remove( 0, 2 );
        changed = true;
    }
    else if ( format.startsWith( "h" ) )
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:hours" );
        elementWriter.addAttribute( "number:style", "short" );
        elementWriter.endElement();
        format = format.remove( 0, 1 );
        changed = true;
    }
    else if ( format.startsWith( "mm" ) )
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:minutes" );
        elementWriter.addAttribute( "number:style", "long" );
        elementWriter.endElement();
        format = format.remove( 0, 2 );
        changed = true;
    }
    else if ( format.startsWith( "m" ) )
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:minutes" );
        elementWriter.addAttribute( "number:style", "short" );
        elementWriter.endElement();
        format = format.remove( 0, 1 );
        changed = true;
    }
    else if ( format.startsWith( "ss" ) )
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:seconds" );
        elementWriter.addAttribute( "number:style", "long" );
        elementWriter.endElement();
        format = format.remove( 0, 2 );
        changed = true;
    }
    else if ( format.startsWith( "s" ) )
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:seconds" );
        elementWriter.addAttribute( "number:style", "short" );
        elementWriter.endElement();
        format = format.remove( 0, 1 );
        changed = true;
    }
    else if ( format.startsWith( "ap" ) )
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:am-pm" );
        elementWriter.endElement();
        format = format.remove( 0, 2 );
        changed = true;
    }
    return changed;
}

QString KoOasisStyles::saveOasisTimeStyle( KoGenStyles &mainStyles, const QString & _format, bool klocaleFormat )
{
    kDebug(30003)<<"QString KoOasisStyles::saveOasisTimeStyle( KoGenStyles &mainStyles, const QString & _format ) :"<<_format<<endl;
    QString format( _format );
    KoGenStyle currentStyle( KoGenStyle::STYLE_NUMERIC_TIME );
    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter elementWriter( &buffer );  // TODO pass indentation level
    QString text;
    if ( klocaleFormat )
    {
        parseOasisTimeKlocale( elementWriter, format, text );
    }
    else
    {
        bool antislash = false;
        do
        {
            if ( !saveOasisTimeFormat( elementWriter, format, text, antislash ) )
            {
                QString elem( format[0] );
                format = format.remove( 0, 1 );
                if ( elem == "\\" )
                {
                     antislash = true;
                }
                else
                {
                    text += elem;
                    antislash = false;
                }
            }
        }
        while ( format.length() > 0 );
        addTextNumber( text, elementWriter );
    }
    QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    currentStyle.addChildElement( "number", elementContents );
    return mainStyles.lookup( currentStyle, "N" );
}

//convert klocale string to good format
void KoOasisStyles::parseOasisDateKlocale(KoXmlWriter &elementWriter, QString & format, QString & text )
{
    kDebug(30003)<<"KoOasisStyles::parseOasisDateKlocale(KoXmlWriter &elementWriter, QString & format, QString & text ) :"<<format<<endl;
    do
    {
        if ( format.startsWith( "%Y" ) )
        {
            addTextNumber( text, elementWriter );
            elementWriter.startElement( "number:year" );
            elementWriter.addAttribute( "number:style", "long" );
            elementWriter.endElement();
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%y" ) )
        {

            addTextNumber( text, elementWriter );

            elementWriter.startElement( "number:year" );
            elementWriter.addAttribute( "number:style", "short" );
            elementWriter.endElement();
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%n" ) )
        {
            addTextNumber( text, elementWriter );
            elementWriter.startElement( "number:month" );
            elementWriter.addAttribute( "number:style", "short" );
            elementWriter.addAttribute( "number:textual", "false");
            elementWriter.endElement();
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%m" ) )
        {
            addTextNumber( text, elementWriter );
            elementWriter.startElement( "number:month" );
            elementWriter.addAttribute( "number:style", "long" );
            elementWriter.addAttribute( "number:textual", "false"); //not necessary remove it
            elementWriter.endElement();
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%e" ) )
        {
            addTextNumber( text, elementWriter );

            elementWriter.startElement( "number:day" );
            elementWriter.addAttribute( "number:style", "short" );
            elementWriter.endElement();
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%d" ) )
        {
            addTextNumber( text, elementWriter );

            elementWriter.startElement( "number:day" );
            elementWriter.addAttribute( "number:style", "long" );
            elementWriter.endElement();
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%b" ) )
        {
            addTextNumber( text, elementWriter );
            elementWriter.startElement( "number:month" );
            elementWriter.addAttribute( "number:style", "short" );
            elementWriter.addAttribute( "number:textual", "true");
            elementWriter.endElement();
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%B" ) )
        {
            addTextNumber( text, elementWriter );
            elementWriter.startElement( "number:month" );
            elementWriter.addAttribute( "number:style", "long" );
            elementWriter.addAttribute( "number:textual", "true");
            elementWriter.endElement();
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%a" ) )
        {
            addTextNumber( text, elementWriter );
            elementWriter.startElement( "number:day-of-week" );
            elementWriter.addAttribute( "number:style", "short" );
            elementWriter.endElement();

            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%A" ) )
        {
            addTextNumber( text, elementWriter );
            elementWriter.startElement( "number:day-of-week" );
            elementWriter.addAttribute( "number:style", "long" );
            elementWriter.endElement();
            format = format.remove( 0, 2 );
        }
        else
        {
            if ( !saveOasisKlocaleTimeFormat( elementWriter, format, text ) )
            {
                text += format[0];
                format = format.remove( 0, 1 );
            }
        }
    }
    while ( format.length() > 0 );
    addTextNumber( text, elementWriter );
}

QString KoOasisStyles::saveOasisDateStyle( KoGenStyles &mainStyles, const QString & _format, bool klocaleFormat )
{
    kDebug(30003)<<"QString KoOasisStyles::saveOasisDateStyle( KoGenStyles &mainStyles, const QString & _format ) :"<<_format<<endl;
    QString format( _format );

    // Not supported into Qt: "era" "week-of-year" "quarter"

    KoGenStyle currentStyle( KoGenStyle::STYLE_NUMERIC_DATE );
    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter elementWriter( &buffer );  // TODO pass indentation level
    QString text;
    if ( klocaleFormat )
    {
        parseOasisDateKlocale( elementWriter, format, text );
    }
    else
    {
        bool antislash = false;
        do
        {
            if ( antislash )
            {
                text+=format[0];
                format = format.remove( 0, 1 );
            }
            //TODO implement loading ! What is it ?
            else if ( format.startsWith( "MMMMM" ) )
            {
                addTextNumber( text, elementWriter );
                elementWriter.startElement( "number:month" );
                elementWriter.addAttribute( "number:textual", "true");
                elementWriter.endElement();
                format = format.remove( 0, 5 );
            }
            else if ( format.startsWith( "MMMM" ) )
            {
                addTextNumber( text, elementWriter );
                elementWriter.startElement( "number:month" );
                elementWriter.addAttribute( "number:style", "long" );
                elementWriter.addAttribute( "number:textual", "true");
                elementWriter.endElement();
                format = format.remove( 0, 4 );
            }
            else if ( format.startsWith( "MMM" ) )
            {
                addTextNumber( text, elementWriter );
                elementWriter.startElement( "number:month" );
                elementWriter.addAttribute( "number:style", "short" );
                elementWriter.addAttribute( "number:textual", "true");
                elementWriter.endElement();
                format = format.remove( 0, 3 );
            }
            else if ( format.startsWith( "MM" ) )
            {
                addTextNumber( text, elementWriter );
                elementWriter.startElement( "number:month" );
                elementWriter.addAttribute( "number:style", "long" );
                elementWriter.addAttribute( "number:textual", "false"); //not necessary remove it
                elementWriter.endElement();
                format = format.remove( 0, 2 );
            }
            else if ( format.startsWith( "M" ) )
            {
                addTextNumber( text, elementWriter );
                elementWriter.startElement( "number:month" );
                elementWriter.addAttribute( "number:style", "short" );
                elementWriter.addAttribute( "number:textual", "false");
                elementWriter.endElement();
                format = format.remove( 0, 1 );
            }
            else if ( format.startsWith( "PPPP" ) )
            {
                addTextNumber( text, elementWriter );
                //<number:month number:possessive-form="true" number:textual="true" number:style="long"/>
                elementWriter.startElement( "number:month" );
                elementWriter.addAttribute( "number:style", "short" );
                elementWriter.addAttribute( "number:textual", "false");
                elementWriter.addAttribute( "number:possessive-form", "true" );
                elementWriter.endElement();
                format = format.remove( 0, 4 );
            }
            else if ( format.startsWith( "PPP" ) )
            {
                addTextNumber( text, elementWriter );
                //<number:month number:possessive-form="true" number:textual="true" number:style="short"/>
                elementWriter.startElement( "number:month" );
                elementWriter.addAttribute( "number:possessive-form", "true" );

                elementWriter.addAttribute( "number:style", "short" );
                elementWriter.addAttribute( "number:textual", "false");
                elementWriter.endElement();
                format = format.remove( 0, 3 );
            }
            else if ( format.startsWith( "dddd" ) )
            {
                addTextNumber( text, elementWriter );

                elementWriter.startElement( "number:day-of-week" );
                elementWriter.addAttribute( "number:style", "long" );
                elementWriter.endElement();
                format = format.remove( 0, 4 );
            }
            else if ( format.startsWith( "ddd" ) )
            {
                addTextNumber( text, elementWriter );

                elementWriter.startElement( "number:day-of-week" );
                elementWriter.addAttribute( "number:style", "short" );
                elementWriter.endElement();
                format = format.remove( 0, 3 );
            }
            else if ( format.startsWith( "dd" ) )
            {
                addTextNumber( text, elementWriter );

                elementWriter.startElement( "number:day" );
                elementWriter.addAttribute( "number:style", "long" );
                elementWriter.endElement();
                format = format.remove( 0, 2 );
            }
            else if ( format.startsWith( "d" ) )
            {
                addTextNumber( text, elementWriter );

                elementWriter.startElement( "number:day" );
                elementWriter.addAttribute( "number:style", "short" );
                elementWriter.endElement();
                format = format.remove( 0, 1 );
            }
            else if ( format.startsWith( "yyyy" ) )
            {
                addTextNumber( text, elementWriter );

                elementWriter.startElement( "number:year" );
                elementWriter.addAttribute( "number:style", "long" );
                elementWriter.endElement();
                format = format.remove( 0, 4 );
            }
            else if ( format.startsWith( "yy" ) )
            {
                addTextNumber( text, elementWriter );

                elementWriter.startElement( "number:year" );
                elementWriter.addAttribute( "number:style", "short" );
                elementWriter.endElement();
                format = format.remove( 0, 2 );
            }
            else
            {
                if ( !saveOasisTimeFormat( elementWriter, format, text,antislash ) )
                {
                    QString elem( format[0] );
                    format = format.remove( 0, 1 );
                    if ( elem == "\\" )
                    {
                        antislash = true;
                    }
                    else
                    {
                        text += elem;
                        antislash = false;
                    }
                }
            }
        }
        while ( format.length() > 0 );
        addTextNumber( text, elementWriter );
    }

    QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    currentStyle.addChildElement( "number", elementContents );
    return mainStyles.lookup( currentStyle, "N" );
}


QString KoOasisStyles::saveOasisFractionStyle( KoGenStyles &mainStyles, const QString & _format, const QString &_prefix, const QString &_suffix )
{
    kDebug(30003)<<"QString saveOasisFractionStyle( KoGenStyles &mainStyles, const QString & _format ) :"<<_format<<endl;
    QString format( _format );

    KoGenStyle currentStyle( KoGenStyle::STYLE_NUMERIC_FRACTION );
    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter elementWriter( &buffer );  // TODO pass indentation level
    QString text;
    int integer = 0;
    int numerator = 0;
    int denominator = 0;
    int denominatorValue = 0;
    bool beforeSlash = true;
    do
    {
        if ( format[0]=='#' )
            integer++;
        else if ( format[0]=='/' )
            beforeSlash = false;
        else if ( format[0]=='?' )
        {
            if ( beforeSlash )
                numerator++;
            else
                denominator++;
        }
        else
        {
            bool ok;
            int value = format.toInt( &ok );
            if ( ok )
            {
                denominatorValue=value;
                break;
            }
        }
        format.remove( 0,1 );
    }
    while ( format.length() > 0 );

    text= _prefix;
    addTextNumber(text, elementWriter );

    elementWriter.startElement( "number:fraction" );
    elementWriter.addAttribute( "number:min-integer-digits", integer );
    elementWriter.addAttribute( "number:min-numerator-digits",numerator );
    elementWriter.addAttribute( "number:min-denominator-digits",denominator );
    if ( denominatorValue != 0 )
        elementWriter.addAttribute( "number:denominator-value",denominatorValue );
    elementWriter.endElement();

    addKofficeNumericStyleExtension( elementWriter, _suffix, _prefix );

    text=_suffix;
    addTextNumber(text, elementWriter );

    QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    currentStyle.addChildElement( "number", elementContents );
    return mainStyles.lookup( currentStyle, "N" );
}


QString KoOasisStyles::saveOasisNumberStyle( KoGenStyles &mainStyles, const QString & _format, const QString &_prefix, const QString &_suffix )
{
    kDebug(30003)<<"QString saveOasisNumberStyle( KoGenStyles &mainStyles, const QString & _format ) :"<<_format<<endl;
    QString format( _format );

    KoGenStyle currentStyle( KoGenStyle::STYLE_NUMERIC_NUMBER );
    QBuffer buffer;
    buffer.open( IO_WriteOnly );
    KoXmlWriter elementWriter( &buffer );  // TODO pass indentation level
    QString text;
    int decimalplaces = 0;
    int integerdigits = 0;
    bool beforeSeparator = true;
    do
    {
        if ( format[0]=='.' || format[0]==',' )
            beforeSeparator = false;
        else if ( format[0]=='0' && beforeSeparator )
            integerdigits++;
        else if ( format[0]=='0' && !beforeSeparator )
            decimalplaces++;
        else
            kDebug(30003)<<" error format 0 \n";
        format.remove( 0,1 );
    }
    while ( format.length() > 0 );
    text= _prefix ;
    addTextNumber(text, elementWriter );
    elementWriter.startElement( "number:number" );
    kDebug(30003)<<" decimalplaces :"<<decimalplaces<<" integerdigits :"<<integerdigits<<endl;
    if (!beforeSeparator)
        elementWriter.addAttribute( "number:decimal-places", decimalplaces );
    elementWriter.addAttribute( "number:min-integer-digits", integerdigits );
    elementWriter.endElement();

    text =_suffix ;
    addTextNumber(text, elementWriter );
    addKofficeNumericStyleExtension( elementWriter, _suffix,_prefix );

    QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    currentStyle.addChildElement( "number", elementContents );
    return mainStyles.lookup( currentStyle, "N" );
}

QString KoOasisStyles::saveOasisPercentageStyle( KoGenStyles &mainStyles, const QString & _format, const QString &_prefix, const QString &_suffix )
{
    //<number:percentage-style style:name="N11">
    //<number:number number:decimal-places="2" number:min-integer-digits="1"/>
    //<number:text>%</number:text>
    //</number:percentage-style>

    kDebug(30003)<<"QString saveOasisPercentageStyle( KoGenStyles &mainStyles, const QString & _format ) :"<<_format<<endl;
    QString format( _format );

    KoGenStyle currentStyle( KoGenStyle::STYLE_NUMERIC_PERCENTAGE );
    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter elementWriter( &buffer );  // TODO pass indentation level
    QString text;
    int decimalplaces = 0;
    int integerdigits = 0;
    bool beforeSeparator = true;
    do
    {
        if ( format[0]=='.' || format[0]==',' )
            beforeSeparator = false;
        else if ( format[0]=='0' && beforeSeparator )
            integerdigits++;
        else if ( format[0]=='0' && !beforeSeparator )
            decimalplaces++;
        else
            kDebug(30003)<<" error format 0 \n";
        format.remove( 0,1 );
    }
    while ( format.length() > 0 );
    text= _prefix ;
    addTextNumber(text, elementWriter );
    elementWriter.startElement( "number:number" );
    if (!beforeSeparator)
        elementWriter.addAttribute( "number:decimal-places", decimalplaces );
    elementWriter.addAttribute( "number:min-integer-digits", integerdigits );
    elementWriter.endElement();

    addTextNumber(QString( "%" ), elementWriter );

    text =_suffix ;
    addTextNumber(text, elementWriter );
    addKofficeNumericStyleExtension( elementWriter, _suffix,_prefix );

    QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    currentStyle.addChildElement( "number", elementContents );
    return mainStyles.lookup( currentStyle, "N" );

}

QString KoOasisStyles::saveOasisScientificStyle( KoGenStyles &mainStyles, const QString & _format, const QString &_prefix, const QString &_suffix )
{
    //<number:number-style style:name="N60">
    //<number:scientific-number number:decimal-places="2" number:min-integer-digits="1" number:min-exponent-digits="3"/>
    //</number:number-style>

    //example 000,000e+0000
    kDebug(30003)<<"QString saveOasisScientificStyle( KoGenStyles &mainStyles, const QString & _format ) :"<<_format<<endl;
    QString format( _format );

    KoGenStyle currentStyle( KoGenStyle::STYLE_NUMERIC_SCIENTIFIC );
    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    int decimalplace = 0;
    int integerdigits = 0;
    int exponentdigits = 0;
    KoXmlWriter elementWriter( &buffer );  // TODO pass indentation level
    QString text;
    bool beforeSeparator = true;
    bool exponential = false;
    bool positive = true;
    do
    {
        if ( !exponential )
        {
            if ( format[0]=='0' && beforeSeparator )
                integerdigits++;
            else if ( format[0]==',' || format[0]=='.' )
                beforeSeparator = false;
            else if (  format[0]=='0' && !beforeSeparator )
                decimalplace++;
            else if ( format[0].toLower()=='e' )
            {
                format.remove( 0, 1 );
                if ( format[0]=='+' )
                    positive = true;
                else if ( format[0]=='-' )
                    positive = false;
                else
                    kDebug(30003)<<"Error into scientific number\n";
                exponential = true;
            }
        }
        else
        {
            if ( format[0]=='0' && positive )
                exponentdigits++;
            else if ( format[0]=='0' && !positive )
                exponentdigits--;
            else
                kDebug(30003)<<" error into scientific number exponential value\n";
        }
        format.remove( 0,1 );
    }
    while ( format.length() > 0 );
    text =  _prefix ;
    addTextNumber(text, elementWriter );

    elementWriter.startElement( "number:scientific-number" );
    kDebug(30003)<<" decimalplace :"<<decimalplace<<" integerdigits :"<<integerdigits<<" exponentdigits :"<<exponentdigits<<endl;
    if (!beforeSeparator)
        elementWriter.addAttribute( "number:decimal-places", decimalplace );
    elementWriter.addAttribute( "number:min-integer-digits",integerdigits );
    elementWriter.addAttribute( "number:min-exponent-digits",exponentdigits );
    elementWriter.endElement();

    text = _suffix;
    addTextNumber(text, elementWriter );
    addKofficeNumericStyleExtension( elementWriter, _suffix,_prefix );

    QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    currentStyle.addChildElement( "number", elementContents );
    return mainStyles.lookup( currentStyle, "N" );
}

QString KoOasisStyles::saveOasisCurrencyStyle( KoGenStyles &mainStyles, const QString & _format, const QString &symbol, const QString &_prefix, const QString &_suffix )
{

    //<number:currency-style style:name="N107P0" style:volatile="true">
    //<number:number number:decimal-places="2" number:min-integer-digits="1" number:grouping="true"/>
    //<number:text> </number:text>
    //<number:currency-symbol>VEB</number:currency-symbol>
    //</number:currency-style>

    kDebug(30003)<<"QString saveOasisCurrencyStyle( KoGenStyles &mainStyles, const QString & _format ) :"<<_format<<endl;
    QString format( _format );

    KoGenStyle currentStyle( KoGenStyle::STYLE_NUMERIC_CURRENCY );
    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter elementWriter( &buffer );  // TODO pass indentation level
    QString text;
    int decimalplaces = 0;
    int integerdigits = 0;
    bool beforeSeparator = true;
    do
    {
        if ( format[0]=='.' || format[0]==',' )
            beforeSeparator = false;
        else if ( format[0]=='0' && beforeSeparator )
            integerdigits++;
        else if ( format[0]=='0' && !beforeSeparator )
            decimalplaces++;
        else
            kDebug(30003)<<" error format 0 \n";
        format.remove( 0,1 );
    }
    while ( format.length() > 0 );

    text =  _prefix ;
    addTextNumber(text, elementWriter );

    elementWriter.startElement( "number:number" );
    kDebug(30003)<<" decimalplaces :"<<decimalplaces<<" integerdigits :"<<integerdigits<<endl;
    if (!beforeSeparator)
      elementWriter.addAttribute( "number:decimal-places", decimalplaces );
    elementWriter.addAttribute( "number:min-integer-digits", integerdigits );
    elementWriter.endElement();

    text =  _suffix ;
    addTextNumber(text, elementWriter );
    addKofficeNumericStyleExtension( elementWriter, _suffix,_prefix );

    elementWriter.startElement( "number:currency-symbol" );
    kDebug(30003)<<" currency-symbol: "<<symbol<<endl;
    elementWriter.addTextNode( symbol );
    elementWriter.endElement();

    QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    currentStyle.addChildElement( "number", elementContents );
    return mainStyles.lookup( currentStyle, "N" );
}

QString KoOasisStyles::saveOasisTextStyle( KoGenStyles &mainStyles, const QString & _format, const QString &_prefix, const QString &_suffix )
{

    //<number:text-style style:name="N100">
    //<number:text-content/>
    ///</number:text-style>

    kDebug(30003)<<"QString saveOasisTextStyle( KoGenStyles &mainStyles, const QString & _format ) :"<<_format<<endl;
    QString format( _format );

    KoGenStyle currentStyle( KoGenStyle::STYLE_NUMERIC_TEXT );
    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter elementWriter( &buffer );  // TODO pass indentation level
    QString text;
    do
    {
        format.remove( 0,1 );
    }
    while ( format.length() > 0 );
    text =  _prefix ;
    addTextNumber(text, elementWriter );

    elementWriter.startElement( "number:text-style" );

    text =  _suffix ;
    addTextNumber(text, elementWriter );
    addKofficeNumericStyleExtension( elementWriter, _suffix,_prefix );
    elementWriter.endElement();

    QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    currentStyle.addChildElement( "number", elementContents );
    return mainStyles.lookup( currentStyle, "N" );
}

//This is an extension of numeric style. For the moment we used namespace of oasis format for specific koffice extention. change it for the futur.
void KoOasisStyles::addKofficeNumericStyleExtension( KoXmlWriter & elementWriter, const QString &_suffix, const QString &_prefix )
 {
     if ( !_suffix.isEmpty() )
     {
         elementWriter.startElement( "number:suffix" );
         elementWriter.addTextNode( _suffix );
         elementWriter.endElement();
     }
     if ( !_prefix.isEmpty() )
     {
         elementWriter.startElement( "number:prefix" );
         elementWriter.addTextNode( _prefix );
         elementWriter.endElement();
     }
}

void KoOasisStyles::saveOasisFillStyle( KoGenStyle &styleFill, KoGenStyles& mainStyles, const QBrush & brush )
{
    if ( brush.style() == Qt::SolidPattern )
    {
        styleFill.addProperty( "draw:fill","solid" );
        styleFill.addProperty( "draw:fill-color", brush.color().name() );
    }
    else if ( brush.style() == Qt::Dense1Pattern )
    {
        styleFill.addProperty( "draw:transparency", "94%" );
        styleFill.addProperty( "draw:fill","solid" );
        styleFill.addProperty( "draw:fill-color", brush.color().name() );
    }
    else if ( brush.style() == Qt::Dense2Pattern )
    {
        styleFill.addProperty( "draw:transparency", "88%" );
        styleFill.addProperty( "draw:fill","solid" );
        styleFill.addProperty( "draw:fill-color", brush.color().name() );
    }
    else if ( brush.style() == Qt::Dense3Pattern )
    {
        styleFill.addProperty( "draw:transparency", "63%" );
        styleFill.addProperty( "draw:fill","solid" );
        styleFill.addProperty( "draw:fill-color", brush.color().name() );
    }
    else if ( brush.style() == Qt::Dense4Pattern )
    {
        styleFill.addProperty( "draw:transparency", "50%" );
        styleFill.addProperty( "draw:fill","solid" );
        styleFill.addProperty( "draw:fill-color", brush.color().name() );
    }
    else if ( brush.style() == Qt::Dense5Pattern )
    {
        styleFill.addProperty( "draw:transparency", "37%" );
        styleFill.addProperty( "draw:fill","solid" );
        styleFill.addProperty( "draw:fill-color", brush.color().name() );
    }
    else if ( brush.style() == Qt::Dense6Pattern )
    {
        styleFill.addProperty( "draw:transparency", "12%" );
        styleFill.addProperty( "draw:fill","solid" );
        styleFill.addProperty( "draw:fill-color", brush.color().name() );
    }
    else if ( brush.style() == Qt::Dense7Pattern )
    {
        styleFill.addProperty( "draw:transparency", "6%" );
        styleFill.addProperty( "draw:fill","solid" );
        styleFill.addProperty( "draw:fill-color", brush.color().name() );
    }
    else //otherstyle
    {
        styleFill.addProperty( "draw:fill","hatch" );
        styleFill.addProperty( "draw:fill-hatch-name", saveOasisHatchStyle( mainStyles,brush ) );
    }

}

QString KoOasisStyles::saveOasisHatchStyle( KoGenStyles& mainStyles, const QBrush &brush )
{
    KoGenStyle hatchStyle( KoGenStyle::STYLE_HATCH /*no family name*/);
    hatchStyle.addAttribute( "draw:color", brush.color().name() );
    //hatchStyle.addAttribute( "draw:distance", m_distance ); not implemented into kpresenter
    switch( brush.style() )
    {
    case Qt::HorPattern:
        hatchStyle.addAttribute( "draw:style", "single" );
        hatchStyle.addAttribute( "draw:rotation", 0);
        break;
    case Qt::BDiagPattern:
        hatchStyle.addAttribute( "draw:style", "single" );
        hatchStyle.addAttribute( "draw:rotation", 450);
        break;
    case Qt::VerPattern:
        hatchStyle.addAttribute( "draw:style", "single" );
        hatchStyle.addAttribute( "draw:rotation", 900);
        break;
    case Qt::FDiagPattern:
        hatchStyle.addAttribute( "draw:style", "single" );
        hatchStyle.addAttribute( "draw:rotation", 1350);
        break;
    case Qt::CrossPattern:
        hatchStyle.addAttribute( "draw:style", "double" );
        hatchStyle.addAttribute( "draw:rotation", 0);
        break;
    case Qt::DiagCrossPattern:
        hatchStyle.addAttribute( "draw:style", "double" );
        hatchStyle.addAttribute( "draw:rotation", 450);
        break;
    default:
        break;
    }

    return mainStyles.lookup( hatchStyle, "hatch" );
}

QBrush KoOasisStyles::loadOasisFillStyle( const KoStyleStack &styleStack, const QString & fill, const KoOasisStyles & oasisStyles )
{
    QBrush tmpBrush;
    if ( fill == "solid" )
    {
        tmpBrush.setStyle(static_cast<Qt::BrushStyle>( 1 ) );
        if ( styleStack.hasAttributeNS( KoXmlNS::draw, "fill-color" ) )
            tmpBrush.setColor(styleStack.attributeNS( KoXmlNS::draw, "fill-color" ) );
        if ( styleStack.hasAttributeNS( KoXmlNS::draw, "transparency" ) )
        {
            QString transparency = styleStack.attributeNS( KoXmlNS::draw, "transparency" );
            if ( transparency == "94%" )
            {
                tmpBrush.setStyle(Qt::Dense1Pattern);
            }
            else if ( transparency == "88%" )
            {
                tmpBrush.setStyle(Qt::Dense2Pattern);
            }
            else if ( transparency == "63%" )
            {
                tmpBrush.setStyle(Qt::Dense3Pattern);

            }
            else if ( transparency == "50%" )
            {
                tmpBrush.setStyle(Qt::Dense4Pattern);

            }
            else if ( transparency == "37%" )
            {
                tmpBrush.setStyle(Qt::Dense5Pattern);

            }
            else if ( transparency == "12%" )
            {
                tmpBrush.setStyle(Qt::Dense6Pattern);

            }
            else if ( transparency == "6%" )
            {
                tmpBrush.setStyle(Qt::Dense7Pattern);

            }
            else
                kDebug()<<" transparency is not defined into kpresenter :"<<transparency<<endl;
        }
    }
    else if ( fill == "hatch" )
    {
        QString style = styleStack.attributeNS( KoXmlNS::draw, "fill-hatch-name" );
        kDebug()<<" hatch style is  : "<<style<<endl;

        //type not defined by default
        //try to use style.
        KoXmlElement* draw = oasisStyles.drawStyles()[style];
        if ( draw)
        {
            kDebug()<<"We have a style\n";
            int angle = 0;
            if( draw->hasAttributeNS( KoXmlNS::draw, "rotation" ))
            {
                angle = (draw->attributeNS( KoXmlNS::draw, "rotation", QString::null ).toInt())/10;
                kDebug()<<"angle :"<<angle<<endl;
            }
            if(draw->hasAttributeNS( KoXmlNS::draw, "color" ) )
            {
                //kDebug()<<" draw:color :"<<draw->attributeNS( KoXmlNS::draw, "color", QString::null )<<endl;
                tmpBrush.setColor(draw->attributeNS( KoXmlNS::draw, "color", QString::null ) );
            }
            if( draw->hasAttributeNS( KoXmlNS::draw, "distance" ))
            {
                //todo implemente it into kpresenter
            }
            if( draw->hasAttributeNS( KoXmlNS::draw, "display-name"))
            {
                //todo implement it into kpresenter
            }
            if( draw->hasAttributeNS( KoXmlNS::draw, "style" ))
            {
                //todo implemente it into kpresenter
                QString styleHash = draw->attributeNS( KoXmlNS::draw, "style", QString::null );
                if( styleHash == "single")
                {
                    switch( angle )
                    {
                    case 0:
                    case 180:
                        tmpBrush.setStyle(Qt::HorPattern );
                        break;
                    case 45:
                    case 225:
                        tmpBrush.setStyle(Qt::BDiagPattern );
                        break;
                    case 90:
                    case 270:
                        tmpBrush.setStyle(Qt::VerPattern );
                        break;
                    case 135:
                    case 315:
                        tmpBrush.setStyle(Qt::FDiagPattern );
                        break;
                    default:
                        //todo fixme when we will have a kopaint
                        kDebug()<<" draw:rotation 'angle' : "<<angle<<endl;
                        break;
                    }
                }
                else if( styleHash == "double")
                {
                    switch( angle )
                    {
                    case 0:
                    case 180:
                    case 90:
                    case 270:
                        tmpBrush.setStyle(Qt::CrossPattern );
                        break;
                    case 45:
                    case 135:
                    case 225:
                    case 315:
                        tmpBrush.setStyle(Qt::DiagCrossPattern );
                        break;
                    default:
                        //todo fixme when we will have a kopaint
                        kDebug()<<" draw:rotation 'angle' : "<<angle<<endl;
                        break;
                    }

                }
                else if( styleHash == "triple")
                {
                    kDebug()<<" it is not implemented :( \n";
                }
            }
        }
    }
    return tmpBrush;
}

const KoXmlElement* KoOasisStyles::defaultStyle( const QString& family ) const
{
    return m_defaultStyle[family];
}

const KoXmlElement* KoOasisStyles::findStyle( const QString& name ) const
{
    return m_styles[ name ];
}

const KoXmlElement* KoOasisStyles::findStyle( const QString& styleName, const QString& family ) const
{
    const KoXmlElement* style = d->m_styles[ family ][ styleName ];
    if ( style && !family.isEmpty() ) {
        const QString styleFamily = style->attributeNS( KoXmlNS::style, "family", QString::null );
        if ( styleFamily != family ) {
            kWarning() << "KoOasisStyles: was looking for style " << styleName
                        << " in family " << family << " but got " << styleFamily << endl;
        }
    }
    return style;
}

const KoXmlElement* KoOasisStyles::findStyleAutoStyle( const QString& styleName, const QString& family ) const
{
    const KoXmlElement* style = d->m_stylesAutoStyles[ family ][ styleName ];
    if ( style ) {
        const QString styleFamily = style->attributeNS( KoXmlNS::style, "family", QString::null );
        if ( styleFamily != family ) {
            kWarning() << "KoOasisStyles: was looking for style " << styleName
                        << " in family " << family << " but got " << styleFamily << endl;
        }
    }
    return style;
}
