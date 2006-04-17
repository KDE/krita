/* Windows Meta File Loader/Painter Class Implementation
 *
 * Copyright ( C ) 1998 Stefan Taferner
 * Modified 2002 thierry lorthiois
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or ( at your
 * option ) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABLILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details. You should have received a copy
 * of the GNU General Public License along with this program; if not, write
 * to the Free Software Foundation, Inc, 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <math.h>
#include <assert.h>

#include <QFileInfo>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QDataStream>
#include <QApplication>
#include <QBuffer>
//Added by qt3to4:
#include <Q3CString>
#include <QPolygon>

#include <kdebug.h>

bool qwmfDebug = false;

#include "qwmf.h"
#include "wmfstruct.h"
#include "metafuncs.h"

#define QWMF_DEBUG  0


class WmfCmd
{
public:
    ~WmfCmd() { if ( next ) delete next; }
    WmfCmd* next;
    unsigned short funcIndex;
    long  numParm;
    short* parm;
};


class WinObjHandle
{
public:
    virtual void apply( QPainter& p ) = 0;
};

class WinObjBrushHandle: public WinObjHandle
{
public:
    virtual void apply( QPainter& p );
    QBrush brush;
    virtual ~WinObjBrushHandle() {};
};

class WinObjPenHandle: public WinObjHandle
{
public:
    virtual void apply( QPainter& p );
    QPen pen;
    virtual ~WinObjPenHandle() {};
};

class WinObjPatternBrushHandle: public WinObjHandle
{
public:
    virtual void apply( QPainter& p );
    QBrush brush;
    QPixmap image;
    virtual ~WinObjPatternBrushHandle() {};
};

class WinObjFontHandle: public WinObjHandle
{
public:
    virtual void apply( QPainter& p );
    QFont font;
    int rotation;
    virtual ~WinObjFontHandle() {};
};

void WinObjBrushHandle::apply( QPainter& p )
{
    p.setBrush( brush );
}

void WinObjPenHandle::apply( QPainter& p )
{
    p.setPen( pen );
}

void WinObjPatternBrushHandle::apply( QPainter& p )
{
    p.setBrush( brush );
}

void WinObjFontHandle::apply( QPainter& p )
{
    p.setFont( font );
}

#define MAX_OBJHANDLE 64



//-----------------------------------------------------------------------------
QWinMetaFile::QWinMetaFile()
{
    mValid       = false;
    mFirstCmd    = NULL;
    mObjHandleTab = NULL;
    mDpi         = 1000;
}


//-----------------------------------------------------------------------------
QWinMetaFile::~QWinMetaFile()
{
    if ( mFirstCmd ) delete mFirstCmd;
    if ( mObjHandleTab ) delete[] mObjHandleTab;
}


//-----------------------------------------------------------------------------
bool QWinMetaFile::load( const QString &filename )
{
    QFile file( filename );

    if ( !file.exists() )
    {
        kDebug() << "File " << QFile::encodeName(filename) << " does not exist" << endl;
        return false;
    }

    if ( !file.open( QIODevice::ReadOnly ) )
    {
        kDebug() << "Cannot open file " << QFile::encodeName(filename) << endl;
        return false;
    }

    QByteArray ba = file.readAll();
    file.close();

    QBuffer buffer( &ba );
    buffer.open( QIODevice::ReadOnly );
    return load( buffer );
}

//-----------------------------------------------------------------------------
bool QWinMetaFile::load( QBuffer &buffer )
{
    QDataStream st;
    WmfEnhMetaHeader eheader;
    WmfMetaHeader header;
    WmfPlaceableHeader pheader;
    WORD checksum;
    int filePos, idx, i;
    WmfCmd *cmd, *last;
    DWORD rdSize;
    WORD rdFunc;

    mTextAlign = 0;
    mRotation = 0;
    mTextColor = Qt::black;
    if ( mFirstCmd ) delete mFirstCmd;
    mFirstCmd = NULL;

    st.setDevice( &buffer );
    st.setByteOrder( QDataStream::LittleEndian ); // Great, I love Qt !

    //----- Read placeable metafile header
    st >> pheader.key;
    mIsPlaceable = ( pheader.key==( DWORD )APMHEADER_KEY );
    if ( mIsPlaceable )
    {
        st >> pheader.hmf;
        st >> pheader.bbox.left;
        st >> pheader.bbox.top;
        st >> pheader.bbox.right;
        st >> pheader.bbox.bottom;
        st >> pheader.inch;
        st >> pheader.reserved;
        st >> pheader.checksum;
        checksum = calcCheckSum( &pheader );
        if ( pheader.checksum!=checksum ) mIsPlaceable = false;

        mDpi = pheader.inch;
        mBBox.setLeft( pheader.bbox.left );
        mBBox.setTop( pheader.bbox.top );
        mBBox.setRight( pheader.bbox.right );
        mBBox.setBottom( pheader.bbox.bottom );
        mHeaderBoundingBox = mBBox;
        if ( QWMF_DEBUG )
        {
            kDebug() << endl << "-------------------------------------------------" << endl;
            kDebug() << "WMF Placeable Header ( " << static_cast<int>(sizeof( pheader ) ) << "):" << endl;
            kDebug() << "  bbox=( " << mBBox.left() << "; " << mBBox.top() << "; " << mBBox.width()
                      << "; " << mBBox.height() << ")" << endl;
            kDebug() << "  inch=" << pheader.inch << endl;
            kDebug() << "  checksum=" << pheader.checksum << "( "
                      << (pheader.checksum==checksum?"ok":"wrong") << " )" << endl;
        }
    }
    else buffer.reset();

    //----- Read as enhanced metafile header
    filePos = buffer.pos();
    st >> eheader.iType;
    st >> eheader.nSize;
    st >> eheader.rclBounds.left;
    st >> eheader.rclBounds.top;
    st >> eheader.rclBounds.right;
    st >> eheader.rclBounds.bottom;
    st >> eheader.rclFrame.left;
    st >> eheader.rclFrame.top;
    st >> eheader.rclFrame.right;
    st >> eheader.rclFrame.bottom;
    st >> eheader.dSignature;
    mIsEnhanced = ( eheader.dSignature==ENHMETA_SIGNATURE );
    if ( mIsEnhanced ) // is it really enhanced ?
    {
        st >> eheader.nVersion;
        st >> eheader.nBytes;
        st >> eheader.nRecords;
        st >> eheader.nHandles;
        st >> eheader.sReserved;
        st >> eheader.nDescription;
        st >> eheader.offDescription;
        st >> eheader.nPalEntries;
        st >> eheader.szlDevice.width;
        st >> eheader.szlDevice.height;
        st >> eheader.szlMillimeters.width;
        st >> eheader.szlMillimeters.height;

        if ( QWMF_DEBUG )
        {
            kDebug() << endl << "-------------------------------------------------" << endl;
            kDebug() << "WMF Extended Header:" << endl;
            kDebug() << "  iType=" << eheader.iType << endl;
            kDebug() << "  nSize=" << eheader.nSize << endl;
            kDebug() << "  rclBounds=( " << eheader.rclBounds.left << "; " << eheader.rclBounds.top << "; "
                      << eheader.rclBounds.right << "; " << eheader.rclBounds.bottom << ")" << endl;
            kDebug() << "  rclFrame=( " << eheader.rclFrame.left << "; " << eheader.rclFrame.top << "; "
                      << eheader.rclFrame.right << "; " << eheader.rclFrame.bottom << ")" << endl;
            kDebug() << "  nBytes=" << eheader.nBytes << endl;
            kDebug() << "\nNOT YET IMPLEMENTED, SORRY." << endl;
        }
    }
    else // no, not enhanced
    {
        //----- Read as standard metafile header
        buffer.seek( filePos );
        st >> header.mtType;
        st >> header.mtHeaderSize;
        st >> header.mtVersion;
        st >> header.mtSize;
        st >> header.mtNoObjects;
        st >> header.mtMaxRecord;
        st >> header.mtNoParameters;
        if ( QWMF_DEBUG ) {
            kDebug() << "WMF Header: " <<  "mtSize=" << header.mtSize << endl;
        }
    }

    //----- Test header validity
    mValid = ((header.mtHeaderSize == 9) && (header.mtNoParameters == 0)) || mIsEnhanced || mIsPlaceable;
    if ( mValid )
    {
        //----- Read Metafile Records
        last = NULL;
        rdFunc = -1;
        while ( !st.atEnd() && (rdFunc != 0) )
        {
            st >> rdSize;
            st >> rdFunc;
            idx = findFunc( rdFunc );
            rdSize -= 3;

            cmd = new WmfCmd;
            cmd->next = NULL;
            if ( last ) last->next = cmd;
            else mFirstCmd = cmd;

            cmd->funcIndex = idx;
            cmd->numParm = rdSize;
            cmd->parm = new WORD[ rdSize ];
            last = cmd;

            for ( i=0; i<rdSize && !st.atEnd(); i++ )
                st >> cmd->parm[ i ];


            if ( rdFunc == 0x020B ) {         // SETWINDOWORG: dimensions
                mBBox.setLeft( cmd->parm[ 1 ] );
                mBBox.setTop( cmd->parm[ 0 ] );
            }
            if ( rdFunc == 0x020C ) {         // SETWINDOWEXT: dimensions
                mBBox.setWidth( cmd->parm[ 1 ] );
                mBBox.setHeight( cmd->parm[ 0 ] );
            }

            if ( i<rdSize )
            {
                kDebug() << "WMF : file truncated !" << endl;
                return false;
            }
        }
        //----- Test records validities
        mValid = (rdFunc == 0) && (mBBox.width() != 0) && (mBBox.height() != 0);
        if ( !mValid ) {
            kDebug() << "WMF : incorrect file format !" << endl;
        }
    }
    else {
        kDebug() << "WMF Header : incorrect header !" << endl;
    }

    buffer.close();
    return mValid;
}


//-----------------------------------------------------------------------------
bool QWinMetaFile::paint( QPaintDevice* aTarget, bool absolute )
{
    int idx, i;
    WmfCmd* cmd;

    if ( !mValid )  return false;

    assert( aTarget!=NULL );
    if ( mPainter.isActive() ) return false;

    if ( mObjHandleTab ) delete[] mObjHandleTab;
    mObjHandleTab = new WinObjHandle* [ MAX_OBJHANDLE ];
    for ( i=MAX_OBJHANDLE-1; i>=0; i-- )
        mObjHandleTab[ i ] = NULL;

    mPainter.resetMatrix();
    mWinding = false;
    mAbsoluteCoord = absolute;

    mPainter.begin( aTarget );
    if ( QWMF_DEBUG )  {
        kDebug() << "Bounding box : " << mBBox.left()
        << " " << mBBox.top() << " " << mBBox.right() << " " << mBBox.bottom() << endl;
    }

    if ( mAbsoluteCoord ) {
        mPainter.setWindow( mBBox.top(), mBBox.left(), mBBox.width(), mBBox.height() );
    }
    mInternalWorldMatrix.reset();

    for ( cmd=mFirstCmd; cmd; cmd=cmd->next )
    {
        idx = cmd->funcIndex;
        ( this->*metaFuncTab[ idx ].method )( cmd->numParm, cmd->parm );

        if ( QWMF_DEBUG )  {
            QString str = "", param;
            if ( metaFuncTab[ idx ].name == NULL ) {
                str += "UNKNOWN ";
            }
            if ( metaFuncTab[ idx ].method == &QWinMetaFile::noop ) {
                str += "UNIMPLEMENTED ";
            }
            str += metaFuncTab[ idx ].name;
            str += " : ";

            for ( i=0 ; i < cmd->numParm ; i++ ) {
                param.setNum( cmd->parm[ i ] );
                str += param;
                str += " ";
            }
            kDebug() << str << endl;
        }
    }
/*
    // TODO: cleanup this code when QPicture::setBoundingBox() is possible in KOClipart (QT31)
    // because actually QPicture::boundingBox() != mBBox()
    mWindowsCoord += 1;
    if ( mWindowsCoord == 2 )  {
        kDebug() << "DRAW ANGLES " << endl;
        mPainter.setPen( Qt::white );
        mPainter.drawPoint( mBBox.left(), mBBox.top()  );
        mPainter.drawPoint( mBBox.right(), mBBox.bottom() );
    }
*/
    mPainter.end();
    return true;
}


