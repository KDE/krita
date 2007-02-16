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

class KoCanvasResourceProvider::Private {
public:
    QHash<int, QVariant> resources;
};

KoCanvasResourceProvider::KoCanvasResourceProvider(QObject * parent)
    : QObject( parent ),
    d( new Private() )
{
    // initialize handle radius to a sane value
    setHandleRadius( 3 );
}

void KoCanvasResourceProvider::setResource( int key, const QVariant & value )
{
    if ( d->resources.contains( key ) ) {
        d->resources[key] = value;
    }
    else {
        d->resources.insert( key, value );
    }
    emit sigResourceChanged( key, value );
}

QVariant KoCanvasResourceProvider::resource(int key)
{
    if ( !d->resources.contains( key ) ) {
        QVariant empty;
        return empty;
    }
    else
        return d->resources.value( key );
}

void KoCanvasResourceProvider::setResource( int key, const KoColor & color )
{
    QVariant v;
    v.setValue( color );
    setResource( key, v );
}

void KoCanvasResourceProvider::setResource( int key, const KoID & id )
{
    QVariant v;
    v.setValue( id );
    setResource( key, v );
}

KoColor KoCanvasResourceProvider::koColorResource( int key )
{
    if(! d->resources.contains(key)) {
        KoColor empty;
        return empty;
    }
    return resource( key ).value<KoColor>();
}


void KoCanvasResourceProvider::setForegroundColor( const KoColor & color )
{
    //QVariant v;
    //v.setValue( color );
    setResource( KoCanvasResource::ForegroundColor, color );
}

KoColor KoCanvasResourceProvider::foregroundColor()
{
    return koColorResource( KoCanvasResource::ForegroundColor );
}


void KoCanvasResourceProvider::setBackgroundColor( const KoColor & color )
{
    //QVariant v;
    //v.setValue( color );
    setResource( KoCanvasResource::BackgroundColor, color );

}

KoColor KoCanvasResourceProvider::backgroundColor()
{
    return koColorResource( KoCanvasResource::BackgroundColor );
}

KoID KoCanvasResourceProvider::koIDResource(int key)
{
    return resource( key ).value<KoID>();
}

void KoCanvasResourceProvider::setHandleRadius( int handleRadius )
{
    // do not allow arbitrary small handles
    if( handleRadius < 3 )
        handleRadius = 3;
    setResource( KoCanvasResource::HandleRadius, QVariant( handleRadius) );
}

int KoCanvasResourceProvider::handleRadius()
{
    return resource( KoCanvasResource::HandleRadius ).toInt();
}

bool KoCanvasResourceProvider::boolResource(int key) const {
    if(! d->resources.contains(key))
        return false;
    return d->resources[key].toBool();
}

bool KoCanvasResourceProvider::intResource(int key) const {
    if(! d->resources.contains(key))
        return 0;
    return d->resources[key].toInt();
}

bool KoCanvasResourceProvider::doubleResource(int key) const {
    if(! d->resources.contains(key))
        return 0.;
    return d->resources[key].toDouble();
}

QString KoCanvasResourceProvider::stringResource(int key)
{
    if(! d->resources.contains(key)) {
        QString empty;
        return empty;
    }
    return qvariant_cast<QString>(resource(key));
}

#include "KoCanvasResourceProvider.moc"
