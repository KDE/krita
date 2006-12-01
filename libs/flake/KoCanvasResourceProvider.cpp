/*
   Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)

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
#include <KoColor.h> // Zut, do we want this? It's convenient, but
                     // also makes flake dependent on pigment. (BSAR)

KoCanvasResourceProvider::KoCanvasResourceProvider(QObject * parent)
    : QObject( parent )
{
}

void KoCanvasResourceProvider::setResource( KoCanvasResource::EnumCanvasResource key, const QVariant & value )
{
    if ( m_resources.contains( key ) ) {
        m_resources[key] = value;
    }
    else {
        m_resources.insert( key, value );
    }
    emit sigResourceChanged( key, value );
}

QVariant KoCanvasResourceProvider::resource(KoCanvasResource::EnumCanvasResource key)
{
    if ( !m_resources.contains( key ) )
        return m_empty;
    else
        return m_resources.value( key );
}

void KoCanvasResourceProvider::setKoColor( KoCanvasResource::EnumCanvasResource key, const KoColor & color )
{
    QVariant v;
    v.setValue( color );
    setResource( key, v );
}

KoColor KoCanvasResourceProvider::koColor( KoCanvasResource::EnumCanvasResource key )
{
    return resource( key ).value<KoColor>();
}


void KoCanvasResourceProvider::setForegroundColor( const KoColor & color )
{
    QVariant v;
    v.setValue( color );
    setResource( KoCanvasResource::ForegroundColor, v );

}

KoColor KoCanvasResourceProvider::foregroundColor()
{
    return resource( KoCanvasResource::ForegroundColor ).value<KoColor>();
}


void KoCanvasResourceProvider::setBackgroundColor( const KoColor & color )
{
    QVariant v;
    v.setValue( color );
    setResource( KoCanvasResource::BackgroundColor, v );

}

KoColor KoCanvasResourceProvider::backgroundColor()
{
    return resource( KoCanvasResource::BackgroundColor ).value<KoColor>();
}


void KoCanvasResourceProvider::setKoID( KoCanvasResource::EnumCanvasResource key, const KoID & id )
{
    QVariant  v;
    v.setValue( id );
    setResource( key, v );

}

KoID KoCanvasResourceProvider::koID(KoCanvasResource::EnumCanvasResource key)
{
    return resource( key ).value<KoID>();
}


#include "KoCanvasResourceProvider.moc"
