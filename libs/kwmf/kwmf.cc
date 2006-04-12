/*
    Copyright (C) 2000, S.R.Haque <shaheedhaque@hotmail.com>.
    This file is part of the KDE project

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
 * Boston, MA 02110-1301, USA.

DESCRIPTION

    This is based on code originally written by Stefan Taferner
    (taferner@kde.org) and also borrows from libwmf (by Martin Vermeer and
    Caolan McNamara).
*/

#include <kdebug.h>
#include <math.h>
#include <QFile>
#include <q3pointarray.h>
#include "kwmf.h"
#include <QRect>

#define PI (3.14159265358979323846)

const int KWmf::s_area = 30504;
const int KWmf::s_maxHandles = 64;

KWmf::KWmf(
    unsigned dpi)
{
    m_dpi = dpi;
    m_objectHandles = new WinObjHandle*[s_maxHandles];
}

KWmf::~KWmf()
{
    delete[] m_objectHandles;
}

//
//
//

void KWmf::brushSet(
    unsigned colour,
    unsigned style)
{
    m_dc.m_brushColour = colour;
    m_dc.m_brushStyle = style;
}

//-----------------------------------------------------------------------------
unsigned KWmf::getColour(
    S32 colour)
{
    unsigned red, green, blue;

    red = colour & 255;
    green = (colour >> 8) & 255;
    blue = (colour >> 16) & 255;
    return (red << 16) + (green << 8) + blue;
}

void KWmf::genericArc(
    QString type,
    QDataStream &operands)
{
    QPoint topLeft;
    QPoint bottomRight;
    QPoint start;
    QPoint end;

    topLeft = normalisePoint(operands);
    bottomRight = normalisePoint(operands);
    start = normalisePoint(operands);
    end = normalisePoint(operands);

    // WMF defines arcs with the major and minor axes of an ellipse, and two points.
    // From each point draw a line to the center of the ellipse: the intercepts define
    // the ends of the arc.

    QRect ellipse(topLeft, bottomRight);
    QPoint centre = ellipse.center();
    double startAngle = atan2((double)(centre.y() - start.y()), (double)(centre.x() - start.x()));
    double stopAngle = atan2((double)(centre.y() - end.y()), (double)(centre.x() - end.x()));

    startAngle = 180 * startAngle / PI;
    stopAngle = 180 * stopAngle / PI;

    gotEllipse(m_dc, type, centre, ellipse.size() / 2,
               static_cast<unsigned int>(startAngle),
               static_cast<unsigned int>(stopAngle));
}

int KWmf::handleIndex(void) const
{
    int i;

    for (i = 0; i < s_maxHandles; i++)
    {
        if (!m_objectHandles[i])
            return i;
    }
    kError(s_area) << "handle table full !" << endl;
    return -1;
}

//-----------------------------------------------------------------------------
KWmf::WinObjPenHandle *KWmf::handleCreatePen(void)
{
    WinObjPenHandle *handle = new WinObjPenHandle;
    int idx = handleIndex();

    if (idx >= 0)
        m_objectHandles[idx] = handle;
    return handle;
}

//-----------------------------------------------------------------------------
KWmf::WinObjBrushHandle *KWmf::handleCreateBrush(void)
{
    WinObjBrushHandle *handle = new WinObjBrushHandle;
    int idx = handleIndex();

    if (idx >= 0)
        m_objectHandles[idx] = handle;
    return handle;
}

//-----------------------------------------------------------------------------
void KWmf::handleDelete(int idx)
{
    if (idx >= 0 && idx < s_maxHandles && m_objectHandles[idx])
    {
        delete m_objectHandles[idx];
        m_objectHandles[idx] = NULL;
    }
}

//
//
//

