/* This file is part of the KDE project
 *
 * Copyright (C) 2010-11 Inge Wallin <inge@lysator.liu.se>
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
 * Boston, MA 02110-1301, USA
 */


#ifndef WMFENUMS_H
#define WMFENUMS_H


/**
   Namespace for Windows Metafile (WMF) classes
*/
namespace Libwmf
{


// ----------------------------------------------------------------
//                             Enums

/**
   WMF 2.1.1.1 RecordType Enumeration

   The RecordType Enumeration defines the types of records that can be used in WMF metafiles.
*/

enum RecordType {
    META_EOF = 0x0000,
    META_SETBKCOLOR = 0x0201,
    META_SETBKMODE = 0x0102,
    META_SETMAPMODE = 0x0103,
    META_SETROP2 = 0x0104,
    META_SETRELABS = 0x0105,
    META_SETPOLYFILLMODE = 0x0106,
    META_SETSTRETCHBLTMODE = 0x0107,
    META_SETTEXTCHAREXTRA = 0x0108,
    META_SETTEXTCOLOR = 0x0209,
    META_SETTEXTJUSTIFICATION = 0x020A,
    META_SETWINDOWORG = 0x020B,
    META_SETWINDOWEXT = 0x020C,
    META_SETVIEWPORTORG = 0x020D,
    META_SETVIEWPORTEXT = 0x020E,
    META_OFFSETWINDOWORG = 0x020F,
    META_SCALEWINDOWEXT = 0x0410,
    META_OFFSETVIEWPORTORG = 0x0211,
    META_SCALEVIEWPORTEXT = 0x0412,
    META_LINETO = 0x0213,
    META_MOVETO = 0x0214,
    META_EXCLUDECLIPRECT = 0x0415,
    META_INTERSECTCLIPRECT = 0x0416,
    META_ARC = 0x0817,
    META_ELLIPSE = 0x0418,
    META_FLOODFILL = 0x0419,
    META_PIE = 0x081A,
    META_RECTANGLE = 0x041B,
    META_ROUNDRECT = 0x061C,
    META_PATBLT = 0x061D,
    META_SAVEDC = 0x001E,
    META_SETPIXEL = 0x041F,
    META_OFFSETCLIPRGN = 0x0220,
    META_TEXTOUT = 0x0521,
    META_BITBLT = 0x0922,
    META_STRETCHBLT = 0x0B23,
    META_POLYGON = 0x0324,
    META_POLYLINE = 0x0325,
    META_ESCAPE = 0x0626,
    META_RESTOREDC = 0x0127,
    META_FILLREGION = 0x0228,
    META_FRAMEREGION = 0x0429,
    META_INVERTREGION = 0x012A,
    META_PAINTREGION = 0x012B,
    META_SELECTCLIPREGION = 0x012C,
    META_SELECTOBJECT = 0x012D,
    META_SETTEXTALIGN = 0x012E,
    // No 2F
    META_CHORD = 0x0830,
    META_SETMAPPERFLAGS = 0x0231,
    META_EXTTEXTOUT = 0x0a32,
    META_SETDIBTODEV = 0x0d33,
    META_SELECTPALETTE = 0x0234,
    META_REALIZEPALETTE = 0x0035,
    META_ANIMATEPALETTE = 0x0436,
    META_SETPALENTRIES = 0x0037,
    META_POLYPOLYGON = 0x0538,
    META_RESIZEPALETTE = 0x0139,
    META_DIBBITBLT = 0x0940,
    META_DIBSTRETCHBLT = 0x0b41,
    META_DIBCREATEPATTERNBRUSH = 0x0142,
    META_STRETCHDIB = 0x0f43,
    // No 44 - 47
    META_EXTFLOODFILL = 0x0548,
    META_SETLAYOUT = 0x0149,
    // Some strange things in the 4a-5f range, that is not documented,
    // but the old code has a few unimplemented functions here.

