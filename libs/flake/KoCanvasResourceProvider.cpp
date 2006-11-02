/*
 *  Copyright (c) 2006 Boudewijn Rempt
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "KoCanvasResourceProvider.h"

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

#include "KoCanvasResourceProvider.moc"
