/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>
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

#include "KoDocumentInfo.h"
#include "KoDom.h"
#include "KoDocument.h"
#include "kofficeversion.h"

#include <QDateTime>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kuser.h>

#include "KoXmlNS.h"

KoDocumentInfo::KoDocumentInfo( QObject* parent ) : QObject( parent )
{
    m_aboutTags << "title" << "description" << "subject" << "comments"
        << "keyword" << "initial-creator" << "editing-cycles"
        << "date" << "creation-date";

    m_authorTags << "creator" << "initial" << "author-title"
        << "email" << "telephone" << "telephone-work"
        << "fax" << "country" << "postal-code" << "city"
        << "street" << "position" << "company";

    setAboutInfo( "editing-cycles", "0" );
    setAboutInfo( "initial-creator", i18n( "Unknown" ) );
    setAboutInfo( "creation-date", QDateTime::currentDateTime()
            .toString( Qt::ISODate ) );
    KUser user( KUser::UseRealUserID );
    setAuthorInfo( "creator", user.fullName() );
    m_firstSave = false;
}

KoDocumentInfo::~KoDocumentInfo()
{
}

bool KoDocumentInfo::load( const KoXmlDocument& doc )
{
    if( !loadAboutInfo( doc.documentElement() ) )
        return false;

    if( !loadAuthorInfo( doc.documentElement() ) )
        return false;

    return true;
}

bool KoDocumentInfo::loadOasis( const KoXmlDocument& metaDoc )
{
    KoXmlNode t = KoDom::namedItemNS( metaDoc, KoXmlNS::office, "document-meta" );
    KoXmlNode office = KoDom::namedItemNS( t, KoXmlNS::office, "meta" );

    if( office.isNull() )
        return false;

    if( !loadOasisAboutInfo( office ) )
        return false;

    if( !loadOasisAuthorInfo( office ) )
        return false;

    return true;
}

QDomDocument KoDocumentInfo::save()
{
    saveParameters();

    QDomDocument doc = KoDocument::createDomDocument( "document-info"
            /*DTD name*/, "document-info" /*tag name*/, "1.1" );

    QDomElement s = saveAboutInfo( doc );
    if ( !s.isNull() )
        doc.documentElement().appendChild( s );

    s = saveAuthorInfo( doc );
    if ( !s.isNull() )
        doc.documentElement().appendChild( s );


    if( doc.documentElement().isNull() )
        return QDomDocument();

    return doc;
}

bool KoDocumentInfo::saveOasis( KoStore* store )
{
    saveParameters();

    KoStoreDevice dev( store );
    KoXmlWriter* xmlWriter = KoDocument::createOasisXmlWriter( &dev,
            "office:document-meta" );
    xmlWriter->startElement( "office:meta" );

    xmlWriter->startElement( "meta:generator");
    xmlWriter->addTextNode( QString( "KOffice/%1" )
            .arg( KOFFICE_VERSION_STRING ) );
    xmlWriter->endElement();

    if( !saveOasisAboutInfo( *xmlWriter ) )
        return false;
    if( !saveOasisAuthorInfo( *xmlWriter ) )
        return false;

    xmlWriter->endElement();
    xmlWriter->endElement(); // root element
    xmlWriter->endDocument();
    delete xmlWriter;
    return true;
}

void KoDocumentInfo::setAuthorInfo( const QString& info, const QString& data )
{
    if( !m_authorTags.contains( info ) )
        return;

    m_authorInfo.insert( info, data );
}

QString KoDocumentInfo::authorInfo( const QString& info ) const
{
    if( !m_authorTags.contains( info ) )
        return QString();

    return m_authorInfo[ info ];
}

void KoDocumentInfo::setAboutInfo( const QString& info, const QString& data )
{
    m_aboutInfo.insert( info, data );
}

QString KoDocumentInfo::aboutInfo( const QString& info ) const
{
    if( !m_aboutTags.contains( info ) )
    {
        kWarning() << info + " page not found in documentInfo !" << endl;
        return QString();
    }

    return m_aboutInfo[ info ];
}

bool KoDocumentInfo::saveOasisAuthorInfo( KoXmlWriter &xmlWriter )
{
    foreach( QString tag, m_authorTags )
    {
        if( !authorInfo( tag ).isEmpty() && tag == "creator" )
        {
            xmlWriter.startElement( "dc:creator");
            xmlWriter.addTextNode( authorInfo( "creator" ) );
            xmlWriter.endElement();
        }
        else if( !authorInfo( tag ).isEmpty() )
        {
            xmlWriter.startElement( "meta:user-defined");
            xmlWriter.addAttribute( "meta:name", tag );
            xmlWriter.addTextNode( authorInfo( tag ) );
            xmlWriter.endElement();
        }
    }

    return true;
}

bool KoDocumentInfo::loadOasisAuthorInfo( const KoXmlNode& metaDoc )
{
    KoXmlElement e = KoDom::namedItemNS( metaDoc, KoXmlNS::dc, "creator" );
    if ( !e.isNull() && !e.text().isEmpty() )
        setAuthorInfo( "creator", e.text() );

    KoXmlNode n = metaDoc.firstChild();
    for ( ; !n.isNull(); n = n.nextSibling() )
    {
        if ( !n.isElement())
            continue;

        KoXmlElement e = n.toElement();
        if ( !( e.namespaceURI() == KoXmlNS::meta &&
                    e.localName() == "user-defined" && !e.text().isEmpty() ) )
            continue;

        QString name = e.attributeNS( KoXmlNS::meta, "name", QString::null );
        setAuthorInfo( name, e.text() );
    }

    return true;
}