void KWmf::invokeHandler(
    S16 opcode,
    U32 words,
    QDataStream &operands)
{
    typedef void (KWmf::*method)(U32 words, QDataStream &operands);

    typedef struct
    {
        const char *name;
        unsigned short opcode;
        method handler;
    } opcodeEntry;

    static const opcodeEntry funcTab[] =
    {
        { "ANIMATEPALETTE",       0x0436, 0 },
        { "ARC",                  0x0817, &KWmf::opArc },
        { "BITBLT",               0x0922, 0 },
        { "CHORD",                0x0830, 0 },
        { "CREATEBRUSHINDIRECT",  0x02FC, &KWmf::opBrushCreateIndirect },
        { "CREATEFONTINDIRECT",   0x02FB, 0 },
        { "CREATEPALETTE",        0x00F7, 0 },
        { "CREATEPATTERNBRUSH",   0x01F9, 0 },
        { "CREATEPENINDIRECT",    0x02FA, &KWmf::opPenCreateIndirect },
        { "CREATEREGION",         0x06FF, 0 },
        { "DELETEOBJECT",         0x01F0, &KWmf::opObjectDelete },
        { "DIBBITBLT",            0x0940, 0 },
        { "DIBCREATEPATTERNBRUSH",0x0142, 0 },
        { "DIBSTRETCHBLT",        0x0b41, 0 },
        { "ELLIPSE",              0x0418, &KWmf::opEllipse },
        { "ESCAPE",               0x0626, &KWmf::opNoop },
        { "EXCLUDECLIPRECT",      0x0415, 0 },
        { "EXTFLOODFILL",         0x0548, 0 },
        { "EXTTEXTOUT",           0x0a32, 0 },
        { "FILLREGION",           0x0228, 0 },
        { "FLOODFILL",            0x0419, 0 },
        { "FRAMEREGION",          0x0429, 0 },
        { "INTERSECTCLIPRECT",    0x0416, 0 },
        { "INVERTREGION",         0x012A, 0 },
        { "LINETO",               0x0213, &KWmf::opLineTo },
        { "MOVETO",               0x0214, &KWmf::opMoveTo },
        { "OFFSETCLIPRGN",        0x0220, 0 },
        { "OFFSETVIEWPORTORG",    0x0211, 0 },
        { "OFFSETWINDOWORG",      0x020F, 0 },
        { "PAINTREGION",          0x012B, 0 },
        { "PATBLT",               0x061D, 0 },
        { "PIE",                  0x081A, &KWmf::opPie },
        { "POLYGON",              0x0324, &KWmf::opPolygon },
        { "POLYLINE",             0x0325, &KWmf::opPolyline },
        { "POLYPOLYGON",          0x0538, 0 },
        { "REALIZEPALETTE",       0x0035, 0 },
        { "RECTANGLE",            0x041B, &KWmf::opRectangle },
        { "RESIZEPALETTE",        0x0139, 0 },
        { "RESTOREDC",            0x0127, &KWmf::opRestoreDc },
        { "ROUNDRECT",            0x061C, 0 },
        { "SAVEDC",               0x001E, &KWmf::opSaveDc },
        { "SCALEVIEWPORTEXT",     0x0412, 0 },
        { "SCALEWINDOWEXT",       0x0410, 0 },
        { "SELECTCLIPREGION",     0x012C, 0 },
        { "SELECTOBJECT",         0x012D, &KWmf::opObjectSelect },
        { "SELECTPALETTE",        0x0234, 0 },
        { "SETBKCOLOR",           0x0201, 0 },
        { "SETBKMODE",            0x0102, 0 },
        { "SETDIBTODEV",          0x0d33, 0 },
        { "SETMAPMODE",           0x0103, 0 },
        { "SETMAPPERFLAGS",       0x0231, 0 },
        { "SETPALENTRIES",        0x0037, 0 },
        { "SETPIXEL",             0x041F, 0 },
        { "SETPOLYFILLMODE",      0x0106, &KWmf::opPolygonSetFillMode },
        { "SETRELABS",            0x0105, 0 },
        { "SETROP2",              0x0104, 0 },
        { "SETSTRETCHBLTMODE",    0x0107, 0 },
        { "SETTEXTALIGN",         0x012E, 0 },
        { "SETTEXTCHAREXTRA",     0x0108, 0 },
        { "SETTEXTCOLOR",         0x0209, 0 },
        { "SETTEXTJUSTIFICATION", 0x020A, 0 },
        { "SETVIEWPORTEXT",       0x020E, 0 },
        { "SETVIEWPORTORG",       0x020D, 0 },
        { "SETWINDOWEXT",         0x020C, &KWmf::opWindowSetExt },
        { "SETWINDOWORG",         0x020B, &KWmf::opWindowSetOrg },
        { "STRETCHBLT",           0x0B23, 0 },
        { "STRETCHDIB",           0x0f43, 0 },
        { "TEXTOUT",              0x0521, 0 },
        { NULL,                   0,      0 }
    };
    unsigned i;
    method result;

    // Scan lookup table for operation.

    for (i = 0; funcTab[i].name; i++)
    {
        if (funcTab[i].opcode == opcode)
        {
            break;
        }
    }

    // Invoke handler.

    result = funcTab[i].handler;
    if (!result)
    {
        if (funcTab[i].name)
            kError(s_area) << "invokeHandler: unsupported opcode: " <<
                funcTab[i].name <<
                " operands: " << words << endl;
        else
            kError(s_area) << "invokeHandler: unsupported opcode: 0x" <<
                QString::number(opcode, 16) <<
                " operands: " << words << endl;

        // Skip data we cannot use.

        for (i = 0; i < words; i++)
        {
            S16 discard;

            operands >> discard;
        }
    }
    else
    {
        kDebug(s_area) << "invokeHandler: opcode: " << funcTab[i].name <<
            " operands: " << words << endl;

        // We don't invoke the handler directly on the incoming operands, but
        // via a temporary datastream. This adds overhead, but eliminates the
        // need for the individual handlers to read *exactly* the right amount
        // of data (thus speeding development, and possibly adding some
        // future-proofing).

        if (words)
        {
            QByteArray *record = new QByteArray();
	    record->resize( words*2 );
            QDataStream *body;

            operands.readRawData(record->data(), words * 2);
            body = new QDataStream(record, QIODevice::ReadOnly);
            body->setByteOrder(QDataStream::LittleEndian);
            (this->*result)(words, *body);
            delete body;
            delete record;
        }
        else
        {
            QDataStream *body = new QDataStream();

            (this->*result)(words, *body);
            delete body;
        }
    }
}

