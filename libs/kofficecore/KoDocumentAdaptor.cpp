/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2006 Fredrik Edemar <f_edemar@linux.se>

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
#include "KoDocumentAdaptor.h"
#include "KoDocumentInfoDlg.h"
#include "KoDocumentInfo.h"
#include "KoView.h"
#include <kapplication.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kdebug.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3PtrList>


KoDocumentAdaptor::KoDocumentAdaptor( KoDocument * doc )
    : QDBusAbstractAdaptor( doc )
{
    setAutoRelaySignals(true);
    m_pDoc = doc;
//   m_actionProxy = new KDCOPActionProxy( doc->actionCollection(), this );
}

KoDocumentAdaptor::~KoDocumentAdaptor()
{
//     delete m_actionProxy;
}

void KoDocumentAdaptor::openURL( QString url )
{
  m_pDoc->openURL( KUrl( url ) );
}

bool KoDocumentAdaptor::isLoading()
{
  return m_pDoc->isLoading();
}

QString KoDocumentAdaptor::url()
{
  return m_pDoc->url().url();
}

bool KoDocumentAdaptor::isModified()
{
  return m_pDoc->isModified();
}

int KoDocumentAdaptor::viewCount()
{
  return m_pDoc->viewCount();
}

QString KoDocumentAdaptor::view( int idx )
{
  QList<KoView*> views = m_pDoc->views();
  KoView *v = views.at( idx );
  if ( !v )
    return QString();

  return v->objectName();
}

// DCOPRef KoDocumentAdaptor::action( const DCOPCString &name )
// {
//     return DCOPRef( kapp->dcopClient()->appId(), m_actionProxy->actionObjectId( name ) );
// }

QStringList KoDocumentAdaptor::actions()
{
//     DCOPCStringList res;
//     Q3ValueList<KAction *> lst = m_actionProxy->actions();
//     Q3ValueList<KAction *>::ConstIterator it = lst.begin();
//     Q3ValueList<KAction *>::ConstIterator end = lst.end();
//     for (; it != end; ++it )
//         res.append( (*it)->objectName().toUtf8() );
// 
//     return res;
    QStringList tmp_actions;
    QList<KAction *> lst = m_pDoc->actionCollection()->actions();
    foreach( KAction* it, lst ) {
        if (it->isPlugged())
            tmp_actions.append( it->objectName() );
    }
    return tmp_actions;
}

// QMap<DCOPCString,DCOPRef> KoDocumentAdaptor::actionMap()
// {
//     return m_actionProxy->actionMap();
// }

void KoDocumentAdaptor::save()
{
    m_pDoc->save();
}

void KoDocumentAdaptor::saveAs( const QString & url )
{
    m_pDoc->saveAs( KUrl( url ) );
    m_pDoc->waitSaveComplete(); // see ReadWritePart
}

void KoDocumentAdaptor::setOutputMimeType( const QByteArray& mimetype )
{
    m_pDoc->setOutputMimeType( mimetype );
}

QString KoDocumentAdaptor::documentInfoAuthorName() const
{
    return m_pDoc->documentInfo()->authorInfo( "creator" );
}

QString KoDocumentAdaptor::documentInfoEmail() const
{
    return m_pDoc->documentInfo()->authorInfo( "email" );
}

QString KoDocumentAdaptor::documentInfoCompanyName() const
{
    return m_pDoc->documentInfo()->authorInfo( "company" );
}

QString KoDocumentAdaptor::documentInfoTelephone() const
{
    kDebug()<<" Keep compatibility with koffice <= 1.3 : use documentInfoTelephoneWork\n";
    return documentInfoTelephoneWork();
}

QString KoDocumentAdaptor::documentInfoTelephoneWork() const
{
    return m_pDoc->documentInfo()->authorInfo( "telephone-work" );
}

QString KoDocumentAdaptor::documentInfoTelephoneHome() const
{
    return m_pDoc->documentInfo()->authorInfo( "telephone-home" );
}


QString KoDocumentAdaptor::documentInfoFax() const
{
    return m_pDoc->documentInfo()->authorInfo( "fax" );

}
QString KoDocumentAdaptor::documentInfoCountry() const
{
    return m_pDoc->documentInfo()->authorInfo( "country" );

}
QString KoDocumentAdaptor::documentInfoPostalCode() const
{
    return m_pDoc->documentInfo()->authorInfo( "postal-code" );

}
QString KoDocumentAdaptor::documentInfoCity() const
{
    return m_pDoc->documentInfo()->authorInfo( "city" );
}

