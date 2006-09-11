/* This file is part of the KDE libraries
 * Copyright (c) 1998 Stefan Taferner
 *                    2001/2003 thierry lorthiois (lorthioist@wanadoo.fr)
 * With the help of WMF documentation by Caolan Mc Namara
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

#include <QFileInfo>
#include <QImage>
#include <QMatrix>
#include <q3ptrlist.h>
#include <q3pointarray.h>
#include <QPainter>
#include <QDataStream>
#include <QByteArray>
#include <kdebug.h>

#include "kowmfreadprivate.h"
#include "kowmfread.h"


KoWmfReadPrivate::KoWmfReadPrivate()
{
    mNbrFunc = 0;
    mValid = false;
    mStandard = false;
    mPlaceable = false;
    mEnhanced = false;
    mBuffer = 0;
    mObjHandleTab = 0;
}


KoWmfReadPrivate::~KoWmfReadPrivate()
{
    if ( mObjHandleTab != 0 ) {
        for ( int i=0 ; i < mNbrObject ; i++ ) {
            if ( mObjHandleTab[i] != 0 )
                delete mObjHandleTab[i];
        }
        delete[] mObjHandleTab;
    }
    if ( mBuffer != 0 ) {
        mBuffer->close();
        delete mBuffer;
    }
}


bool KoWmfReadPrivate::load( const QByteArray& array )
{
    // delete previous buffer
    if ( mBuffer != 0 ) {
        mBuffer->close();
        delete mBuffer;
    }

    // load into buffer
    QByteArray nonConstArray( array ); // hack
    mBuffer = new QBuffer( &nonConstArray );
    mBuffer->open( QIODevice::ReadOnly );

    // read and check the header
    WmfEnhMetaHeader eheader;
    WmfMetaHeader header;
    WmfPlaceableHeader pheader;
    unsigned short checksum;
    int filePos;

    QDataStream st( mBuffer );
    st.setByteOrder( QDataStream::LittleEndian );
    mStackOverflow = mWinding = false;
    mTextAlign = mTextRotation = 0;
    mTextColor = Qt::black;
    mValid = false;
    mStandard = false;
    mPlaceable = false;
    mEnhanced = false;

    //----- Read placeable metafile header
    st >> pheader.key;
    if ( pheader.key==( quint32 )APMHEADER_KEY ) {
        mPlaceable = true;
        st >> pheader.handle;
        st >> pheader.left;
        st >> pheader.top;
        st >> pheader.right;
        st >> pheader.bottom;
        st >> pheader.inch;
        st >> pheader.reserved;
        st >> pheader.checksum;
        checksum = calcCheckSum( &pheader );
        if ( pheader.checksum!=checksum ) {
            return false;
        }
        st >> header.fileType;
        st >> header.headerSize;
        st >> header.version;
        st >> header.fileSize;
        st >> header.numOfObjects;
        st >> header.maxRecordSize;
        st >> header.numOfParameters;
        mNbrObject = header.numOfObjects;
        mBBox.setLeft( pheader.left );
        mBBox.setTop( pheader.top );
        mBBox.setRight( pheader.right );
        mBBox.setBottom( pheader.bottom );
        mDpi = pheader.inch;
    }
    else {
        mBuffer->reset();
        //----- Read as enhanced metafile header
        filePos = mBuffer->pos();
        st >> eheader.recordType;
        st >> eheader.recordSize;
        st >> eheader.boundsLeft;
        st >> eheader.boundsTop;
        st >> eheader.boundsRight;
        st >> eheader.boundsBottom;
        st >> eheader.frameLeft;
        st >> eheader.frameTop;
        st >> eheader.frameRight;
        st >> eheader.frameBottom;
        st >> eheader.signature;
        if ( eheader.signature==ENHMETA_SIGNATURE ) {
            mEnhanced = true;
            st >> eheader.version;
            st >> eheader.size;
            st >> eheader.numOfRecords;
            st >> eheader.numHandles;
            st >> eheader.reserved;
            st >> eheader.sizeOfDescription;
            st >> eheader.offsetOfDescription;
            st >> eheader.numPaletteEntries;
            st >> eheader.widthDevicePixels;
            st >> eheader.heightDevicePixels;
            st >> eheader.widthDeviceMM;
            st >> eheader.heightDeviceMM;
        }
        else {
            //----- Read as standard metafile header
            mStandard = true;
            mBuffer->seek( filePos );
            st >> header.fileType;
            st >> header.headerSize;
            st >> header.version;
            st >> header.fileSize;
            st >> header.numOfObjects;
            st >> header.maxRecordSize;
            st >> header.numOfParameters;
            mNbrObject = header.numOfObjects;
        }
    }
    mOffsetFirstRecord = mBuffer->pos();

    //----- Test header validity
    if ( ((header.headerSize == 9) && (header.numOfParameters == 0)) || (mPlaceable) ) {
        // valid wmf file
        mValid = true;
    }
    else {
        kDebug() << "KoWmfReadPrivate : incorrect file format !" << endl;
    }

    // check bounding rectangle for standard meta file
    if ( (mValid) && (mStandard) ) {
        quint16 numFunction = 1;
        quint32 size;
        bool firstOrg=true, firstExt=true;

        // search functions setWindowOrg and setWindowExt
        while ( numFunction ) {
            filePos = mBuffer->pos();
            st >> size >> numFunction;

            if ( size == 0 ) {
                kDebug() << "KoWmfReadPrivate : incorrect file!" << endl;
                mValid = 0;
                break;
            }

            numFunction &= 0xFF;
            if ( numFunction == 11 ) {
                qint16 top, left;

                st >> top >> left;
                if ( firstOrg ) {
                    firstOrg = false;
                    mBBox.setLeft( left );
                    mBBox.setTop( top );
                }
                else {
                    if ( left < mBBox.left() ) mBBox.setLeft( left );
                    if ( top < mBBox.top() ) mBBox.setTop( top );
                }
            }
            if ( numFunction == 12 ) {
                qint16 width, height;

                st >> height >> width;
                if ( width < 0 ) width = -width;
                if ( height < 0 ) height = -height;
                if ( firstExt ) {
                    firstExt = false;
                    mBBox.setWidth( width );
                    mBBox.setHeight( height );
                }
                else {
                    if ( width > mBBox.width() ) mBBox.setWidth( width );
                    if ( height > mBBox.height() ) mBBox.setHeight( height );
                }
            }
            mBuffer->seek( filePos + (size<<1) );
            // ## shouldn't we break from the loop as soon as we found what we were looking for?
        }
    }

    return (mValid);
}


bool KoWmfReadPrivate::play( KoWmfRead* readWmf )
{
    if ( !(mValid) ) {
        kDebug() << "KoWmfReadPrivate::play : invalid WMF file" << endl;
        return false;
    }

    if ( mNbrFunc ) {
        if ( (mStandard) ) {
            kDebug() << "Standard : " << mBBox.left() << "  "  << mBBox.top() << "  "  << mBBox.width() << "  "  << mBBox.height() << endl;
        }
        else {
            kDebug() << "DPI : " << mDpi << " : " << mBBox.left() << "  "  << mBBox.top() << "  "  << mBBox.width() << "  "  << mBBox.height() << endl;
            kDebug() << "inch : " << mBBox.width()/mDpi << "  " << mBBox.height()/mDpi << endl;
            kDebug() << "mm : " << mBBox.width()*25.4/mDpi << "  " << mBBox.height()*25.4/mDpi << endl;
        }
        kDebug() << mValid << "  " << mStandard << "  " << mPlaceable << endl;
    }

    // stack of handle
    mObjHandleTab = new KoWmfHandle* [ mNbrObject ];
    for ( int i=0; i < mNbrObject ; i++ ) {
        mObjHandleTab[ i ] = 0;
    }

    quint16 numFunction;
    quint32 size;
    int  bufferOffset, j;

    // buffer with functions
    QDataStream st( mBuffer );
    st.setByteOrder( QDataStream::LittleEndian );

    mReadWmf = readWmf;
    mWindow = mBBox;
    if ( mReadWmf->begin() ) {
        // play wmf functions
        mBuffer->seek( mOffsetFirstRecord );
        numFunction = j = 1;
        mWinding = false;

        while ( ( numFunction ) && ( !mStackOverflow ) ) {
            bufferOffset = mBuffer->pos();
            st >> size >> numFunction;

            /**
             * mapping between n function and index of table 'metaFuncTab'
             * lower 8 digits of the function => entry in the table
             */
            numFunction &= 0xFF;
            if ( numFunction > 0x5F ) {
                numFunction -= 0x90;
            }
            if ( (numFunction > 111) || (koWmfFunc[ numFunction ].method == 0) ) {
                // function outside WMF specification
                kDebug() << "KoWmfReadPrivate::paint : BROKEN WMF file" << endl;
                mValid = false;
                break;
            }

            if ( mNbrFunc ) {
                // debug mode
                if ( (j+12) > mNbrFunc ) {
                    // output last 12 functions
                    int offBuff = mBuffer->pos();
                    quint16 param;

                    kDebug() <<  j << " : " << numFunction << " : ";
                    for ( quint16 i=0 ; i < (size-3) ; i++ ) {
                        st >> param;
                        kDebug() <<  param << " ";
                    }
                    kDebug() <<  endl;
                    mBuffer->seek( offBuff );
                }
                if ( j >= mNbrFunc ) {
                    break;
                }
                j++;
            }

            // execute the function
            (this->*koWmfFunc[ numFunction ].method)( size, st );

            mBuffer->seek( bufferOffset + (size<<1) );
        }

        mReadWmf->end();
    }

    for ( int i=0 ; i < mNbrObject ; i++ ) {
        if ( mObjHandleTab[ i ] != 0 )
            delete mObjHandleTab[ i ];
    }
    delete[] mObjHandleTab;
    mObjHandleTab = 0;

    return true;
}


