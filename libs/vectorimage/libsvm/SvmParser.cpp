/* This file is part of the Calligra project

  Copyright 2011 Inge Wallin <inge@lysator.liu.se>
  Copyright 2011 Pierre Ducroquet <pinaraf@pinaraf.info>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either 
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public 
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/


// Own
#include "SvmParser.h"

// Qt
#include <QByteArray>
#include <QBuffer>
#include <QDataStream>
#include <QString>
#include <QPolygon>

// KDE
#include <VectorImageDebug.h>

// Libsvm
#include "SvmEnums.h"
#include "SvmStructs.h"

// 0 - No debug
// 1 - Print a lot of debug info
// 2 - Just print all the records instead of parsing them
#define DEBUG_SVMPARSER 0

namespace Libsvm
{


static void soakBytes( QDataStream &stream, int numBytes )
{
    quint8 scratch;
    for ( int i = 0; i < numBytes; ++i ) {
        stream >> scratch;
    }
}


SvmParser::SvmParser()
    : mContext()
    , mBackend(0)
{
}


static const struct ActionNames {
    int     actionNumber;
    QString actionName;
} actionNames[] = {
    { META_NULL_ACTION,                  "META_NULL_ACTION" },
    { META_PIXEL_ACTION,                 "META_PIXEL_ACTION" },
    { META_POINT_ACTION,                 "META_POINT_ACTION" },
    { META_LINE_ACTION,                  "META_LINE_ACTION" },
    { META_RECT_ACTION,                  "META_RECT_ACTION" },
    { META_ROUNDRECT_ACTION,             "META_ROUNDRECT_ACTION" },
    { META_ELLIPSE_ACTION,               "META_ELLIPSE_ACTION" },
    { META_ARC_ACTION,                   "META_ARC_ACTION" },
    { META_PIE_ACTION,                   "META_PIE_ACTION" },
    { META_CHORD_ACTION,                 "META_CHORD_ACTION" },
    { META_POLYLINE_ACTION,              "META_POLYLINE_ACTION" },
    { META_POLYGON_ACTION,               "META_POLYGON_ACTION" },
    { META_POLYPOLYGON_ACTION,           "META_POLYPOLYGON_ACTION" },
    { META_TEXT_ACTION,                  "META_TEXT_ACTION" },
    { META_TEXTARRAY_ACTION,             "META_TEXTARRAY_ACTION" },
    { META_STRETCHTEXT_ACTION,           "META_STRETCHTEXT_ACTION" },
    { META_TEXTRECT_ACTION,              "META_TEXTRECT_ACTION" },
    { META_BMP_ACTION,                   "META_BMP_ACTION" },
    { META_BMPSCALE_ACTION,              "META_BMPSCALE_ACTION" },
    { META_BMPSCALEPART_ACTION,          "META_BMPSCALEPART_ACTION" },
    { META_BMPEX_ACTION,                 "META_BMPEX_ACTION" },
    { META_BMPEXSCALE_ACTION,            "META_BMPEXSCALE_ACTION" },
    { META_BMPEXSCALEPART_ACTION,        "META_BMPEXSCALEPART_ACTION" },
    { META_MASK_ACTION,                  "META_MASK_ACTION" },
    { META_MASKSCALE_ACTION,             "META_MASKSCALE_ACTION" },
    { META_MASKSCALEPART_ACTION,         "META_MASKSCALEPART_ACTION" },
    { META_GRADIENT_ACTION,              "META_GRADIENT_ACTION" },
    { META_HATCH_ACTION,                 "META_HATCH_ACTION" },
    { META_WALLPAPER_ACTION,             "META_WALLPAPER_ACTION" },
    { META_CLIPREGION_ACTION,            "META_CLIPREGION_ACTION" },
    { META_ISECTRECTCLIPREGION_ACTION,   "META_ISECTRECTCLIPREGION_ACTION" },
    { META_ISECTREGIONCLIPREGION_ACTION, "META_ISECTREGIONCLIPREGION_ACTION" },
    { META_MOVECLIPREGION_ACTION,        "META_MOVECLIPREGION_ACTION" },
    { META_LINECOLOR_ACTION,             "META_LINECOLOR_ACTION" },
    { META_FILLCOLOR_ACTION,             "META_FILLCOLOR_ACTION" },
    { META_TEXTCOLOR_ACTION,             "META_TEXTCOLOR_ACTION" },
    { META_TEXTFILLCOLOR_ACTION,         "META_TEXTFILLCOLOR_ACTION" },
    { META_TEXTALIGN_ACTION,             "META_TEXTALIGN_ACTION" },
    { META_MAPMODE_ACTION,               "META_MAPMODE_ACTION" },
    { META_FONT_ACTION,                  "META_FONT_ACTION" },
    { META_PUSH_ACTION,                  "META_PUSH_ACTION" },
    { META_POP_ACTION,                   "META_POP_ACTION" },
    { META_RASTEROP_ACTION,              "META_RASTEROP_ACTION" },
    { META_TRANSPARENT_ACTION,           "META_TRANSPARENT_ACTION" },
    { META_EPS_ACTION,                   "META_EPS_ACTION" },
    { META_REFPOINT_ACTION,              "META_REFPOINT_ACTION" },
    { META_TEXTLINECOLOR_ACTION,         "META_TEXTLINECOLOR_ACTION" },
    { META_TEXTLINE_ACTION,              "META_TEXTLINE_ACTION" },
    { META_FLOATTRANSPARENT_ACTION,      "META_FLOATTRANSPARENT_ACTION" },
    { META_GRADIENTEX_ACTION,            "META_GRADIENTEX_ACTION" },
    { META_LAYOUTMODE_ACTION,            "META_LAYOUTMODE_ACTION" },
    { META_TEXTLANGUAGE_ACTION,          "META_TEXTLANGUAGE_ACTION" },
    { META_OVERLINECOLOR_ACTION,         "META_OVERLINECOLOR_ACTION" },
    { META_RENDERGRAPHIC_ACTION,         "META_RENDERGRAPHIC_ACTION" },
    { META_COMMENT_ACTION,               "META_COMMENT_ACTION" }
};


void SvmParser::setBackend(SvmAbstractBackend *backend)
{
    mBackend = backend;
}


bool SvmParser::parse(const QByteArray &data)
{
    // Check the signature "VCLMTF"
    if (!data.startsWith("VCLMTF"))
        return false;

    QBuffer buffer((QByteArray *) &data);
    buffer.open(QIODevice::ReadOnly);

    QDataStream mainStream(&buffer);
    mainStream.setByteOrder(QDataStream::LittleEndian);

    // Start reading from the stream: read past the signature and get the header.
    soakBytes(mainStream, 6);
    SvmHeader header(mainStream);
#if DEBUG_SVMPARSER
    debugVectorImage << "================ SVM HEADER ================";
    debugVectorImage << "version, length:" << header.versionCompat.version << header.versionCompat.length;
    debugVectorImage << "compressionMode:" << header.compressionMode;
    debugVectorImage << "mapMode:" << "Origin" << header.mapMode.origin
                  << "scaleX"
                  << header.mapMode.scaleX.numerator << header.mapMode.scaleX.denominator
                  << (qreal(header.mapMode.scaleX.numerator) / header.mapMode.scaleX.denominator)
                  << "scaleY"
                  << header.mapMode.scaleY.numerator << header.mapMode.scaleY.denominator
                  << (qreal(header.mapMode.scaleY.numerator) / header.mapMode.scaleY.denominator);
    debugVectorImage << "size:" << header.width << header.height;
    debugVectorImage << "actionCount:" << header.actionCount;
    debugVectorImage << "================ SVM HEADER ================";
#endif    

    mBackend->init(header);

#if DEBUG_SVMPARSER
    {
        QPolygon polygon;
        polygon << QPoint(0, 0);
        polygon << QPoint(header.width, header.height);
        mBackend->polyLine(mContext, polygon);
    }
#endif

    // Parse all actions and call the appropriate backend callback for
    // the graphics drawing actions.  The context actions will
    // manipulate the graphics context, which is maintained here.
    for (uint action = 0; action < header.actionCount; ++action) {
        quint16  actionType;
        quint16  version;
        quint32  totalSize;

        // Here starts the Action itself. The first two bytes is the action type. 
        mainStream >> actionType;

        // The VersionCompat object
        mainStream >> version;
        mainStream >> totalSize;
        
        char *rawData = new char[totalSize];
        mainStream.readRawData(rawData, totalSize);
        QByteArray dataArray(rawData, totalSize);
        QDataStream stream(&dataArray, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::LittleEndian);

        // Debug
#if DEBUG_SVMPARSER
        {
            QString name;
            if (actionType == 0)
                name = actionNames[0].actionName;
            else if (100 <= actionType && actionType <= META_LAST_ACTION)
                name = actionNames[actionType - 99].actionName;
            else if (actionType == 512)
                name = "META_COMMENT_ACTION";
            else
                name = "(out of bounds)";

            debugVectorImage << name << "(" << actionType << ")" << "version" << version
                          << "totalSize" << totalSize;
        }
#endif

        // Parse all actions.
        switch (actionType) {
        case META_NULL_ACTION:
            break;
        case META_PIXEL_ACTION:
            break;
        case META_POINT_ACTION:
            break;
        case META_LINE_ACTION:
            break;
        case META_RECT_ACTION:
            {
                QRect  rect;

                parseRect(stream, rect);
                debugVectorImage << "Rect:"  << rect;
                mBackend->rect(mContext, rect);
            }
            break;
        case META_ROUNDRECT_ACTION:
            break;
        case META_ELLIPSE_ACTION:
            break;
        case META_ARC_ACTION:
            break;
        case META_PIE_ACTION:
            break;
        case META_CHORD_ACTION:
            break;
        case META_POLYLINE_ACTION:
            {
                QPolygon  polygon;

                parsePolygon(stream, polygon);
                debugVectorImage << "Polyline:"  << polygon;
                mBackend->polyLine(mContext, polygon);

                // FIXME: Version 2: Lineinfo, Version 3: polyflags
                if (version > 1)
                    soakBytes(stream, totalSize - 2 - 4 * 2 * polygon.size());
            }
            break;
        case META_POLYGON_ACTION:
            {
                QPolygon  polygon;

                parsePolygon(stream, polygon);
                debugVectorImage << "Polygon:"  << polygon;
                mBackend->polygon(mContext, polygon);

                // FIXME: Version 2: Lineinfo, Version 3: polyflags
                if (version > 1)
                    soakBytes(stream, totalSize - 2 - 4 * 2 * polygon.size());
            }
            break;
        case META_POLYPOLYGON_ACTION:
            {
                quint16 polygonCount;
                stream >> polygonCount;
                //debugVectorImage << "Number of polygons:"  << polygonCount;

                QList<QPolygon> polygons;
                for (quint16 i = 0 ; i < polygonCount ; i++) {
                    QPolygon polygon;
                    parsePolygon(stream, polygon);
                    polygons << polygon;
                    //debugVectorImage << "Polygon:"  << polygon;
                }
                
                if (version > 1) {
                    quint16 complexPolygonCount;
                    stream >> complexPolygonCount;
                    //debugVectorImage << "Number of complex polygons:"  << complexPolygonCount;

                    // Parse the so called "complex polygons". For
                    // each one, there is an index and a polygon.  The
                    // index tells which of the original polygons to
                    // replace.
                    for (quint16 i = 0; i < complexPolygonCount; i++) {
                        quint16 complexPolygonIndex;
                        stream >> complexPolygonIndex;

                        QPolygon polygon;
                        parsePolygon(stream, polygon);
                        //debugVectorImage << "polygon index:"  << complexPolygonIndex << polygon;

                        // FIXME: The so called complex polygons have something to do
                        //        with modifying the polygons, but I have not yet been
                        //        able to understand how.  So until I do, we'll disable
                        //        this.
                        //polygons[complexPolygonIndex] = polygon;
                    }
                }
                
                mBackend->polyPolygon(mContext, polygons);
            }
            break;
        case META_TEXT_ACTION:
            break;
        case META_TEXTARRAY_ACTION:
            {
                QPoint   startPoint;
                QString  string;
                quint16  startIndex;
                quint16  len;
                quint32  dxArrayLen;
                qint32  *dxArray = 0;

                stream >> startPoint;
                parseString(stream, string);
                stream >> startIndex;
                stream >> len;
                stream >> dxArrayLen;
                if (dxArrayLen > 0) {
                    quint32 maxDxArrayLen = totalSize - stream.device()->pos();
                    if (dxArrayLen > maxDxArrayLen) {
                        debugVectorImage << "Defined dxArrayLen= " << dxArrayLen << "exceeds available size" << maxDxArrayLen;
                        dxArrayLen = maxDxArrayLen;
                    }

                    dxArray = new qint32[dxArrayLen];

                    for (uint i = 0; i < dxArrayLen; ++i)
                        stream >> dxArray[i];
                }

                if (version > 1) {
                    quint16  len2;

                    stream >> len2;
                    // FIXME: More here
                }

#if 0
                debugVectorImage << "Text: " << startPoint << string
                              << startIndex << len;
                if (dxArrayLen > 0) {
                    debugVectorImage << "dxArrayLen:" << dxArrayLen;
                    for (uint i = 0; i < dxArrayLen; ++i)
                        debugVectorImage << dxArray[i];
                }
                else
                    debugVectorImage << "dxArrayLen = 0";
#endif
                mBackend->textArray(mContext, startPoint, string, startIndex, len,
                                    dxArrayLen, dxArray);

                if (dxArrayLen)
                    delete[] dxArray;
            }
            break;
        case META_STRETCHTEXT_ACTION:
            break;
        case META_TEXTRECT_ACTION:
            break;
        case META_BMP_ACTION:
            break;
        case META_BMPSCALE_ACTION:
            break;
        case META_BMPSCALEPART_ACTION:
            break;
        case META_BMPEX_ACTION:
            break;
        case META_BMPEXSCALE_ACTION:
            break;
        case META_BMPEXSCALEPART_ACTION:
            break;
        case META_MASK_ACTION:
            break;
        case META_MASKSCALE_ACTION:
            break;
        case META_MASKSCALEPART_ACTION:
            break;
        case META_GRADIENT_ACTION:
            break;
        case META_HATCH_ACTION:
            break;
        case META_WALLPAPER_ACTION:
            break;
        case META_CLIPREGION_ACTION:
            break;
        case META_ISECTRECTCLIPREGION_ACTION:
            break;
        case META_ISECTREGIONCLIPREGION_ACTION:
            break;
        case META_MOVECLIPREGION_ACTION:
            break;
        case META_LINECOLOR_ACTION:
            {
                quint32  colorData;

                stream >> colorData;
                stream >> mContext.lineColorSet;

                mContext.lineColor = QColor::fromRgb(colorData);
                debugVectorImage << "Color:"  << mContext.lineColor 
                              << '(' << mContext.lineColorSet << ')';
                mContext.changedItems |= GCLineColor;
            }
            break;
        case META_FILLCOLOR_ACTION:
            {
                quint32  colorData;

                stream >> colorData;
                stream >> mContext.fillColorSet;
                //mContext.fillColorSet = false;
                
                debugVectorImage << "Fill color :" << hex << colorData << dec
                              << '(' << mContext.fillColorSet << ')';

                mContext.fillColor = QColor::fromRgb(colorData);
                mContext.changedItems |= GCFillColor;
            }
            break;
        case META_TEXTCOLOR_ACTION:
            {
                quint32  colorData;
                stream >> colorData;

                mContext.textColor = QColor::fromRgb(colorData);
                debugVectorImage << "Color:"  << mContext.textColor;
                mContext.changedItems |= GCTextColor;
            }
            break;
        case META_TEXTFILLCOLOR_ACTION:
            {
                quint32  colorData;

                stream >> colorData;
                stream >> mContext.textFillColorSet;
                
                debugVectorImage << "Text fill color :" << hex << colorData << dec
                              << '(' << mContext.textFillColorSet << ')';

                mContext.textFillColor = QColor::fromRgb(colorData);
                debugVectorImage << "Color:"  << mContext.textFillColor
                              << '(' << mContext.textFillColorSet << ')';
                mContext.changedItems |= GCTextFillColor;
            }
            break;
        case META_TEXTALIGN_ACTION:
            {
                quint16  textAlign;
                stream >> textAlign;

                mContext.textAlign = (TextAlign)textAlign;
                debugVectorImage << "TextAlign:"  << mContext.textAlign;
                mContext.changedItems |= GCTextAlign;
            }
            break;
        case META_MAPMODE_ACTION:
            {
                stream >> mContext.mapMode;
                debugVectorImage << "mapMode:" << "Origin" << mContext.mapMode.origin
                              << "scaleX"
                              << mContext.mapMode.scaleX.numerator << mContext.mapMode.scaleX.denominator
                              << (qreal(mContext.mapMode.scaleX.numerator) / mContext.mapMode.scaleX.denominator)
                              << "scaleY"
                              << mContext.mapMode.scaleY.numerator << mContext.mapMode.scaleY.denominator
                              << (qreal(mContext.mapMode.scaleY.numerator) / mContext.mapMode.scaleY.denominator);
                mContext.changedItems |= GCMapMode;
            }
            break;
        case META_FONT_ACTION:
            {
                parseFont(stream, mContext.font);
                debugVectorImage << "Font:"  << mContext.font;
                mContext.changedItems |= GCFont;
            }
            break;
        case META_PUSH_ACTION:
            {
                debugVectorImage << "Push action : " << totalSize;
                quint16 pushValue;
                stream >> pushValue;
                debugVectorImage << "Push value : " << pushValue;
            }
            break;
        case META_POP_ACTION:
            {
                debugVectorImage << "Pop action : " << totalSize;
                /*quint16 pushValue;
                stream >> pushValue;
                debugVectorImage << "Push value : " << pushValue;*/
            }
            break;
        case META_RASTEROP_ACTION:
            break;
        case META_TRANSPARENT_ACTION:
            break;
        case META_EPS_ACTION:
            break;
        case META_REFPOINT_ACTION:
            break;
        case META_TEXTLINECOLOR_ACTION:
            break;
        case META_TEXTLINE_ACTION:
            break;
        case META_FLOATTRANSPARENT_ACTION:
            break;
        case META_GRADIENTEX_ACTION:
            break;
        case META_LAYOUTMODE_ACTION:
            {
                stream >> mContext.layoutMode;
                debugVectorImage << "New layout mode:" << hex << mContext.layoutMode << dec << "hex";
            }
            break;
        case META_TEXTLANGUAGE_ACTION:
            break;
        case META_OVERLINECOLOR_ACTION:
            {
                quint32  colorData;

                stream >> colorData;
                stream >> mContext.overlineColorSet;
                
                debugVectorImage << "Overline color :" << colorData
                              << '(' << mContext.overlineColorSet << ')';

                mContext.overlineColor = QColor::fromRgb(colorData);
                mContext.changedItems |= GCOverlineColor;
            }
            break;
        case META_RENDERGRAPHIC_ACTION:
            //dumpAction(stream, version, totalSize);
            break;
        case META_COMMENT_ACTION:
            break;

        default:
