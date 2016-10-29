/* This file is part of the KDE libraries
 *
 * Copyright (c) 1998 Stefan Taferner
 *               2001/2003 thierry lorthiois (lorthioist@wanadoo.fr)
 *               2009-2011 Inge Wallin <inge@lysator.liu.se>
 * With the help of WMF documentation by Caolan Mc Namara

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
 * Boston, MA 02110-1301, USA.
 */

#ifndef _WMFPARSER_H_
#define _WMFPARSER_H_

#include <QColor>
#include <QRect>

#include "WmfEnums.h"
#include "WmfStructs.h"
#include "WmfDeviceContext.h"
#include "WmfStack.h"

class WmfAbstractBackend;
class QBuffer;
class QPolygon;

/**
   Namespace for Windows Metafile (WMF) classes
*/
namespace Libwmf
{


/**
 * WmfParser allows to read WMF files
 *
 */

class WmfParser
{
public:
    WmfParser();
    virtual ~WmfParser();

    /**
     * Load WMF file. Returns true on success.
     */
    bool load(const QByteArray& array);

    /**
     * Plays a metafile using @p backend as backend and returns true on success.
     * To draw on a device you have to inherit the class WmfAbstractBackend.
     */
    bool play(WmfAbstractBackend* backend);

    /****************** Object handle *******************/
    /// create an empty object in the object list
    void createEmptyObject();

    /****************** misc *******************/

    /** Calculate header checksum */
    static quint16 calcCheckSum(WmfPlaceableHeader*);

private:
    //-----------------------------------------------------------------------------
    // Utilities and conversion Wmf -> Qt

    // Create a boundingbox from all set{Window,Viewport}{Org,Ext} records.
    void createBoundingBox(QDataStream &st);

    /** Handle win-object-handles */
    bool addHandle(KoWmfHandle*);
    void deleteHandle(int);

    /** Convert QINT16 points into QPointArray */
    void pointArray(QDataStream& stream, QPolygon& pa);

    /** Convertion between windows color and QColor */
    QColor qtColor(quint32 color) const {
        return QColor(color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF);
    }

    /** Convert (x1,y1) and (x2, y2) positions in angle and angleLength */
    void xyToAngle(int xStart, int yStart, int xEnd, int yEnd, int& angle, int& aLength);

    /** Convert windows rasterOp in QT rasterOp */
    QPainter::CompositionMode winToQtComposition(quint16 param) const;
    QPainter::CompositionMode winToQtComposition(quint32 param) const;

    /** Converts DIB to BMP */
    bool dibToBmp(QImage& bmp, QDataStream& stream, quint32 size);


public:
    // state of the WMF
    bool mValid;
    bool mStandard;
    bool mPlaceable;
    bool mEnhanced;

    // Bounding rectangle.  In a placeable file this is in the header,
    // otherwise its comprised of calls to setWindowOrg and setWindowExt.
    //
    // We can't use a QRect here because width and/or height may be negative.
    qint16 mBBoxTop;
    qint16 mBBoxLeft;
    qint16 mBBoxRight;
    qint16 mBBoxBottom;
    qint16 mMaxWidth;
    qint16 mMaxHeight;

    // standard file : this is the value in setWindowOrg and setWindowExt
    // number of points per inch for the default size
    int mDpi;

    /// number of functions to draw (==0 for all)
    int mNbrFunc;

private:
    // the output strategy
    WmfAbstractBackend *m_backend;

    // Current state of the drawing
    WmfDeviceContext  mDeviceContext;

    WmfLayout   mLayout;
    QColor      mTextColor;
    bool        mWinding;
    quint16     mMapMode;

    // Memory allocation for WMF file
    QBuffer*  mBuffer;
    int    mOffsetFirstRecord;

    // stack of object handle
    KoWmfHandle**  mObjHandleTab;
    int    mNbrObject;          // number of object on the stack
    bool   mStackOverflow;
};


}

#endif

