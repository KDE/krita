/* This file is part of the KDE project
   Copyright (c) 2004 David Faure <faure@kde.org>

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

#include "KoListStyleStack.h"
#include <KoDom.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>

KoListStyleStack::KoListStyleStack()
    : m_initialLevel( 0 )
{
}

KoListStyleStack::~KoListStyleStack()
{
}

void KoListStyleStack::pop()
{
    m_stack.pop();
}

void KoListStyleStack::push( const KoXmlElement& style )
{
    m_stack.push( style );
}

void KoListStyleStack::setInitialLevel( int initialLevel )
{
    Q_ASSERT( m_stack.isEmpty() );
    m_initialLevel = initialLevel;
}

KoXmlElement KoListStyleStack::currentListStyle() const
{
    Q_ASSERT( !m_stack.isEmpty() );
    return m_stack.top();
}

KoXmlElement KoListStyleStack::currentListStyleProperties() const
{
    KoXmlElement style = currentListStyle();
    return KoDom::namedItemNS( style, KoXmlNS::style, "list-level-properties" );
}

KoXmlElement KoListStyleStack::currentListStyleTextProperties() const
{
    KoXmlElement style = currentListStyle();
    return KoDom::namedItemNS( style, KoXmlNS::style, "text-properties" );
}
