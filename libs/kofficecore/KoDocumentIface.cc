/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoDocument.h"
#include "KoDocumentIface.h"
#include "KoDocumentInfoDlg.h"
#include "KoDocumentInfo.h"
#include "KoView.h"
#include <kapplication.h>
#include <dcopclient.h>
#include <kdcopactionproxy.h>
#include <kaction.h>
#include <kdebug.h>
#include <kdcoppropertyproxy.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3PtrList>

//static
DCOPCString KoDocumentIface::newIfaceName()
{
    static int s_docIFNumber = 0;
    DCOPCString name; name.setNum( s_docIFNumber++ ); name.prepend("Document-");
    return name;
}

KoDocumentIface::KoDocumentIface( KoDocument * doc, const char * name )
    : DCOPObject( name ? DCOPCString(name) : newIfaceName() )
{
  m_pDoc = doc;
  m_actionProxy = new KDCOPActionProxy( doc->actionCollection(), this );
}

KoDocumentIface::~KoDocumentIface()
{
    delete m_actionProxy;
}

void KoDocumentIface::openURL( QString url )
{
  m_pDoc->openURL( KUrl( url ) );
}

bool KoDocumentIface::isLoading()
{
  return m_pDoc->isLoading();
}

QString KoDocumentIface::url()
{
  return m_pDoc->url().url();
}

bool KoDocumentIface::isModified()
{
  return m_pDoc->isModified();
}

int KoDocumentIface::viewCount()
{
  return m_pDoc->viewCount();
}

DCOPRef KoDocumentIface::view( int idx )
{
  Q3PtrList<KoView> views = m_pDoc->views();
  KoView *v = views.at( idx );
  if ( !v )
    return DCOPRef();

  DCOPObject *obj = v->dcopObject();

  if ( !obj )
    return DCOPRef();

  return DCOPRef( kapp->dcopClient()->appId(), obj->objId() );
}

DCOPRef KoDocumentIface::action( const DCOPCString &name )
{
    return DCOPRef( kapp->dcopClient()->appId(), m_actionProxy->actionObjectId( name ) );
}

DCOPCStringList KoDocumentIface::actions()
{
    DCOPCStringList res;
    Q3ValueList<KAction *> lst = m_actionProxy->actions();
    Q3ValueList<KAction *>::ConstIterator it = lst.begin();
    Q3ValueList<KAction *>::ConstIterator end = lst.end();
    for (; it != end; ++it )
        res.append( (*it)->objectName().toUtf8() );

    return res;
}

QMap<DCOPCString,DCOPRef> KoDocumentIface::actionMap()
{
    return m_actionProxy->actionMap();
}

void KoDocumentIface::save()
{
    m_pDoc->save();
}

void KoDocumentIface::saveAs( const QString & url )
{
    m_pDoc->saveAs( KUrl( url ) );
    m_pDoc->waitSaveComplete(); // see ReadWritePart
}

void KoDocumentIface::setOutputMimeType( const QByteArray& mimetype )
{
    m_pDoc->setOutputMimeType( mimetype );
}

QString KoDocumentIface::documentInfoAuthorName() const
{
    return m_pDoc->documentInfo()->authorInfo( "creator" );
}

QString KoDocumentIface::documentInfoEmail() const
{
    return m_pDoc->documentInfo()->authorInfo( "email" );
}

QString KoDocumentIface::documentInfoCompanyName() const
{
    return m_pDoc->documentInfo()->authorInfo( "company" );
}

QString KoDocumentIface::documentInfoTelephone() const
{
    kDebug()<<" Keep compatibility with koffice <= 1.3 : use documentInfoTelephoneWork\n";
    return documentInfoTelephoneWork();
}

QString KoDocumentIface::documentInfoTelephoneWork() const
{
    return m_pDoc->documentInfo()->authorInfo( "telephone-work" );
}

QString KoDocumentIface::documentInfoTelephoneHome() const
{
    return m_pDoc->documentInfo()->authorInfo( "telephone-home" );
}