QPoint KWmf::normalisePoint(
    QDataStream &operands)
{
    S16 x;
    S16 y;

    operands >> x >> y;
    return QPoint((x - m_windowOrgX) * m_windowFlipX / m_dpi, (y - m_windowOrgY) * m_windowFlipY / m_dpi);
}

QSize KWmf::normaliseSize(
    QDataStream &operands)
{
    S16 width;
    S16 height;

    operands >> width >> height;
    return QSize(width / m_dpi, height / m_dpi);
}

bool KWmf::parse(
    const QString &file)
{
    QFile in(file);
    if (!in.open(QIODevice::ReadOnly))
    {
        kError(s_area) << "Unable to open input file!" << endl;
        in.close();
        return false;
    }
    QDataStream stream(&in);
    bool result = parse(stream, in.size());
    in.close();
    return result;
}

bool KWmf::parse(
    QDataStream &stream,
    unsigned size)
{
    int startedAt;
    bool isPlaceable;
    bool isEnhanced;

    startedAt = stream.device()->pos();
    stream.setByteOrder(QDataStream::LittleEndian); // Great, I love Qt !

    for (int i = 0; i < s_maxHandles; i++)
        m_objectHandles[i] = NULL;

    typedef struct _RECT
    {
        S16 left;
        S16 top;
        S16 right;
        S16 bottom;
    } RECT;

    typedef struct _RECTL
    {
        S32 left;
        S32 top;
        S32 right;
        S32 bottom;
    } RECTL;

    typedef struct _SIZE
    {
        S16 width;
        S16 height;
    } SIZE;

    typedef struct _SIZEL
    {
        S32 width;
        S32 height;
    } SIZEL;

    struct WmfEnhMetaHeader
    {
        S32 iType;                  // Record type EMR_HEADER
        S32 nSize;                  // Record size in bytes.  This may be greater
                                    // than the sizeof(ENHMETAHEADER).
        RECTL rclBounds;            // Inclusive-inclusive bounds in device units
        RECTL rclFrame;             // Inclusive-inclusive Picture Frame of metafile
                                    // in .01 mm units
        S32 dSignature;             // Signature.  Must be ENHMETA_SIGNATURE.
        S32 nVersion;               // Version number
        S32 nBytes;                 // Size of the metafile in bytes
        S32 nRecords;               // Number of records in the metafile
        S16 nHandles;               // Number of handles in the handle table
                                    // Handle index zero is reserved.
        S16 sReserved;              // Reserved.  Must be zero.
        S32 nDescription;           // Number of chars in the unicode description string
                                    // This is 0 if there is no description string
        S32 offDescription;         // Offset to the metafile description record.
                                    // This is 0 if there is no description string
        S32 nPalEntries;            // Number of entries in the metafile palette.
        SIZEL szlDevice;            // Size of the reference device in pels
        SIZEL szlMillimeters;       // Size of the reference device in millimeters
    };
    #define ENHMETA_SIGNATURE       0x464D4520

    struct WmfMetaHeader
    {
        S16 mtType;
        S16 mtHeaderSize;
        S16 mtVersion;
        S32 mtSize;
        S16 mtNoObjects;
        S32 mtMaxRecord;
        S16 mtNoParameters;
    };

    struct WmfPlaceableHeader
    {
        S32 key;
        S16 hmf;
        RECT bbox;
        S16 inch;
        S32 reserved;
        S16 checksum;
    };
    #define APMHEADER_KEY 0x9AC6CDD7L

    WmfPlaceableHeader pheader;
    WmfEnhMetaHeader eheader;
    WmfMetaHeader header;
    S16 checksum;
    int fileAt;

    //----- Read placeable metafile header

    stream >> pheader.key;
    isPlaceable = (pheader.key == (S32)APMHEADER_KEY);
    if (isPlaceable)
    {
        stream >> pheader.hmf;
        stream >> pheader.bbox.left;
        stream >> pheader.bbox.top;
        stream >> pheader.bbox.right;
        stream >> pheader.bbox.bottom;
        stream >> pheader.inch;
        stream >> pheader.reserved;
        stream >> pheader.checksum;
        checksum = 0;
        S16 *ptr = (S16 *)&pheader;

        // XOR in each of the S16s.

        for (unsigned i = 0; i < sizeof(WmfPlaceableHeader)/sizeof(S16); i++)
        {
            checksum ^= ptr[i];
        }
        if (pheader.checksum != checksum)
            isPlaceable = false;
        m_dpi = (unsigned)((double)pheader.inch / m_dpi);
        m_windowOrgX = pheader.bbox.left;
        m_windowOrgY = pheader.bbox.top;
        if (pheader.bbox.right > pheader.bbox.left)
            m_windowFlipX = 1;
        else
            m_windowFlipX = -1;
        if (pheader.bbox.bottom > pheader.bbox.top)
            m_windowFlipY = 1;
        else
            m_windowFlipY = -1;
    }
    else
    {
        stream.device()->seek(startedAt);
        m_dpi = (unsigned)((double)576 / m_dpi);
        m_windowOrgX = 0;
        m_windowOrgY = 0;
        m_windowFlipX = 1;
        m_windowFlipY = 1;
    }

    //----- Read as enhanced metafile header

    fileAt = stream.device()->pos();
    stream >> eheader.iType;
    stream >> eheader.nSize;
    stream >> eheader.rclBounds.left;
    stream >> eheader.rclBounds.top;
    stream >> eheader.rclBounds.right;
    stream >> eheader.rclBounds.bottom;
    stream >> eheader.rclFrame.left;
    stream >> eheader.rclFrame.top;
    stream >> eheader.rclFrame.right;
    stream >> eheader.rclFrame.bottom;
    stream >> eheader.dSignature;
    isEnhanced = (eheader.dSignature == ENHMETA_SIGNATURE);
    if (isEnhanced) // is it really enhanced ?
    {
        stream >> eheader.nVersion;
        stream >> eheader.nBytes;
        stream >> eheader.nRecords;
        stream >> eheader.nHandles;
        stream >> eheader.sReserved;
        stream >> eheader.nDescription;
        stream >> eheader.offDescription;
        stream >> eheader.nPalEntries;
        stream >> eheader.szlDevice.width;
        stream >> eheader.szlDevice.height;
        stream >> eheader.szlMillimeters.width;
        stream >> eheader.szlMillimeters.height;

        kError(s_area) << "WMF Extended Header NOT YET IMPLEMENTED, SORRY." << endl;
        /*
        if (mSingleStep)
        {
            debug("  iType=%d", eheader.iType);
            debug("  nSize=%d", eheader.nSize);
            debug("  rclBounds=(%ld;%ld;%ld;%ld)",
                    eheader.rclBounds.left, eheader.rclBounds.top,
                    eheader.rclBounds.right, eheader.rclBounds.bottom);
            debug("  rclFrame=(%ld;%ld;%ld;%ld)",
                    eheader.rclFrame.left, eheader.rclFrame.top,
                    eheader.rclFrame.right, eheader.rclFrame.bottom);
            debug("  dSignature=%d", eheader.dSignature);
            debug("  nVersion=%d", eheader.nVersion);
            debug("  nBytes=%d", eheader.nBytes);
        }
        debug("NOT YET IMPLEMENTED, SORRY.");
        */
        return false;
    }
    else // no, not enhanced
    {
        //    debug("WMF Header");
        //----- Read as standard metafile header
        stream.device()->seek(fileAt);
        stream >> header.mtType;
        stream >> header.mtHeaderSize;
        stream >> header.mtVersion;
        stream >> header.mtSize;
        stream >> header.mtNoObjects;
        stream >> header.mtMaxRecord;
        stream >> header.mtNoParameters;
        /*
        if (mSingleStep)
        {
            debug("  mtType=%u", header.mtType);
            debug("  mtHeaderSize=%u", header.mtHeaderSize);
            debug("  mtVersion=%u", header.mtVersion);
            debug("  mtSize=%ld", header.mtSize);
        }
        */
    }

    walk((size - (stream.device()->pos() - startedAt)) / 2, stream);
    return true;
}

