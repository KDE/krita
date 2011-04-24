/* This file is part of the Calligra project

  Copyright 2011 Inge Wallin <inge@lysator.liu.se>

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

// KDE
#include <KDebug>

// Libsvm
#include "SvmEnums.h"
#include "SvmStructs.h"

// 0 - No debug
// 1 - Print a lot of debug info
// 2 - Just print all the records instead of parsing them
#define DEBUG_SVMPARSER 1

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
    { META_SVG_SOMETHING_ACTION,         "META_SVG_SOMETHING_ACTION" },
    { META_COMMENT_ACTION,               "META_COMMENT_ACTION" }
};


bool SvmParser::parse(const QByteArray &data)
{
    // Check the signature "VCLMTF"
    if (!data.startsWith("VCLMTF"))
        return false;

    QBuffer buffer((QByteArray *) &data);
    buffer.open(QIODevice::ReadOnly);

    QDataStream stream(&buffer);
    stream.setByteOrder(QDataStream::LittleEndian);

    // Start reading from the stream: read past the signature and get the header.
    soakBytes(stream, 6);
    SvmHeader  header(stream);
#if DEBUG_SVMPARSER
    kDebug(31000) << "================ SVM HEADER ================";
    kDebug(31000) << "version, length:" << header.versionCompat.version << header.versionCompat.length;
    kDebug(31000) << "compressionMode:" << header.compressionMode;
    kDebug(31000) << "mapMode:" << "...";
    kDebug(31000) << "size:" << header.width << header.height;
    kDebug(31000) << "actionCount:" << header.actionCount;
    kDebug(31000) << "================ SVM HEADER ================";
#endif    

    for (uint action = 0; action < header.actionCount; ++action) {
        quint16  actionType;
        quint16  version;
        quint32  length;

        // Here starts the Action itself. The first two bytes is the action type. 
        stream >> actionType;
        actionType = (actionType >> 8) & 0xff;

        // The VersionCompat object;
        stream >> version;
        stream >> length;

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

            kDebug(31000) << "Action type " << hex << actionType << dec << "(" << actionType << ")"
                          << "length" << length << "version" << version
                          << name;
        }
#endif

        // Parse all actions.
        switch (actionType) {
        case META_NULL_ACTION:
        case META_PIXEL_ACTION:
        case META_POINT_ACTION:
        case META_LINE_ACTION:
        case META_RECT_ACTION:
        case META_ROUNDRECT_ACTION:
        case META_ELLIPSE_ACTION:
        case META_ARC_ACTION:
        case META_PIE_ACTION:
        case META_CHORD_ACTION:
        case META_POLYLINE_ACTION:
        case META_POLYGON_ACTION:
        case META_POLYPOLYGON_ACTION:
        case META_TEXT_ACTION:
        case META_TEXTARRAY_ACTION:
        case META_STRETCHTEXT_ACTION:
        case META_TEXTRECT_ACTION:
        case META_BMP_ACTION:
        case META_BMPSCALE_ACTION:
        case META_BMPSCALEPART_ACTION:
        case META_BMPEX_ACTION:
        case META_BMPEXSCALE_ACTION:
        case META_BMPEXSCALEPART_ACTION:
        case META_MASK_ACTION:
        case META_MASKSCALE_ACTION:
        case META_MASKSCALEPART_ACTION:
        case META_GRADIENT_ACTION:
        case META_HATCH_ACTION:
        case META_WALLPAPER_ACTION:
        case META_CLIPREGION_ACTION:
        case META_ISECTRECTCLIPREGION_ACTION:
        case META_ISECTREGIONCLIPREGION_ACTION:
        case META_MOVECLIPREGION_ACTION:
        case META_LINECOLOR_ACTION:
        case META_FILLCOLOR_ACTION:
        case META_TEXTCOLOR_ACTION:
        case META_TEXTFILLCOLOR_ACTION:
        case META_TEXTALIGN_ACTION:
        case META_MAPMODE_ACTION:
        case META_FONT_ACTION:
        case META_PUSH_ACTION:
        case META_POP_ACTION:
        case META_RASTEROP_ACTION:
        case META_TRANSPARENT_ACTION:
        case META_EPS_ACTION:
        case META_REFPOINT_ACTION:
        case META_TEXTLINECOLOR_ACTION:
        case META_TEXTLINE_ACTION:
        case META_FLOATTRANSPARENT_ACTION:
        case META_GRADIENTEX_ACTION:
        case META_LAYOUTMODE_ACTION:
        case META_TEXTLANGUAGE_ACTION:
        case META_OVERLINECOLOR_ACTION:
        case META_COMMENT_ACTION:
            // Use this for unhandled actions: 6 is the number of
            // bytes in the stream for the VersionCompat object,
            // i.e. the version + length fields.
            soakBytes(stream, length - 6);
            break;

        default:
#if DEBUG_SVMPARSER
            kDebug(31000) << "unknown action type:" << actionType;
#endif
            // We couldn't recognize the type so let's just read past it.
            soakBytes(stream, length - 6);
        }

        // Security measure
        if (stream.atEnd())
            break;
    }

    return true;
}


} // namespace Libsvm