//----------------s-------------------------------------------------------------
// Metafile painter methods
//-----------------------------------------------------------------------------
void QWinMetaFile::setWindowOrg( long, short* parm )
{
    if ( mAbsoluteCoord ) {
        QRect r = mPainter.window();
        mPainter.setWindow( parm[ 1 ], parm[ 0 ], r.width(), r.height() );
    }
    else {
        double dx = mInternalWorldMatrix.dx();
        double dy = mInternalWorldMatrix.dy();

        mInternalWorldMatrix.translate( -dx, -dy );
        mInternalWorldMatrix.translate( -parm[ 1 ], -parm[ 0 ] );
        mPainter.translate( -dx, -dy );
        mPainter.translate( -parm[ 1 ], -parm[ 0 ] );
    }
}


//-----------------------------------------------------------------------------
void QWinMetaFile::setWindowExt( long, short* parm )
{
    // negative value allowed for width and height : QABS() forbidden
    if ( mAbsoluteCoord ) {
        QRect r = mPainter.window();
        mPainter.setWindow( r.left(), r.top(), parm[ 1 ], parm[ 0 ] );
    }
    else {
        if ( (parm[ 0 ] != 0) && (parm[ 1 ] != 0) ) {
            QRect r = mPainter.window();
            double dx = mInternalWorldMatrix.dx();
            double dy = mInternalWorldMatrix.dy();
            double sx = mInternalWorldMatrix.m11();
            double sy = mInternalWorldMatrix.m22();

            mInternalWorldMatrix.translate( -dx, -dy );
            mInternalWorldMatrix.scale( 1/sx, 1/sy );
            mPainter.translate( -dx, -dy );
            mPainter.scale( 1/sx, 1/sy );

            sx = (double)r.width() / (double)parm[ 1 ];
            sy = (double)r.height() / (double)parm[ 0 ];

            mInternalWorldMatrix.scale( sx, sy );
            mInternalWorldMatrix.translate( dx, dy );
            mPainter.scale( sx, sy );
            mPainter.translate( dx, dy );
        }
    }
}


