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

KoPASavingContext::KoPASavingContext( KoXmlWriter &xmlWriter, KoSavingContext &context, int page )
: KoShapeSavingContext( xmlWriter, context )
, m_page( page )    
{
}

KoPASavingContext::~KoPASavingContext()
{
}

void KoPASavingContext::addMasterPage( const KoPAMasterPage * masterPage, QString name )
{
    m_masterPageNames.insert( masterPage, name );
}

QString KoPASavingContext::masterPageName( const KoPAMasterPage * masterPage )
{
    QMap<const KoPAMasterPage *, QString>::const_iterator it( m_masterPageNames.find( masterPage ) );
    if (  it != m_masterPageNames.constEnd() ) {
        return it.value();
    }

    // this should not happen
    Q_ASSERT( it != m_masterPageNames.constEnd() );
    return QString();
}

void KoPASavingContext::incrementPage() 
{
    m_page++;
}

int KoPASavingContext::page()
{
    return m_page;
}
