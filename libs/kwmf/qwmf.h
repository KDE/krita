/* Windows Meta File Loader
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
#ifndef qwmf_h
#define qwmf_h

#include <qstring.h>
#include <qpainter.h>
#include <qmatrix.h>
#include <q3pointarray.h>
#include <qpen.h>
#include <qcolor.h>
#include <qimage.h>
#include <qrect.h>

class QBuffer;
class QString;
class WmfCmd;
class WinObjHandle;
class WinObjPenHandle;
class WinObjBrushHandle;
struct WmfPlaceableHeader;

/**
 * QWinMetaFile is a WMF viewer based on QT toolkit
 * How to use QWinMetaFile :
 * <pre>
 * #include "qwmf.h"
 * QWinMetaFile wmf;
 * QPicture pic;     // or QImage pic;
 * if ( wmf.load( filename )
 *    wmf.paint( &pic );
 * </pre>
 */


class QWinMetaFile
{
public:
    QWinMetaFile();
    virtual ~QWinMetaFile();

    /**
     * Load WMF file.
     * @return true on success.
     */
    virtual bool load( const QString &fileName );
    virtual bool load( QBuffer &buffer );

    /**
     * Paint metafile to given paint-device using absolute or relative coordinate.
     * - absolute coord. Reset the world transfomation Matrix
     * - relative coord. Use the existing world transfomation Matrix
     *
     * @return true on success.
     */
    virtual bool paint( QPaintDevice* target, bool absolute=false );

    /**
     * @return true if the metafile is placeable.
     */
    bool isPlaceable( void ) const { return mIsPlaceable; }

    /**
     * @return true if the metafile is enhanced.
     */
    bool isEnhanced( void ) const { return mIsEnhanced; }

    /**
     * @return bounding rectangle
     */
    QRect bbox( void ) const { return mBBox; }

public: // should be protected but cannot
    /* Metafile painter methods */

    /** set window origin */
    void setWindowOrg( long num, short* parms );
    /** set window extents */
    void setWindowExt( long num, short* parms );

    /****************** Drawing *******************/
    /** draw line to coord */
    void lineTo( long num, short* parms );
    /** move pen to coord */
    void moveTo( long num, short* parms );
    /** draw ellipse */
    void ellipse( long num, short* parms );
    /** draw polygon */
    void polygon( long num, short* parms );
    /** draw a list of polygons */
    void polyPolygon( long num, short* parms );
    /** draw series of lines */
    void polyline( long num, short* parms );
    /** draw a rectangle */
    void rectangle( long num, short* parms );
    /** draw round rectangle */
    void roundRect( long num, short* parms );
    /** draw arc */
    void arc( long num, short* parms );
    /** draw chord */
    void chord( long num, short* parms );
    /** draw pie */
    void pie( long num, short* parms );
    /** set polygon fill mode */
    void setPolyFillMode( long num, short* parms );
    /** set background pen color */
    void setBkColor( long num, short* parms );
    /** set background pen mode */
    void setBkMode( long num, short* parms );
    /** set a pixel */
    void setPixel( long num, short* parms );
    /** Set raster operation mode */
    void setRop( long num, short* parms );
    /** save device context */
    void saveDC( long num, short* parms );
    /** restore device context */
    void restoreDC( long num, short* parms );
    /**  clipping region is the intersection of this region and the original region */
    void intersectClipRect( long num, short* parms );
    /** delete a clipping rectangle of the original region */
    void excludeClipRect( long num, short* parms );

    /****************** Text *******************/
    /** set text color */
    void setTextColor( long num, short* parms );
    /** set text alignment */
    void setTextAlign( long num, short* parms );
    /** draw text */
    void textOut( long num, short* parms );
    void extTextOut( long num, short* parms );

    /****************** Bitmap *******************/
    /** copies a DIB into a dest location */
    void dibBitBlt( long num, short* parms );
    /** stretches a DIB into a dest location */
    void dibStretchBlt( long num, short* parms );
    void stretchDib( long num, short* parms );
    /** create a pattern brush */
    void dibCreatePatternBrush( long num, short* parms );

    /****************** Object handle *******************/
    /** Activate object handle */
    void selectObject( long num, short* parms );
    /** Free object handle */
    void deleteObject( long num, short* parms );
    /** create an empty object in the object list */
    void createEmptyObject( long num, short* parms );
    /** create a logical brush */
    void createBrushIndirect( long num, short* parms );
    /** create a logical pen */
    void createPenIndirect( long num, short* parms );
    /** create a logical font */
    void createFontIndirect( long num, short* parms );

    /****************** misc *******************/
    /** nothing to do */
    void noop( long , short* );
    /** end of meta file */
    void end( long /*num*/, short* /*parms*/ );
    /** Resolution of the image in dots per inch */
    int dpi( void ) const { return mDpi; }

protected:
    /** Calculate header checksum */
    unsigned short calcCheckSum( WmfPlaceableHeader* );

    /** Find function in metafunc table by metafile-function.
        Returns index or -1 if not found. */
    virtual int findFunc( unsigned short aFunc ) const;

    /** Fills given parms into mPoints. */
    QPolygon* pointArray( short num, short* parms );

    /** Returns color given by the two parameters */
    QColor color( short* parm );

    /** Converts two parameters to long */
    unsigned int toDWord( short* parm );

    /** Convert (x1,y1) and (x2, y2) positions in angle and angleLength */
    void xyToAngle( int xStart, int yStart, int xEnd, int yEnd, int& angle, int& aLength );

    /** Handle win-object-handles */
    void addHandle( WinObjHandle*  );
    void deleteHandle( int );

    /** Convert windows rasterOp in QT rasterOp */
    QPainter::CompositionMode winToQtComposition( short parm ) const;
    QPainter::CompositionMode winToQtComposition( long parm ) const;

    /** Converts DIB to BMP */
    bool dibToBmp( QImage& bmp, const char* dib, long size);

protected:
    QPainter mPainter;
    bool mIsPlaceable, mIsEnhanced, mValid;

    // coordinate system
    bool   mAbsoluteCoord;
    QMatrix  mInternalWorldMatrix;   // memorisation of WMF matrix transformation
    QRect mHeaderBoundingBox;
    QRect mBBox;

    // information shared between Metafile Functions
    QColor mTextColor;
    int mTextAlign, mRotation;
    bool mWinding;

    WmfCmd* mFirstCmd;
    WinObjHandle** mObjHandleTab;
    QPolygon mPoints;
    int mDpi;
    QPoint mLastPos;
};

#endif /*qwmf_h*/
