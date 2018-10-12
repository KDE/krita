/* This file is part of the KDE libraries
 *
 * Copyright (c) 1998 Stefan Taferner
 *               2001/2003 thierry lorthiois (lorthioist@wanadoo.fr)
 *               2007-2008 Jan Hambrecht <jaham@gmx.net>
 *               2009-2011 Inge Wallin <inge@lysator.liu.se>
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

#include "WmfParser.h"
#include "WmfAbstractBackend.h"

#include <VectorImageDebug.h>

#include <QImage>
#include <QMatrix>
#include <QDataStream>
#include <QByteArray>
#include <QBuffer>
#include <QPolygon>

#include <math.h>


#define DEBUG_BBOX    0
#define DEBUG_RECORDS 0


/**
   Namespace for Windows Metafile (WMF) classes
*/
namespace Libwmf
{

#if defined(DEBUG_RECORDS)
// Used for debugging of records
static const struct KoWmfFunc {
    const char *name;
} koWmfFunc[] = {
    //                                    index metafunc
    { "end" }, // 0 0x00
    { "setBkColor" }, // 1 0x01
    { "setBkMode" }, // 2 0x02
    { "setMapMode" }, // 3 0x03
    { "setRop" }, // 4 0x04
    { "setRelAbs" }, // 5 0x05
    { "setPolyFillMode" }, // 6 0x06
    { "setStretchBltMode" }, // 7 0x07
    { "setTextCharExtra" }, // 8 0x08
    { "setTextColor" }, // 9 0x09
    { "setTextJustification" }, // 10 0x0a
    { "setWindowOrg" }, // 11 0x0b
    { "setWindowExt" }, // 12 0x0c
    { "setViewportOrg" }, // 13 0x0d
    { "setViewportExt" }, // 14 0x0e
    { "offsetWindowOrg" }, // 15 0x0f
    { "scaleWindowExt" }, // 16 0x10
    { "offsetViewportOrg" }, // 17 0x11
    { "scaleViewportExt" }, // 18 0x12
    { "lineTo" }, // 19 0x13
    { "moveTo" }, // 20 0x14
    { "excludeClipRect" }, // 21 0x15
    { "intersectClipRect" }, // 22 0x16
    { "arc" }, // 23 0x17
    { "ellipse" }, // 24 0x18
    { "floodfill" }, // 25 0x19  floodfill
    { "pie" }, // 26 0x1a
    { "rectangle" }, // 27 0x1b
    { "roundRect" }, // 28 0x1c
    { "patBlt" }, // 29 0x1d
    { "saveDC" }, // 30 0x1e
    { "setPixel" }, // 31 0x1f
    { "offsetClipRegion" }, // 32 0x20
    { "textOut" }, // 33 0x21
    { "bitBlt" }, // 34 0x22
    { "stretchBlt" }, // 35 0x23
    { "polygon" }, // 36 0x24
    { "polyline" }, // 37 0x25
    { "escape" }, // 38 0x26
    { "restoreDC" }, // 39 0x27
    { "fillRegion" }, // 40 0x28
    { "frameRegion" }, // 41 0x29
    { "invertRegion" }, // 42 0x2a
    { "paintRegion" }, // 43 0x2b
    { "selectClipRegion" }, // 44 0x2c
    { "selectObject" }, // 45 0x2d
    { "setTextAlign" }, // 46 0x2e
    { "noSuchRecord" }, // 47 0x2f
    { "chord" }, // 48 0x30
    { "setMapperFlags" }, // 49 0x31
    { "extTextOut" }, // 50 0x32
    { "setDibToDev" }, // 51 0x33
    { "selectPalette" }, // 52 0x34
    { "realizePalette" }, // 53 0x35
    { "animatePalette" }, // 54 0x36
    { "setPalEntries" }, // 55 0x37
    { "polyPolygon" }, // 56 0x38
    { "resizePalette" }, // 57 0x39
    { "noSuchRecord" }, // 58 0x3a
    { "noSuchRecord" }, // 59 0x3b
    { "noSuchRecord" }, // 60 0x3c
    { "noSuchRecord" }, // 61 0x3d
    { "noSuchRecord" }, // 62 0x3e
    { "unimplemented" }, // 63 0x3f
    { "dibBitBlt" }, // 64 0x40
    { "dibStretchBlt" }, // 65 0x41
    { "dibCreatePatternBrush" }, // 66 0x42
    { "stretchDib" }, // 67 0x43
    { "noSuchRecord" }, // 68 0x44
    { "noSuchRecord" }, // 69 0x45
    { "noSuchRecord" }, // 70 0x46
    { "noSuchRecord" }, // 71 0x47
    { "extFloodFill" }, // 72 0x48
    { "setLayout" }, // 73 0x49
    { "unimplemented" }, // 74 0x4a
    { "unimplemented" }, // 75 0x4b
    { "resetDC" }, // 76 0x4c
    { "startDoc" }, // 77 0x4d
    { "unimplemented" }, // 78 0x4e
    { "startPage" }, // 79 0x4f
    { "endPage" }, // 80 0x50
    { "unimplemented" }, // 81 0x51
    { "unimplemented" }, // 82 0x52
    { "unimplemented" }, // 83 0x53
    { "unimplemented" }, // 84 0x54
    { "unimplemented" }, // 85 0x55
    { "unimplemented" }, // 86 0x56
    { "unimplemented" }, // 87 0x57
    { "unimplemented" }, // 88 0x58
    { "unimplemented" }, // 89 0x59
    { "unimplemented" }, // 90 0x5a
    { "unimplemented" }, // 91 0x5b
    { "unimplemented" }, // 92 0x5c
    { "unimplemented" }, // 93 0x5d
    { "endDoc" }, // 94 0x5e
    { "unimplemented" }, // 95 0x5f
    { "deleteObject" }, // 96 0xf0
    { "noSuchRecord" }, // 97 0xf1
    { "noSuchRecord" }, // 98 0xf2
    { "noSuchRecord" }, // 99 0xf3
    { "noSuchRecord" }, // 100 0xf4
    { "noSuchRecord" }, // 101 0xf5
    { "noSuchRecord" }, // 102 0xf6
    { "createPalette" }, // 103 0xf7
    { "createBrush" }, // 104 0xf8
    { "createPatternBrush" }, // 105 0xf9
    { "createPenIndirect" }, // 106 0xfa
    { "createFontIndirect" }, // 107 0xfb
    { "createBrushIndirect" }, //108 0xfc
    { "createBitmapIndirect" }, //109 0xfd
    { "createBitmap" }, // 110 0xfe
    { "createRegion" } // 111 0xff
};
#endif

WmfParser::WmfParser()
{
    mNbrFunc = 0;
    mValid = false;
    mStandard = false;
    mPlaceable = false;
    mEnhanced = false;
    mBuffer = 0;
    mObjHandleTab = 0;
}


WmfParser::~WmfParser()
{
    if (mObjHandleTab != 0) {
        for (int i = 0 ; i < mNbrObject ; i++) {
            if (mObjHandleTab[i] != 0)
                delete mObjHandleTab[i];
        }
        delete[] mObjHandleTab;
    }
    if (mBuffer != 0) {
        mBuffer->close();
        delete mBuffer;
    }
}


bool WmfParser::load(const QByteArray& array)
{
    // delete previous buffer
    if (mBuffer != 0) {
        mBuffer->close();
        delete mBuffer;
        mBuffer = 0;
    }

    if (array.size() == 0)
        return false;

    // load into buffer
    mBuffer = new QBuffer();
    mBuffer->setData(array);
    mBuffer->open(QIODevice::ReadOnly);

    // read and check the header
    WmfMetaHeader      header;
    WmfEnhMetaHeader   eheader;
    WmfPlaceableHeader pheader; // Contains a bounding box
    unsigned short checksum;
    int filePos;

    QDataStream stream(mBuffer);
    stream.setByteOrder(QDataStream::LittleEndian);

    mStackOverflow = false;
    mLayout = LAYOUT_LTR;
    mTextColor = Qt::black;
    mMapMode = MM_ANISOTROPIC;

    mValid = false;
    mStandard = false;
    mPlaceable = false;
    mEnhanced = false;

    // Initialize the bounding box.
    //mBBoxTop = 0;           // The default origin is (0, 0).
    //mBBoxLeft = 0;
    mBBoxTop = 32767;
    mBBoxLeft = 32767;
    mBBoxRight = -32768;
    mBBoxBottom = -32768;
    mMaxWidth = 0;
    mMaxHeight = 0;

#if DEBUG_RECORDS
    debugVectorImage << "--------------------------- Starting parsing WMF ---------------------------";
#endif
    stream >> pheader.key;
    if (pheader.key == (quint32)APMHEADER_KEY) {
        //----- Read placeable metafile header
        mPlaceable = true;
#if DEBUG_RECORDS
        debugVectorImage << "Placeable header!  Yessss!";
#endif

        stream >> pheader.handle;
        stream >> pheader.left;
        stream >> pheader.top;
        stream >> pheader.right;
        stream >> pheader.bottom;
        stream >> pheader.inch;
        stream >> pheader.reserved;
        stream >> pheader.checksum;
        checksum = calcCheckSum(&pheader);
        if (pheader.checksum != checksum) {
            warnVectorImage << "Checksum for placeable metafile header is incorrect ( actual checksum" << pheader.checksum << ", expected checksum" << checksum << ")";
            return false;
        }
        stream >> header.fileType;
        stream >> header.headerSize;
        stream >> header.version;
        stream >> header.fileSize;
        stream >> header.numOfObjects;
        stream >> header.maxRecordSize;
        stream >> header.numOfParameters;

        mNbrObject = header.numOfObjects;

        // The bounding box of the WMF
        mBBoxLeft   = pheader.left;
        mBBoxTop    = pheader.top;
        mBBoxRight  = pheader.right;
        mBBoxBottom = pheader.bottom;
#if DEBUG_RECORDS
        debugVectorImage << "bounding box in header: " << mBBoxLeft << mBBoxTop << mBBoxRight << mBBoxBottom
                      << "width, height: " << mBBoxRight - mBBoxLeft << mBBoxBottom - mBBoxTop;
#endif
        mMaxWidth   = abs(pheader.right - pheader.left);
        mMaxHeight  = abs(pheader.bottom - pheader.top);

        mDpi = pheader.inch;
    } else {
        mBuffer->reset();
        //----- Read as enhanced metafile header
        filePos = mBuffer->pos();
        stream >> eheader.recordType;
        stream >> eheader.recordSize;
        stream >> eheader.boundsLeft;
        stream >> eheader.boundsTop;
        stream >> eheader.boundsRight;
        stream >> eheader.boundsBottom;
        stream >> eheader.frameLeft;
        stream >> eheader.frameTop;
        stream >> eheader.frameRight;
        stream >> eheader.frameBottom;

        stream >> eheader.signature;
        if (eheader.signature == ENHMETA_SIGNATURE) {
            mEnhanced = true;
            stream >> eheader.version;
            stream >> eheader.size;
            stream >> eheader.numOfRecords;
            stream >> eheader.numHandles;
            stream >> eheader.reserved;
            stream >> eheader.sizeOfDescription;
            stream >> eheader.offsetOfDescription;
            stream >> eheader.numPaletteEntries;
            stream >> eheader.widthDevicePixels;
            stream >> eheader.heightDevicePixels;
            stream >> eheader.widthDeviceMM;
            stream >> eheader.heightDeviceMM;
        } else {
            //----- Read as standard metafile header
            mStandard = true;
            mBuffer->seek(filePos);
            stream >> header.fileType;
            stream >> header.headerSize;
            stream >> header.version;
            stream >> header.fileSize;
            stream >> header.numOfObjects;
            stream >> header.maxRecordSize;
            stream >> header.numOfParameters;
            mNbrObject = header.numOfObjects;
        }
    }
    mOffsetFirstRecord = mBuffer->pos();

    //----- Test header validity
    if (((header.headerSize == 9) && (header.numOfParameters == 0)) || (mPlaceable)) {
        // valid wmf file
        mValid = true;
    } else {
        debugVectorImage << "WmfParser : incorrect file format !";
    }

    // check bounding rectangle for standard meta file
    if (mStandard && mValid) {
        // Note that this call can change mValid.
        createBoundingBox(stream);

#if DEBUG_RECORDS
        debugVectorImage << "bounding box created by going through all records: "
                      << mBBoxLeft << mBBoxTop << mBBoxRight << mBBoxBottom
                      << "width, height: " << mBBoxRight - mBBoxLeft << mBBoxBottom - mBBoxTop;
#endif
    }

    return mValid;
}


bool WmfParser::play(WmfAbstractBackend* backend)
{
    if (!(mValid)) {
        debugVectorImage << "WmfParser::play : invalid WMF file";
        return false;
    }

    if (mNbrFunc) {
#if DEBUG_RECORDS
        if ((mStandard)) {
            debugVectorImage << "Standard :" << mBBoxLeft << ""  << mBBoxTop << ""  << mBBoxRight - mBBoxLeft << ""  << mBBoxBottom - mBBoxTop;
        } else {
            debugVectorImage << "DPI :" << mDpi;
            debugVectorImage << "size (inch):" << (mBBoxRight - mBBoxLeft) / mDpi
                          << "" << (mBBoxBottom - mBBoxTop) / mDpi;
            debugVectorImage << "size (mm):" << (mBBoxRight - mBBoxLeft) * 25.4 / mDpi
                          << "" << (mBBoxBottom - mBBoxTop) * 25.4 / mDpi;
        }
        debugVectorImage << mValid << "" << mStandard << "" << mPlaceable;
#endif
    }

    // Stack of handles
    mObjHandleTab = new KoWmfHandle* [ mNbrObject ];
    for (int i = 0; i < mNbrObject ; i++) {
        mObjHandleTab[ i ] = 0;
    }

    mDeviceContext.reset();

    quint16 recordType;
    quint32 size;
    int  bufferOffset;

    // Create a stream from which the records will be read.
    QDataStream stream(mBuffer);
    stream.setByteOrder(QDataStream::LittleEndian);

    // Set the output backend.
    m_backend = backend;

    // Set some initial values.
    mDeviceContext.windowOrg = QPoint(0, 0);
    mDeviceContext.windowExt = QSize(1, 1);

    QRect bbox(QPoint(mBBoxLeft,mBBoxTop),
               QSize(mBBoxRight - mBBoxLeft, mBBoxBottom - mBBoxTop));
    if (m_backend->begin(bbox)) {
        // Play WMF functions.
        mBuffer->seek(mOffsetFirstRecord);
        recordType = 1;

        while ((recordType) && (!mStackOverflow)) {
            int j = 1;

            bufferOffset = mBuffer->pos();
            stream >> size;
            stream >> recordType;

            // mapping between n function and index of table 'metaFuncTab'
            // lower 8 digits of the function => entry in the table
            quint16 index = recordType & 0xff;
            if (index > 0x5F) {
                index -= 0x90;
            }

#if defined(DEBUG_RECORDS)
            debugVectorImage << "Record = " << koWmfFunc[ index ].name
                          << " (" << hex << recordType
                          << ", index" << dec << index << ")";
#endif

            if (mNbrFunc) {
                // debug mode
                if ((j + 12) > mNbrFunc) {
                    // output last 12 functions
                    int offBuff = mBuffer->pos();
                    quint16 param;

                    debugVectorImage <<  j << " :" << index << " :";
                    for (quint16 i = 0 ; i < (size - 3) ; i++) {
                        stream >> param;
                        debugVectorImage <<  param << "";
                    }
                    debugVectorImage;
                    mBuffer->seek(offBuff);
                }
                if (j >= mNbrFunc) {
                    break;
                }
                j++;
            }

            // Execute the function and parse the record.
            switch (recordType & 0xff) {
            case (META_EOF & 0xff):
                // Don't need to do anything here.
                break;
            case (META_SETBKCOLOR & 0xff):
                {
                    quint32 color;

                    stream >> color;
                    mDeviceContext.backgroundColor = qtColor(color);
                    mDeviceContext.changedItems |= DCBgTextColor;
                }
                break;
            case (META_SETBKMODE & 0xff):
                {
                    quint16 bkMode;

                    stream >> bkMode;
                    //debugVectorImage << "New bkMode: " << bkMode;

                    mDeviceContext.bgMixMode = bkMode;
                    mDeviceContext.changedItems |= DCBgMixMode;
                }
                break;
            case (META_SETMAPMODE & 0xff):
                {
                    stream >> mMapMode;
                    //debugVectorImage << "New mapmode: " << mMapMode;

                    //mDeviceContext.FontMapMode = mMapMode;Not defined yet
                    mDeviceContext.changedItems |= DCFontMapMode;
                }
                break;
            case (META_SETROP2 & 0xff):
                {
                    quint16  rop;

                    stream >> rop;
                    m_backend->setCompositionMode(winToQtComposition(rop));

                    mDeviceContext.rop = rop;
                    mDeviceContext.changedItems |= DCFgMixMode;
                }                break;
            case (META_SETRELABS & 0xff):
                break;
            case (META_SETPOLYFILLMODE & 0xff):
                {
                    stream >> mDeviceContext.polyFillMode;
                    mDeviceContext.changedItems |= DCPolyFillMode;
                }
                break;
            case (META_SETSTRETCHBLTMODE & 0xff):
            case (META_SETTEXTCHAREXTRA & 0xff):
                break;
            case (META_SETTEXTCOLOR & 0xff):
                {
                    quint32 color;

                    stream >> color;
                    mDeviceContext.foregroundTextColor = qtColor(color);
                    mDeviceContext.changedItems |= DCFgTextColor;
                }
                break;
            case (META_SETTEXTJUSTIFICATION & 0xff):
                break;
            case (META_SETWINDOWORG & 0xff):
                {
                    qint16 top, left;

                    stream >> top >> left;

                    m_backend->setWindowOrg(left, top);
                    mDeviceContext.windowOrg = QPoint(left, top);
#if DEBUG_RECORDS
                    debugVectorImage <<"Org: (" << left <<","  << top <<")";
#endif
                }
                break;
            case (META_SETWINDOWEXT & 0xff):
                {
                    qint16 width, height;

                    // negative value allowed for width and height
                    stream >> height >> width;
#if DEBUG_RECORDS
                    debugVectorImage <<"Ext: (" << width <<","  << height <<")";
#endif

                    m_backend->setWindowExt(width, height);
                    mDeviceContext.windowExt = QSize(width, height);
                }
                break;
            case (META_SETVIEWPORTORG & 0xff):
                {
                    qint16 top, left;

                    stream >> top >> left;
                    m_backend->setViewportOrg(left, top);
                    mDeviceContext.viewportOrg = QPoint(left, top);

#if DEBUG_RECORDS
                    debugVectorImage <<"Org: (" << left <<","  << top <<")";
#endif
                }
                break;
            case (META_SETVIEWPORTEXT & 0xff):
                {
                    qint16 width, height;

                    // Negative value allowed for width and height
                    stream >> height >> width;
#if DEBUG_RECORDS
                    debugVectorImage <<"Ext: (" << width <<","  << height <<")";
#endif

                    m_backend->setViewportExt(width, height);
                    mDeviceContext.viewportExt = QSize(width, height);
                }
                break;
            case (META_OFFSETWINDOWORG & 0xff):
                {
                    qint16 offTop, offLeft;

                    stream >> offTop >> offLeft;
                    m_backend->setWindowOrg(mDeviceContext.windowOrg.x() + offLeft,
                                            mDeviceContext.windowOrg.y() + offTop);

                    mDeviceContext.windowOrg += QPoint(offLeft, offTop);
                }
                break;
            case (META_SCALEWINDOWEXT & 0xff):
                {
                    // Use 32 bits in the calculations to not lose precision.
                    qint32 width, height;
                    qint16 heightDenum, heightNum, widthDenum, widthNum;

                    stream >> heightDenum >> heightNum >> widthDenum >> widthNum;

                    if ((widthDenum != 0) && (heightDenum != 0)) {
                        width = (qint32(mDeviceContext.windowExt.width()) * widthNum) / widthDenum;
                        height = (qint32(mDeviceContext.windowExt.height()) * heightNum) / heightDenum;
                        m_backend->setWindowExt(width, height);
                        mDeviceContext.windowExt = QSize(width, height);
                    }
                    //debugVectorImage <<"WmfParser::ScaleWindowExt :" << widthDenum <<"" << heightDenum;
                }
                break;
            case (META_OFFSETVIEWPORTORG & 0xff):
                {
                    qint16 offTop, offLeft;

                    stream >> offTop >> offLeft;
                    m_backend->setViewportOrg(mDeviceContext.windowOrg.x() + offLeft,
                                              mDeviceContext.windowOrg.y() + offTop);
                    mDeviceContext.viewportOrg += QPoint(offLeft, offTop);
                }
                break;
            case (META_SCALEVIEWPORTEXT & 0xff):
                {
                    // Use 32 bits in the calculations to not lose precision.
                    qint32 width, height;
                    qint16 heightDenum, heightNum, widthDenum, widthNum;

                    stream >> heightDenum >> heightNum >> widthDenum >> widthNum;

                    if ((widthDenum != 0) && (heightDenum != 0)) {
                        width = (qint32(mDeviceContext.windowExt.width()) * widthNum) / widthDenum;
                        height = (qint32(mDeviceContext.windowExt.height()) * heightNum) / heightDenum;
                        m_backend->setViewportExt(width, height);
                        mDeviceContext.viewportExt = QSize(width, height);
                    }
                    //debugVectorImage <<"WmfParser::ScaleWindowExt :" << widthDenum <<"" << heightDenum;
                }
                break;

            // ----------------------------------------------------------------
            //                         Drawing records

            case (META_LINETO & 0xff):
                {
                    qint16 top, left;

                    stream >> top >> left;
                    m_backend->lineTo(mDeviceContext, left, top);
                }
                break;
            case (META_MOVETO & 0xff):
                {
                    qint16 top, left;

                    stream >> top >> left;
                    mDeviceContext.currentPosition = QPoint(left, top);
                }
                break;
            case (META_EXCLUDECLIPRECT & 0xff):
                {
                    qint16 top, left, right, bottom;

                    stream >> bottom >> right >> top >> left;

                    QRegion region = mDeviceContext.clipRegion;
                    QRegion newRegion(left, top, right - left, bottom - top);
                    if (region.isEmpty()) {
                        // FIXME: I doubt that if the region is previously empty,
                        //        it should be set to the new region.  /iw
                        region = newRegion;
                    } else {
                        region = region.subtracted(newRegion);
                    }

                    mDeviceContext.clipRegion = region;
                    mDeviceContext.changedItems |= DCClipRegion;
                }
                break;
            case (META_INTERSECTCLIPRECT & 0xff):
                {
                    qint16 top, left, right, bottom;

                    stream >> bottom >> right >> top >> left;

                    QRegion region = mDeviceContext.clipRegion;
                    QRegion newRegion(left, top, right - left, bottom - top);
                    if (region.isEmpty()) {
                        // FIXME: I doubt that if the region is previously empty,
                        //        it should be set to the new region.  /iw
                        region = newRegion;
                    } else {
                        region = region.intersected(newRegion);
                    }

                    mDeviceContext.clipRegion = region;
                    mDeviceContext.changedItems |= DCClipRegion;
                }
                break;
            case (META_ARC & 0xff):
                {
                    int xCenter, yCenter, angleStart, aLength;
                    qint16  topEnd, leftEnd, topStart, leftStart;
                    qint16  top, left, right, bottom;

                    stream >> topEnd >> leftEnd >> topStart >> leftStart;
                    stream >> bottom >> right >> top >> left;

                    xCenter = left + ((right - left) / 2);
                    yCenter = top + ((bottom - top) / 2);
                    xyToAngle(leftStart - xCenter, yCenter - topStart,
                              leftEnd - xCenter, yCenter - topEnd, angleStart, aLength);

                    m_backend->drawArc(mDeviceContext, left, top, right - left, bottom - top,
                                       angleStart, aLength);
                }
                break;
            case (META_ELLIPSE & 0xff):
                {
                    qint16 top, left, right, bottom;

                    stream >> bottom >> right >> top >> left;
                    m_backend->drawEllipse(mDeviceContext, left, top, right - left, bottom - top);
                }
                break;
            case (META_FLOODFILL & 0xff):
                break;
            case (META_PIE & 0xff):
                {
                    int xCenter, yCenter, angleStart, aLength;
                    qint16  topEnd, leftEnd, topStart, leftStart;
                    qint16  top, left, right, bottom;

                    stream >> topEnd >> leftEnd >> topStart >> leftStart;
                    stream >> bottom >> right >> top >> left;

                    xCenter = left + ((right - left) / 2);
                    yCenter = top + ((bottom - top) / 2);
                    xyToAngle(leftStart - xCenter, yCenter - topStart, leftEnd - xCenter, yCenter - topEnd, angleStart, aLength);

                    m_backend->drawPie(mDeviceContext, left, top, right - left, bottom - top,
                                       angleStart, aLength);
                }
                break;
            case (META_RECTANGLE & 0xff):
                {
                    qint16 top, left, right, bottom;

                    stream >> bottom >> right >> top >> left;
                    //debugVectorImage << left << top << right << bottom;
                    m_backend->drawRect(mDeviceContext, left, top, right - left, bottom - top);
                }
                break;
            case (META_ROUNDRECT & 0xff):
                {
                    int xRnd = 0, yRnd = 0;
                    quint16 widthCorner, heightCorner;
                    qint16  top, left, right, bottom;

                    stream >> heightCorner >> widthCorner;
                    stream >> bottom >> right >> top >> left;

                    // convert (widthCorner, heightCorner) in percentage
                    if ((right - left) != 0)
                        xRnd = (widthCorner * 100) / (right - left);
                    if ((bottom - top) != 0)
                        yRnd = (heightCorner * 100) / (bottom - top);

                    m_backend->drawRoundRect(mDeviceContext, left, top, right - left, bottom - top,
                                             xRnd, yRnd);
                }
                break;
            case (META_PATBLT & 0xff):
                {
                    quint32 rasterOperation;
                    quint16 height, width;
                    qint16  y, x;

                    stream >> rasterOperation;
                    stream >> height >> width;
                    stream >> y >> x;

                    //debugVectorImage << "patBlt record" << hex << rasterOperation << dec
                    //              << x << y << width << height;

                    m_backend->patBlt(mDeviceContext, x, y, width, height, rasterOperation);
                }
                break;
            case (META_SAVEDC & 0xff):
                m_backend->save();
                break;
            case (META_SETPIXEL & 0xff):
                {
                    qint16  left, top;
                    quint32 color;

                    stream >> color >> top >> left;
                    m_backend->setPixel(mDeviceContext, left, top, qtColor(color));
                }
                break;
            case (META_OFFSETCLIPRGN & 0xff):
                break;
            case (META_TEXTOUT & 0xff):
                {
                    quint16 textLength;
                    qint16 x, y;

                    stream >> textLength;

                    QByteArray text;
                    text.resize(textLength);
                    stream.readRawData(text.data(), textLength);

                    // The string is always of even length, so if the actual data is
                    // of uneven length, read an extra byte.
                    if (textLength & 0x01) {
                        quint8 dummy;
                        stream >> dummy;
                    }

                    stream >> y;
                    stream >> x;

                    m_backend->drawText(mDeviceContext, x, y, text);
                }
                break;
            case (META_BITBLT & 0xff):
            case (META_STRETCHBLT & 0xff):
                break;
            case (META_POLYGON & 0xff):
                {
                    quint16 num;

                    stream >> num;

                    QPolygon pa(num);

                    pointArray(stream, pa);
                    m_backend->drawPolygon(mDeviceContext, pa);
                }
                break;
            case (META_POLYLINE & 0xff):
                {
                    quint16 num;

                    stream >> num;
                    QPolygon pa(num);

                    pointArray(stream, pa);
                    m_backend->drawPolyline(mDeviceContext, pa);
                }
                break;
            case (META_ESCAPE & 0xff):
                break;
            case (META_RESTOREDC & 0xff):
                {
                    qint16  num;

                    stream >> num;
                    for (int i = 0; i > num ; i--)
                        m_backend->restore();
                }
                break;
            case (META_FILLREGION & 0xff):
            case (META_FRAMEREGION & 0xff):
            case (META_INVERTREGION & 0xff):
            case (META_PAINTREGION & 0xff):
            case (META_SELECTCLIPREGION & 0xff):
                break;
            case (META_SELECTOBJECT & 0xff):
                {
                    quint16 idx;

                    stream >> idx;
                    if ((idx < mNbrObject) && (mObjHandleTab[ idx ] != 0))
                        mObjHandleTab[ idx ]->apply(&mDeviceContext);
                    else
                        debugVectorImage << "WmfParser::selectObject : selection of an empty object";
                }
                break;
            case (META_SETTEXTALIGN & 0xff):
                stream >> mDeviceContext.textAlign;
                mDeviceContext.changedItems |= DCTextAlignMode;
                break;
            case (META_CHORD & 0xff):
                {
                    int xCenter, yCenter, angleStart, aLength;
                    qint16  topEnd, leftEnd, topStart, leftStart;
                    qint16  top, left, right, bottom;

                    stream >> topEnd >> leftEnd >> topStart >> leftStart;
                    stream >> bottom >> right >> top >> left;

                    xCenter = left + ((right - left) / 2);
                    yCenter = top + ((bottom - top) / 2);
                    xyToAngle(leftStart - xCenter, yCenter - topStart, leftEnd - xCenter, yCenter - topEnd, angleStart, aLength);

                    m_backend->drawChord(mDeviceContext, left, top, right - left, bottom - top,
                                         angleStart, aLength);
                }
                break;
            case (META_SETMAPPERFLAGS & 0xff):
                break;
            case (META_EXTTEXTOUT & 0xff):
                {
                    qint16 y, x;
                    qint16 stringLength;
                    quint16 fwOpts;
                    qint16 top, left, right, bottom; // optional cliprect

                    stream >> y;
                    stream >> x;
                    stream >> stringLength;
                    stream >> fwOpts;

                    // ETO_CLIPPED flag adds 4 parameters
                    if (fwOpts & (ETO_CLIPPED | ETO_OPAQUE)) {
                        // read the optional clip rect
                        stream >> bottom >> right >> top >> left;
                    }

                    // Read the string. Note that it's padded to 16 bits.
                    QByteArray text;
                    text.resize(stringLength);
                    stream.readRawData(text.data(), stringLength);

                    if (stringLength & 0x01) {
                        quint8  padding;
                        stream >> padding;
                    }

#if DEBUG_RECORDS
                    debugVectorImage << "text at" << x << y << "length" << stringLength
                                  << ':' << text;
                    //debugVectorImage << "flags:" << hex << fwOpts << dec;
                    debugVectorImage << "flags:" << fwOpts;
                    debugVectorImage << "record length:" << size;
#endif
                    m_backend->drawText(mDeviceContext, x, y, text);
                }
                break;
            case (META_SETDIBTODEV & 0xff):
            case (META_SELECTPALETTE & 0xff):
            case (META_REALIZEPALETTE & 0xff):
            case (META_ANIMATEPALETTE & 0xff):
            case (META_SETPALENTRIES & 0xff):
                break;
            case (META_POLYPOLYGON & 0xff):
                {
                    quint16 numberPoly;
                    quint16 sizePoly;
                    QList<QPolygon> listPa;

                    stream >> numberPoly;

                    for (int i = 0 ; i < numberPoly ; i++) {
                        stream >> sizePoly;
                        listPa.append(QPolygon(sizePoly));
                    }

                    // list of point array
                    for (int i = 0; i < numberPoly; i++) {
                        pointArray(stream, listPa[i]);
                    }

                    // draw polygon's
                    m_backend->drawPolyPolygon(mDeviceContext, listPa);
                    listPa.clear();
                }
                break;
            case (META_RESIZEPALETTE & 0xff):
                break;
            case (META_DIBBITBLT & 0xff):
                {
                    quint32 raster;
                    qint16  topSrc, leftSrc, widthSrc, heightSrc;
                    qint16  topDst, leftDst;

                    stream >> raster;
                    stream >> topSrc >> leftSrc >> heightSrc >> widthSrc;
                    stream >> topDst >> leftDst;

                    if (size > 11) {      // DIB image
                        QImage bmpSrc;

                        if (dibToBmp(bmpSrc, stream, (size - 11) * 2)) {
                            m_backend->setCompositionMode(winToQtComposition(raster));

                            m_backend->save();
                            if (widthSrc < 0) {
                                // negative width => horizontal flip
                                QMatrix m(-1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F);
                                m_backend->setMatrix(mDeviceContext, m, true);
                            }
                            if (heightSrc < 0) {
                                // negative height => vertical flip
                                QMatrix m(1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F);
                                m_backend->setMatrix(mDeviceContext, m, true);
                            }
                            m_backend->drawImage(mDeviceContext, leftDst, topDst,
                                                 bmpSrc, leftSrc, topSrc, widthSrc, heightSrc);
                            m_backend->restore();
                        }
                    } else {
                        debugVectorImage << "WmfParser::dibBitBlt without image not implemented";
                    }
                }
                break;
            case (META_DIBSTRETCHBLT & 0xff):
                {
                    quint32 raster;
                    qint16  topSrc, leftSrc, widthSrc, heightSrc;
                    qint16  topDst, leftDst, widthDst, heightDst;
                    QImage   bmpSrc;

                    stream >> raster;
                    stream >> heightSrc >> widthSrc >> topSrc >> leftSrc;
                    stream >> heightDst >> widthDst >> topDst >> leftDst;

                    if (dibToBmp(bmpSrc, stream, (size - 13) * 2)) {
                        m_backend->setCompositionMode(winToQtComposition(raster));

                        m_backend->save();
                        if (widthDst < 0) {
                            // negative width => horizontal flip
                            QMatrix m(-1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F);
                            m_backend->setMatrix(mDeviceContext, m, true);
                        }
                        if (heightDst < 0) {
                            // negative height => vertical flip
                            QMatrix m(1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F);
                            m_backend->setMatrix(mDeviceContext, m, true);
                        }
                        bmpSrc = bmpSrc.copy(leftSrc, topSrc, widthSrc, heightSrc);
                        // TODO: scale the bitmap : QImage::scale(widthDst, heightDst)
                        // is actually too slow

                        m_backend->drawImage(mDeviceContext, leftDst, topDst, bmpSrc);
                        m_backend->restore();
                    }
                }
                break;
            case (META_DIBCREATEPATTERNBRUSH & 0xff):
                {
                    KoWmfPatternBrushHandle* handle = new KoWmfPatternBrushHandle;

                    if (addHandle(handle)) {
                        quint32 arg;
                        QImage bmpSrc;

                        stream >> arg;
                        if (dibToBmp(bmpSrc, stream, (size - 5) * 2)) {
                            handle->image = bmpSrc;
                            handle->brush.setTextureImage(handle->image);
                        } else {
                            debugVectorImage << "WmfParser::dibCreatePatternBrush : incorrect DIB image";
                        }
                    }
                }
                break;
            case (META_STRETCHDIB & 0xff):
                {
                    quint32 raster;
                    qint16  arg, topSrc, leftSrc, widthSrc, heightSrc;
                    qint16  topDst, leftDst, widthDst, heightDst;
                    QImage   bmpSrc;

                    stream >> raster >> arg;
                    stream >> heightSrc >> widthSrc >> topSrc >> leftSrc;
                    stream >> heightDst >> widthDst >> topDst >> leftDst;

                    if (dibToBmp(bmpSrc, stream, (size - 14) * 2)) {
                        m_backend->setCompositionMode(winToQtComposition(raster));

                        m_backend->save();
                        if (widthDst < 0) {
                            // negative width => horizontal flip
                            QMatrix m(-1.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F);
                            m_backend->setMatrix(mDeviceContext, m, true);
                        }
                        if (heightDst < 0) {
                            // negative height => vertical flip
                            QMatrix m(1.0F, 0.0F, 0.0F, -1.0F, 0.0F, 0.0F);
                            m_backend->setMatrix(mDeviceContext, m, true);
                        }
                        bmpSrc = bmpSrc.copy(leftSrc, topSrc, widthSrc, heightSrc);
                        // TODO: scale the bitmap ( QImage::scale(param[ 8 ], param[ 7 ]) is actually too slow )

                        m_backend->drawImage(mDeviceContext, leftDst, topDst, bmpSrc);
                        m_backend->restore();
                    }
                }
                break;
            case (META_EXTFLOODFILL & 0xff):
                break;
            case (META_SETLAYOUT & 0xff):
                {
                    quint16 layout;
                    quint16 reserved;

                    // negative value allowed for width and height
                    stream >> layout >> reserved;
#if DEBUG_RECORDS
                    debugVectorImage << "layout=" << layout;
#endif
                    mLayout = (WmfLayout)layout;

                    mDeviceContext.layoutMode = mLayout;
                    mDeviceContext.changedItems |= DCLayoutMode;

                }
                break;
            case (META_DELETEOBJECT & 0xff):
                {
                    quint16 idx;

                    stream >> idx;
                    deleteHandle(idx);
                }
                break;
            case (META_CREATEPALETTE & 0xff):
                // Unimplemented
                createEmptyObject();
                break;
            case (META_CREATEBRUSH & 0xff):
            case (META_CREATEPATTERNBRUSH & 0xff):
                break;
            case (META_CREATEPENINDIRECT & 0xff):
                {
                    // TODO: userStyle and alternateStyle
                    quint32 color;
                    quint16 style, width, arg;

                    KoWmfPenHandle* handle = new KoWmfPenHandle;

                    if (addHandle(handle)) {
                        stream >> style >> width >> arg >> color;

                        // set the style defaults
                        handle->pen.setStyle(Qt::SolidLine);
                        handle->pen.setCapStyle(Qt::RoundCap);
                        handle->pen.setJoinStyle(Qt::RoundJoin);

                        const int PenStyleMask = 0x0000000F;
                        const int PenCapMask   = 0x00000F00;
                        const int PenJoinMask  = 0x0000F000;

                        quint16 penStyle = style & PenStyleMask;
                        if (penStyle < 7)
                            handle->pen.setStyle(koWmfStylePen[ penStyle ]);
                        else
                            debugVectorImage << "WmfParser::createPenIndirect: invalid pen" << style;

                        quint16 capStyle = (style & PenCapMask) >> 8;
                        if (capStyle < 3)
                            handle->pen.setCapStyle(koWmfCapStylePen[ capStyle ]);
                        else
                            debugVectorImage << "WmfParser::createPenIndirect: invalid pen cap style" << style;

                        quint16 joinStyle = (style & PenJoinMask) >> 12;
                        if (joinStyle < 3)
                            handle->pen.setJoinStyle(koWmfJoinStylePen[ joinStyle ]);
                        else
                            debugVectorImage << "WmfParser::createPenIndirect: invalid pen join style" << style;

                        handle->pen.setColor(qtColor(color));
                        handle->pen.setWidth(width);
                        debugVectorImage << "Creating pen" << handle->pen;
                    }
                }
                break;
            case (META_CREATEFONTINDIRECT & 0xff):
                {
                    qint16  height;             // Height of the character cell
                    qint16  width;              // Average width (not used)
                    qint16  escapement;         // The rotation of the text in 1/10th degrees
                    qint16  orientation;        // The rotation of each character
                    quint16 weight, property, fixedPitch, arg;

                    KoWmfFontHandle* handle = new KoWmfFontHandle;

                    if (addHandle(handle)) {
                        stream >> height >> width;
                        stream >> escapement >> orientation;
                        stream >> weight >> property >> arg >> arg;
                        stream >> fixedPitch;

                        //debugVectorImage << height << width << weight << property;
                        // text rotation (in 1/10 degree)
                        handle->font.setFixedPitch(((fixedPitch & 0x01) == 0));
                        handle->escapement = escapement;
                        handle->orientation = orientation;

                        // A negative height means to use device units.
                        //debugVectorImage << "Font height:" << height;
                        handle->height = height;

                        // FIXME: For some reason this value needs to be multiplied by
                        //        a factor.  0.6 seems to give a good result, but why??
                        // ANSWER(?): The doc says the height is the height of the character cell.
                        //            But normally the font height is only the height above the
                        //            baseline, isn't it?
                        handle->font.setPointSize(qAbs(height) * 6 / 10);
                        if (weight == 0)
                            weight = QFont::Normal;
                        else {
                            // Linear transform between MS weights to Qt weights
                            // MS: 400=normal, 700=bold
                            // Qt: 50=normal, 75=bold
                            // This makes the line cross x=0 at y=50/3.  (x=MS weight, y=Qt weight)
                            //
                            // FIXME: Is this a linear relationship?
                            weight = (50 + 3 * ((weight * (75-50))/(700-400))) / 3;
                        }
                        handle->font.setWeight(weight);
                        handle->font.setItalic((property & 0x01));
                        handle->font.setUnderline((property & 0x100));
                        // TODO: Strikethrough

                        // font name
                        int    maxChar = (size - 12) * 2;
                        char*  nameFont = new char[maxChar];
                        stream.readRawData(nameFont, maxChar);
                        handle->font.setFamily(nameFont);
                        delete[] nameFont;
                    }
                }
                break;
            case (META_CREATEBRUSHINDIRECT & 0xff):
                {
                    Qt::BrushStyle style;
                    quint16 sty, arg2;
                    quint32 color;
                    KoWmfBrushHandle* handle = new KoWmfBrushHandle;

                    if (addHandle(handle)) {
                        stream >> sty >> color >> arg2;

                        if (sty == 2) {
                            if (arg2 < 6)
                                style = koWmfHatchedStyleBrush[ arg2 ];
                            else {
                                debugVectorImage << "WmfParser::createBrushIndirect: invalid hatched brush" << arg2;
                                style = Qt::SolidPattern;
                            }
                        } else {
                            if (sty < 9)
                                style = koWmfStyleBrush[ sty ];
                            else {
                                debugVectorImage << "WmfParser::createBrushIndirect: invalid brush" << sty;
                                style = Qt::SolidPattern;
                            }
                        }
                        handle->brush.setStyle(style);
                        handle->brush.setColor(qtColor(color));
                    }
                }
                break;
#if 0
 UNSPECIFIED in the Spec:
    { &WmfParser::createBitmapIndirect, "createBitmapIndirect" }, //109 0xfd
    { &WmfParser::createBitmap, "createBitmap" }, // 110 0xfe
#endif
            case (META_CREATEREGION & 0xff):
                // FIXME: Unimplemented
                createEmptyObject();
               break;
            default:
                // function outside WMF specification
                errorVectorImage << "BROKEN WMF file: Record number" << hex << recordType << dec
                              << " index " << index;
                mValid = false;
                break;
            }

            mBuffer->seek(bufferOffset + (size << 1));
        }

        // Let the backend clean up it's internal state.
        m_backend->end();
    }

    for (int i = 0 ; i < mNbrObject ; i++) {
        if (mObjHandleTab[ i ] != 0)
            delete mObjHandleTab[ i ];
    }
    delete[] mObjHandleTab;
    mObjHandleTab = 0;

    return true;
}


//-----------------------------------------------------------------------------


void WmfParser::createBoundingBox(QDataStream &stream)
{
    // Check bounding rectangle for standard meta file.
    // This calculation is done in device coordinates.
    if (!mStandard || !mValid)
        return;

    bool windowExtIsSet = false;
    bool viewportExtIsSet = false;

    quint16 recordType = 1;
    quint32 size;

    int filePos;

    // Search for records setWindowOrg and setWindowExt to
    // determine what the total bounding box of this WMF is.
    // This initialization assumes that setWindowOrg comes before setWindowExt.
    qint16 windowOrgX = 0;
    qint16 windowOrgY = 0;
    qint16 windowWidth = 0;
    qint16 windowHeight = 0;
    qint16 viewportOrgX = 0;
    qint16 viewportOrgY = 0;
    qint16 viewportWidth = 0;
    qint16 viewportHeight = 0;
    bool   bboxRecalculated = false;
    while (recordType) {

        filePos = mBuffer->pos();
        stream >> size >> recordType;

        if (size == 0) {
            debugVectorImage << "WmfParser: incorrect file!";
            mValid = 0;
            return;
        }

        bool  doRecalculateBBox = false;
        qint16  orgX = 0;
        qint16  orgY = 0;
        qint16  extX = 0;
        qint16  extY = 0;
        switch (recordType &= 0xFF) {
        case 11: // setWindowOrg
            {
                stream >> windowOrgY >> windowOrgX;
#if DEBUG_BBOX
                debugVectorImage << "setWindowOrg" << windowOrgX << windowOrgY;
#endif
                if (!windowExtIsSet)
                    break;

                // The bounding box doesn't change just because we get
                // a new window.  Remember we are working in device
                // (viewport) coordinates when deciding the bounding
                // box.
                if (viewportExtIsSet)
                    break;

                // If there is no viewport, then use the window ext as
                // size, and (0, 0) as origin.
                //
                // FIXME: Handle the case where the window is defined
                //        first and then the viewport, without any
                //        drawing in between.  If that happens, I
                //        don't think that the window definition
                //        should influence the bounding box.
                orgX = 0;
                orgY = 0;
                extX = windowWidth;
                extY = windowHeight;
            }
            break;

        case 12: // setWindowExt
            {
                stream >> windowHeight >> windowWidth;
                windowExtIsSet = true;
                bboxRecalculated = false;

#if DEBUG_BBOX
                debugVectorImage << "setWindowExt" << windowWidth << windowHeight
                              << "(viewportOrg = " << viewportOrgX << viewportOrgY << ")";
#endif

                // If the viewport is set, then a changed window
                // changes nothing in the bounding box.
                if (viewportExtIsSet)
                    break;

                bboxRecalculated = false;

                // Collect the maximum width and height.
                if (abs(windowWidth - windowOrgX) > mMaxWidth)
                    mMaxWidth = abs(windowWidth - windowOrgX);
                if (abs(windowHeight - windowOrgY) > mMaxHeight)
                    mMaxHeight = abs(windowHeight - windowOrgY);

                orgX = 0;
                orgY = 0;
                extX = windowWidth;
                extY = windowHeight;
            }
            break;

        case 13: //setViewportOrg
            {
                stream >> viewportOrgY >> viewportOrgX;
                bboxRecalculated = false;

#if DEBUG_BBOX
                debugVectorImage << "setViewportOrg" << viewportOrgX << viewportOrgY;
#endif
                orgX = viewportOrgX;
                orgY = viewportOrgY;
                if (viewportExtIsSet) {
                    extX = viewportWidth;
                    extY = viewportHeight;
                }
                else {
                    // If the viewportExt is not set, then either a
                    // subsequent setViewportExt will set it, or the
                    // windowExt will be used instead.
                    extX = windowWidth;
                    extY = windowHeight;
                }
                    break;

                // FIXME: Handle the case where the org changes but
                //        there is no subsequent Ext change (should be
                //        rather uncommon).
            }
            break;

        case 14: //setViewportExt
            {
                stream >> viewportHeight >> viewportWidth;
                viewportExtIsSet = true;
                bboxRecalculated = false;

#if DEBUG_BBOX
                debugVectorImage << "setViewportExt" << viewportWidth << viewportHeight;
#endif
                orgX = viewportOrgX;
                orgY = viewportOrgY;
                extX = viewportWidth;
                extY = viewportHeight;
            }
            break;

            // FIXME: Also support:
            //          ScaleWindowExt, ScaleViewportExt,
            //          OffsetWindowOrg, OffsetViewportOrg

            // The following are drawing commands.  It is only when
            // there is an actual drawing command that we should check
            // the bounding box. It seems that some WMF files have
            // lots of changes of the window or viewports but without
            // any drawing commands in between. These changes should
            // not affect the bounding box.
        case 19: // lineTo
        //case 20: // moveTo
        case 23: // arc
        case 24: // ellipse
        case 26: // pie
        case 27: // rectangle
        case 28: // roundRect
        case 29: // patBlt
        case 31: // setPixel
        case 33: // textOut
        case 34: // bitBlt
        case 36: // polygon
        case 37: // polyline
        //case 38: // escape  FIXME: is this a drawing command?
        case 40: // fillRegion
        case 41:
        case 42:
        case 43:
        case 44:
        case 48: // chord
        case 50: // extTextOut
        case 56: // polyPolygon
        case 64: // dibBitBlt
        case 65: // dibStretchBlt
        case 67: // stretchDib
        case 72: // extFloodFill
#if DEBUG_BBOX
            debugVectorImage << "drawing record: " << (recordType & 0xff);
#endif
            doRecalculateBBox = true;
            break;

        default:
          ;
        }

        // Recalculate the BBox if it was indicated above that it should be.
        if (doRecalculateBBox && !bboxRecalculated) {
#if DEBUG_BBOX
            debugVectorImage << "Recalculating BBox";
#endif
            // If we have a viewport, always use that one.
            if (viewportExtIsSet) {
                orgX = viewportOrgX;
                orgY = viewportOrgY;
                extX = viewportWidth;
                extY = viewportHeight;
            }
            else {
                // If there is no defined viewport, then use the
                // window as the fallback viewport. But only the size,
                // the origin is always (0, 0).
                orgX = 0;
                orgY = 0;
                extX = qAbs(windowWidth);
                extY = qAbs(windowHeight);
            }

            // If ext < 0, switch the org and org+ext
            if (extX < 0) {
                orgX += extX;
                extX = -extX;
            }
            if (extY < 0) {
                orgY += extY;
                extY = -extY;
            }

            // At this point, the ext is always >= 0, i.e. org <= org+ext
#if DEBUG_BBOX
            debugVectorImage << orgX << orgY << extX << extY;
#endif
            if (orgX < mBBoxLeft)          mBBoxLeft = orgX;
            if (orgY < mBBoxTop)           mBBoxTop  = orgY;
            if (orgX + extX > mBBoxRight)  mBBoxRight  = orgX + extX;
            if (orgY + extY > mBBoxBottom) mBBoxBottom = orgY + extY;

            bboxRecalculated = true;
        }

#if DEBUG_BBOX
        if (isOrgOrExt) {
            debugVectorImage << "              mBBoxTop = " << mBBoxTop;
            debugVectorImage << "mBBoxLeft = " << mBBoxLeft << "  mBBoxRight = " << mBBoxRight;
            debugVectorImage << "           MBBoxBotton = " << mBBoxBottom;
            debugVectorImage << "Max width,height = " << mMaxWidth << mMaxHeight;
        }
#endif

        mBuffer->seek(filePos + (size << 1));
    }
}


//-----------------------------------------------------------------------------
// Object handle


void WmfParser::createEmptyObject()
{
    // allocation of an empty object (to keep object counting in sync)
    KoWmfPenHandle* handle = new KoWmfPenHandle;

    addHandle(handle);
}


//-----------------------------------------------------------------------------
// Misc functions


quint16 WmfParser::calcCheckSum(WmfPlaceableHeader* apmfh)
{
    quint16*  lpWord;
    quint16   wResult, i;

    // Start with the first word
    wResult = *(lpWord = (quint16*)(apmfh));
    // XOR in each of the other 9 words
    for (i = 1; i <= 9; i++) {
        wResult ^= lpWord[ i ];
    }
    return wResult;
}


//-----------------------------------------------------------------------------
// Utilities and conversion Wmf -> Qt

bool WmfParser::addHandle(KoWmfHandle* handle)
{
    int idx;

    for (idx = 0; idx < mNbrObject ; idx++) {
        if (mObjHandleTab[ idx ] == 0)  break;
    }

    if (idx < mNbrObject) {
        mObjHandleTab[ idx ] = handle;
        return true;
    } else {
        delete handle;
        mStackOverflow = true;
        debugVectorImage << "WmfParser::addHandle : stack overflow = broken file !";
        return false;
    }
}


void WmfParser::deleteHandle(int idx)
{
    if ((idx < mNbrObject) && (mObjHandleTab[idx] != 0)) {
        delete mObjHandleTab[ idx ];
        mObjHandleTab[ idx ] = 0;
    } else {
        debugVectorImage << "WmfParser::deletehandle() : bad index number";
    }
}


void WmfParser::pointArray(QDataStream& stream, QPolygon& pa)
{
    qint16 left, top;
    int  i, max;

    for (i = 0, max = pa.size() ; i < max ; i++) {
        stream >> left >> top;
        pa.setPoint(i, left, top);
    }
}


void WmfParser::xyToAngle(int xStart, int yStart, int xEnd, int yEnd, int& angleStart, int& angleLength)
{
    double aStart, aLength;

    aStart = atan2((double)yStart, (double)xStart);
    aLength = atan2((double)yEnd, (double)xEnd) - aStart;

    angleStart = (int)((aStart * 2880) / 3.14166);
    angleLength = (int)((aLength * 2880) / 3.14166);
    if (angleLength < 0) angleLength = 5760 + angleLength;
}


QPainter::CompositionMode WmfParser::winToQtComposition(quint16 param) const
{
    if (param < 17)
        return koWmfOpTab16[ param ];
    else
        return QPainter::CompositionMode_Source;
}


QPainter::CompositionMode  WmfParser::winToQtComposition(quint32 param) const
{
    /* TODO: Ternary raster operations
    0x00C000CA  dest = (source AND pattern)
    0x00F00021  dest = pattern
    0x00FB0A09  dest = DPSnoo
    0x005A0049  dest = pattern XOR dest   */
    int i;

    for (i = 0 ; i < 15 ; i++) {
        if (koWmfOpTab32[ i ].winRasterOp == param)  break;
    }

    if (i < 15)
        return koWmfOpTab32[ i ].qtRasterOp;
    else
        return QPainter::CompositionMode_SourceOver;
}


bool WmfParser::dibToBmp(QImage& bmp, QDataStream& stream, quint32 size)
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
    pattern.resize(sizeBmp);
    pattern.fill(0);
    stream.readRawData(pattern.data() + 14, size);

    // add BMP header
    // First cast to void* to silence alignment warnings
    BMPFILEHEADER* bmpHeader;
    bmpHeader = (BMPFILEHEADER*)(void *)(pattern.data());
    bmpHeader->bmType = 0x4D42;
    bmpHeader->bmSize = sizeBmp;

//    if ( !bmp.loadFromData( (const uchar*)bmpHeader, pattern.size(), "BMP" ) ) {
    if (!bmp.loadFromData(pattern, "BMP")) {
        debugVectorImage << "WmfParser::dibToBmp: invalid bitmap";
        return false;
    } else {
        return true;
    }
}


}