void KWmf::opArc(
    U32 /*words*/,
    QDataStream &operands)
{
    genericArc("arc", operands);
}

void KWmf::opBrushCreateIndirect(
    U32 /*words*/,
    QDataStream &operands)
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
    {
        Qt::SolidPattern,
        Qt::NoBrush,
        Qt::FDiagPattern,   // hatched
        Qt::Dense4Pattern,  // should be custom bitmap pattern
        Qt::HorPattern,     // should be BS_INDEXED (?)
        Qt::VerPattern,     // should be device-independent bitmap
        Qt::Dense6Pattern,  // should be device-independent packed-bitmap
        Qt::Dense2Pattern,  // should be BS_PATTERN8x8
        Qt::Dense3Pattern   // should be device-independent BS_DIBPATTERN8x8
    };
    Qt::BrushStyle style;
    WinObjBrushHandle *handle = handleCreateBrush();
    S16 arg;
    S32 colour;
    S16 discard;

    operands >> arg >> colour;
    handle->m_colour = getColour(colour);
    if (arg == 2)
    {
        operands >> arg;
        if (arg >= 0 && arg < 6)
        {
            style = hatchedStyleTab[arg];
        }
        else
        {
            kError(s_area) << "createBrushIndirect: invalid hatched brush " << arg << endl;
            style = Qt::SolidPattern;
        }
    }
    else
    if (arg >= 0 && arg < 9)
    {
        style = styleTab[arg];
        operands >> discard;
    }
    else
    {
        kError(s_area) << "createBrushIndirect: invalid brush " << arg << endl;
        style = Qt::SolidPattern;
        operands >> discard;
    }
    handle->m_style = style;
}