#if DEBUG_SVMPARSER
            debugVectorImage << "unknown action type:" << actionType;
#endif
            break;
        }

        delete [] rawData;
        
        // Security measure
        if (mainStream.atEnd())
            break;
    }

    mBackend->cleanup();

    return true;
}


// ----------------------------------------------------------------
//                         Private methods


void SvmParser::parseRect(QDataStream &stream, QRect &rect)
{
    qint32 left;
    qint32 top;
    qint32 right;
    qint32 bottom;

    stream >> left;
    stream >> top;
    stream >> right;
    stream >> bottom;

    rect.setLeft(left);
    rect.setTop(top);
    rect.setRight(right);
    rect.setBottom(bottom);
}

void SvmParser::parsePolygon(QDataStream &stream, QPolygon &polygon)
{
    quint16   numPoints;
    QPoint    point;

    stream >> numPoints;
    for (uint i = 0; i < numPoints; ++i) {
        stream >> point;
        polygon << point;
    }
}

void SvmParser::parseString(QDataStream &stream, QString &string)
{
    quint16  length;

    stream >> length;
    for (uint i = 0; i < length; ++i) {
        quint8  ch;
        stream >> ch;
        string += char(ch);
    }
}

void SvmParser::parseFont(QDataStream &stream, QFont &font)
{
    quint16  version;
    quint32  totalSize;

    // the VersionCompat struct
    stream >> version;
    stream >> totalSize;

    // Name and style
    QString  family;
    QString  style;
    parseString(stream, family);
    parseString(stream, style);
    font.setFamily(family);

    // Font size
    quint32  width;
    quint32  height;
    stream >> width;
    stream >> height;
    // Multiply by 0.7 since it seems that, just like WMF, the height
    // in the font struct is actually the height of the character cell.
    font.setPointSize(height * 7 / 10);

    qint8   temp8;
    bool    tempbool;
    quint16 tempu16;
    stream >> tempu16;          // charset
    stream >> tempu16;          // family
    stream >> tempu16;          // pitch
    stream >> tempu16;          // weight
    stream >> tempu16;          // underline
    font.setUnderline(tempu16);
    stream >> tempu16;          // strikeout
    stream >> tempu16;          // italic
    font.setItalic(tempu16);
    stream >> tempu16;          // language
    stream >> tempu16;          // width
    stream >> tempu16;          // orientation

    stream >> tempbool;         // wordline
    stream >> tempbool;         // outline
    stream >> tempbool;         // shadow
    stream >> temp8;            // kerning

    if (version > 1) {
        stream >> temp8;        // relief
        stream >> tempu16;      // language
        stream >> tempbool;     // vertical
        stream >> tempu16;      // emphasis
    }

    if (version > 2) {
        stream >> tempu16;      // overline
    }

    // FIXME: Read away the rest of font here to allow for higher versions than 3.
}


void SvmParser::dumpAction(QDataStream &stream, quint16 version, quint32 totalSize)
{
    debugVectorImage << "Version: " << version;
    for (uint i = 0; i < totalSize; ++i) {
        quint8  temp;
        stream >> temp;
        debugVectorImage << hex << i << temp << dec;
    }
}

} // namespace Libsvm