//-----------------------------------------------------------------------------
// Drawing
//-----------------------------------------------------------------------------
void QWinMetaFile::lineTo( long, short* parm )
{
  mLastPos = QPoint( parm[ 1 ], parm[ 0 ] );
}


//-----------------------------------------------------------------------------
void QWinMetaFile::moveTo( long, short* parm )
{
  mPainter.drawLine( mLastPos, QPoint( parm[1], parm[0] ) );
}


//-----------------------------------------------------------------------------
void QWinMetaFile::ellipse( long, short* parm )
{
    mPainter.drawEllipse( parm[ 3 ], parm[ 2 ], parm[ 1 ]-parm[ 3 ], parm[ 0 ]-parm[ 2 ] );
}


//-----------------------------------------------------------------------------
void QWinMetaFile::polygon( long, short* parm )
{
    QPolygon* pa;              // causing a memleck ???

    pa = pointArray( parm[ 0 ], &parm[ 1 ] );
    if( mWinding )
      mPainter.drawPolygon( *pa, Qt::WindingFill );
    else
      mPainter.drawPolygon( *pa, Qt::OddEvenFill );

    delete pa;
}


//-----------------------------------------------------------------------------
void QWinMetaFile::polyPolygon( long, short* parm )
{
    QRegion region;
    int  i, j, startPolygon;

    mPainter.save();

    // define clipping region
    QRect win = bbox();
    startPolygon = 1+parm[ 0 ];
    for ( i=0 ; i < parm[ 0 ] ; i++ ) {
        QPolygon pa1( parm[ 1+i ] );
        for ( j=0 ; j < parm[ 1+i ] ; j++) {
            pa1.setPoint ( j, parm[ startPolygon ], parm[ startPolygon+1 ] );
            startPolygon += 2;
        }
        QRegion r( pa1 );
        region = region.eor( r );
    }
    mPainter.setClipRegion( region );

    // fill polygons
    mPainter.fillRect( win.left(), win.top(), win.width(), win.height(), mPainter.brush() );

    // draw polygon's border if necessary
    if ( mPainter.pen().style() != Qt::NoPen ) {
        mPainter.setClipping( false );
        mPainter.setBrush( Qt::NoBrush );

        QPolygon* pa;
        int idxPolygon = 1 + parm[ 0 ];
        for ( i=0 ; i < parm[ 0 ] ; i++ ) {
            pa = pointArray( parm[ 1+i ], &parm[ idxPolygon ] );
            mPainter.drawPolygon( *pa );
            idxPolygon += parm[ 1+i ] * 2;
        }
    }

    mPainter.restore();
}