//-----------------------------------------------------------------------------
// Metafile painter methods

void KoWmfReadPrivate::setWindowOrg( quint32, QDataStream& stream )
{
    qint16 top, left;

    stream >> top >> left;
    mReadWmf->setWindowOrg( left, top );
    mWindow.setLeft( left );
    mWindow.setTop( top );
//    kDebug() << "Org : (" << left << ", "  << top << ")  " << endl;
}

/*  TODO : deeper look in negative width and height
*/

void KoWmfReadPrivate::setWindowExt( quint32, QDataStream& stream )
{
    qint16 width, height;

    // negative value allowed for width and height
    stream >> height >> width;
    mReadWmf->setWindowExt( width, height );
    mWindow.setWidth( width );
    mWindow.setHeight( height );
//    kDebug() << "Ext : (" << width << ", "  << height << ") "<< endl;
}


void KoWmfReadPrivate::OffsetWindowOrg( quint32, QDataStream &stream )
{
    qint16 offTop, offLeft;

    stream >> offTop >> offLeft;
    mReadWmf->setWindowOrg( mWindow.left() + offLeft, mWindow.top() + offTop );
    mWindow.setLeft( mWindow.left() + offLeft );
    mWindow.setTop( mWindow.top() + offTop );
}


void KoWmfReadPrivate::ScaleWindowExt( quint32, QDataStream &stream )
{
    qint16 width, height;
    qint16 heightDenom, heightNum, widthDenom, widthNum;

    stream >> heightDenom >> heightNum >> widthDenom >> widthNum;

    if ( ( widthDenom != 0 ) && ( heightDenom != 0 ) ) {
        width = (mWindow.width() * widthNum) / widthDenom;
        height = (mWindow.height() * heightNum) / heightDenom;
        mReadWmf->setWindowExt( width, height );
        mWindow.setWidth( width );
        mWindow.setHeight( height );
    }
//    kDebug() << "KoWmfReadPrivate::ScaleWindowExt : " << widthDenom << "  " << heightDenom << endl;
}


