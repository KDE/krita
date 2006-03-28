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
#include "KoApplication.h"

#include <KoStoreDevice.h>
#include <KoXmlWriter.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include <qobject.h>
#include <qdatetime.h>
#include "KoXmlNS.h"

/*****************************************
 *
 * KoDocumentInfo
 *
 *****************************************/

KoDocumentInfo::KoDocumentInfo( QObject* parent, const char* name )
    : QObject( parent, name )
{
    (void)new KoDocumentInfoUserMetadata( this );
    (void)new KoDocumentInfoAuthor( this );
    (void)new KoDocumentInfoAbout( this );
}

KoDocumentInfo::~KoDocumentInfo()
{
}

// KOffice-1.3 format
bool KoDocumentInfo::load( const QDomDocument& doc )
{
    QStringList lst = pages();
    QStringList::ConstIterator it = lst.begin();
    for( ; it != lst.end(); ++it )
    {
        KoDocumentInfoPage* p = page( *it );
        Q_ASSERT( p );
        if ( !p->load( doc.documentElement() ) )
            return false;
    }

    return true;
}

bool KoDocumentInfo::loadOasis( const QDomDocument& metaDoc )
{
    //kDebug()<<" metaDoc.toString() :"<<metaDoc.toString()<<endl;
    QStringList lst = pages();
    QStringList::ConstIterator it = lst.begin();
    for( ; it != lst.end(); ++it )
    {
        KoDocumentInfoPage* p = page( *it );
        Q_ASSERT( p );

        QDomNode meta   = KoDom::namedItemNS( metaDoc, KoXmlNS::office, "document-meta" );
        QDomNode office = KoDom::namedItemNS( meta, KoXmlNS::office, "meta" );

        if ( office.isNull() )
            return false;

        if ( !p->loadOasis( office ) )
            return false;
    }
    return true;
}

// KOffice-1.3 format
QDomDocument KoDocumentInfo::save()
{
    QDomDocument doc = KoDocument::createDomDocument( "document-info" /*DTD name*/, "document-info" /*tag name*/, "1.1" );
    QDomElement e = doc.documentElement();

    const QStringList lst = pages();
    QStringList::ConstIterator it = lst.begin();
    for( ; it != lst.end(); ++it )
    {
        KoDocumentInfoPage* p = page( *it );
        Q_ASSERT( p );
        QDomElement s = p->save( doc );
        if ( s.isNull() )
            continue;
        e.appendChild( s );
    }
    if ( e.isNull() )
      return QDomDocument();

    return doc;
}

bool KoDocumentInfo::saveOasis( KoStore* store )
{
    KoStoreDevice dev( store );
    KoXmlWriter* xmlWriter = KoDocument::createOasisXmlWriter( &dev, "office:document-meta" );
    xmlWriter->startElement( "office:meta" );

    xmlWriter->startElement( "meta:generator");
    xmlWriter->addTextNode( QString( "KOffice/%1" ).arg( KOFFICE_VERSION_STRING ) );
    xmlWriter->endElement();
    QStringList lst = pages();
    QStringList::ConstIterator it = lst.begin();
    for( ; it != lst.end(); ++it )
    {
        KoDocumentInfoPage* p = page( *it );
        Q_ASSERT( p );
        if ( !p->saveOasis( *xmlWriter ) )
            return false;
    }
    xmlWriter->endElement();
    xmlWriter->endElement(); // root element
    xmlWriter->endDocument();
    delete xmlWriter;
    return true;
}

KoDocumentInfoPage* KoDocumentInfo::page( const QString& name ) const
{
    QObject* obj = const_cast<KoDocumentInfo*>(this)->child( name.latin1() );

    return (KoDocumentInfoPage*)obj;
}

QStringList KoDocumentInfo::pages() const
{
    QStringList ret;

    const QList<QObject*> list = children();
    QList<QObject*>::ConstIterator it( list.begin() );
    QList<QObject*>::ConstIterator end( list.end() );
    while ( it != end )
    {
        ret.prepend( (*it)->name() );
        ++it;
    }

    return ret;
}

QString KoDocumentInfo::title() const
{
    KoDocumentInfoAbout * aboutPage = static_cast<KoDocumentInfoAbout *>(page( "about" ));
    if ( !aboutPage ) {
        kWarning() << "'About' page not found in documentInfo !" << endl;
        return QString::null;
    }
    else
        return aboutPage->title();
}