//-----------------------------------------------------------------------------
void QWinMetaFile::polyline( long, short* parm )
{
    QPolygon* pa;

    pa = pointArray( parm[ 0 ], &parm[ 1 ] );
    mPainter.drawPolyline( *pa );
}


//-----------------------------------------------------------------------------
void QWinMetaFile::rectangle( long, short* parm )
{
    mPainter.drawRect( parm[ 3 ], parm[ 2 ], parm[ 1 ]-parm[ 3 ], parm[ 0 ]-parm[ 2 ] );
}


//-----------------------------------------------------------------------------
void QWinMetaFile::roundRect( long, short* parm )
{
    int xRnd = 0, yRnd = 0;

    // convert (xRound, yRound) in percentage
    if ( (parm[ 3 ] - parm[ 5 ]) != 0  )
        xRnd = (parm[ 1 ] * 100) / (parm[ 3 ] - parm[ 5 ])  ;
    if ( (parm[ 2 ] - parm[ 4 ]) != 0  )
        yRnd = (parm[ 0 ] * 100) / (parm[ 2 ] - parm[ 4 ])  ;

    mPainter.drawRoundRect( parm[ 5 ], parm[ 4 ], parm[ 3 ]-parm[ 5 ], parm[ 2 ]-parm[ 4 ], xRnd, yRnd );
}


//-----------------------------------------------------------------------------
void QWinMetaFile::arc( long, short* parm )
{
    int xCenter, yCenter, angleStart, aLength;

    xCenter = parm[ 7 ] + ((parm[ 5 ] - parm[ 7 ]) / 2);
    yCenter = parm[ 6 ] + ((parm[ 4 ] - parm[ 6 ]) / 2);

    xyToAngle ( parm[ 3 ] - xCenter, yCenter - parm[ 2 ], parm[ 1 ] - xCenter, yCenter - parm[ 0 ], angleStart, aLength );

    mPainter.drawArc( parm[ 7 ], parm[ 6 ], parm[ 5 ]-parm[ 7 ], parm[ 4 ]-parm[ 6 ], angleStart, aLength);
}


//-----------------------------------------------------------------------------
void QWinMetaFile::chord( long, short* parm )
{
    int xCenter, yCenter, angleStart, aLength;

    xCenter = parm[ 7 ] + ((parm[ 5 ] - parm[ 7 ]) / 2);
    yCenter = parm[ 6 ] + ((parm[ 4 ] - parm[ 6 ]) / 2);

    xyToAngle ( parm[ 3 ] - xCenter, yCenter - parm[ 2 ], parm[ 1 ] - xCenter, yCenter - parm[ 0 ], angleStart, aLength );

    mPainter.drawChord( parm[ 7 ], parm[ 6 ], parm[ 5 ]-parm[ 7 ], parm[ 4 ]-parm[ 6 ], angleStart, aLength);
}


//-----------------------------------------------------------------------------
void QWinMetaFile::pie( long, short* parm )
{
    int xCenter, yCenter, angleStart, aLength;

    xCenter = parm[ 7 ] + ((parm[ 5 ] - parm[ 7 ]) / 2);
    yCenter = parm[ 6 ] + ((parm[ 4 ] - parm[ 6 ]) / 2);

    xyToAngle ( parm[ 3 ] - xCenter, yCenter - parm[ 2 ], parm[ 1 ] - xCenter, yCenter - parm[ 0 ], angleStart, aLength );

    mPainter.drawPie( parm[ 7 ], parm[ 6 ], parm[ 5 ]-parm[ 7 ], parm[ 4 ]-parm[ 6 ], angleStart, aLength);
}


//-----------------------------------------------------------------------------
void QWinMetaFile::setPolyFillMode( long, short* parm )
{
    mWinding = parm[ 0 ];
}


//-----------------------------------------------------------------------------
void QWinMetaFile::setBkColor( long, short* parm )
{
    mPainter.setBackground( QBrush( color( parm ) ) );
}


//-----------------------------------------------------------------------------
void QWinMetaFile::setBkMode( long, short* parm )
{
    if ( parm[ 0 ]==1 ) mPainter.setBackgroundMode( Qt::TransparentMode );
    else mPainter.setBackgroundMode( Qt::OpaqueMode );
}


//-----------------------------------------------------------------------------
void QWinMetaFile::setPixel( long, short* parm )
{
    QPen pen = mPainter.pen();
    mPainter.setPen( color( parm ) );
    mPainter.drawPoint( parm[ 3 ], parm[ 2 ] );
    mPainter.setPen( pen );
}


//-----------------------------------------------------------------------------
void QWinMetaFile::setRop( long, short* parm )
{
    mPainter.setCompositionMode( winToQtComposition( parm[ 0 ] ) );
}


//-----------------------------------------------------------------------------
void QWinMetaFile::saveDC( long, short* )
{
    mPainter.save();
}


//-----------------------------------------------------------------------------
void QWinMetaFile::restoreDC( long, short *parm )
{
    for ( int i=0; i > parm[ 0 ] ; i-- )
        mPainter.restore();
}