//-----------------------------------------------------------------------------
// Drawing

void KoWmfReadPrivate::lineTo( quint32, QDataStream& stream )
{
    qint16 top, left;

    stream >> top >> left;
    mReadWmf->lineTo( left, top );
}


void KoWmfReadPrivate::moveTo( quint32, QDataStream& stream )
{
    qint16 top, left;

    stream >> top >> left;
    mReadWmf->moveTo( left, top );
}


void KoWmfReadPrivate::ellipse( quint32, QDataStream& stream )
{
    qint16 top, left, right, bottom;

    stream >> bottom >> right >> top >> left;
    mReadWmf->drawEllipse( left, top, right-left, bottom-top );
}


void KoWmfReadPrivate::polygon( quint32, QDataStream& stream )
{
    quint16 num;

    stream >> num;

    QPolygon pa( num );

    pointArray( stream, pa );
    mReadWmf->drawPolygon( pa, mWinding );
}


void KoWmfReadPrivate::polyPolygon( quint32, QDataStream& stream )
{
    quint16 numberPoly;
    quint16 sizePoly;
    Q3PtrList<QPolygon> listPa;

    stream >> numberPoly;

    listPa.setAutoDelete( true );
    for ( int i=0 ; i < numberPoly ; i++ ) {
        stream >> sizePoly;
        listPa.append( new QPolygon( sizePoly ) );
    }

    // list of point array
    QPolygon *pa;
    for ( pa = listPa.first() ; pa ; pa = listPa.next() ) {
        pointArray( stream, *pa );
    }

    // draw polygon's
    mReadWmf->drawPolyPolygon( listPa, mWinding );
    listPa.clear();
}


void KoWmfReadPrivate::polyline( quint32, QDataStream& stream )
{
    quint16 num;

    stream >> num;
    QPolygon pa( num );

    pointArray( stream, pa );
    mReadWmf->drawPolyline( pa );
}


void KoWmfReadPrivate::rectangle( quint32, QDataStream& stream )
{
    qint16 top, left, right, bottom;

    stream >> bottom >> right >> top >> left;
    mReadWmf->drawRect( left, top, right-left, bottom-top );
}


