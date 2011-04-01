/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoPASavingContext.h"

#include <QPixmap>
#include <QRegExp>

#include "KoPAPage.h"

KoPASavingContext::KoPASavingContext(KoXmlWriter &xmlWriter, KoGenStyles& mainStyles,
        KoEmbeddedDocumentSaver &embeddedSaver, int page)
    : KoShapeSavingContext(xmlWriter, mainStyles, embeddedSaver)
, m_page( page )
, m_masterPageIndex( 0 )
, m_clearDrawIds( false )
{
}

KoPASavingContext::~KoPASavingContext()
{
}

void KoPASavingContext::addMasterPage( const KoPAMasterPage * masterPage, const QString &name )
{
    m_masterPageNames.insert( masterPage, name );
}

QString KoPASavingContext::masterPageName( const KoPAMasterPage * masterPage ) const
{
    QMap<const KoPAMasterPage *, QString>::const_iterator it( m_masterPageNames.find( masterPage ) );
    if (  it != m_masterPageNames.constEnd() ) {
        return it.value();
    }

    // this should not happen
    Q_ASSERT( it != m_masterPageNames.constEnd() );
    return QString();
}

QString KoPASavingContext::masterPageElementName()
{
    if ( ! isSet( KoShapeSavingContext::UniqueMasterPages ) ) {
        ++m_masterPageIndex;
    }
    return QString( "content_%1" ).arg( m_masterPageIndex );
}

void KoPASavingContext::incrementPage()
{
    m_page++;
}

int KoPASavingContext::page()
{
    return m_page;
}

void KoPASavingContext::setClearDrawIds( bool clear )
{
    m_clearDrawIds = clear;
}

bool KoPASavingContext::isSetClearDrawIds()
{
    return m_clearDrawIds;
}

QString KoPASavingContext::pageName( const KoPAPage * page )
{
    QString name;
    QMap<const KoPAPage *, QString>::iterator it( m_pageToNames.find( page ) );
    if ( it != m_pageToNames.end() ) {
        name = it.value();
    }
    else {
        name = page->name();
        QRegExp rx( "^page[0-9]+$" );
        if ( name.isEmpty() || m_pageNames.contains( name ) || rx.indexIn( name ) != -1 ) {
            name = "page" + QString::number( m_page );
        }
        Q_ASSERT( !m_pageNames.contains( name ) );
        m_pageNames.insert( name );
        m_pageToNames.insert( page, name );
    }
    return name;
}
