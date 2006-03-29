/* This file is part of the KDE libraries
 * Copyright (c) 2003 thierry lorthiois (lorthioist@wanadoo.fr)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <math.h>
#include <qfile.h>
#include <qdatastream.h>
//Added by qt3to4:
#include <Q3PointArray>
#include <Q3PtrList>

#include <kdebug.h>

#include "kowmfstruct.h"
#include "kowmfreadprivate.h"
#include "kowmfwrite.h"

/**
 * Private data
 */
class KoWmfWritePrivate
{
public:
    QRect    mBBox;      // bounding rectangle
    int      mDpi;       // number of point per inch for the default size
    int      mMaxRecordSize;

    // memory allocation for WMF file
    QFile mFileOut;
    QDataStream mSt;
};



KoWmfWrite::KoWmfWrite( const QString& fileName ) {
    d = new KoWmfWritePrivate;

    d->mDpi = 1024;
    d->mMaxRecordSize = 0;
    d->mFileOut.setName( fileName );
}

KoWmfWrite::~KoWmfWrite() {
    delete d;
}


void KoWmfWrite::setDefaultDpi( int dpi ) {
    d->mDpi = dpi;
}


//-----------------------------------------------------------------------------
// Virtual Painter => create the WMF

bool KoWmfWrite::begin() {

    if ( !d->mFileOut.open( QIODevice::WriteOnly ) )
    {
        kDebug() << "Cannot open file " << QFile::encodeName(d->mFileOut.name()) << endl;
        return false;
    }
    d->mSt.setDevice( &d->mFileOut );
    d->mSt.setByteOrder( QDataStream::LittleEndian );

    // reserved placeable and standard header
    for ( int i=0 ; i < 10 ; i++ ) {
        d->mSt << (quint32)0;
    }

    // initialize the stack of objects
    // Pen
    d->mSt << (quint32)8 << (quint16)0x02FA;
    d->mSt << (quint16)5 << (quint16)0 << (quint16)0 << (quint32)0;
    // Brush
    d->mSt << (quint32)7 << (quint16)0x02FC;
    d->mSt << (quint16)1 << (quint32)0 << (quint16)0;
    for ( int i=0 ; i < 4 ; i++ ) {
        d->mSt << (quint32)8 << (quint16)0x02FA << (quint16)0 << (quint32)0 << (quint32)0;
    }
    d->mMaxRecordSize = 8;

    return true;
}


bool KoWmfWrite::end() {
    WmfPlaceableHeader pheader = { 0x9AC6CDD7, 0, 0, 0, 0, 0, 0, 0, 0 };
    quint16  checksum;

    // End of the wmf file
    d->mSt << (quint32)3 << (quint16)0;

    // adjust header
    pheader.left = d->mBBox.left();
    pheader.top = d->mBBox.top();
    pheader.right = d->mBBox.right();
    pheader.bottom = d->mBBox.bottom();
    pheader.inch = d->mDpi;
    checksum = KoWmfReadPrivate::calcCheckSum( &pheader );

    // write headers
    d->mFileOut.at( 0 );
    d->mSt << (quint32)0x9AC6CDD7 << (quint16)0;
    d->mSt << (qint16)d->mBBox.left() << (qint16)d->mBBox.top() << (qint16)d->mBBox.right() << (qint16)d->mBBox.bottom();
    d->mSt << (quint16)d->mDpi << (quint32)0 << checksum;
    d->mSt << (quint16)1 << (quint16)9 << (quint16)0x300 << (quint32)(d->mFileOut.size()/2);
    d->mSt << (quint16)6 << (quint32)d->mMaxRecordSize << (quint16)0;

    d->mFileOut.close();

    return true;
}


void KoWmfWrite::save() {
    d->mSt << (quint32)3 << (quint16)0x001E;
}


void KoWmfWrite::restore() {
    d->mSt << (quint32)4 << (quint16)0x0127 << (quint16)1;
}


void KoWmfWrite::setPen( const QPen &pen ) {
    int style;
    int max = sizeof(koWmfStylePen) / sizeof(Qt::SolidLine);

    // we can't delete an object currently selected
    // select another object
    d->mSt << (quint32)4 << (quint16)0x012D << (quint16)0;
    // delete object
    d->mSt << (quint32)4 << (quint16)0x01f0 << (quint16)2;

    for ( style=0 ; style < max ; style++ ) {
        if ( koWmfStylePen[ style ] == pen.style() ) break;
    }
    if ( style == max ) {
        // SolidLine
        style = 0;
    }
    d->mSt << (quint32)8 << (quint16)0x02FA;
    d->mSt << (quint16)style << (quint16)pen.width() << (quint16)0 << (quint32)winColor( pen.color() );

    // select object
    d->mSt << (quint32)4 << (quint16)0x012D << (quint16)2;
}