void KoWmfReadPrivate::roundRect( quint32, QDataStream& stream )
{
    int xRnd = 0, yRnd = 0;
    quint16 widthCorner, heightCorner;
    qint16  top, left, right, bottom;

    stream >> heightCorner >> widthCorner;
    stream >> bottom >> right >> top >> left;

    // convert (widthCorner, heightCorner) in percentage
    if ( (right - left) != 0  )
        xRnd = (widthCorner * 100) / (right - left);
    if ( (bottom - top) != 0  )
        yRnd = (heightCorner * 100) / (bottom - top);

    mReadWmf->drawRoundRect( left, top, right-left, bottom-top, xRnd, yRnd );
}


void KoWmfReadPrivate::arc( quint32, QDataStream& stream )
{
    int xCenter, yCenter, angleStart, aLength;
    qint16  topEnd, leftEnd, topStart, leftStart;
    qint16  top, left, right, bottom;

    stream >> topEnd >> leftEnd >> topStart >> leftStart;
    stream >> bottom >> right >> top >> left;

    xCenter = left + ((right-left) / 2);
    yCenter = top + ((bottom-top) / 2);
    xyToAngle ( leftStart-xCenter, yCenter-topStart, leftEnd-xCenter, yCenter-topEnd, angleStart, aLength );

    mReadWmf->drawArc( left, top, right-left, bottom-top, angleStart, aLength);
}


void KoWmfReadPrivate::chord( quint32, QDataStream& stream )
{
    int xCenter, yCenter, angleStart, aLength;
    qint16  topEnd, leftEnd, topStart, leftStart;
    qint16  top, left, right, bottom;

    stream >> topEnd >> leftEnd >> topStart >> leftStart;
    stream >> bottom >> right >> top >> left;

    xCenter = left + ((right-left) / 2);
    yCenter = top + ((bottom-top) / 2);
    xyToAngle ( leftStart-xCenter, yCenter-topStart, leftEnd-xCenter, yCenter-topEnd, angleStart, aLength );

    mReadWmf->drawChord( left, top, right-left, bottom-top, angleStart, aLength);
}


void KoWmfReadPrivate::pie( quint32, QDataStream& stream )
{
    int xCenter, yCenter, angleStart, aLength;
    qint16  topEnd, leftEnd, topStart, leftStart;
    qint16  top, left, right, bottom;

    stream >> topEnd >> leftEnd >> topStart >> leftStart;
    stream >> bottom >> right >> top >> left;

    xCenter = left + ((right-left) / 2);
    yCenter = top + ((bottom-top) / 2);
    xyToAngle ( leftStart-xCenter, yCenter-topStart, leftEnd-xCenter, yCenter-topEnd, angleStart, aLength );

    mReadWmf->drawPie( left, top, right-left, bottom-top, angleStart, aLength);
}


void KoWmfReadPrivate::setPolyFillMode( quint32, QDataStream& stream )
{
    quint16 winding;

    stream >> winding;
    mWinding = (winding != 0);
}


void KoWmfReadPrivate::setBkColor( quint32, QDataStream& stream )
{
    quint32 color;

    stream >> color;
    mReadWmf->setBackgroundColor( qtColor( color ) );
}


void KoWmfReadPrivate::setBkMode( quint32, QDataStream& stream )
{
    quint16 bkMode;

    stream >> bkMode;
    if ( bkMode == 1 )
        mReadWmf->setBackgroundMode( Qt::TransparentMode );
    else
        mReadWmf->setBackgroundMode( Qt::OpaqueMode );
}


void KoWmfReadPrivate::setPixel( quint32, QDataStream& stream )
{
    qint16  top, left;
    quint32 color;

    stream >> color >> top >> left;

    QPen oldPen = mReadWmf->pen();
    QPen pen = oldPen;
    pen.setColor( qtColor( color ) );
    mReadWmf->setPen( pen );
    mReadWmf->moveTo( left, top );
    mReadWmf->lineTo( left, top );
    mReadWmf->setPen( oldPen );
}


void KoWmfReadPrivate::setRop( quint32, QDataStream& stream )
{
    quint16  rop;

    stream >> rop;
    mReadWmf->setCompositionMode( winToQtComposition( rop ) );
}


void KoWmfReadPrivate::saveDC( quint32, QDataStream& )
{
    mReadWmf->save();
}


void KoWmfReadPrivate::restoreDC( quint32, QDataStream& stream )
{
    qint16  num;

    stream >> num;
    for ( int i=0; i > num ; i-- )
        mReadWmf->restore();
}