QString KoDocumentInfo::creator() const
{
    KoDocumentInfoAuthor * authorPage = static_cast<KoDocumentInfoAuthor *>(page( "author" ));
    if ( !authorPage ) {
        kWarning() << "'Author' page not found in documentInfo !" << endl;
        return QString::null;
    }
    else
        return authorPage->fullName();
}

/*****************************************
 *
 * KoDocumentInfoPage
 *
 *****************************************/

KoDocumentInfoPage::KoDocumentInfoPage( QObject* parent, const char* name )
    : QObject( parent, name )
{
}

/*****************************************
 *
 * KoDocumentInfoAuthor
 *
 *****************************************/

KoDocumentInfoAuthor::KoDocumentInfoAuthor( KoDocumentInfo* info )
    : KoDocumentInfoPage( info, "author" )
{
    initParameters();
}

KoDocumentInfoAuthor::~KoDocumentInfoAuthor()
{
    delete m_emailCfg;
}
void KoDocumentInfoAuthor::initParameters()
{
    KConfig* config = KoGlobal::kofficeConfig();
    if ( config->hasGroup( "Author" ) ) {
        KConfigGroup cgs( config, "Author" );
        m_telephoneHome=config->readEntry( "telephone" );
        m_telephoneWork=config->readEntry( "telephone-work" );
        m_fax=config->readEntry( "fax" );
        m_country=config->readEntry( "country" );
        m_postalCode=config->readEntry( "postal-code" );
        m_city=config->readEntry( "city" );
        m_street=config->readEntry( "street" );
    }

  m_emailCfg = new KConfig( "emaildefaults", true );
  m_emailCfg->setGroup( "Defaults" );
  QString group = m_emailCfg->readEntry("Profile","Default");
  m_emailCfg->setGroup(QString("PROFILE_%1").arg(group));

  if ( m_fullName.isNull() ) // only if null. Empty means the user made it explicitly empty.
  {
    QString name = m_emailCfg->readEntry( "FullName" );
    if ( !name.isEmpty() )
      m_fullName = name;
  }
  if ( m_company.isNull() )
  {
    QString name = m_emailCfg->readEntry( "Organization" );
    if ( !name.isEmpty() )
      m_company = name;
  }
}

bool KoDocumentInfoAuthor::saveOasis( KoXmlWriter &xmlWriter )
{
    if ( !m_fullName.isEmpty() )
    {
     xmlWriter.startElement( "dc:creator");
     xmlWriter.addTextNode( m_fullName );
     xmlWriter.endElement();
    }
    if ( !m_initial.isEmpty() )
    {
     xmlWriter.startElement( "meta:user-defined");
     xmlWriter.addAttribute( "meta:name", "initial" );
     xmlWriter.addTextNode( m_initial );
     xmlWriter.endElement();
    }
    if ( !m_title.isEmpty() )
    {
     xmlWriter.startElement( "meta:user-defined");
     xmlWriter.addAttribute( "meta:name", "author-title" );
     xmlWriter.addTextNode( m_title );
     xmlWriter.endElement();
    }
    if ( !m_company.isEmpty() )
    {
     xmlWriter.startElement( "meta:user-defined");
     xmlWriter.addAttribute( "meta:name", "company" );
     xmlWriter.addTextNode( m_company );
     xmlWriter.endElement();
    }
    if ( !m_email.isEmpty() )
    {
     xmlWriter.startElement( "meta:user-defined");
     xmlWriter.addAttribute( "meta:name", "email" );
     xmlWriter.addTextNode( m_email );
     xmlWriter.endElement();
    }
    if ( !m_telephoneHome.isEmpty() )
    {
     xmlWriter.startElement( "meta:user-defined");
     xmlWriter.addAttribute( "meta:name", "telephone" );
     xmlWriter.addTextNode( m_telephoneHome );
     xmlWriter.endElement();
    }
    if ( !m_telephoneWork.isEmpty() )
    {
     xmlWriter.startElement( "meta:user-defined");
     xmlWriter.addAttribute( "meta:name", "telephone-work" );
     xmlWriter.addTextNode( m_telephoneWork );
     xmlWriter.endElement();
    }
    if ( !m_fax.isEmpty() )
    {
     xmlWriter.startElement( "meta:user-defined");
     xmlWriter.addAttribute( "meta:name", "fax" );
     xmlWriter.addTextNode( m_fax );
     xmlWriter.endElement();
    }
    if ( !m_country.isEmpty() )
    {
     xmlWriter.startElement( "meta:user-defined");
     xmlWriter.addAttribute( "meta:name", "country" );
     xmlWriter.addTextNode( m_country );
     xmlWriter.endElement();
    }
    if ( !m_postalCode.isEmpty() )
    {
     xmlWriter.startElement( "meta:user-defined");
     xmlWriter.addAttribute( "meta:name", "postal-code" );
     xmlWriter.addTextNode( m_postalCode );
     xmlWriter.endElement();
    }
    if ( !m_city.isEmpty() )
    {
     xmlWriter.startElement( "meta:user-defined");
     xmlWriter.addAttribute( "meta:name", "city" );
     xmlWriter.addTextNode( m_city );
     xmlWriter.endElement();
    }
    if ( !m_street.isEmpty() )
    {
     xmlWriter.startElement( "meta:user-defined");
     xmlWriter.addAttribute( "meta:name", "street" );
     xmlWriter.addTextNode( m_street );
     xmlWriter.endElement();
    }
    if ( !m_position.isEmpty() )
    {
     xmlWriter.startElement( "meta:user-defined");
     xmlWriter.addAttribute( "meta:name", "position" );
     xmlWriter.addTextNode( m_position );
     xmlWriter.endElement();
    }
    return true;
}