bool KoDocumentInfo::loadAuthorInfo( const KoXmlElement& e )
{
    KoXmlNode n = e.namedItem( "author" ).firstChild();
    for( ; !n.isNull(); n = n.nextSibling() )
    {
        KoXmlElement e = n.toElement();
        if( e.isNull() )
            continue;

        if( e.tagName() == "full-name" )
            setAuthorInfo( "creator", e.text() );
        else
            setAuthorInfo( e.tagName(), e.text() );
    }

    return true;
}

QDomElement KoDocumentInfo::saveAuthorInfo( QDomDocument& doc )
{
    QDomElement e = doc.createElement( "author" );
    QDomElement t;

    foreach( QString tag, m_authorTags )
    {
        if( tag == "creator" )
            t = doc.createElement( "full-name" );
        else
            t = doc.createElement( tag );

        e.appendChild( t );
        t.appendChild( doc.createTextNode( authorInfo( tag ) ) );
    }

    return e;
}

bool KoDocumentInfo::saveOasisAboutInfo( KoXmlWriter &xmlWriter )
{
    saveParameters();

    foreach( QString tag, m_aboutTags )
    {
        if( !aboutInfo( tag ).isEmpty() || tag == "title" )
        {
            if( tag == "keyword" )
            {
                foreach( QString tmp, aboutInfo( "keyword" ).split(";") )
                {
                    xmlWriter.startElement( "meta:keyword" );
                    xmlWriter.addTextNode( tmp );
                    xmlWriter.endElement();
                }
            }
            else if( tag == "title" || tag == "description" || tag == "subject" ||
                    tag == "date" )
            {
              if ( tag == "title" && aboutInfo( tag ).isEmpty() )
                {
                  KoDocument* doc = dynamic_cast< KoDocument* >( parent() );
                  setAboutInfo( "title", doc->url().fileName() );
                }
                QByteArray elementName( QString( "dc:" + tag ).toLatin1().constData() );
                xmlWriter.startElement( elementName );
                xmlWriter.addTextNode( aboutInfo( tag ) );
                xmlWriter.endElement();
            }
            else
            {
                QByteArray elementName( QString( "meta:" + tag).toLatin1().constData() );
                xmlWriter.startElement( elementName );
                xmlWriter.addTextNode( aboutInfo( tag ) );
                xmlWriter.endElement();
            }
        }
    }

    return true;
}

bool KoDocumentInfo::loadOasisAboutInfo( const KoXmlNode& metaDoc )
{
    KoXmlElement e;

    foreach( QString tag, m_aboutTags )
    {
        if( tag == "keyword" )
        {
            // this aren't all tags -> FIXME
            e  = KoDom::namedItemNS( metaDoc, KoXmlNS::meta, tag.toLatin1().constData() );

            if ( !e.isNull() && !e.text().isEmpty() )
                setAboutInfo( tag, e.text() );
        }
        else if( tag == "title" || tag == "description" || tag == "subject" ||
                tag == "date" )
        {
            e  = KoDom::namedItemNS( metaDoc, KoXmlNS::dc, tag.toLatin1().constData() );

            if ( !e.isNull() && !e.text().isEmpty() )
                setAboutInfo( tag, e.text() );
        }
        else
        {
            e  = KoDom::namedItemNS( metaDoc, KoXmlNS::meta, tag.toLatin1().constData() );

            if ( !e.isNull() && !e.text().isEmpty() )
                setAboutInfo( tag, e.text() );
        }
    }

    return true;
}

bool KoDocumentInfo::loadAboutInfo( const KoXmlElement& e )
{
    KoXmlNode n = e.namedItem( "about" ).firstChild();
    KoXmlElement tmp;
    for( ; !n.isNull(); n = n.nextSibling()  )
    {
        tmp = n.toElement();
        if ( tmp.isNull() )
            continue;

        if ( tmp.tagName() == "abstract" )
            setAboutInfo( "comments", tmp.text() );

        setAboutInfo( tmp.tagName(), tmp.text() );
    }

    return true;
}

QDomElement KoDocumentInfo::saveAboutInfo( QDomDocument& doc )
{
    saveParameters();

    QDomElement e = doc.createElement( "about" );
    QDomElement t;

    foreach( QString tag, m_aboutTags )
    {
        if( tag == "comments" )
        {
            t = doc.createElement( "abstract" );
            e.appendChild( t );
            t.appendChild( doc.createCDATASection( aboutInfo( tag ) ) );
        }
        else
        {
            t = doc.createElement( tag );
            e.appendChild( t );
            t.appendChild( doc.createTextNode( aboutInfo( tag ) ) );
        }
    }

    return e;
}

void KoDocumentInfo::saveParameters()
{
    KoDocument* doc = dynamic_cast< KoDocument* >( parent() );
    if ( doc && doc->isAutosaving() )
        return;

    int cycles = aboutInfo( "editing-cycles" ).toInt();
    setAboutInfo( "editing-cycles", QString::number( cycles ) );
    if(m_firstSave) {
        cycles++;
        m_firstSave = false;
    }
    setAboutInfo( "date", QDateTime::currentDateTime().toString( Qt::ISODate ) );
}

void KoDocumentInfo::resetMetaData()
{
    setAboutInfo( "editing-cycles", QString::number( 0 ) );
    setAboutInfo( "initial-creator", authorInfo( "creator" ) );
    setAboutInfo( "creation-date", QDateTime::currentDateTime().toString( Qt::ISODate ) );
}

#include "KoDocumentInfo.moc"