void KoWmfReadPrivate::intersectClipRect( quint32, QDataStream& stream )
{
    qint16 top, left, right, bottom;

    stream >> bottom >> right >> top >> left;

    QRegion region = mReadWmf->clipRegion();
    QRegion newRegion( left, top, right-left, bottom-top );
    if ( region.isEmpty() ) {
        region = newRegion;
    }
    else {
        region = region.intersect( newRegion );
    }

    mReadWmf->setClipRegion( region );
}


void KoWmfReadPrivate::excludeClipRect( quint32, QDataStream& stream )
{
    qint16 top, left, right, bottom;

    stream >> bottom >> right >> top >> left;

    QRegion region = mReadWmf->clipRegion();
    QRegion newRegion( left, top, right-left, bottom-top );
    if ( region.isEmpty() ) {
        region = newRegion;
    }
    else {
        region = region.subtract( newRegion );
    }

    mReadWmf->setClipRegion( region );
}


//-----------------------------------------------------------------------------
// Text

void KoWmfReadPrivate::setTextColor( quint32, QDataStream& stream )
{
    quint32 color;

    stream >> color;
    mTextColor = qtColor( color );
}


void KoWmfReadPrivate::setTextAlign( quint32, QDataStream& stream )
{
    stream >> mTextAlign;
}


void KoWmfReadPrivate::textOut( quint32, QDataStream& )
{
    if ( mNbrFunc ) {
        kDebug() << "textOut : unimplemented " << endl;
    }
}


void KoWmfReadPrivate::extTextOut( quint32 , QDataStream&  )
{
    if ( mNbrFunc ) {
        kDebug() << "extTextOut : unimplemented " << endl;
    }
}



//-----------------------------------------------------------------------------
// Bitmap

void KoWmfReadPrivate::SetStretchBltMode( quint32, QDataStream& )
{
    if ( mNbrFunc ) {
        kDebug() << "SetStretchBltMode : unimplemented " << endl;
    }
}


void KoWmfReadPrivate::dibBitBlt( quint32 size, QDataStream& stream )
{
    quint32 raster;
    qint16  topSrc, leftSrc, widthSrc, heightSrc;
    qint16  topDst, leftDst;

    stream >> raster;
    stream >> topSrc >> leftSrc >> heightSrc >> widthSrc;
    stream >> topDst >> leftDst;

    if ( size > 11 ) {    // DIB image
        QImage bmpSrc;

        if ( dibToBmp( bmpSrc, stream, (size - 11) * 2 ) ) {
            mReadWmf->setCompositionMode( winToQtComposition( raster )  );

            mReadWmf->save();
            if ( widthSrc < 0 ) {
                // negative width => horizontal flip
                QMatrix m( -1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F );
                mReadWmf->setMatrix( m, true );
            }
            if ( heightSrc < 0 ) {
                // negative height => vertical flip
                QMatrix m( 1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F );
                mReadWmf->setMatrix( m, true );
            }
            mReadWmf->drawImage( leftDst, topDst, bmpSrc, leftSrc, topSrc, widthSrc, heightSrc );
            mReadWmf->restore();
        }
    }
    else {
        kDebug() << "KoWmfReadPrivate::dibBitBlt without image not implemented " << endl;
    }
}


void KoWmfReadPrivate::dibStretchBlt( quint32 size, QDataStream& stream )
{
    quint32 raster;
    qint16  topSrc, leftSrc, widthSrc, heightSrc;
    qint16  topDst, leftDst, widthDst, heightDst;
    QImage   bmpSrc;

    stream >> raster;
    stream >> heightSrc >> widthSrc >> topSrc >> leftSrc;
    stream >> heightDst >> widthDst >> topDst >> leftDst;

    if ( dibToBmp( bmpSrc, stream, (size - 13) * 2 ) ) {
        mReadWmf->setCompositionMode( winToQtComposition( raster )  );

        mReadWmf->save();
        if ( widthDst < 0 ) {
            // negative width => horizontal flip
            QMatrix m( -1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F );
            mReadWmf->setMatrix( m, true );
        }
        if ( heightDst < 0 ) {
            // negative height => vertical flip
            QMatrix m( 1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F );
            mReadWmf->setMatrix( m, true );
        }
        bmpSrc = bmpSrc.copy( leftSrc, topSrc, widthSrc, heightSrc );
        // TODO: scale the bitmap : QImage::scale(widthDst, heightDst)
        // is actually too slow

        mReadWmf->drawImage( leftDst, topDst, bmpSrc );
        mReadWmf->restore();
    }
}


