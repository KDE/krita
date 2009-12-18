/*
  Copyright 2008 Brad Hards <bradh@frogmouth.net>
  Copyright 2009 Inge Wallin <inge@lysator.liu.se>

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

#ifndef ENHENUMS_H
#define ENHENUMS_H

#include <QDataStream>
#include <QRect> // also provides QSize
#include <QString>

/**
   \file

   Enumerations used in various parts of EMF files
*/

/**
   Namespace for Enhanced Metafile (EMF) classes
*/
namespace Libemf
{

    /**
       World Transform modification modes

    See [MS-EMF] Section 2.1.24
    */
    enum ModifyWorldTransformMode {
        MWT_IDENTITY            = 0x01,
        MWT_LEFTMULTIPLY        = 0x02,
        MWT_RIGHTMULTIPLY       = 0x03,
        MWT_SET                 = 0x04
    };

    /**
       Pen Styles

       See [MS-EMF] Section 2.1.25
    */
    enum PenStyle {
	PS_COSMETIC      = 0x00000000,
	PS_ENDCAP_ROUND  = 0x00000000,
	PS_JOIN_ROUND    = 0x00000000,
	PS_SOLID         = 0x00000000,
	PS_DASH          = 0x00000001,
	PS_DOT           = 0x00000002,
	PS_DASHDOT       = 0x00000003,
	PS_DASHDOTDOT    = 0x00000004,
	PS_NULL          = 0x00000005,
	PS_INSIDEFRAME   = 0x00000006,
	PS_USERSTYLE     = 0x00000007,
	PS_ALTERNATE     = 0x00000008,
	PS_ENDCAP_SQUARE = 0x00000100,
	PS_ENDCAP_FLAT   = 0x00000200,
	PS_JOIN_BEVEL    = 0x00001000,
	PS_JOIN_MITER    = 0x00002000,
	PS_GEOMETRIC     = 0x00010000
    };


    /**
       Image bit/byte organisation

       See [MS-WMF] Section 2.1.1.3
    */
    enum BitCount {
        BI_BITCOUNT_0 = 0x0000,
        BI_BITCOUNT_1 = 0x0001,
        BI_BITCOUNT_2 = 0x0004,
        BI_BITCOUNT_3 = 0x0008,
        BI_BITCOUNT_4 = 0x0010, ///< Sometimes equivalent to QImage::Format_RGB555
        BI_BITCOUNT_5 = 0x0018, ///< equivalent to QImage::Format_RGB888
        BI_BITCOUNT_6 = 0x0020
    };

    /**
       Brush Styles

       See [MS-WMF] Section 2.1.1.4
    */
    enum BrushStyle {
	BS_SOLID	= 0x00,
	BS_NULL		= 0x01,
	BS_HATCHED	= 0x02,
	BS_PATTERN	= 0x03,
	BS_INDEXED	= 0x04,
	BS_DIBPATTERN	= 0x05,
	BS_DIBPATTERNPT = 0x06,
	BS_PATTERN8X8	= 0x07,
	BS_DIBPATTERN8X8 = 0x08,
	BS_MONOPATTERN	= 0x09
    };

    /**
        Compression Types

        See [MS-WMF] Section 2.1.1.8
    */
    enum Compression {
        BI_RGB          = 0x0000,
        BI_RLE8         = 0x0001,
        BI_RLE4         = 0x0002,
        BI_BITFIELDS    = 0x0003,
        BI_JPEG         = 0x0004,
        BI_PNG          = 0x0005,
        BI_CMYK         = 0x000B,
        BI_CMYKRLE8     = 0x000C,
        BI_CMYKRLE4     = 0x000D
    };

    /**
        Layout Direction

        See [MS-WMF] Section 2.1.1.13
    */
    enum LayoutMode {
        LAYOUT_LTR                              = 0x00,
        LAYOUT_RTL                              = 0x01,
        LAYOUT_BTT                              = 0x02,
        LAYOUT_VBH                              = 0x04,
        LAYOUT_BITMAPORIENTATIONPRESERVED       = 0x08
    };

    /**
        Horizontal Text Alignment
        
        See [MS-WMF] Section 2.1.2.3
    */
    enum TextAlignFlags {
        TA_TOP          = 0x0000,
        TA_LEFT         = 0x0000,
        TA_NOUPDATECP   = 0x0000,
        TA_UPDATECP     = 0x0001,
        TA_RIGHT        = 0x0002,
        TA_CENTER       = 0x0006,
        TA_HORZMASK     = 0x0006,
        TA_BOTTOM       = 0x0008,
        TA_BASELINE     = 0x0018,
        TA_VERTMASK     = 0x0018,
        TA_RTLREADING   = 0x0100
    };

    /**
       Stock Objects

       See [MS-EMF] Section 2.1.31
    */
    enum StockObject {
	WHITE_BRUSH	= 0x80000000,
	LTGRAY_BRUSH	= 0x80000001,
	GRAY_BRUSH	= 0x80000002,
	DKGRAY_BRUSH	= 0x80000003,
	BLACK_BRUSH	= 0x80000004,
	NULL_BRUSH	= 0x80000005,
	WHITE_PEN	= 0x80000006,
	BLACK_PEN	= 0x80000007,
	NULL_PEN	= 0x80000008,
	OEM_FIXED_FONT	= 0x8000000A,
	ANSI_FIXED_FONT	= 0x8000000B,
	ANSI_VAR_FONT	= 0x8000000C,
	SYSTEM_FONT	= 0x8000000D,
	DEVICE_DEFAULT_FONT = 0x8000000E,
	DEFAULT_PALETTE = 0x8000000F,
	SYSTEM_FIXED_FONT = 0x80000010,
	DEFAULT_GUI_FONT = 0x80000011,
	DC_BRUSH	= 0x80000012,
	DC_PEN		= 0x80000013
    };

    /**
       Fill mode

       See [MS-EMF] Section 2.1.27
    */
    enum PolygonFillMode {
	    ALTERNATE = 0x01, ///< Equivalent to Qt::OddEvenFill
	    WINDING = 0x02    ///< Equivalent to Qt::WindingFill
    };
    
    /**
       Background fill mode
       
       See [MS-EMF] Section 2.1.4
    */
    enum BackgroundMode {
        TRANSPARENT = 0x01, ///< Equivalent to Qt::TransparentMode
        OPAQUE = 0x2        ///< Equivalent to Qt::OpaqueMode
    };
    
    /**
      Clipping region mode
      
      See [MS-EMF] Section 2.1.29
    */
    enum RegionMode {
        RGN_AND = 0x01,   ///< Equivalent to Qt::IntersectClip
        RGN_OR = 0x02,    ///< Equivalent to Qt::UniteClip
        RGN_XOR = 0x03,
        RGN_DIFF = 0x04,
        RGN_COPY = 0x05   ///< Equivalent to Qt::ReplaceClip
    };
}
#endif
