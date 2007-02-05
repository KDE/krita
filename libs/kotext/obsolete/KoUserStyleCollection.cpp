/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#include "KoUserStyleCollection.h"
#include "KoUserStyle.h"
#include <kdebug.h>




KoUserStyleCollection::KoUserStyleCollection( const QString& prefix )
    : m_prefix( prefix ),
      m_lastStyle( 0 ),
      m_default( false /*to be safe*/ )
{
}

KoUserStyle* KoUserStyleCollection::findStyle( const QString & _name, const QString& defaultStyleName ) const
{
    // Caching, to speed things up
    if ( m_lastStyle && m_lastStyle->name() == _name )
        return m_lastStyle;

    KoUserStyle* style = 0;
    foreach( style, m_styleList )
    {
        if ( style->name() == _name )
       	{
            m_lastStyle = style;
            return m_lastStyle;
        }
    }

    if( !defaultStyleName.isEmpty() && _name == defaultStyleName && !m_styleList.isEmpty() )
        return m_styleList.first(); // fallback..

    return 0;
}

KoUserStyle* KoUserStyleCollection::findStyleByDisplayName( const QString& displayName ) const
{
    if ( m_lastStyle && m_lastStyle->displayName() == displayName )
        return m_lastStyle;

    KoUserStyle* style = 0;
    foreach( style, m_styleList )
    {
        if ( style->displayName() == displayName )
       	{
            m_lastStyle = style;
            return m_lastStyle;
        }
    }

    return 0;
}

QString KoUserStyleCollection::generateUniqueName() const
{
    int count = 1;
    QString name;
    do {
        name = m_prefix + QString::number( count++ );
    } while ( findStyle( name, QString::null ) );
    return name;
}

KoUserStyleCollection::~KoUserStyleCollection()
{
    clear();
}

void KoUserStyleCollection::clear()
{
    KoUserStyle* style = 0;
    foreach( style, m_styleList )
	delete style;
    foreach( style, m_deletedStyles )
	delete style;
    
    m_styleList.clear();
    m_deletedStyles.clear();
    m_lastStyle = 0;
}

QStringList KoUserStyleCollection::displayNameList() const
{
    QStringList lst;
    KoUserStyle* style = 0;
    foreach( style, m_styleList )
       lst.append( style->displayName() );
    
    return lst;
}

KoUserStyle* KoUserStyleCollection::addStyle( KoUserStyle* sty )
{
    // First check for duplicates.
    KoUserStyle* p = 0;
    foreach( p, m_styleList )
    {
        if ( p->name() == sty->name() ) {
            if ( p->displayName() == sty->displayName() ) {
                // Replace existing style
                if ( sty != p )
                {
                    *p = *sty;
                    delete sty;
                }
                return p;
            } else { // internal name conflict, but it's not the same style as far as the user is concerned
                sty->setName( generateUniqueName() );
            }
        }
    }
    m_styleList.append( sty );
    return sty;
}

void KoUserStyleCollection::removeStyle ( KoUserStyle *style ) {
    if( m_styleList.removeAll(style)) {
        if ( m_lastStyle == style )
            m_lastStyle = 0;
        // Remember to delete this style when deleting the document
        m_deletedStyles.append(style);
    }
}

void KoUserStyleCollection::updateStyleListOrder( const QStringList &lst )
{
    QList<KoUserStyle *> orderStyle;
    QString tmpString;
    KoUserStyle* style = 0;
    foreach( tmpString, lst )
    {
	style = findStyle( tmpString, QString::null );
        if( style )
	    orderStyle.append( style );
        else
            kWarning(32500) << "updateStyleListOrder: style " << tmpString << " not found!" << endl;
    }
    // we'll lose (and leak) styles if the list didn't have all the styles
    Q_ASSERT( m_styleList.count() == orderStyle.count() );
    m_styleList.clear();
    m_styleList = orderStyle;
}