void KoWmfWrite::setBrush( const QBrush &brush ) {
    int style;
    int max = sizeof(koWmfStyleBrush) / sizeof(Qt::NoBrush);

    // we can't delete an object currently selected
    // select another object
    d->mSt << (quint32)4 << (quint16)0x012D << (quint16)1;
    // delete object
    d->mSt << (quint32)4 << (quint16)0x01f0 << (quint16)3;

    for ( style=0 ; style < max ; style++ ) {
        if ( koWmfStyleBrush[ style ] == brush.style() ) break;
    }
    if ( style == max ) {
        // SolidPattern
        style = 0;
    }
    d->mSt << (quint32)7 << (quint16)0x02FC;
    d->mSt << (quint16)style << (quint32)winColor( brush.color() ) << (quint16)0;

    // select object
    d->mSt << (quint32)4 << (quint16)0x012D << (quint16)3;
}


void KoWmfWrite::setFont( const QFont & ) {
}


void KoWmfWrite::setBackgroundColor( const QColor &c ) {
    d->mSt << (quint32)5 << (quint16)0x0201 << (quint32)winColor( c );
}


void KoWmfWrite::setBackgroundMode( Qt::BGMode mode ) {
    d->mSt << (quint32)4 << (quint16)0x0102;
    if ( mode == Qt::TransparentMode )
        d->mSt << (quint16)1;
    else
        d->mSt << (quint16)0;
}


void KoWmfWrite::setCompositionMode( QPainter::CompositionMode op ) {
    d->mSt << (quint32)5 << (quint16)0x0104 << (quint32)qtRasterToWin32( op );
}


void KoWmfWrite::setWindow( int left, int top, int width, int height ) {
    d->mBBox.setRect( left, top, width, height );

    // windowOrg
    d->mSt << (quint32)5 << (quint16)0x020B << (quint16)top << (quint16)left;

    // windowExt
    d->mSt << (quint32)5 << (quint16)0x020C << (quint16)height << (quint16)width;
}


void KoWmfWrite::setClipRegion( const QRegion & ) {

}


void KoWmfWrite::clipping( bool enable ) {
    if ( !enable ) {
        // clipping region == bounding rectangle
        setClipRegion( d->mBBox );
    }
}


void KoWmfWrite::moveTo( int left, int top ) {
    d->mSt << (quint32)5 << (quint16)0x0214 << (quint16)top << (quint16)left;
}


void KoWmfWrite::lineTo( int left, int top ) {
    d->mSt << (quint32)5 << (quint16)0x0213 << (quint16)top << (quint16)left;
}


void KoWmfWrite::drawRect( int left, int top, int width, int height ) {
    QRect rec( left, top, width, height );

    d->mSt << (quint32)7 << (quint16)0x041B;
    d->mSt << (quint16)rec.bottom() << (quint16)rec.right() << (quint16)rec.top() << (quint16)rec.left();
}


void KoWmfWrite::drawRoundRect( int left, int top, int width, int height , int roudw, int roudh ) {
    int  widthCorner, heightCorner;
    QRect rec( left, top, width, height );

    // convert percentage (roundw, roudh) in (widthCorner, heightCorner)
    widthCorner = ( roudw * width ) / 100;
    heightCorner = ( roudh * height ) / 100;

    d->mSt << (quint32)9 << (quint16)0x061C << (quint16)heightCorner << (quint16)widthCorner;
    d->mSt << (quint16)rec.bottom() << (quint16)rec.right() << (quint16)rec.top() << (quint16)rec.left();

    d->mMaxRecordSize = qMax( d->mMaxRecordSize, 9 );
}


void KoWmfWrite::drawEllipse( int left, int top, int width, int height  ) {
    QRect rec( left, top, width, height );

    d->mSt << (quint32)7 << (quint16)0x0418;
    d->mSt << (quint16)rec.bottom() << (quint16)rec.right() << (quint16)rec.top() << (quint16)rec.left();
}


void KoWmfWrite::drawArc( int left, int top, int width, int height , int a, int alen ) {
    int xCenter, yCenter;
    int  offXStart, offYStart, offXEnd, offYEnd;

    angleToxy( offXStart, offYStart, offXEnd, offYEnd, a, alen );
    xCenter = left + (width / 2);
    yCenter = top + (height / 2);

    d->mSt << (quint32)11 << (quint16)0x0817;
    d->mSt << (quint16)(yCenter + offYEnd) << (quint16)(xCenter + offXEnd);
    d->mSt << (quint16)(yCenter + offYStart) << (quint16)(xCenter + offXStart);
    d->mSt << (quint16)(top + height) << (quint16)(left + width);
    d->mSt << (quint16)top << (quint16)left;

    d->mMaxRecordSize = qMax( d->mMaxRecordSize, 11 );
}


void KoWmfWrite::drawPie( int left, int top, int width, int height , int a, int alen ) {
    int xCenter, yCenter;
    int  offXStart, offYStart, offXEnd, offYEnd;

    angleToxy( offXStart, offYStart, offXEnd, offYEnd, a, alen );
    xCenter = left + (width / 2);
    yCenter = top + (height / 2);

    d->mSt << (quint32)11 << (quint16)0x081A;
    d->mSt << (quint16)(yCenter + offYEnd) << (quint16)(xCenter + offXEnd);
    d->mSt << (quint16)(yCenter + offYStart) << (quint16)(xCenter + offXStart);
    d->mSt << (quint16)(top + height) << (quint16)(left + width);
    d->mSt << (quint16)top << (quint16)left;

    d->mMaxRecordSize = qMax( d->mMaxRecordSize, 11 );
}