//-----------------------------------------------------------------------------
void QWinMetaFile::intersectClipRect( long, short* parm )
{
/*  TODO: better implementation : need QT 3.0.2
    QRegion region = mPainter.clipRegion();
    if ( region.isEmpty() )
        region = bbox();
*/
    QRegion region( bbox() );

    QRegion newRegion( parm[ 3 ], parm[ 2 ], parm[ 1 ] - parm[ 3 ], parm[ 0 ] - parm[ 2 ] );
    region = region.intersect( newRegion );

    mPainter.setClipRegion( region );
}


//-----------------------------------------------------------------------------
void QWinMetaFile::excludeClipRect( long, short* parm )
{
/*  TODO: better implementation : need QT 3.0.2
    QRegion region = mPainter.clipRegion();
    if ( region.isEmpty() )
        region = bbox();
*/
    QRegion region( bbox() );

    QRegion newRegion( parm[ 3 ], parm[ 2 ], parm[ 1 ] - parm[ 3 ], parm[ 0 ] - parm[ 2 ] );
    region = region.subtract( newRegion );

    mPainter.setClipRegion( region );
}


//-----------------------------------------------------------------------------
// Text
//-----------------------------------------------------------------------------
void QWinMetaFile::setTextColor( long, short* parm )
{
    mTextColor = color( parm );
}


//-----------------------------------------------------------------------------
void QWinMetaFile::setTextAlign( long, short* parm )
{
    mTextAlign = parm[ 0 ];
}


//-----------------------------------------------------------------------------
void QWinMetaFile::textOut( long num, short* parm )
{

    short *copyParm = new short[ num + 1 ];

    // re-order parameters
    int idxOffset = (parm[ 0 ] / 2) + 1 + (parm[ 0 ] & 1);
    copyParm[ 0 ] = parm[ idxOffset ];
    copyParm[ 1 ] = parm[ idxOffset + 1 ];
    copyParm[ 2 ] = parm[ 0 ];
    copyParm[ 3 ] = 0;
    memcpy( &copyParm[ 4 ], &parm[ 1 ], parm[ 0 ] );

    extTextOut( num + 1, copyParm );
    delete [] copyParm;
}


//-----------------------------------------------------------------------------
void QWinMetaFile::extTextOut( long num, short* parm )
{
    char* ptStr;
    int x, y, width, height;
    int idxOffset;

    if ( parm[ 3 ] != 0 )       // ETO_CLIPPED flag add 4 parameters
        ptStr = (char*)&parm[ 8 ];
    else
        ptStr = (char*)&parm[ 4 ];

    Q3CString text( ptStr, parm[ 2 ] + 1 );

    QFontMetrics fm( mPainter.font() );
    width = fm.width( text ) + fm.descent();  // because fm.width(text) isn't rigth with Italic text
    height = fm.height();

    mPainter.save();

    if ( mTextAlign & 0x01 ) {       // (left, top) position = current logical position
        x = mLastPos.x();
        y = mLastPos.y();
    }
    else {                           // (left, top) position = parameters
        x = parm[ 1 ];
        y = parm[ 0 ];
    }

    if ( mRotation ) {
        mPainter.translate( parm[ 1 ], parm[ 0 ]);
        mPainter.rotate ( mRotation );
        mPainter.translate( -parm[ 1 ], -parm[ 0 ] );
    }

    // alignment
    if ( mTextAlign & 0x06 )
        x -= ( width / 2 );
    if ( mTextAlign & 0x08 )
        y -= (height - fm.descent());

    mPainter.setPen( mTextColor );
    idxOffset = (parm[ 2 ] / 2) + 4 + (parm[ 2 ] & 1);
    if ( ( parm[ 2 ] > 1 ) && ( num >= (idxOffset + parm[ 2 ]) ) && ( parm[ 3 ] == 0 ) ) {
        // offset for each char
        int left = x;
        mPainter.drawText( left, y, width, height, Qt::AlignLeft | Qt::AlignTop, text.mid(0, 1) );
        for ( int i = 1; i < parm[ 2 ] ; i++ ) {
            left += parm[ idxOffset + i - 1 ];
            mPainter.drawText( left, y, width, height, Qt::AlignLeft | Qt::AlignTop, text.mid(i, 1) );
        }
    }
    else {
        mPainter.drawText( x, y, width, height, Qt::AlignLeft | Qt::AlignTop, text );
    }

    mPainter.restore();

}



//-----------------------------------------------------------------------------
// Bitmap
//-----------------------------------------------------------------------------
void QWinMetaFile::dibBitBlt( long num, short* parm )
{
    if ( num > 9 ) {      // DIB image
        QImage bmpSrc;

        if ( dibToBmp( bmpSrc, (char*)&parm[ 8 ], (num - 8) * 2 ) ) {
            long raster = toDWord( parm );

            mPainter.setCompositionMode( winToQtComposition( raster )  );

            // wmf file allow negative width or height
            mPainter.save();
            if ( parm[ 5 ] < 0 ) {  // width < 0 => horizontal flip
                QMatrix m( -1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F );
                mPainter.setMatrix( m, true );
            }
            if ( parm[ 4 ] < 0 ) {  // height < 0 => vertical flip
                QMatrix m( 1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F );
                mPainter.setMatrix( m, true );
            }
            mPainter.drawImage( parm[ 7 ], parm[ 6 ], bmpSrc, parm[ 3 ], parm[ 2 ], parm[ 5 ], parm[ 4 ] );
            mPainter.restore();
        }
    }
    else {
        kDebug() << "QWinMetaFile::dibBitBlt without image: not implemented " << endl;
    }
}