void KWmf::opEllipse(
    U32 /*words*/,
    QDataStream &operands)
{
    QPoint topLeft;
    QPoint bottomRight;

    topLeft = normalisePoint(operands);
    bottomRight = normalisePoint(operands);

    QRect ellipse(topLeft, bottomRight);

    gotEllipse(m_dc, "full", ellipse.center(), ellipse.size() / 2, 0, 0);
}

void KWmf::opLineTo(
    U32 /*words*/,
    QDataStream &operands)
{
    QPoint lineTo;

    lineTo = normalisePoint(operands);
    QPolygon points(2);
    points.setPoint(0, m_lineFrom);
    points.setPoint(1, lineTo);
    gotPolyline(m_dc, points);

    // Remember this point for next time.

    m_lineFrom = lineTo;
}

void KWmf::opMoveTo(
    U32 /*words*/,
    QDataStream &operands)
{
    m_lineFrom = normalisePoint(operands);
}

void KWmf::opNoop(
    U32 words,
    QDataStream &operands)
{
    skip(words, operands);
}

//-----------------------------------------------------------------------------
void KWmf::opObjectDelete(
    U32 /*words*/,
    QDataStream &operands)
{
    S16 idx;

    operands >> idx;
    handleDelete(idx);
}

//-----------------------------------------------------------------------------
void KWmf::opObjectSelect(
    U32 /*words*/,
    QDataStream &operands)
{
    S16 idx;

    operands >> idx;
    if (idx >= 0 && idx < s_maxHandles && m_objectHandles[idx])
        m_objectHandles[idx]->apply(*this);
}