    // BIG GAP HERE
    META_DELETEOBJECT = 0x01f0,
    // No f1-f6
    META_CREATEPALETTE = 0x00f7,
    META_CREATEBRUSH = 0x00f8,  // Can't find this in [MS-WMF].pdf, but is unimplemented here.
    META_CREATEPATTERNBRUSH = 0x01F9,
    META_CREATEPENINDIRECT = 0x02FA,
    META_CREATEFONTINDIRECT = 0x02FB,
    META_CREATEBRUSHINDIRECT = 0x02FC,
    META_CREATEREGION = 0x06FF
};

// MS-WMF 2.1.1.2 Binary RasterOperation Enumeration

enum WmfBinaryRasterOperation {
    R2_BLACK       = 0x0001,
    R2_NOTMERGEPEN = 0x0002,
    R2_MASKNOTPEN  = 0x0003,
    R2_NOTCOPYPEN  = 0x0004,
    R2_MASKPENNOT  = 0x0005,
    R2_NOT         = 0x0006,
    R2_XORPEN      = 0x0007,
    R2_NOTMASKPEN  = 0x0008,
    R2_MASKPEN     = 0x0009,
    R2_NOTXORPEN   = 0x000A,
    R2_NOP         = 0x000B,
    R2_MERGENOTPEN = 0x000C,
    R2_COPYPEN     = 0x000D,
    R2_MERGEPENNOT = 0x000E,
    R2_MERGEPEN    = 0x000F,
    R2_WHITE       = 0x0010
};


/**
   WMF 2.1.1.3 BitCount Enumeration

   The BitCount Enumeration specifies the number of bits that define
   each pixel and the maximum number of colors in a device-independent
   bitmap (DIB).
*/
enum WmfBitCount {
    BI_BITCOUNT_0 = 0x0000,
    BI_BITCOUNT_1 = 0x0001,
    BI_BITCOUNT_2 = 0x0004,
    BI_BITCOUNT_3 = 0x0008,
    BI_BITCOUNT_4 = 0x0010,
    BI_BITCOUNT_5 = 0x0018,
    BI_BITCOUNT_6 = 0x0020
};


/**
   MS-WMF 2.1.1.4 BrushStyle Enumeration

   The BrushStyle Enumeration specifies the different possible brush
   types that can be used in graphics operations. For more
   information, see the specification of the Brush Object (section 2.2.1.1).
*/

enum WmfBrushStyle {
    BS_SOLID         = 0x0000,
    BS_NULL          = 0x0001,
    BS_HATCHED       = 0x0002,
    BS_PATTERN       = 0x0003,
    BS_INDEXED       = 0x0004,
    BS_DIBPATTERN    = 0x0005,
    BS_DIBPATTERNPT  = 0x0006,
    BS_PATTERN8X8    = 0x0007,
    BS_DIBPATTERN8X8 = 0x0008,
    BS_MONOPATTERN   = 0x0009
};


/**
   MS-WMF 2.1.1.5 CharacterSet Enumeration

   The CharacterSet Enumeration defines the possible sets of character
   glyphs that are defined in fonts for graphics output.
*/

enum WmfCharacterSet {
    ANSI_CHARSET        = 0x00000000,
    DEFAULT_CHARSET     = 0x00000001,
    SYMBOL_CHARSET      = 0x00000002,
    MAC_CHARSET         = 0x0000004D,
    SHIFTJIS_CHARSET    = 0x00000080,
    HANGUL_CHARSET      = 0x00000081,
    JOHAB_CHARSET       = 0x00000082,
    GB2312_CHARSET      = 0x00000086,
    CHINESEBIG5_CHARSET = 0x00000088,
    GREEK_CHARSET       = 0x000000A1,
    TURKISH_CHARSET     = 0x000000A2,
    VIETNAMESE_CHARSET  = 0x000000A3,
    HEBREW_CHARSET      = 0x000000B1,
    ARABIC_CHARSET      = 0x000000B2,
    BALTIC_CHARSET      = 0x000000BA,
    RUSSIAN_CHARSET     = 0x000000CC,
    THAI_CHARSET        = 0x000000DE,
    EASTEUROPE_CHARSET  = 0x000000EE,
    OEM_CHARSET         = 0x000000FF
};


/**
   MS-WMF 2.1.1.6 ColorUsage Enumeration

   The ColorUsage Enumeration specifies whether a color table exists
   in a device -independent bitmap (DIB) and how to interpret its values.
*/

enum WmfColorUsage {
    DIB_RGB_COLORS  = 0x0000,
    DIB_PAL_COLORS  = 0x0001,
    DIB_PAL_INDICES = 0x0002
};


/**
   MS-WMF 2.1.1.7 Compression Enumeration

   The Compression Enumeration specifies the type of compression for a
   bitmap image.
*/

enum WmfCompression {
    BI_RGB       = 0x0000,
    BI_RLE8      = 0x0001,
    BI_RLE4      = 0x0002,
    BI_BITFIELDS = 0x0003,
    BI_JPEG      = 0x0004,
    BI_PNG       = 0x0005,
    BI_CMYK      = 0x000B,
    BI_CMYKRLE8  = 0x000C,
    BI_CMYKRLE4  = 0x000D
};


/**
   MS-WMF 2.1.1.8 FamilyFont Enumeration

   The FamilyFont enumeration specifies the font family. Font families
   describe the look of a font in a general way. They are intended for
   specifying fonts when the exact typeface desired is not available.
*/

typedef enum {
    FF_DONTCARE   = 0x00,
    FF_ROMAN      = 0x01,
    FF_SWISS      = 0x02,
    FF_MODERN     = 0x03,
    FF_SCRIPT     = 0x04,
    FF_DECORATIVE = 0x05
} WmfFamilyFont;


/**
   MS-WMF 2.1.1.9 FloodFill Enumeration

   The FloodFill Enumeration specifies the type of fill operation to
   be performed.
*/

enum WmfFloodFill {
    FLOODFILLBORDER  = 0x0000,
    FLOODFILLSURFACE = 0x0001
};


/**
   MS-WMF 2.1.1.10 FontQuality Enumeration

   The FontQuality Enumeration specifies how closely the attributes of
   the logical font should match those of the physical font when
   rendering text.
*/

enum WmfFontQuality {
    DEFAULT_QUALITY        = 0x00,
    DRAFT_QUALITY          = 0x01,
    PROOF_QUALITY          = 0x02,
    NONANTIALIASED_QUALITY = 0x03,
    ANTIALIASED_QUALITY    = 0x04,
    CLEARTYPE_QUALITY      = 0x05
};


/*
  MS-WMF 2.1.1.11 GamutMappingIntent Enumeration

  The GamutMappingIntent Enumeration specifies the relationship
  between logical and physical colors.
*/

enum WmfGamutMappingIntent {
    LCS_GM_ABS_COLORIMETRIC = 0x00000008,
    LCS_GM_BUSINESS         = 0x00000001,
    LCS_GM_GRAPHICS         = 0x00000002,
    LCS_GM_IMAGES           = 0x00000004
};


/**
   MS-WMF 2.1.1.12 HatchStyle Enumeration

   The HatchStyle Enumeration specifies the hatch pattern.
*/

enum WmfHatchStyle {
    HS_HORIZONTAL = 0x0000,
    HS_VERTICAL   = 0x0001,
    HS_FDIAGONAL  = 0x0002,
    HS_BDIAGONAL  = 0x0003,
    HS_CROSS      = 0x0004,
    HS_DIAGCROSS  = 0x0005
};


/**
   MS-WMF 2.1.1.13 Layout Enumeration

   The Layout Enumeration defines options for controlling the
   direction in which text and graphics are drawn.
*/

enum WmfLayout {
    LAYOUT_LTR = 0x0000,
    LAYOUT_RTL = 0x0001,
    LAYOUT_BTT = 0x0002,
    LAYOUT_VBH = 0x0004,
    LAYOUT_BITMAPORIENTATIONPRESERVED = 0x0008
};


/**
   MS-WMF 2.1.1.14 LogicalColorSpace Enumeration

   The LogicalColorSpace Enumeration specifies the type of color space.
*/

enum WmfLogicalColorSpace {
    LCS_CALIBRATED_RGB      = 0x00000000,
    LCS_sRGB                = 0x73524742,
    LCS_WINDOWS_COLOR_SPACE = 0x57696E20
};


/**
   MS-WMF 2.1.1.15 LogicalColorSpaceV5 Enumeration

   The LogicalColorSpaceV5 Enumeration is used to specify where to
   find color profile information for a DeviceIndependentBitmap (DIB)
   Object (section 2.2.2.9) that has a header of type BitmapV5Header
   Object (section 2.2.2.5).
*/

enum WmfLogicalColorSpaceV5 {
    LCS_PROFILE_LINKED   = 0x4C494E4B,
    LCS_PROFILE_EMBEDDED = 0x4D424544
};


/**
   MS-WMF 2.1.1.16 MapMode Enumeration

   The MapMode Enumeration defines how logical units are mapped to
   physical units; that is, assuming that the origins in both the
   logical and physical coordinate systems are at the same point on
   the drawing surface, what is the physical coordinate (x',y') that
   corresponds to logical coordinate (x,y).
*/

enum WmfMapMode {
    MM_TEXT        = 0x0001,
    MM_LOMETRIC    = 0x0002,
    MM_HIMETRIC    = 0x0003,
    MM_LOENGLISH   = 0x0004,
    MM_HIENGLISH   = 0x0005,
    MM_TWIPS       = 0x0006,
    MM_ISOTROPIC   = 0x0007,
    MM_ANISOTROPIC = 0x0008
};


/**
   MS-WMF 2.1.1.18 Metafile Type Enumeration

   The MetafileType Enumeration specifies where the metafile is stored.
*/

enum WmfMetafileType {
    MEMORYMETAFILE = 0x0001,
    DISKMETAFILE   = 0x0002
};


/**
   MS-WMF 2.1.1.19 MetafileVersion Enumeration
  
   The MetafileVersion Enumeration defines values that specify support
   for device-independent bitmaps (DIBs) in metafiles.
*/

enum WmfMetafileVersion {
    METAVERSION100 = 0x0100,
    METAVERSION300 = 0x0300
};


/**
   MS-WMF 2.1.1.20 MixMode Enumeration
  
   The MixMode Enumeration specifies the background mix mode for text,
   hatched brushes, and other nonsolid pen styles.
*/

enum WmfMixMode {
    TRANSPARENT = 0x0001,
    OPAQUE      = 0x0002
};


/**
   MS-WMF 2.1.1.21 OutPrecision Enumeration
  
   The OutPrecision enumeration defines values for output precision,
   which is the requirement for the font mapper to match specific font
   parameters, including height, width, character orientation,
   escapement, pitch, and font type.
*/

enum WmfOutPrecision {
    OUT_DEFAULT_PRECIS        = 0x00000000,
    OUT_STRING_PRECIS         = 0x00000001,
    OUT_STROKE_PRECIS         = 0x00000003,
    OUT_TT_PRECIS             = 0x00000004,
    OUT_DEVICE_PRECIS         = 0x00000005,
    OUT_RASTER_PRECIS         = 0x00000006,
    OUT_TT_ONLY_PRECIS        = 0x00000007,
    OUT_OUTLINE_PRECIS        = 0x00000008,
    OUT_SCREEN_OUTLINE_PRECIS = 0x00000009,
    OUT_PS_ONLY_PRECIS        = 0x0000000A
};


/**
   MS-WMF 2.1.1.22 PaletteEntryFlag Enumeration
  
   The PaletteEntryFlag Enumeration specifies how the palette entry should be used.
*/

enum WmfPaletteEntryFlag {
    PC_RESERVED   = 0x01,
    PC_EXPLICIT   = 0x02,
    PC_NOCOLLAPSE = 0x04
};


/**
   MS-WMF 2.1.1.23 Pe nStyle Enumeration
  
   The 16-bit PenStyle Enumeration is used to specify different types
   of pens that can be used in graphics operations.
  
   Various styles can be combined by using a logical OR statement, one
   from each subsection of Style, EndCap, Join, and Type (Cosmetic).
*/

enum WmfPenStyle {
    PS_COSMETIC      = 0x0000,
    PS_ENDCAP_ROUND  = 0x0000,
    PS_JOIN_ROUND    = 0x0000,
    PS_SOLID         = 0x0000,
    PS_DASH          = 0x0001,
    PS_DOT           = 0x0002,
    PS_DASHDOT       = 0x0003,
    PS_DASHDOTDOT    = 0x0004,
    PS_NULL          = 0x0005,
    PS_INSIDEFRAME   = 0x0006,
    PS_USERSTYLE     = 0x0007,
    PS_ALTERNATE     = 0x0008,
    PS_ENDCAP_SQUARE = 0x0100,
    PS_ENDCAP_FLAT   = 0x0200,
    PS_JOIN_BEVEL    = 0x1000,
    PS_JOIN_MITER    = 0x2000
};


/**
   MS-WMF 2.1.1.24 PitchFont Enumeration
  
   The PitchFont enumeration defines values that are used for
   specifying characteristics of a font. The values are used to
   indicate whether the characters in a font have a fixed or variable
   width, or pitch.
*/

enum WmfPitchFont {
    DEFAULT_PITCH  = 0,
    FIXED_PITCH    = 1,
    VARIABLE_PITCH = 2
};


/*
   MS-WMF 2.1.1.25 PolyFillMode Enumeration
  
   The PolyFillMode Enumeration specifies the method used for filling
   a polygon.
*/

enum WmfPolyFillMode {
    ALTERNATE = 0x0001,
    WINDING   = 0x0002
};


/**
   MS-WMF 2.1.1.29 StretchMode Enumeration
  
   The StretchMode Enumeration specifies the bitmap stretching mode,
   which defines how the system combines rows or columns of a bitmap
   with existing pixels.
*/

enum WmfStretchMode {
    BLACKONWHITE = 0x0001,
    WHITEONBLACK = 0x0002,
    COLORONCOLOR = 0x0003,
    HALFTONE     = 0x0004
};


/**
   MS-WMF 2.1.1.30 Ternary RasterOperation Enumeration
  
   The TernaryRasterOperation Enumeration specifies ternary raster
   operation codes, which define how to combine the bits in a source
   bitmap with the bits in a destination bitmap.
*/

enum WmfTernaryRasterOperation {
    BLACKNESS = 0x00,
    DPSOON = 0x01,
    DPSONA = 0x02,
    PSON = 0x03,
    SDPONA = 0x04,
    DPON = 0x05,
    PDSXNON = 0x06,
    PDSAON = 0x07,
    SDPNAA = 0x08,
    PDSXON = 0x09,
    DPNA = 0x0A,
    PSDNAON = 0x0B,
    SPNA = 0x0C,
    PDSNAON = 0x0D,
    PDSONON = 0x0E,
    PN = 0x0F,
    PDSONA = 0x10,
    NOTSRCERASE = 0x11,
    SDPXNON = 0x12,
    SDPAON = 0x13,
    DPSXNON = 0x14,
    DPSAON = 0x15,
    PSDPSANAXX = 0x16,
    SSPXDSXAXN = 0x17,
    SPXPDXA = 0x18,
    SDPSANAXN = 0x19,
    PDSPAOX = 0x1A,
    SDPSXAXN = 0x1B,
    PSDPAOX = 0x1C,
    DSPDXAXN = 0x1D,
    PDSOX = 0x1E,
    PDSOAN = 0x1F,
    DPSNAA = 0x20,
    SDPXON = 0x21,
    DSNA = 0x22,
    SPDNAON = 0x23,
    SPXDSXA = 0x24,
    PDSPANAXN = 0x25,
    SDPSAOX = 0x26,
    SDPSXNOX = 0x27,
    DPSXA = 0x28,
    PSDPSAOXXN = 0x29,
    DPSANA = 0x2A,
    SSPXPDXAXN = 0x2B,
    SPDSOAX = 0x2C,
    PSDNOX = 0x2D,
    PSDPXOX = 0x2E,
    PSDNOAN = 0x2F,
    PSNA = 0x30,
    SDPNAON = 0x31,
    SDPSOOX = 0x32,
    NOTSRCCOPY = 0x33,
    SPDSAOX = 0x34,
    SPDSXNOX = 0x35,
    SDPOX = 0x36,
    SDPOAN = 0x37,
    PSDPOAX = 0x38,
    SPDNOX = 0x39,
    SPDSXOX = 0x3A,
    SPDNOAN = 0x3B,
    PSX = 0x3C,
    SPDSONOX = 0x3D,
    SPDSNAOX = 0x3E,
    PSAN = 0x3F,
    PSDNAA = 0x40,
    DPSXON = 0x41,
    SDXPDXA = 0x42,
    SPDSANAXN = 0x43,
    SRCERASE = 0x44,
    DPSNAON = 0x45,
    DSPDAOX = 0x46,
    PSDPXAXN = 0x47,
    SDPXA = 0x48,
    PDSPDAOXXN = 0x49,
    DPSDOAX = 0x4A,
    PDSNOX = 0x4B,
    SDPANA = 0x4C,
    SSPXDSXOXN = 0x4D,
    PDSPXOX = 0x4E,
    PDSNOAN = 0x4F,
    PDNA = 0x50,
    DSPNAON = 0x51,
    DPSDAOX = 0x52,
    SPDSXAXN = 0x53,
    DPSONON = 0x54,
    DSTINVERT = 0x55,
    DPSOX = 0x56,
    DPSOAN = 0x57,
    PDSPOAX = 0x58,
    DPSNOX = 0x59,
    PATINVERT = 0x5A,
    DPSDONOX = 0x5B,
    DPSDXOX = 0x5C,
    DPSNOAN = 0x5D,
    DPSDNAOX = 0x5E,
    DPAN = 0x5F,
    PDSXA = 0x60,
    DSPDSAOXXN = 0x61,
    DSPDOAX = 0x62,
    SDPNOX = 0x63,
    SDPSOAX = 0x64,
    DSPNOX = 0x65,
    SRCINVERT = 0x66,
    SDPSONOX = 0x67,
    DSPDSONOXXN = 0x68,
    PDSXXN = 0x69,
    DPSAX = 0x6A,
    PSDPSOAXXN = 0x6B,
    SDPAX = 0x6C,
    PDSPDOAXXN = 0x6D,
    SDPSNOAX = 0x6E,
    PDXNAN = 0x6F,
    PDSANA = 0x70,
    SSDXPDXAXN = 0x71,
    SDPSXOX = 0x72,
    SDPNOAN = 0x73,
    DSPDXOX = 0x74,
    DSPNOAN = 0x75,
    SDPSNAOX = 0x76,
    DSAN = 0x77,
    PDSAX = 0x78,
    DSPDSOAXXN = 0x79,
    DPSDNOAX = 0x7A,
    SDPXNAN = 0x7B,
    SPDSNOAX = 0x7C,
    DPSXNAN = 0x7D,
    SPXDSXO = 0x7E,
    DPSAAN = 0x7F,
    DPSAA = 0x80,
    SPXDSXON = 0x81,
    DPSXNA = 0x82,
    SPDSNOAXN = 0x83,
    SDPXNA = 0x84,
    PDSPNOAXN = 0x85,
    DSPDSOAXX = 0x86,
    PDSAXN = 0x87,
    SRCAND = 0x88,
    SDPSNAOXN = 0x89,
    DSPNOA = 0x8A,
    DSPDXOXN = 0x8B,
    SDPNOA = 0x8C,
    SDPSXOXN = 0x8D,
    SSDXPDXAX = 0x8E,
    PDSANAN = 0x8F,
    PDSXNA = 0x90,
    SDPSNOAXN = 0x91,
    DPSDPOAXX = 0x92,
    SPDAXN = 0x93,
    PSDPSOAXX = 0x94,
    DPSAXN = 0x95,
    DPSXX = 0x96,
    PSDPSONOXX = 0x97,
    SDPSONOXN = 0x98,
    DSXN = 0x99,
    DPSNAX = 0x9A,
    SDPSOAXN = 0x9B,
    SPDNAX = 0x9C,
    DSPDOAXN = 0x9D,
    DSPDSAOXX = 0x9E,
    PDSXAN = 0x9F,
    DPA = 0xA0,
    PDSPNAOXN = 0xA1,
    DPSNOA = 0xA2,
    DPSDXOXN = 0xA3,
    PDSPONOXN = 0xA4,
    PDXN = 0xA5,
    DSPNAX = 0xA6,
    PDSPOAXN = 0xA7,
    DPSOA = 0xA8,
    DPSOXN = 0xA9,
    D = 0xAA,
    DPSONO = 0xAB,
    SPDSXAX = 0xAC,
    DPSDAOXN = 0xAD,
    DSPNAO = 0xAE,
    DPNO = 0xAF,
    PDSNOA = 0xB0,
    PDSPXOXN = 0xB1,
    SSPXDSXOX = 0xB2,
    SDPANAN = 0xB3,
    PSDNAX = 0xB4,
    DPSDOAXN = 0xB5,
    DPSDPAOXX = 0xB6,
    SDPXAN = 0xB7,
    PSDPXAX = 0xB8,
    DSPDAOXN = 0xB9,
    DPSNAO = 0xBA,
    MERGEPAINT = 0xBB,
    SPDSANAX = 0xBC,
    SDXPDXAN = 0xBD,
    DPSXO = 0xBE,
    DPSANO = 0xBF,
    MERGECOPY = 0xC0,
    SPDSNAOXN = 0xC1,
    SPDSONOXN = 0xC2,
    PSXN = 0xC3,
    SPDNOA = 0xC4,
    SPDSXOXN = 0xC5,
    SDPNAX = 0xC6,
    PSDPOAXN = 0xC7,
    SDPOA = 0xC8,
    SPDOXN = 0xC9,
    DPSDXAX = 0xCA,
    SPDSAOXN = 0xCB,
    SRCCOPY = 0xCC,
    SDPONO = 0xCD,
    SDPNAO = 0xCE,
    SPNO = 0xCF,
    PSDNOA = 0xD0,
    PSDPXOXN = 0xD1,

