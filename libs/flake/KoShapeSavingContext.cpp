/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
 
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or ( at your option ) any later version.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoShapeSavingContext.h"

#include <KoGenStyles.h>
#include <KoSavingContext.h>

KoShapeSavingContext::KoShapeSavingContext( KoXmlWriter &xmlWriter, KoSavingContext &context )
: m_xmlWriter( xmlWriter )
, m_context( context )
, m_savingOptions( 0 )
, m_drawId( 0 )
{
}

KoShapeSavingContext::~KoShapeSavingContext() 
{
}

KoXmlWriter & KoShapeSavingContext::xmlWriter() 
{
    return m_xmlWriter; 
}

KoGenStyles & KoShapeSavingContext::mainStyles() 
{
    return m_context.mainStyles(); 
}

bool KoShapeSavingContext::isSet( KoShapeSavingOption option ) const
{
    return m_savingOptions && option;
}

const QString & KoShapeSavingContext::drawId( KoShape * shape, bool insert )
{
    QMap<KoShape *, QString>::const_iterator it( m_drawIds.find( shape ) );
    if ( it == m_drawIds.constEnd() && insert == true )
    {
        it = m_drawIds.insert( shape, QString( "shape" ).arg( ++m_drawId ) );
        return it.value();
    }
    return m_emptyString;
}