QString KoDocumentAdaptor::documentInfoInitial() const
{
    return m_pDoc->documentInfo()->authorInfo( "initial" );
}

QString KoDocumentAdaptor::documentInfoAuthorPostion() const
{
    return m_pDoc->documentInfo()->authorInfo( "position" );
}

QString KoDocumentAdaptor::documentInfoStreet() const
{
    return m_pDoc->documentInfo()->authorInfo( "street" );
}

QString KoDocumentAdaptor::documentInfoTitle() const
{
    return m_pDoc->documentInfo()->aboutInfo( "title" );
}

QString KoDocumentAdaptor::documentInfoAbstract() const
{
    return m_pDoc->documentInfo()->aboutInfo( "comments" );
}

QString KoDocumentAdaptor::documentInfoKeywords() const
{
    return m_pDoc->documentInfo()->aboutInfo( "keywords" );
}

QString KoDocumentAdaptor::documentInfoSubject() const
{
    return m_pDoc->documentInfo()->aboutInfo( "subject" );
}
void KoDocumentAdaptor::setDocumentInfoKeywords(const QString & text )
{
    m_pDoc->documentInfo()->setAboutInfo( "keywords", text );
}

void KoDocumentAdaptor::setDocumentInfoSubject(const QString & text)
{
    m_pDoc->documentInfo()->setAboutInfo( "subject", text );
}

void KoDocumentAdaptor::setDocumentInfoAuthorName(const QString & text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "creator", text );
}

void KoDocumentAdaptor::setDocumentInfoEmail(const QString &text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "email", text );
}

void KoDocumentAdaptor::setDocumentInfoCompanyName(const QString &text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "company", text );
}

void KoDocumentAdaptor::setDocumentInfoAuthorPosition(const QString &text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "position", text );
}


void KoDocumentAdaptor::setDocumentInfoTelephone(const QString &text)
{
    kDebug()<<"Keep compatibility with koffice <= 1.3 : use setDocumentInfoTelephoneWork\n";
    setDocumentInfoTelephoneWork(text);
}

void KoDocumentAdaptor::setDocumentInfoTelephoneWork(const QString &text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "telephone-work", text );
}

void KoDocumentAdaptor::setDocumentInfoTelephoneHome(const QString &text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "telephone", text );
}

void KoDocumentAdaptor::setDocumentInfoFax(const QString &text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "fax", text );
}

void KoDocumentAdaptor::setDocumentInfoCountry(const QString &text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "country", text );
}

void KoDocumentAdaptor::setDocumentInfoTitle(const QString & text)
{
    m_pDoc->documentInfo()->setAboutInfo( "title", text );
}

void KoDocumentAdaptor::setDocumentInfoPostalCode(const QString &text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "postal-code", text );
}

void KoDocumentAdaptor::setDocumentInfoCity(const QString & text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "city", text );
}

void KoDocumentAdaptor::setDocumentInfoInitial(const QString & text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "initial", text );
}

void KoDocumentAdaptor::setDocumentInfoStreet(const QString &text)
{
    m_pDoc->documentInfo()->setAuthorInfo( "street", text );
}

void KoDocumentAdaptor::setDocumentInfoAbstract(const QString &text)
{
    m_pDoc->documentInfo()->setAboutInfo( "comments", text );
}

// DCOPCStringList KoDocumentAdaptor::functionsDynamic()
// {
//     return DCOPObject::functionsDynamic() + KDCOPPropertyProxy::functions( m_pDoc );
// }
// 
// bool KoDocumentAdaptor::processDynamic( const DCOPCString &fun, const QByteArray &data,
//                                       DCOPCString& replyType, QByteArray &replyData )
// {
//     if ( KDCOPPropertyProxy::isPropertyRequest( fun, m_pDoc ) )
//         return KDCOPPropertyProxy::processPropertyRequest( fun, data, replyType, replyData, m_pDoc );
// 
//     return DCOPObject::processDynamic( fun, data, replyType, replyData );
// }

#include "KoDocumentAdaptor.moc"