//-----------------------------------------------------------------------------
void QWinMetaFile::dibStretchBlt( long num, short* parm )
{
    QImage bmpSrc;

    if ( dibToBmp( bmpSrc, (char*)&parm[ 10 ], (num - 10) * 2 ) ) {
        long raster = toDWord( parm );

        mPainter.setCompositionMode( winToQtComposition( raster )  );

        // wmf file allow negative width or height
        mPainter.save();
        if ( parm[ 7 ] < 0 ) {  // width < 0 => horizontal flip
            QMatrix m( -1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F );
            mPainter.setMatrix( m, true );
        }
        if ( parm[ 6 ] < 0 ) {  // height < 0 => vertical flip
            QMatrix m( 1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F );
            mPainter.setMatrix( m, true );
        }
        bmpSrc = bmpSrc.copy( parm[ 5 ], parm[ 4 ], parm[ 3 ], parm[ 2 ] );
        // TODO: scale the bitmap ( QImage::scale(parm[ 7 ], parm[ 6 ]) is actually too slow )

        mPainter.drawImage( parm[ 9 ], parm[ 8 ], bmpSrc );
        mPainter.restore();
    }
}


//-----------------------------------------------------------------------------
void QWinMetaFile::stretchDib( long num, short* parm )
{
    QImage bmpSrc;

    if ( dibToBmp( bmpSrc, (char*)&parm[ 11 ], (num - 11) * 2 ) ) {
        long raster = toDWord( parm );

        mPainter.setCompositionMode( winToQtComposition( raster )  );

        // wmf file allow negative width or height
        mPainter.save();
        if ( parm[ 8 ] < 0 ) {  // width < 0 => horizontal flip
            QMatrix m( -1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F );
            mPainter.setMatrix( m, true );
        }
        if ( parm[ 7 ] < 0 ) {  // height < 0 => vertical flip
            QMatrix m( 1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F );
            mPainter.setMatrix( m, true );
        }
        bmpSrc = bmpSrc.copy( parm[ 6 ], parm[ 5 ], parm[ 4 ], parm[ 3 ] );
        // TODO: scale the bitmap ( QImage::scale(parm[ 8 ], parm[ 7 ]) is actually too slow )

        mPainter.drawImage( parm[ 10 ], parm[ 9 ], bmpSrc );
        mPainter.restore();
    }
}


//-----------------------------------------------------------------------------
void QWinMetaFile::dibCreatePatternBrush( long num, short* parm )
{
    WinObjPatternBrushHandle* handle = new WinObjPatternBrushHandle;
    addHandle( handle );
    QImage bmpSrc;

    if ( dibToBmp( bmpSrc, (char*)&parm[ 2 ], (num - 2) * 2 ) ) {
        handle->image = QPixmap::fromImage( bmpSrc );
        handle->brush.setTexture( handle->image );
    }
}


//-----------------------------------------------------------------------------
// Object handle
//-----------------------------------------------------------------------------
void QWinMetaFile::selectObject( long, short* parm )
{
    int idx = parm[ 0 ];
    if ( idx>=0 && idx < MAX_OBJHANDLE && mObjHandleTab[ idx ] )
        mObjHandleTab[ idx ]->apply( mPainter );
}


//-----------------------------------------------------------------------------
void QWinMetaFile::deleteObject( long, short* parm )
{
    deleteHandle( parm[ 0 ] );
}


//-----------------------------------------------------------------------------
void QWinMetaFile::createEmptyObject( long, short* )
{
    // allocation of an empty object (to keep object counting in sync)
    WinObjPenHandle* handle = new WinObjPenHandle;
    addHandle( handle );
    kDebug() << "QWinMetaFile: unimplemented createObject " << endl;
}


//-----------------------------------------------------------------------------
void QWinMetaFile::createBrushIndirect( long, short* parm )
{
    static Qt::BrushStyle hatchedStyleTab[] =
    {
        Qt::HorPattern,
        Qt::FDiagPattern,
        Qt::BDiagPattern,
        Qt::CrossPattern,
        Qt::DiagCrossPattern
    };
    static Qt::BrushStyle styleTab[] =
    { Qt::SolidPattern,
      Qt::NoBrush,
      Qt::FDiagPattern,   /* hatched */
      Qt::Dense4Pattern,  /* should be custom bitmap pattern */
      Qt::HorPattern,     /* should be BS_INDEXED (?) */
      Qt::VerPattern,     /* should be device-independent bitmap */
      Qt::Dense6Pattern,  /* should be device-independent packed-bitmap */
      Qt::Dense2Pattern,  /* should be BS_PATTERN8x8 */
      Qt::Dense3Pattern   /* should be device-independent BS_DIBPATTERN8x8 */
    };
    Qt::BrushStyle style;
    short arg;
    WinObjBrushHandle* handle = new WinObjBrushHandle;
    addHandle( handle );

    arg = parm[ 0 ];
    if ( arg==2 )
    {
        arg = parm[ 3 ];
        if ( arg>=0 && arg<5 ) style = hatchedStyleTab[ arg ];
        else
        {
            kDebug() << "QWinMetaFile::createBrushIndirect: invalid hatched brush " << arg << endl;
            style = Qt::SolidPattern;
        }
    }
    else if ( arg>=0 && arg<9 )
        style = styleTab[ arg ];
    else
    {
        kDebug() << "QWinMetaFile::createBrushIndirect: invalid brush " << arg << endl;
        style = Qt::SolidPattern;
    }
    handle->brush.setStyle( style );
    handle->brush.setColor( color( parm+1 ) );
}