void KoWmfWrite::drawChord( int left, int top, int width, int height , int a, int alen ) {
    int xCenter, yCenter;
    int  offXStart, offYStart, offXEnd, offYEnd;

    angleToxy( offXStart, offYStart, offXEnd, offYEnd, a, alen );
    xCenter = left + (width / 2);
    yCenter = top + (height / 2);

    d->mSt << (quint32)11 << (quint16)0x0830;
    d->mSt << (quint16)(yCenter + offYEnd) << (quint16)(xCenter + offXEnd);
    d->mSt << (quint16)(yCenter + offYStart) << (quint16)(xCenter + offXStart);
    d->mSt << (quint16)(top + height) << (quint16)(left + width);
    d->mSt << (quint16)top << (quint16)left;

    d->mMaxRecordSize = qMax( d->mMaxRecordSize, 11 );
}


void KoWmfWrite::drawPolyline( const Q3PointArray &pa ) {
    int size = 4 + (pa.size() * 2);

    d->mSt << (quint32)size << (quint16)0x0325 << (quint16)pa.size();
    pointArray( pa );

    d->mMaxRecordSize = qMax( d->mMaxRecordSize, size );
}


void KoWmfWrite::drawPolygon( const Q3PointArray &pa, bool  ) {
    int size = 4 + (pa.size() * 2);

    d->mSt << (quint32)size << (quint16)0x0324 << (quint16)pa.size();
    pointArray( pa );

    d->mMaxRecordSize = qMax( d->mMaxRecordSize, size );
}


void KoWmfWrite::drawPolyPolygon( Q3PtrList<Q3PointArray>& listPa, bool ) {

    Q3PointArray *pa;
    int sizeArrayPoly = 0;

    for ( pa = listPa.first() ; pa ; pa = listPa.next() ) {
        sizeArrayPoly += (pa->size() * 2);
    }
    int size = 4 + listPa.count() + sizeArrayPoly;
    d->mSt << (quint32)size << (quint16)0x0538 << (quint16)listPa.count();

    // number of point for each Polygon
    for ( pa = listPa.first() ; pa ; pa = listPa.next() ) {
        d->mSt << (quint16)pa->size();
    }

    // list of points
    for ( pa = listPa.first() ; pa ; pa = listPa.next() ) {
        pointArray( *pa );
    }

    d->mMaxRecordSize = qMax( d->mMaxRecordSize, size );

}


void KoWmfWrite::drawImage( int , int , const QImage &, int , int , int , int  ) {
/*
    QImage img;

    img = image;
    img.setFormat( "BMP" );

    QIODevice io = img.ioDevice();
    io.at( 14 );   // skip the BMP header
    d->mSt << io.readAll();
*/
}


void KoWmfWrite::drawText( int , int , int , int , int , const QString& , double ) {
//    d->mSt << (quint32)3 << (quint16)0x0A32;
}

//-----------------------------------------------------------------------------
// Utilities and conversion Qt --> Wmf

void KoWmfWrite::pointArray( const Q3PointArray &pa ) {
    int  left, top, i, max;

    for ( i=0, max=pa.size() ; i < max ; i++ ) {
        pa.point( i, &left, &top );
        d->mSt << (qint16)left << (qint16)top;
    }
}


quint32 KoWmfWrite::winColor( QColor color ) {
    quint32 c;

    c = (color.Qt::red() & 0xFF);
    c += ( (color.Qt::green() & 0xFF) << 8 );
    c += ( (color.Qt::blue() & 0xFF) << 16 );

    return c;
}


void KoWmfWrite::angleToxy( int &xStart, int &yStart, int &xEnd, int &yEnd, int a, int alen ) {
    double angleStart, angleLength;

    angleStart = ((double)a * 3.14166) / 2880;
    angleLength = ((double)alen * 3.14166) / 2880;

    xStart = (int)(cos(angleStart) * 50);
    yStart = -(int)(sin(angleStart) * 50);
    xEnd = (int)(cos(angleLength) * 50);
    yEnd = -(int)(sin(angleLength) * 50);
}


quint16  KoWmfWrite::qtRasterToWin16( QPainter::CompositionMode op ) const {
    int i;

    for ( i=0 ; i < 17 ; i++ ) {
        if ( koWmfOpTab16[ i ] == op )  break;
    }

    if ( i < 17 )
        return  (quint16)i;
    else
        return (quint16)0;
}


quint32  KoWmfWrite::qtRasterToWin32( QPainter::CompositionMode op ) const {
    int i;

    for ( i=0 ; i < 15 ; i++ ) {
        if ( koWmfOpTab32[ i ].qtRasterOp == op )  break;
    }

    if ( i < 15 )
        return koWmfOpTab32[ i ].winRasterOp;
    else
        return koWmfOpTab32[ 0 ].winRasterOp;
}