QString KoDocumentIface::documentInfoFax() const
{
    return m_pDoc->documentInfo()->authorInfo( "fax" );

}
QString KoDocumentIface::documentInfoCountry() const
{
    return m_pDoc->documentInfo()->authorInfo( "country" );

}
QString KoDocumentIface::documentInfoPostalCode() const
{
    return m_pDoc->documentInfo()->authorInfo( "postal-code" );

}
QString KoDocumentIface::documentInfoCity() const
{
    return m_pDoc->documentInfo()->authorInfo( "city" );
}

QString KoDocumentIface::documentInfoInitial() const
{
    return m_pDoc->documentInfo()->authorInfo( "initial" );
}

QString KoDocumentIface::documentInfoAuthorPostion() const
{
    return m_pDoc->documentInfo()->authorInfo( "position" );
}

QString KoDocumentIface::documentInfoStreet() const
{
    return m_pDoc->documentInfo()->authorInfo( "street" );
}

QString KoDocumentIface::documentInfoTitle() const
{
    return m_pDoc->documentInfo()->aboutInfo( "title" );
}

QString KoDocumentIface::documentInfoAbstract() const
{
    return m_pDoc->documentInfo()->aboutInfo( "comments" );
}

QString KoDocumentIface::documentInfoKeywords() const
{
    return m_pDoc->documentInfo()->aboutInfo( "keywords" );
}

QString KoDocumentIface::documentInfoSubject() const
{
    return m_pDoc->documentInfo()->aboutInfo( "subject" );
}
void KoDocumentIface::setDocumentInfoKeywords(const QString & text )
{
    m_pDoc->documentInfo()->setAboutInfo( "keywords", text );
}

void KoDocumentIface::setDocumentInfoSubject(const QString & text)
{
    m_pDoc->documentInfo()->setAboutInfo( "subject", text );
}

void KoDocumentIface::setDocumentInfoAuthorName(const QString & text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "creator", text );
}

void KoDocumentIface::setDocumentInfoEmail(const QString &text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "email", text );
}

void KoDocumentIface::setDocumentInfoCompanyName(const QString &text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "company", text );
}

void KoDocumentIface::setDocumentInfoAuthorPosition(const QString &text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "position", text );
}


void KoDocumentIface::setDocumentInfoTelephone(const QString &text)
{
    kDebug()<<"Keep compatibility with koffice <= 1.3 : use setDocumentInfoTelephoneWork\n";
    setDocumentInfoTelephoneWork(text);
}

void KoDocumentIface::setDocumentInfoTelephoneWork(const QString &text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "telephone-work", text );
}

void KoDocumentIface::setDocumentInfoTelephoneHome(const QString &text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "telephone", text );
}

void KoDocumentIface::setDocumentInfoFax(const QString &text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "fax", text );
}

void KoDocumentIface::setDocumentInfoCountry(const QString &text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "country", text );
}

void KoDocumentIface::setDocumentInfoTitle(const QString & text)
{
    m_pDoc->documentInfo()->setAboutInfo( "title", text );
}

void KoDocumentIface::setDocumentInfoPostalCode(const QString &text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "postal-code", text );
}

void KoDocumentIface::setDocumentInfoCity(const QString & text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "city", text );
}

void KoDocumentIface::setDocumentInfoInitial(const QString & text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "initial", text );
}

void KoDocumentIface::setDocumentInfoStreet(const QString &text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "street", text );
}

void KoDocumentIface::setDocumentInfoAbstract(const QString &text)
{
    m_pDoc->documentInfo()->setAboutInfo( "comments", text );
}

DCOPCStringList KoDocumentIface::functionsDynamic()
{
    return DCOPObject::functionsDynamic() + KDCOPPropertyProxy::functions( m_pDoc );
}

bool KoDocumentIface::processDynamic( const DCOPCString &fun, const QByteArray &data,
                                      DCOPCString& replyType, QByteArray &replyData )
{
    if ( KDCOPPropertyProxy::isPropertyRequest( fun, m_pDoc ) )
        return KDCOPPropertyProxy::processPropertyRequest( fun, data, replyType, replyData, m_pDoc );

    return DCOPObject::processDynamic( fun, data, replyType, replyData );
}

