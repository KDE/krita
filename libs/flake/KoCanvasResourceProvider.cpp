/*
 *  Copyright (c) 2006 Boudewijn Rempt

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
   Boston, MA 02110-1301, USA.
 */
#include "KoCanvasResourceProvider.h"

#include <QVariant>
#include <KoColor.h>

KoCanvasResourceProvider::KoCanvasResourceProvider(QObject * parent)
    : QObject( parent )
{
}

void KoCanvasResourceProvider::setResource( enumCanvasResource key, const QVariant & value )
{
    KoCanvasResource r ( key, value );
    setResource( r );
}

void KoCanvasResourceProvider::setResource( KoCanvasResource & res)
{
    if ( m_resources.contains( res.key ) ) {
        m_resources[res.key] = res;
    }
    else {
        m_resources.insert( res.key, res );
    }
    emit sigResourceChanged( res );
}

QVariant KoCanvasResourceProvider::resource(enumCanvasResource key)
{
    if ( !m_resources.contains( key ) )
        return m_empty;
    else
        return m_resources.value( key ).value;
}

void KoCanvasResourceProvider::setKoColor( enumCanvasResource key, const KoColor & color )
{
    QVariant v;
    v.setValue( color );
    setResource( key, v );
}

KoColor KoCanvasResourceProvider::koColor( enumCanvasResource key )
{
    return resource( key ).value<KoColor>();
}


void KoCanvasResourceProvider::setForegroundColor( const KoColor & color )
{
    QVariant v;
    v.setValue( color );
    setResource( FOREGROUND_COLOR, v );

}

KoColor KoCanvasResourceProvider::foregroundColor()
{
    return resource( FOREGROUND_COLOR ).value<KoColor>();
}


void KoCanvasResourceProvider::setBackgroundColor( const KoColor & color )
{
    QVariant v;
    v.setValue( color );
    setResource( BACKGROUND_COLOR, v );

}

KoColor KoCanvasResourceProvider::backgroundColor()
{
    return resource( BACKGROUND_COLOR ).value<KoColor>();
}


void KoCanvasResourceProvider::setKoID( enumCanvasResource key, const KoID & id )
{
    QVariant  v;
    v.setValue( id );
    setResource( key, v );

}

KoID KoCanvasResourceProvider::koID(enumCanvasResource key)
{
    return resource( key ).value<KoID>();
}


#include "KoCanvasResourceProvider.moc"