void KoWmfReadPrivate::stretchDib( quint32 size, QDataStream& stream )
{
    quint32 raster;
    qint16  arg, topSrc, leftSrc, widthSrc, heightSrc;
    qint16  topDst, leftDst, widthDst, heightDst;
    QImage   bmpSrc;

    stream >> raster >> arg;
    stream >> heightSrc >> widthSrc >> topSrc >> leftSrc;
    stream >> heightDst >> widthDst >> topDst >> leftDst;

    if ( dibToBmp( bmpSrc, stream, (size - 14) * 2 ) ) {
        mReadWmf->setCompositionMode( winToQtComposition( raster )  );

        mReadWmf->save();
        if ( widthDst < 0 ) {
            // negative width => horizontal flip
            QMatrix m( -1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F );
            mReadWmf->setMatrix( m, true );
        }
        if ( heightDst < 0 ) {
            // negative height => vertical flip
            QMatrix m( 1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F );
            mReadWmf->setMatrix( m, true );
        }
        bmpSrc = bmpSrc.copy( leftSrc, topSrc, widthSrc, heightSrc );
        // TODO: scale the bitmap ( QImage::scale(param[ 8 ], param[ 7 ]) is actually too slow )

        mReadWmf->drawImage( leftDst, topDst, bmpSrc );
        mReadWmf->restore();
    }
}


void KoWmfReadPrivate::dibCreatePatternBrush( quint32 size, QDataStream& stream )
{
    KoWmfPatternBrushHandle* handle = new KoWmfPatternBrushHandle;

    if ( addHandle( handle ) ) {
        quint32 arg;
        QImage bmpSrc;

        stream >> arg;
        if ( dibToBmp( bmpSrc, stream, (size - 5) * 2 ) ) {
            handle->image = QPixmap::fromImage( bmpSrc );
            handle->brush.setTexture( handle->image );
        }
        else {
            kDebug() << "KoWmfReadPrivate::dibCreatePatternBrush : incorrect DIB image" << endl;
        }
    }
}


//-----------------------------------------------------------------------------
// Object handle

void KoWmfReadPrivate::selectObject( quint32, QDataStream& stream )
{
    quint16 idx;

    stream >> idx;
    if ( (idx < mNbrObject) && (mObjHandleTab[ idx ] != 0) )
        mObjHandleTab[ idx ]->apply( mReadWmf );
    else
        kDebug() << "KoWmfReadPrivate::selectObject : selection of an empty object" << endl;
}


void KoWmfReadPrivate::deleteObject( quint32, QDataStream& stream )
{
    quint16 idx;

    stream >> idx;
    deleteHandle( idx );
}


void KoWmfReadPrivate::createEmptyObject()
{
    // allocation of an empty object (to keep object counting in sync)
    KoWmfPenHandle* handle = new KoWmfPenHandle;

    addHandle( handle );
}


void KoWmfReadPrivate::createBrushIndirect( quint32, QDataStream& stream )
{
    Qt::BrushStyle style;
    quint16 sty, arg2;
    quint32 color;
    KoWmfBrushHandle* handle = new KoWmfBrushHandle;

    if ( addHandle( handle ) ) {
        stream >> sty >> color >> arg2;

        if ( sty == 2 ) {
            if ( arg2 < 6 )
                style = koWmfHatchedStyleBrush[ arg2 ];
            else
            {
                kDebug() << "KoWmfReadPrivate::createBrushIndirect: invalid hatched brush " << arg2 << endl;
                style = Qt::SolidPattern;
            }
        }
        else {
            if ( sty < 9 )
                style = koWmfStyleBrush[ sty ];
            else {
                kDebug() << "KoWmfReadPrivate::createBrushIndirect: invalid brush " << sty << endl;
                style = Qt::SolidPattern;
            }
        }
        handle->brush.setStyle( style );
        handle->brush.setColor( qtColor( color ) );
    }
}


void KoWmfReadPrivate::createPenIndirect( quint32, QDataStream& stream )
{
    // TODO: userStyle and alternateStyle
    Qt::PenStyle penStyle;
    quint32 color;
    quint16 style, width, arg;

    KoWmfPenHandle* handle = new KoWmfPenHandle;

    if ( addHandle( handle ) ) {
        stream >> style >> width >> arg >> color;

        if ( style < 7 )
            penStyle=koWmfStylePen[ style ];
        else {
            kDebug() << "KoWmfReadPrivate::createPenIndirect: invalid pen " << style << endl;
            penStyle = Qt::SolidLine;
        }

        handle->pen.setStyle( penStyle );
        handle->pen.setColor( qtColor( color ) );
        handle->pen.setCapStyle( Qt::RoundCap );
        handle->pen.setWidth( width );
    }
}