//-----------------------------------------------------------------------------
void QWinMetaFile::createPenIndirect( long, short* parm )
{
    static Qt::PenStyle styleTab[] =
    { Qt::SolidLine, Qt::DashLine, Qt::DotLine, Qt::DashDotLine, Qt::DashDotDotLine,
      Qt::NoPen, Qt::SolidLine };
    Qt::PenStyle style;
    WinObjPenHandle* handle = new WinObjPenHandle;
    addHandle( handle );

    if ( parm[ 0 ]>=0 && parm[ 0 ]<6 ) style=styleTab[ parm[ 0 ] ];
    else
    {
        kDebug() << "QWinMetaFile::createPenIndirect: invalid pen " << parm[ 0 ] << endl;
        style = Qt::SolidLine;
    }

    handle->pen.setStyle( style );
    handle->pen.setColor( color( parm+3 ) );
    handle->pen.setCapStyle( Qt::RoundCap );

    //int width = 0;
    // TODO : width of pen proportional to device context width
    // DOESN'T WORK
/*
    QRect devRec;
    devRec = mPainter.transformed( mBBox );
    width = ( parm[ 0 ] * devRec.width() ) / mBBox.width() ;
    kDebug() << "CreatePenIndirect: " <<  endl;
    kDebug() << "   log coord. : " << mBBox.width() << "   " << mBBox.height() << endl;
    kDebug() << "   log. pen : " << parm[ 1 ] << "   " << parm[ 2 ] << endl;
    kDebug() << "   dev. pen : " << width << endl;
    handle->pen.setWidth( width );
*/
}


//-----------------------------------------------------------------------------
void QWinMetaFile::createFontIndirect( long , short* parm)
{
    WinObjFontHandle* handle = new WinObjFontHandle;
    addHandle( handle );

    QString family( (const char*)&parm[ 9 ] );

    mRotation = -parm[ 2 ]  / 10;               // text rotation (in 1/10 degree)
                                                // TODO: memorisation of rotation in object Font
    handle->font.setFamily( family );
    handle->font.setFixedPitch( ((parm[ 8 ] & 0x01) == 0) );
    // TODO: investigation why some test case need -2. (size of font in logical point)
    handle->font.setPointSize( QABS(parm[ 0 ]) - 2 );
    handle->font.setWeight( (parm[ 4 ] >> 3) );
    handle->font.setItalic( (parm[ 5 ] & 0x01) );
    handle->font.setUnderline( (parm[ 5 ] & 0x100) );
}


//-----------------------------------------------------------------------------
// Misc
//-----------------------------------------------------------------------------
void QWinMetaFile::noop( long, short* )
{
}


void QWinMetaFile::end( long, short* )
{
    // end of file :
//    kDebug() << "END bbox=( " << mBBox.left() << "; " << mBBox.top() << "; " << mBBox.width() << "; " << mBBox.height() << ")" << endl;
}


//-----------------------------------------------------------------------------
unsigned short QWinMetaFile::calcCheckSum( WmfPlaceableHeader* apmfh )
{
    WORD*  lpWord;
    WORD   wResult, i;

    // Start with the first word
    wResult = *( lpWord = ( WORD* )( apmfh ) );
    // XOR in each of the other 9 words
    for( i=1; i<=9; i++ )
    {
        wResult ^= lpWord[ i ];
    }
    return wResult;
}


//-----------------------------------------------------------------------------
int QWinMetaFile::findFunc( unsigned short aFunc ) const
{
    int i;

    for ( i=0; metaFuncTab[ i ].name; i++ )
        if ( metaFuncTab[ i ].func == aFunc ) return i;

    // here : unknown function
    return i;
}

//-----------------------------------------------------------------------------
QPolygon* QWinMetaFile::pointArray( short num, short* parm )
{
    int i;

    mPoints.resize( num );

    for ( i=0; i<num; i++, parm+=2 )
        mPoints.setPoint( i, parm[ 0 ], parm[ 1 ] );

    return &mPoints;
}

//-----------------------------------------------------------------------------
unsigned int QWinMetaFile::toDWord( short* parm )
{
    unsigned int l;

#if !defined( WORDS_BIGENDIAN )
    l = *( unsigned int* )( parm );
#else
    char *bytes;
    char swap[ 4 ];
    bytes = ( char* )parm;
    swap[ 0 ] = bytes[ 2 ];
    swap[ 1 ] = bytes[ 3 ];
    swap[ 2 ] = bytes[ 0 ];
    swap[ 3 ] = bytes[ 1 ];
    l = *( unsigned int* )( swap );
#endif

    return l;
}


//-----------------------------------------------------------------------------
QColor QWinMetaFile::color( short* parm )
{
    unsigned int colorRef;
    int red, green, blue;

    colorRef = toDWord( parm ) & 0xffffff;
    red      = colorRef & 255;
    green    = ( colorRef>>8 ) & 255;
    blue     = ( colorRef>>16 ) & 255;

    return QColor( red, green, blue );
}


//-----------------------------------------------------------------------------
void QWinMetaFile::xyToAngle( int xStart, int yStart, int xEnd, int yEnd, int& angleStart, int& angleLength )
{
    float aStart, aLength;

    aStart = atan2( yStart,  xStart );
    aLength = atan2( yEnd, xEnd ) - aStart;

    angleStart = (int)(aStart * 2880 / 3.14166);
    angleLength = (int)(aLength * 2880 / 3.14166);
    if ( angleLength < 0 ) angleLength = 5760 + angleLength;
}


//-----------------------------------------------------------------------------
void QWinMetaFile::addHandle( WinObjHandle* handle )
{
    int idx;

    for ( idx=0; idx < MAX_OBJHANDLE ; idx++ )
        if ( mObjHandleTab[ idx ] == NULL )  break;

    if ( idx < MAX_OBJHANDLE )
        mObjHandleTab[ idx ] = handle;
    else
        kDebug() << "QWinMetaFile error: handle table full !" << endl;
}

