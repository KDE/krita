/* This file is part of the Calligra project
 * Copyright (C) 2002/2003 thierry lorthiois
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#ifndef _WMFSTRUCT_H_
#define _WMFSTRUCT_H_

#include <QtGlobal>
#include <qnamespace.h>
#include <QPainter>

/**
   Namespace for Windows Metafile (WMF) classes
*/
namespace Libwmf
{


#define APMHEADER_KEY 0x9AC6CDD7
#define ENHMETA_SIGNATURE       0x464D4520

struct WmfMetaHeader {
    quint16  fileType;      // Type of metafile (0=memory, 1=disk)
    quint16  headerSize;    // always 9
    quint16  version;
    quint32  fileSize;      // Total size of the metafile in WORDs
    quint16  numOfObjects;    // Maximum Number of objects in the stack
    quint32  maxRecordSize;   // The size of largest record in WORDs
    quint16  numOfParameters; // not used (always 0)
};


struct WmfPlaceableHeader {
    quint32  key;        // Magic number (always 9AC6CDD7h)
    quint16  handle;     // Metafile HANDLE number (always 0)
    qint16   left;       // Left coordinate in metafile units
    qint16   top;
    qint16   right;
    qint16   bottom;
    quint16  inch;       // Number of metafile units per inch
    quint32  reserved;
    quint16  checksum;   // Checksum value for previous 10 WORDs
};


struct WmfEnhMetaHeader {
    quint32  recordType;       // Record type (is always 00000001h)
    quint32  recordSize;       // Record size in bytes.  This may be greater
    // than the sizeof( ENHMETAHEADER ).
    qint32   boundsLeft;       // Inclusive-inclusive bounds in device units
    qint32   boundsTop;
    qint32   boundsRight;
    qint32   boundsBottom;
    qint32   frameLeft;        // Inclusive-inclusive Picture Frame
    qint32   frameTop;
    qint32   frameRight;
    qint32   frameBottom;
    quint32  signature;        // Signature.  Must be ENHMETA_SIGNATURE.
    quint32  version;          // Version number
    quint32  size;             // Size of the metafile in bytes
    quint32  numOfRecords;     // Number of records in the metafile
    quint16  numHandles;       // Number of handles in the handle table
    // Handle index zero is reserved.
    quint16  reserved;         // always 0
    quint32  sizeOfDescription;   // Number of chars in the unicode description string
    // This is 0 if there is no description string
    quint32  offsetOfDescription; // Offset to the metafile description record.
    // This is 0 if there is no description string
    quint32  numPaletteEntries;   // Number of color palette entries
    qint32   widthDevicePixels;   // Size of the reference device in pixels
    qint32   heightDevicePixels;
    qint32   widthDeviceMM;       // Size of the reference device in millimeters
    qint32   heightDeviceMM;
};


struct WmfMetaRecord {
    quint32  size;         // Total size of the record in WORDs
    quint16  function;     // Record function number
    quint16  param[ 1 ];   // quint16 array of parameters
};


struct WmfEnhMetaRecord {
    quint32  function;     // Record function number
    quint32  size;         // Record size in bytes
    quint32  param[ 1 ];   // quint32 array of parameters
};

// Static data
static const struct OpTab {
    quint32  winRasterOp;
    QPainter::CompositionMode  qtRasterOp;
} koWmfOpTab32[] = {
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

static const QPainter::CompositionMode koWmfOpTab16[] = {
    // ### untested (conversion from Qt::RasterOp)
    QPainter::CompositionMode_Source, // CopyROP
    QPainter::CompositionMode_Clear, // ClearROP
    QPainter::CompositionMode_SourceOut, // NandROP
    QPainter::CompositionMode_SourceOut, // NotAndROP
    QPainter::CompositionMode_DestinationOut, // NotCopyROP
    QPainter::CompositionMode_DestinationOut, // AndNotROP
    QPainter::CompositionMode_DestinationOut, // NotROP
    QPainter::CompositionMode_Xor, // XorROP
    QPainter::CompositionMode_Source, // NorROP
    QPainter::CompositionMode_SourceIn, // AndROP
    QPainter::CompositionMode_SourceIn, //NotXorROP
    QPainter::CompositionMode_Destination, // NopROP
    QPainter::CompositionMode_Destination, // NotOrROP
    QPainter::CompositionMode_Source, // CopyROP
    QPainter::CompositionMode_Source, // OrNotROP
    QPainter::CompositionMode_SourceOver, // OrROP
    QPainter::CompositionMode_Source // SetROP
};

static const Qt::BrushStyle koWmfHatchedStyleBrush[] = {
    Qt::HorPattern,
    Qt::VerPattern,
    Qt::FDiagPattern,
    Qt::BDiagPattern,
    Qt::CrossPattern,
    Qt::DiagCrossPattern
};

static const Qt::BrushStyle koWmfStyleBrush[] = {
    Qt::SolidPattern,
    Qt::NoBrush,
    Qt::FDiagPattern,   /* hatched */
    Qt::Dense4Pattern,  /* should be custom bitmap pattern */
    Qt::HorPattern,     /* should be BS_INDEXED (?) */
    Qt::VerPattern,     /* should be device-independent bitmap */
    Qt::Dense6Pattern,  /* should be device-independent packed-bitmap */
    Qt::Dense2Pattern,  /* should be BS_PATTERN8x8 */
    Qt::Dense3Pattern   /* should be device-independent BS_DIBPATTERN8x8 */
};

static const Qt::PenStyle koWmfStylePen[] = {
    Qt::SolidLine,
    Qt::DashLine,
    Qt::DotLine,
    Qt::DashDotLine,
    Qt::DashDotDotLine,
    Qt::NoPen,
    Qt::SolidLine
};

static const Qt::PenCapStyle koWmfCapStylePen[] = {
    Qt::RoundCap,
    Qt::SquareCap,
    Qt::FlatCap
};

static const Qt::PenJoinStyle koWmfJoinStylePen[] = {
    Qt::RoundJoin,
    Qt::BevelJoin,
    Qt::MiterJoin
};


}

#endif