void KoWmfReadPrivate::createFontIndirect( quint32 size, QDataStream& stream )
{
    qint16  pointSize, rotation;
    quint16 weight, property, fixedPitch, arg;

    KoWmfFontHandle* handle = new KoWmfFontHandle;

    if ( addHandle( handle ) ) {
        stream >> pointSize >> arg;
        stream >> rotation >> arg;
        stream >> weight >> property >> arg >> arg;
        stream >> fixedPitch;

        // text rotation (in 1/10 degree)
        // TODO: memorisation of rotation in object Font
        mTextRotation = -rotation / 10;
        handle->font.setFixedPitch( ((fixedPitch & 0x01) == 0) );
        // TODO: investigation why some test case need -2
        // size of font in logical point
        handle->font.setPointSize( qAbs(pointSize) - 2 );
        handle->font.setWeight( (weight >> 3) );
        handle->font.setItalic( (property & 0x01) );
        handle->font.setUnderline( (property & 0x100) );

        // font name
        int    maxChar = (size-12) * 2;
        char*  nameFont = new char[maxChar];
        stream.readRawData( nameFont, maxChar );
        handle->font.setFamily( nameFont );
        delete[] nameFont;
    }
}


//-----------------------------------------------------------------------------
// Misc functions

void KoWmfReadPrivate::end( quint32, QDataStream& )
{
}

quint16 KoWmfReadPrivate::calcCheckSum( WmfPlaceableHeader* apmfh )
{
    quint16*  lpWord;
    quint16   wResult, i;

    // Start with the first word
    wResult = *( lpWord = ( quint16* )( apmfh ) );
    // XOR in each of the other 9 words
    for( i=1; i<=9; i++ )
    {
        wResult ^= lpWord[ i ];
    }
    return wResult;
}


void KoWmfReadPrivate::notyet( quint32, QDataStream& )
{
    if ( mNbrFunc ) {
        kDebug() << "unimplemented " << endl;
    }
}

void KoWmfReadPrivate::region( quint32, QDataStream& )
{
    if ( mNbrFunc ) {
        kDebug() << "region : unimplemented " << endl;
    }
}

void KoWmfReadPrivate::palette( quint32, QDataStream& )
{
    if ( mNbrFunc ) {
        kDebug() << "palette : unimplemented " << endl;
    }
}

void KoWmfReadPrivate::escape( quint32, QDataStream& )
{
    if ( mNbrFunc ) {
        kDebug() << "escape : unimplemented " << endl;
    }
}

void KoWmfReadPrivate::setRelAbs( quint32, QDataStream& )
{
    if ( mNbrFunc ) {
        kDebug() << "setRelAbs : unimplemented " << endl;
    }
}

void KoWmfReadPrivate::setMapMode( quint32, QDataStream& )
{
    if ( mNbrFunc ) {
        kDebug() << "setMapMode : unimplemented " << endl;
    }
}

void KoWmfReadPrivate::extFloodFill( quint32, QDataStream&  )
{
    if ( mNbrFunc ) {
        kDebug() << "extFloodFill : unimplemented " << endl;
    }
}

void KoWmfReadPrivate::startDoc( quint32, QDataStream&  )
{
    if ( mNbrFunc ) {
        kDebug() << "startDoc : unimplemented " << endl;
    }
}

void KoWmfReadPrivate::startPage( quint32, QDataStream&  )
{
    if ( mNbrFunc ) {
        kDebug() << "startPage : unimplemented " << endl;
    }
}

void KoWmfReadPrivate::endDoc( quint32, QDataStream&  )
{
    if ( mNbrFunc ) {
        kDebug() << "endDoc : unimplemented " << endl;
    }
}

void KoWmfReadPrivate::endPage( quint32, QDataStream&  )
{
    if ( mNbrFunc ) {
        kDebug() << "endPage : unimplemented " << endl;
    }
}

void KoWmfReadPrivate::resetDC( quint32, QDataStream&  )
{
    if ( mNbrFunc ) {
        kDebug() << "resetDC : unimplemented " << endl;
    }
}

void KoWmfReadPrivate::bitBlt( quint32, QDataStream&  )
{
    if ( mNbrFunc ) {
        kDebug() << "bitBlt : unimplemented " << endl;
    }
}

void KoWmfReadPrivate::setDibToDev( quint32, QDataStream&  )
{
    if ( mNbrFunc ) {
        kDebug() << "setDibToDev : unimplemented " << endl;
    }
}

void KoWmfReadPrivate::createBrush( quint32, QDataStream&  )
{
    if ( mNbrFunc ) {
        kDebug() << "createBrush : unimplemented " << endl;
    }
}

void KoWmfReadPrivate::createPatternBrush( quint32, QDataStream&  )
{
    if ( mNbrFunc ) {
        kDebug() << "createPatternBrush : unimplemented " << endl;
    }
}