//
//
//

void KWmf::opPenCreateIndirect(
    U32 /*words*/,
    QDataStream &operands)
{
    static Qt::PenStyle styleTab[] =
    {
        Qt::SolidLine,
        Qt::DashLine,
        Qt::DotLine,
        Qt::DashDotLine,
        Qt::DashDotDotLine,
        Qt::NoPen,
        Qt::SolidLine,  // PS_INSIDEFRAME
        Qt::SolidLine,  // PS_USERSTYLE
        Qt::SolidLine   // PS_ALTERNATE
    };
    WinObjPenHandle *handle = handleCreatePen();
    S16 arg;
    S32 colour;

    operands >> arg;
    if (arg >= 0 && arg < 8)
    {
        handle->m_style = styleTab[arg];
    }
    else
    {
        kError(s_area) << "createPenIndirect: invalid pen " << arg << endl;
        handle->m_style = Qt::SolidLine;
    }
    operands >> arg;
    handle->m_width = arg;
    operands >> arg >> colour;
    handle->m_colour = getColour(colour);
}

void KWmf::opPie(
    U32 /*words*/,
    QDataStream &operands)
{
    genericArc("pie", operands);
}

void KWmf::opPolygonSetFillMode(
    U32 /*words*/,
    QDataStream &operands)
{
    S16 tmp;

    operands >> tmp;
    m_dc.m_winding = tmp != 0;
}

void KWmf::opPolygon(
    U32 /*words*/,
    QDataStream &operands)
{
    S16 tmp;

    operands >> tmp;
    QPolygon points(tmp);

    for (int i = 0; i < tmp; i++)
    {
        points.setPoint(i, normalisePoint(operands));
    }
    gotPolygon(m_dc, points);
}

void KWmf::opPolyline(
    U32 /*words*/,
    QDataStream &operands)
{
    S16 tmp;

    operands >> tmp;
    QPolygon points(tmp);

    for (int i = 0; i < tmp; i++)
    {
        points.setPoint(i, normalisePoint(operands));
    }
    gotPolyline(m_dc, points);
}