bool KoDocumentInfoAuthor::loadOasis( const QDomNode& metaDoc )
{
    QDomElement e = KoDom::namedItemNS( metaDoc, KoXmlNS::dc, "creator" );
    if ( !e.isNull() && !e.text().isEmpty() )
        m_fullName = e.text();
    QDomNode n = metaDoc.firstChild();
    for ( ; !n.isNull(); n = n.nextSibling() )
    {
        if (n.isElement())
        {
            QDomElement e = n.toElement();
            if ( e.namespaceURI() == KoXmlNS::meta && e.localName() == "user-defined" && !e.text().isEmpty() )
            {
                QString name = e.attributeNS( KoXmlNS::meta, "name", QString::null );
                if ( name == "initial" )
                    m_initial = e.text();
                else if ( name == "author-title" )
                    m_title = e.text();
                else if ( name == "company" )
                    m_company = e.text();
                else if ( name == "email" )
                    m_email = e.text();
                else if ( name == "telephone" )
                    m_telephoneHome = e.text();
                else if ( name == "telephone-work" )
                    m_telephoneWork = e.text();
                else if ( name == "fax" )
                    m_fax = e.text();
                else if ( name == "country" )
                    m_country = e.text();
                else if ( name == "postal-code" )
                    m_postalCode = e.text();
                else if ( name == "city" )
                    m_city = e.text();
                else if ( name == "street" )
                    m_street = e.text();
                else if ( name == "position" )
                    m_position = e.text();
            }
        }
    }
    return true;
}

// KOffice-1.3 format
bool KoDocumentInfoAuthor::load( const QDomElement& e )
{
    QDomNode n = e.namedItem( "author" ).firstChild();
    for( ; !n.isNull(); n = n.nextSibling() )
    {
        QDomElement e = n.toElement();
        if ( e.isNull() ) continue;
        if ( e.tagName() == "full-name" )
            m_fullName = e.text();
        else if ( e.tagName() == "initial" )
            m_initial = e.text();
        else if ( e.tagName() == "title" )
            m_title = e.text();
        else if ( e.tagName() == "company" )
            m_company = e.text();
        else if ( e.tagName() == "email" )
            m_email = e.text();
        else if ( e.tagName() == "telephone" )
            m_telephoneHome = e.text();
        else if ( e.tagName() == "telephone-work" )
            m_telephoneWork = e.text();
        else if ( e.tagName() == "fax" )
            m_fax = e.text();
        else if ( e.tagName() == "country" )
            m_country = e.text();
        else if ( e.tagName() == "postal-code" )
            m_postalCode = e.text();
        else if ( e.tagName() == "city" )
            m_city = e.text();
        else if ( e.tagName() == "street" )
            m_street = e.text();
        else if ( e.tagName() == "position" )
            m_position = e.text();
    }
    return true;
}