//-----------------------------------------------------------------------------
void QWinMetaFile::deleteHandle( int idx )
{
    if ( idx >= 0 && idx < MAX_OBJHANDLE && mObjHandleTab[ idx ] )
    {
        delete mObjHandleTab[ idx ];
        mObjHandleTab[ idx ] = NULL;
    }
}

//-----------------------------------------------------------------------------
QPainter::CompositionMode QWinMetaFile::winToQtComposition( short parm ) const
{
    static const QPainter::CompositionMode opTab[] =
    {
        // ### untested (conversion from Qt::RasterOp)
        QPainter::CompositionMode_Source, // Qt::CopyROP
        QPainter::CompositionMode_Clear, // Qt::ClearROP
        QPainter::CompositionMode_SourceOut, // Qt::NandROP
        QPainter::CompositionMode_SourceOut, // Qt::NotAndROP
        QPainter::CompositionMode_DestinationOut, // Qt::NotCopyROP
        QPainter::CompositionMode_DestinationOut, // Qt::AndNotROP
        QPainter::CompositionMode_DestinationOut, // Qt::NotROP
        QPainter::CompositionMode_Xor, // Qt::XorROP
        QPainter::CompositionMode_Source, // Qt::NorROP
        QPainter::CompositionMode_SourceIn, // Qt::AndROP
        QPainter::CompositionMode_SourceIn, // Qt::NotXorROP
        QPainter::CompositionMode_Destination, // Qt::NopROP
        QPainter::CompositionMode_Destination, // Qt::NotOrROP
        QPainter::CompositionMode_Source, // Qt::CopyROP
        QPainter::CompositionMode_Source, // Qt::OrNotROP
        QPainter::CompositionMode_SourceOver, // Qt::OrROP
        QPainter::CompositionMode_Source // Qt::SetROP
    };

    if ( parm > 0 && parm <= 16 )
        return opTab[ parm ];
    else
        return QPainter::CompositionMode_Source;
}

//-----------------------------------------------------------------------------
QPainter::CompositionMode  QWinMetaFile::winToQtComposition( long parm ) const
{
    /* TODO: Ternary raster operations
    0x00C000CA  dest = (source AND pattern)
    0x00F00021  dest = pattern
    0x00FB0A09  dest = DPSnoo
    0x005A0049  dest = pattern XOR dest   */
    static const struct OpTab
    {
        long winRasterOp;
        QPainter::CompositionMode qtRasterOp;
    } opTab[] =
    {
      // ### untested (conversion from Qt::RasterOp)
      { 0x00CC0020, QPainter::CompositionMode_Source }, // CopyROP
      { 0x00EE0086, QPainter::CompositionMode_SourceOver }, // OrROP
      { 0x008800C6, QPainter::CompositionMode_SourceIn }, // AndROP
      { 0x00660046, QPainter::CompositionMode_Xor }, // XorROP
      { 0x00440328, QPainter::CompositionMode_DestinationOut }, // AndNotROP
      { 0x00330008, QPainter::CompositionMode_DestinationOut }, // NotCopyROP
      { 0x001100A6, QPainter::CompositionMode_SourceOut }, // NandROP
      { 0x00C000CA, QPainter::CompositionMode_Source }, // CopyROP
      { 0x00BB0226, QPainter::CompositionMode_Destination }, // NotOrROP
      { 0x00F00021, QPainter::CompositionMode_Source }, // CopyROP
      { 0x00FB0A09, QPainter::CompositionMode_Source }, // CopyROP
      { 0x005A0049, QPainter::CompositionMode_Source }, // CopyROP
      { 0x00550009, QPainter::CompositionMode_DestinationOut }, // NotROP
      { 0x00000042, QPainter::CompositionMode_Clear }, // ClearROP
      { 0x00FF0062, QPainter::CompositionMode_Source } // SetROP
    };

    int i;
    for ( i=0 ; i < 15 ; i++ )
        if ( opTab[ i ].winRasterOp == parm )
            break;

    if ( i < 15 )
        return opTab[ i ].qtRasterOp;
    else
        return QPainter::CompositionMode_Source;
}

//-----------------------------------------------------------------------------
bool QWinMetaFile::dibToBmp( QImage& bmp, const char* dib, long size )
{
    typedef struct _BMPFILEHEADER {
        WORD bmType;
        DWORD bmSize;
        WORD bmReserved1;
        WORD bmReserved2;
        DWORD bmOffBits;
    }  BMPFILEHEADER;

    int sizeBmp = size + 14;
    QByteArray pattern;       // BMP header and DIB data
    pattern.fill( 0, sizeBmp );  //resize and fill
    pattern.insert( 14, QByteArray::fromRawData(dib, size) );

    // add BMP header
    BMPFILEHEADER* bmpHeader;
    bmpHeader = (BMPFILEHEADER*)((const char*)pattern);
    bmpHeader->bmType = 0x4D42;
    bmpHeader->bmSize = sizeBmp;

    if ( !bmp.loadFromData( (const uchar*)bmpHeader, pattern.size(), "BMP" ) ) {
        kDebug() << "QWinMetaFile::dibToBmp: invalid bitmap " << endl;
        return false;
    }
    else {
//        if ( bmp.save("/home/software/kde-cvs/qt/examples/wmf/test.bmp", "BMP") )
//        if ( bmp.load( "/home/software/kde-cvs/qt/examples/wmf/test.bmp", "BMP" ) )
//            fprintf(stderr, "Bitmap ok \n");
        return true;
    }
}