void KWmf::opRectangle(
    U32 /*words*/,
    QDataStream &operands)
{
    QPoint topLeft;
    QSize size;

    topLeft = normalisePoint(operands);
    size = normaliseSize(operands);
    QRect rect(topLeft, size);
    QPolygon points(4);

    points.setPoint(0, topLeft);
    points.setPoint(1, rect.topRight());
    points.setPoint(2, rect.bottomRight());
    points.setPoint(3, rect.bottomLeft());
    gotRectangle(m_dc, points);
}

void KWmf::opRestoreDc(
    U32 /*words*/,
    QDataStream &operands)
{
    S16 pop;
    S16 i;

    operands >> pop;
    for (i = 0; i < pop; i++)
    {
        m_dc = m_savedDcs.pop();
    }
}

void KWmf::opSaveDc(
    U32 /*words*/,
    QDataStream &/*operands*/)
{
    m_savedDcs.push(m_dc);

    // TBD: reinitialise m_dc.
}

void KWmf::opWindowSetOrg(
    U32 /*words*/,
    QDataStream &operands)
{
    S16 top;
    S16 left;

    operands >> top >> left;
    m_windowOrgX = left;
    m_windowOrgY = top;
}

void KWmf::opWindowSetExt(
    U32 /*words*/,
    QDataStream &operands)
{
    S16 height;
    S16 width;

    operands >> height >> width;
    if (width > 0)
        m_windowFlipX = 1;
    else
        m_windowFlipX = -1;
    if (height > 0)
        m_windowFlipY = 1;
    else
        m_windowFlipY = -1;
}

void KWmf::penSet(
    unsigned colour,
    unsigned style,
    unsigned width)
{
    m_dc.m_penColour = colour;
    m_dc.m_penStyle = style;
    m_dc.m_penWidth = width;
}

void KWmf::skip(
    U32 words,
    QDataStream &operands)
{
    if ((int)words < 0)
    {
        kError(s_area) << "skip: " << (int)words << endl;
        return;
    }
    if (words)
    {
        U32 i;
        S16 discard;

        kDebug(s_area) << "skip: " << words << endl;
        for (i = 0; i < words; i++)
        {
            operands >> discard;
        }
    }
}

void KWmf::walk(
    U32 words,
    QDataStream &operands)
{
    // Read bits:
    //
    //     struct WmfMetaRecord
    //     {
    //         S32 rdSize;                 // Record size (in words) of the function
    //         S16 rdFunction;             // Record function number
    //         S16 rdParm[1];              // WORD array of parameters
    //     };
    //
    //     struct WmfEnhMetaRecord
    //     {
    //         S32 iType;                  // Record type EMR_xxx
    //         S32 nSize;                  // Record size in bytes
    //         S32 dParm[1];               // DWORD array of parameters
    //     };

    S32 wordCount;
    S16 opcode;
    U32 length = 0;

    while (length < words)
    {
        operands >> wordCount;
        operands >> opcode;

        // If we get some duff data, protect ourselves.
        if (length + wordCount > words)
        {
            wordCount = words - length;
        }
        length += wordCount;
        if (opcode == 0)
        {
            // This appears to be an EOF marker.
            break;
        }

        // Package the arguments...

        invokeHandler(opcode, wordCount - 3, operands);
    }

    // Eat unexpected data that the caller may expect us to consume.
    skip(words - length, operands);
}

KWmf::DrawContext::DrawContext()
{
    // TBD: initalise with proper values.
    m_brushColour = 0x808080;
    m_brushStyle = 1;
    m_penColour = 0x808080;
    m_penStyle = 1;
    m_penWidth = 1;
}

void KWmf::WinObjBrushHandle::apply(
    KWmf &p)
{
    p.brushSet(m_colour, m_style);
}

void KWmf::WinObjPenHandle::apply(
    KWmf &p)
{
    p.penSet(m_colour, m_style, m_width);
}