QDomElement KoDocumentInfoAuthor::save( QDomDocument& doc )
{
    QDomElement e = doc.createElement( "author" );

    QDomElement t = doc.createElement( "full-name" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_fullName ) );

    t = doc.createElement( "initial" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_initial ) );


    t = doc.createElement( "title" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_title ) );

    t = doc.createElement( "company" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_company ) );

    t = doc.createElement( "email" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_email ) );

    t = doc.createElement( "telephone" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_telephoneHome ) );

    t = doc.createElement( "telephone-work" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_telephoneWork ) );

    t = doc.createElement( "fax" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_fax ) );

    t = doc.createElement( "country" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_country ) );

    t = doc.createElement( "postal-code" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_postalCode ) );

    t = doc.createElement( "city" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_city ) );

    t = doc.createElement( "street" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_street ) );

    t = doc.createElement( "position" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_position ) );

    return e;
}

QString KoDocumentInfoAuthor::fullName() const
{
    return m_fullName;
}

QString KoDocumentInfoAuthor::initial() const
{
    return m_initial;
}

QString KoDocumentInfoAuthor::title() const
{
    return m_title;
}

QString KoDocumentInfoAuthor::company() const
{
    return m_company;
}

QString KoDocumentInfoAuthor::email() const
{
    return m_email;
}

QString KoDocumentInfoAuthor::telephoneHome() const
{
    return m_telephoneHome;
}

QString KoDocumentInfoAuthor::telephoneWork() const
{
    return m_telephoneWork;
}

QString KoDocumentInfoAuthor::fax() const
{
    return m_fax;
}

QString KoDocumentInfoAuthor::country() const
{
    return m_country;
}

QString KoDocumentInfoAuthor::postalCode() const
{
    return m_postalCode;
}

QString KoDocumentInfoAuthor::city() const
{
    return m_city;
}

QString KoDocumentInfoAuthor::street() const
{
    return m_street;
}

QString KoDocumentInfoAuthor::position() const
{
    return m_position;
}

void KoDocumentInfoAuthor::setFullName( const QString& n )
{
    m_fullName = n;
}

void KoDocumentInfoAuthor::setInitial( const QString& n )
{
    m_initial = n;
}

void KoDocumentInfoAuthor::setTitle( const QString& n )
{
    m_title = n;
}

void KoDocumentInfoAuthor::setCompany( const QString& n )
{
    m_company = n;
}

void KoDocumentInfoAuthor::setEmail( const QString& n )
{
    m_email = n;
}

void KoDocumentInfoAuthor::setTelephoneHome( const QString& n )
{
    m_telephoneHome = n;
}

void KoDocumentInfoAuthor::setTelephoneWork( const QString& n )
{
    m_telephoneWork = n;
}

void KoDocumentInfoAuthor::setFax( const QString& n )
{
    m_fax = n;
}

void KoDocumentInfoAuthor::setCountry( const QString& n )
{
    m_country = n;
}

void KoDocumentInfoAuthor::setPostalCode( const QString& n )
{
    m_postalCode = n;
}

void KoDocumentInfoAuthor::setCity( const QString& n )
{
    m_city = n;
}

void KoDocumentInfoAuthor::setStreet( const QString& n )
{
    m_street = n;
}

void KoDocumentInfoAuthor::setPosition( const QString& n )
{
    m_position = n;
}


/*****************************************
 *
 * KoDocumentInfoAbout
 *
 *****************************************/

KoDocumentInfoAbout::KoDocumentInfoAbout( KoDocumentInfo* info )
    : KoDocumentInfoPage( info, "about" )
{
    m_docInfo = info;
    m_editingCycles = 0;
    m_initialCreator = m_docInfo->creator();
    m_creationDate = QDateTime::currentDateTime();
}

void KoDocumentInfoAbout::saveParameters()
{
    KoDocument* doc = dynamic_cast< KoDocument* >( m_docInfo->parent() );
    if ( doc && !doc->isAutosaving() )
       m_editingCycles++;
    m_modificationDate = QDateTime::currentDateTime();
}

