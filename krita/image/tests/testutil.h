/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef TEST_UTIL
#define TEST_UTIL

#include <QList>
#include <QTime>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
/**
 * Routines that are useful for writing efficient tests
 */

namespace TestUtil {

bool compareQImages( QPoint & pt, const QImage & img1, const QImage & img2 )
{
//     QTime t;
//     t.start();

    int w1 = img1.width();
    int h1 = img1.height();
    int w2 = img2.width();
    int h2 = img2.height();

    if ( w1 != w2 || h1 != h2 ) {
        pt.setX( -1 );
        pt.setY( -1 );
        return false;
    }

    for ( int x = 0; x < w1; ++x ) {
        for ( int y = 0; y < h1; ++y ) {
            if ( img1.pixel(x, y) != img2.pixel( x, y ) ) {
                pt.setX( x );
                pt.setY( y );
                return false;
            }
        }
    }
//     qDebug() << "compareQImages time elapsed:" << t.elapsed();
    return true;
}

bool comparePaintDevices( QPoint & pt, const KisPaintDeviceSP dev1, const KisPaintDeviceSP dev2 )
{
//     QTime t;
//     t.start();

    QRect rc1 = dev1->exactBounds();
    QRect rc2 = dev2->exactBounds();

    if ( rc1 != rc2 ) {
        pt.setX( -1 );
        pt.setY( -1 );
    }

    KisHLineConstIteratorPixel iter1 = dev1->createHLineConstIterator( 0, 0, rc1.width() );
    KisHLineConstIteratorPixel iter2 = dev2->createHLineConstIterator( 0, 0, rc1.width() );

    int pixelSize = dev1->pixelSize();

    for ( int y = 0; y < rc1.height(); ++y ) {

        while ( !iter1.isDone() ) {
            if ( memcmp( iter1.rawData(), iter2.rawData(), pixelSize ) != 0 )
                return false;
            ++iter1;
            ++iter2;
        }

        iter1.nextRow();
        iter2.nextRow();
    }
//     qDebug() << "comparePaintDevices time elapsed:" << t.elapsed();
    return true;
}



QList<KoColorSpace*> allColorSpaces()
{


    QList<KoColorSpace*> colorSpaces;

    QList<QString> csIds = KoColorSpaceRegistry::instance()->keys();

    foreach( QString csId, csIds ) {
        QList<KoColorProfile*> profiles = KoColorSpaceRegistry::instance()->profilesFor ( csId );
        if ( profiles.size() == 0 ) {
            KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace( csId, 0 );
            colorSpaces.append( cs );
        }
        else {
            foreach( KoColorProfile * profile, profiles ) {
                KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace( csId, profile );
                colorSpaces.append( cs );
            }
        }
    }
    return colorSpaces;
}
}

#endif
