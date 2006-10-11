/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>

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

#include "KoXmlReader.h"

static const char* const KoXmlNS_office = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
static const char* const KoXmlNS_text = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";

#include <QApplication>
//Added by qt3to4:
#include <QByteArray>
#include <assert.h>

//static void debugElemNS( const QDomElement& elem )
//{
//    qDebug( "nodeName=%s tagName=%s localName=%s prefix=%s namespaceURI=%s", elem.nodeName().latin1(), elem.tagName().latin1(), elem.localName().latin1(), elem.prefix().latin1(), elem.namespaceURI().latin1() );
//}

void testQDom( const KoXmlDocument& doc )
{
    KoXmlElement docElem = doc.documentElement();
    //debugElemNS( docElem );
    assert( docElem.nodeName() == "o:document-content" );
    assert( docElem.tagName() == "document-content" );
    assert( docElem.localName() == "document-content" );
    assert( docElem.prefix() == "o" );
    assert( docElem.namespaceURI() == KoXmlNS_office );

    KoXmlElement elem = KoXml::namedItemNS( docElem, KoXmlNS_office, "body" );

    //debugElemNS( elem );
    assert( elem.tagName() == "body" );
    assert( elem.localName() == "body" );
    assert( elem.prefix() == "o" );
    assert( elem.namespaceURI() == KoXmlNS_office );

    KoXmlNode n = elem.firstChild();
    for ( ; !n.isNull() ; n = n.nextSibling() ) {
        if ( !n.isElement() )
            continue;
        KoXmlElement e = n.toElement();
        //debugElemNS( e );
        assert( e.tagName() == "p" );
        assert( e.localName() == "p" );
        assert( e.prefix().isEmpty() );
        assert( e.namespaceURI() == KoXmlNS_text );
    }

    qDebug("testQDom... ok");
}

void testKoDom( const KoXmlDocument& doc )
{
    KoXmlElement docElem = KoXml::namedItemNS( doc, KoXmlNS_office, "document-content" );
    assert( !docElem.isNull() );
    assert( docElem.localName() == "document-content" );
    assert( docElem.namespaceURI() == KoXmlNS_office );

    KoXmlElement body = KoXml::namedItemNS( docElem, KoXmlNS_office, "body" );
    assert( !body.isNull() );
    assert( body.localName() == "body" );
    assert( body.namespaceURI() == KoXmlNS_office );

    KoXmlElement p = KoXml::namedItemNS( body, KoXmlNS_text, "p" );
    assert( !p.isNull() );
    assert( p.localName() == "p" );
    assert( p.namespaceURI() == KoXmlNS_text );

    const KoXmlElement officeStyle = KoXml::namedItemNS( docElem, KoXmlNS_office, "styles" );
    assert( !officeStyle.isNull() );

    // Look for a non-existing element
    KoXmlElement notexist = KoXml::namedItemNS( body, KoXmlNS_text, "notexist" );
    assert( notexist.isNull() );

    int count = 0;
    KoXmlElement elem;
    forEachElement( elem, body ) {
        assert( elem.localName() == "p" );
        assert( elem.namespaceURI() == KoXmlNS_text );
        ++count;
    }
    assert( count == 2 );

    // Attributes
    // ### Qt bug: it doesn't work if using style-name instead of text:style-name in the XML
    const QString styleName = p.attributeNS( KoXmlNS_text, "style-name", QString::null );
    qDebug( "%s", qPrintable( styleName ) );
    assert( styleName == "L1" );

    qDebug("testKoDom... ok");
}

int main( int argc, char** argv ) {
    QApplication app( argc, argv, QApplication::Tty );

    const QByteArray xml = QByteArray( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                   "<o:document-content xmlns:o=\"" )
                         + KoXmlNS_office
                         + "\" xmlns=\"" + KoXmlNS_text
                         + "\" xmlns:text=\"" + KoXmlNS_text
                         + "\">\n"
		"<o:body><p text:style-name=\"L1\">foobar</p><p>2nd</p></o:body><o:styles></o:styles>\n"
		"</o:document-content>\n";
    KoXmlDocument doc;
    //QXmlSimpleReader reader;
    //reader.setFeature( "http://xml.org/sax/features/namespaces", TRUE );
    //reader.setFeature( "http://xml.org/sax/features/namespace-prefixes", FALSE );
    bool ok = doc.setContent( xml, true /* namespace processing */ );
    assert( ok );

    testQDom(doc);
    testKoDom(doc);
}