void KoWmfReadPrivate::createBitmap( quint32, QDataStream&  )
{
    if ( mNbrFunc ) {
        kDebug() << "createBitmap : unimplemented " << endl;
    }
}

void KoWmfReadPrivate::createBitmapIndirect( quint32, QDataStream&  )
{
    createEmptyObject();
    if ( mNbrFunc ) {
        kDebug() << "createBitmapIndirect : unimplemented " << endl;
    }
}

void KoWmfReadPrivate::createPalette( quint32, QDataStream&  )
{
    createEmptyObject();
    if ( mNbrFunc ) {
        kDebug() << "createPalette : unimplemented " << endl;
    }
}

void KoWmfReadPrivate::createRegion( quint32, QDataStream&  )
{
    createEmptyObject();
    if ( mNbrFunc ) {
        kDebug() << "createRegion : unimplemented " << endl;
    }
}



//-----------------------------------------------------------------------------
// Utilities and conversion Wmf -> Qt

bool KoWmfReadPrivate::addHandle( KoWmfHandle* handle )
{
    int idx;

    for ( idx=0; idx < mNbrObject ; idx++ ) {
        if ( mObjHandleTab[ idx ] == 0 )  break;
    }

    if ( idx < mNbrObject ) {
        mObjHandleTab[ idx ] = handle;
        return true;
    }
    else {
        delete handle;
        mStackOverflow = true;
        kDebug() << "KoWmfReadPrivate::addHandle : stack overflow = broken file !" << endl;
        return false;
    }
}


void KoWmfReadPrivate::deleteHandle( int idx )
{
    if ( (idx < mNbrObject) && (mObjHandleTab[idx] != 0) ) {
        delete mObjHandleTab[ idx ];
        mObjHandleTab[ idx ] = 0;
    }
    else {
        kDebug() << "KoWmfReadPrivate::deletehandle() : bad index number" << endl;
    }
}


void KoWmfReadPrivate::pointArray( QDataStream& stream, QPolygon& pa )
{
    qint16 left, top;
    int  i, max;

    for ( i=0, max=pa.size() ; i < max ; i++ ) {
        stream >> left >> top;
        pa.setPoint( i, left, top );
    }
}


void KoWmfReadPrivate::xyToAngle( int xStart, int yStart, int xEnd, int yEnd, int& angleStart, int& angleLength )
{
    double aStart, aLength;

    aStart = atan2( yStart,  xStart );
    aLength = atan2( yEnd, xEnd ) - aStart;

    angleStart = (int)((aStart * 2880) / 3.14166);
    angleLength = (int)((aLength * 2880) / 3.14166);
    if ( angleLength < 0 ) angleLength = 5760 + angleLength;
}


QPainter::CompositionMode KoWmfReadPrivate::winToQtComposition( quint16 param ) const
{
    if ( param < 17 )
        return koWmfOpTab16[ param ];
    else
        return QPainter::CompositionMode_Source;
}


QPainter::CompositionMode  KoWmfReadPrivate::winToQtComposition( quint32 param ) const
{
    /* TODO: Ternary raster operations
    0x00C000CA  dest = (source AND pattern)
    0x00F00021  dest = pattern
    0x00FB0A09  dest = DPSnoo
    0x005A0049  dest = pattern XOR dest   */
    int i;

    for ( i=0 ; i < 15 ; i++ ) {
        if ( koWmfOpTab32[ i ].winRasterOp == param )  break;
    }

    if ( i < 15 )
        return koWmfOpTab32[ i ].qtRasterOp;
    else
        return QPainter::CompositionMode_SourceOver;
}


bool KoWmfReadPrivate::dibToBmp( QImage& bmp, QDataStream& stream, quint32 size )
{
    typedef struct _BMPFILEHEADER {
        quint16 bmType;
        quint32 bmSize;
        quint16 bmReserved1;
        quint16 bmReserved2;
        quint32 bmOffBits;
    }  BMPFILEHEADER;

    int sizeBmp = size + 14;

    QByteArray pattern;           // BMP header and DIB data
    pattern.resize( sizeBmp );
    pattern.fill(0);
    stream.readRawData( pattern.data() + 14, size );

    // add BMP header
    BMPFILEHEADER* bmpHeader;
    bmpHeader = (BMPFILEHEADER*)(pattern.data());
    bmpHeader->bmType = 0x4D42;
    bmpHeader->bmSize = sizeBmp;

//    if ( !bmp.loadFromData( (const uchar*)bmpHeader, pattern.size(), "BMP" ) ) {
    if ( !bmp.loadFromData( pattern, "BMP" ) ) {
        kDebug() << "KoWmfReadPrivate::dibToBmp: invalid bitmap " << endl;
        return false;
    }
    else {
        return true;
    }
}