bool KoDocumentInfoAbout::saveOasis( KoXmlWriter &xmlWriter )
{
    saveParameters();
    if ( !m_title.isEmpty() )
    {
     xmlWriter.startElement( "dc:title" );
     xmlWriter.addTextNode( m_title );
     xmlWriter.endElement();
    }
    if ( !m_abstract.isEmpty() )
    {
     xmlWriter.startElement( "dc:description" );
     xmlWriter.addTextNode( m_abstract );
     xmlWriter.endElement();
    }
    if ( !m_keywords.isEmpty() )
    {
     xmlWriter.startElement( "meta:keyword" );
     xmlWriter.addTextNode( m_keywords );
     xmlWriter.endElement();
    }
    if ( !m_subject.isEmpty() )
    {
     xmlWriter.startElement( "dc:subject" );
     xmlWriter.addTextNode( m_subject );
     xmlWriter.endElement();
    }
    if ( !m_initialCreator.isEmpty() )
    {
     xmlWriter.startElement( "meta:initial-creator" );
     xmlWriter.addTextNode( m_initialCreator );
     xmlWriter.endElement();
    }

    xmlWriter.startElement( "meta:editing-cycles" );
    xmlWriter.addTextNode( QString::number( m_editingCycles ) );
    xmlWriter.endElement();

    if ( m_creationDate.isValid() )
    {
        xmlWriter.startElement( "meta:creation-date" );
        xmlWriter.addTextNode( m_creationDate.toString( Qt::ISODate ) );
        xmlWriter.endElement();
    }

    if ( m_modificationDate.isValid() )
    {
        xmlWriter.startElement( "dc:date" );
        xmlWriter.addTextNode( m_modificationDate.toString( Qt::ISODate ) );
        xmlWriter.endElement();
    }
    return true;
}

bool KoDocumentInfoAbout::loadOasis( const QDomNode& metaDoc )
{
    QDomElement e  = KoDom::namedItemNS( metaDoc, KoXmlNS::dc, "title" );
    if ( !e.isNull() && !e.text().isEmpty() )
    {
        m_title = e.text();
    }
    e  = KoDom::namedItemNS( metaDoc, KoXmlNS::dc, "description" );
    if ( !e.isNull() && !e.text().isEmpty() )
    {
        m_abstract = e.text();
    }
    e  = KoDom::namedItemNS( metaDoc, KoXmlNS::dc, "subject" );
    if ( !e.isNull() && !e.text().isEmpty() )
    {
        m_subject = e.text();
    }
    e  = KoDom::namedItemNS( metaDoc, KoXmlNS::meta, "keyword" );
    if ( !e.isNull() && !e.text().isEmpty() )
    {
        m_keywords = e.text();
    }
    e = KoDom::namedItemNS( metaDoc, KoXmlNS::meta, "initial-creator" );
    if ( !e.isNull() && !e.text().isEmpty() )
        m_initialCreator = e.text();
    else
	m_initialCreator = i18n( "Unknown" );

    e = KoDom::namedItemNS( metaDoc, KoXmlNS::meta, "editing-cycles" );
    if ( !e.isNull() && !e.text().isEmpty() )
        m_editingCycles = e.text().toInt();

    e  = KoDom::namedItemNS( metaDoc, KoXmlNS::meta, "creation-date" );
    if ( !e.isNull() && !e.text().isEmpty() )
        m_creationDate = QDateTime::fromString( e.text(), Qt::ISODate );
    else
        m_creationDate = QDateTime();

    e  = KoDom::namedItemNS( metaDoc, KoXmlNS::dc, "date"  );
    if ( !e.isNull() && !e.text().isEmpty() )
        m_modificationDate = QDateTime::fromString( e.text(), Qt::ISODate );
    return true;
}

// KOffice-1.3 format
bool KoDocumentInfoAbout::load( const QDomElement& e )
{
    QDomNode n = e.namedItem( "about" ).firstChild();
    for( ; !n.isNull(); n = n.nextSibling()  )
    {
        QDomElement e = n.toElement();
        if ( e.isNull() ) continue;
        if ( e.tagName() == "abstract" )
            m_abstract = e.text();
        else if ( e.tagName() == "title" )
            m_title = e.text();
        else if ( e.tagName() == "subject" )
            m_subject = e.text();
        else if ( e.tagName() == "keyword" )
            m_keywords = e.text();
        else if ( e.tagName() == "initial-creator" )
            m_initialCreator = e.text();
        else if ( e.tagName() == "editing-cycles" )
            m_editingCycles = e.text().toInt();
        else if ( e.tagName() == "creation-date" )
            m_creationDate = QDateTime::fromString( e.text(), Qt::ISODate );
        else if ( e.tagName() == "date" )
            m_modificationDate = QDateTime::fromString( e.text(), Qt::ISODate );
    }

    return true;
}