    PDSNAX = 0xD2,
    SPDSOAXN = 0xD3,
    SSPXPDXAX = 0xD4,
    DPSANAN = 0xD5,
    PSDPSAOXX = 0xD6,
    DPSXAN = 0xD7,
    PDSPXAX = 0xD8,
    SDPSAOXN = 0xD9,
    DPSDANAX = 0xDA,
    SPXDSXAN = 0xDB,
    SPDNAO = 0xDC,
    SDNO = 0xDD,
    SDPXO = 0xDE,
    SDPANO = 0xDF,
    PDSOA = 0xE0,
    PDSOXN = 0xE1,
    DSPDXAX = 0xE2,
    PSDPAOXN = 0xE3,
    SDPSXAX = 0xE4,
    PDSPAOXN = 0xE5,
    SDPSANAX = 0xE6,
    SPXPDXAN = 0xE7,
    SSPXDSXAX = 0xE8,
    DSPDSANAXXN = 0xE9,
    DPSAO = 0xEA,
    DPSXNO = 0xEB,
    SDPAO = 0xEC,
    SDPXNO = 0xED,
    SRCPAINT = 0xEE,
    SDPNOO = 0xEF,
    PATCOPY = 0xF0,
    PDSONO = 0xF1,
    PDSNAO = 0xF2,
    PSNO = 0xF3,
    PSDNAO = 0xF4,
    PDNO = 0xF5,
    PDSXO = 0xF6,
    PDSANO = 0xF7,
    PDSAO = 0xF8,
    PDSXNO = 0xF9,
    DPO = 0xFA,
    PATPAINT = 0xFB,
    PSO = 0xFC,
    PSDNOO = 0xFD,
    DPSOO = 0xFE,
    WHITENESS = 0xFF
};


// ----------------------------------------------------------------
//                             Flags


/**
   MS-WMF 2.1.2.1 ClipPrecision Flags
  
   ClipPrecision Flags specify clipping precision, which defines how
   to clip characters that are partially outside a clipping
   region. These flags can be combined to specify multiple options.
*/

#define CLIP_DEFAULT_PRECIS   0x00000000
#define CLIP_CHARACTER_PRECIS 0x00000001
#define CLIP_STROKE_PRECIS    0x00000002
#define CLIP_LH_ANGLES        0x00000010
#define CLIP_TT_ALWAYS        0x00000020
#define CLIP_DFA_DISABLE      0x00000040
#define CLIP_EMBEDDED         0x00000080


/**
   MS-WMF 2.1.2.2 ExtTextOutOptions Flags
  
   ExtTextOutOptions Flags specify various characteristics of the
   output of text. These flags can be combined to specify multiple options.
*/

#define ETO_OPAQUE        0x0002
#define ETO_CLIPPED       0x0004
#define ETO_GLYPH_INDEX   0x0010
#define ETO_RTLREADING    0x0080
#define ETO_NUMERICSLOCAL 0x0400
#define ETO_NUMERICSLATIN 0x0800
#define ETO_PDY           0x2000


/**
   MS-WMF 2.1.2.3 TextAlignmentMode Flags
  
   TextAlignmentMode Flags specify the relationship between a
   reference point and a bounding rectangle, for text alignment. These
   flags can be combined to specify multiple options, with the
   restriction that only one flag can be chosen that alters the
   drawing position in the playback device context.
  
   Horizontal text alignment is performed when the font has a
   horizontal default baseline.
*/

#define TA_NOUPDATECP 0x0000  /// Do not update Current Point (default)
#define TA_LEFT       0x0000  /// The reference point is on the left edge of the bounding rectangle
#define TA_TOP        0x0000  /// The reference point is on the top edge of the bounding rectangle 
#define TA_UPDATECP   0x0001  /// Use Current Point. The Current Point must be updated
#define TA_RIGHT      0x0002  /// The reference point is on the right edge of the bounding rectangle
#define TA_CENTER     0x0006  /// The reference point is at the center of the bounding rectangle
#define TA_BOTTOM     0x0008  /// The reference point is on the bottom edge of the bounding rectangle 
#define TA_BASELINE   0x0018  /// The reference point is on the baseline
#define TA_RTLREADING 0x0100  /// The text is laid out in Right-to-Left direction. 

// Some useful masks, not part of the specification:
#define TA_HORZMASK 0x0006
#define TA_VERTMASK 0x0018



// MS-WMF 2.1.2.4 VerticalTextAlignmentMode Flags
//
// VerticalTextAlignmentMode Flags specify the relationship between a
// reference point and a bounding rectangle, for text alignment. These
// flags can be combined to specify multiple options, with the
// restriction that only one flag can be chosen that alters the
// drawing position in the playback device context.
//
// Vertical text alignment is performed when the font has a vertical
// default baseline, such as Kanji.

#define VTA_TOP      0x0000
#define VTA_RIGHT    0x0000
#define VTA_BOTTOM   0x0002
#define VTA_CENTER   0x0006
#define VTA_LEFT     0x0008
#define VTA_BASELINE 0x0018


}

#endif  // KOWMFENUMS_H