// KOffice-1.3 format
QDomElement KoDocumentInfoAbout::save( QDomDocument& doc )
{
    saveParameters();
    QDomElement e = doc.createElement( "about" );

    QDomElement t = doc.createElement( "abstract" );
    e.appendChild( t );
    t.appendChild( doc.createCDATASection( m_abstract ) );

    t = doc.createElement( "title" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_title ) );

    t = doc.createElement( "keyword" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_keywords ) );

    t = doc.createElement( "subject" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_subject ) );

    t = doc.createElement( "initial-creator" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_initialCreator ) );

    t = doc.createElement( "editing-cycles" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( QString::number( m_editingCycles ) ) );

    t = doc.createElement( "creation-date" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_creationDate.toString( Qt::ISODate ) ) );

    t = doc.createElement( "date" );
    e.appendChild( t );
    t.appendChild( doc.createTextNode( m_modificationDate.toString( Qt::ISODate ) ) );
    return e;
}

QString KoDocumentInfoAbout::title() const
{
    return m_title;
}

QString KoDocumentInfoAbout::abstract() const
{
    return m_abstract;
}

QString KoDocumentInfoAbout::initialCreator() const
{
    return m_initialCreator;
}

QString KoDocumentInfoAbout::editingCycles() const
{
    return QString::number( m_editingCycles );
}

QString KoDocumentInfoAbout::creationDate() const
{
    if ( m_creationDate.isValid() )
        return KGlobal::locale()->formatDateTime( m_creationDate );
    else
        return QString::null;
}

QString KoDocumentInfoAbout::modificationDate() const
{
    if ( m_modificationDate.isValid() )
        return KGlobal::locale()->formatDateTime( m_modificationDate );
    else
        return QString::null;
}

void KoDocumentInfoAbout::setTitle( const QString& n )
{
    m_title = n;
}

void KoDocumentInfoAbout::setAbstract( const QString& n )
{
    m_abstract = n;
}

QString KoDocumentInfoAbout::keywords() const
{
    return m_keywords;
}

QString KoDocumentInfoAbout::subject() const
{
    return m_subject;
}

void KoDocumentInfoAbout::setKeywords( const QString& n )
{
    m_keywords = n;
}

void KoDocumentInfoAbout::setSubject( const QString& n )
{
    m_subject = n;
}

void KoDocumentInfoAbout::resetMetaData()
{
    m_editingCycles = 0;
    m_initialCreator = m_docInfo->creator();
    m_creationDate = QDateTime::currentDateTime();
    m_modificationDate = QDateTime();
}

/*****************************************
 *
 * KoDocumentInfoUserMetadata
 *
 *****************************************/

KoDocumentInfoUserMetadata::KoDocumentInfoUserMetadata( KoDocumentInfo* info )
    : KoDocumentInfoPage( info, "user_metadata" )
{
    m_reserved << "initial" << "author-title" << "company" << "email" << "telephone"
    << "telephone-work" << "fax" << "country" << "postal-code" << "city" << "street"
    << "position";
}

bool KoDocumentInfoUserMetadata::saveOasis( KoXmlWriter &xmlWriter )
{
  QMap<QString, QString>::iterator it;
  for ( it = m_metaList.begin(); it != m_metaList.end(); ++it )
  {
    xmlWriter.startElement( "meta:user-defined");
    xmlWriter.addAttribute( "meta:name", it.key() );
    xmlWriter.addTextNode( it.data() );
    xmlWriter.endElement();
  }
  return true;
}

bool KoDocumentInfoUserMetadata::loadOasis( const QDomNode& metaDoc )
{
    QDomNode n = metaDoc.firstChild();
    for ( ; !n.isNull(); n = n.nextSibling() )
    {
        if (n.isElement())
        {
            QDomElement e = n.toElement();
            if ( e.namespaceURI() == KoXmlNS::meta && e.localName() == "user-defined" && !e.text().isEmpty() )
            {
                QString name = e.attributeNS( KoXmlNS::meta, "name", QString::null );
                if ( !m_reserved.contains( name ) )
                    m_metaList[ name ] = e.text();
            }
        }
    }
    return true;
}

// KOffice-1.3 format
bool KoDocumentInfoUserMetadata::load( const QDomElement& )
{
    return true;
}

// KOffice-1.3 format
QDomElement KoDocumentInfoUserMetadata::save( QDomDocument& )
{
    return QDomElement();
}

#include <KoDocumentInfo.moc>
