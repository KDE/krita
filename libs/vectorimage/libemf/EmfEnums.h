/*
  Copyright 2008 Brad Hards <bradh@frogmouth.net>
  Copyright 2009-2011 Inge Wallin <inge@lysator.liu.se>

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

#ifndef EMFENUMS_H
#define EMFENUMS_H

#include <QDataStream>
#include <QRect> // also provides QSize
#include <QString>

/**
   \file

   Enumerations used in various parts of EMF files
*/


// We need most of the WMF enums and flags as well in an EMF file.
#include <WmfEnums.h>

using namespace Libwmf;


/**
   Namespace for Enhanced Metafile (EMF) classes
*/
namespace Libemf
{
   
/**
   Background fill mode
   
   See [MS-EMF] Section 2.1.4
*/
#if 0
    enum BackgroundMode {
        TRANSPARENT = 0x01, ///< Equivalent to Qt::TransparentMode
        OPAQUE      = 0x02  ///< Equivalent to Qt::OpaqueMode
    };
#else
    typedef Libwmf::WmfMixMode EmfBackgroundMode; // This is exactly the same.
#endif
    
    /**
       Parameters for text output.
       
       See [MS-EMF] Section 2.1.11
    */
    enum TextOutOptions {
        //ETO_OPAQUE            = 0x000002,    // Already defined in WmfEnums.h
        //ETO_CLIPPED           = 0x000004,
        //ETO_GLYPH_INDEX       = 0x000010,
        //ETO_RTLREADING        = 0x000080,
        ETO_NO_RECT           = 0x000100,
        ETO_SMALL_CHARS       = 0x000200,
        //ETO_NUMERICSLOCAL     = 0x000400,
        //ETO_NUMERICSLATIN     = 0x000800,
        ETO_IGNORELANGUAGE    = 0x001000,
        //ETO_PDY               = 0x002000,
        ETO_REVERSE_INDEX_MAP = 0x010000
    };

    /**
       Graphics mode, used to interpret shape data such as rectangles
       
       See [MS-EMF] Section 2.1.16
    */
    enum GraphicsMode {
        GM_COMPATIBLE = 0x01,
        GM_ADVANCED   = 0x02
    };
    
    /**
       MapModes

       See [MS-EMF] Section 2.1.21
    */
    typedef enum {
        MM_TEXT        = 0x01,
        MM_LOMETRIC    = 0x02,
        MM_HIMETRIC    = 0x03,
        MM_LOENGLISH   = 0x04,
        MM_HIENGLISH   = 0x05,
        MM_TWIPS       = 0x06,
        MM_ISOTROPIC   = 0x07,
        MM_ANISOTROPIC = 0x08
    } MapMode;

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
	    WINDING   = 0x02  ///< Equivalent to Qt::WindingFill
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

    /**
       Comment type as defined for the EMR_COMMENT record.

       See [MS-EMF] section 2.3.3
     */
    enum CommentType {
        EMR_COMMENT_EMFSPOOL = 0x00000000,
        EMR_COMMENT_EMFPLUS  = 0x2B464D45, // The string "EMF+"
        EMR_COMMENT_PUBLIC   = 0x43494447,

        // The following value is not defined in [MS-EMF].pdf, but
        // according to google it means that the file was created by
        // Microsoft Graph.  It is present in one test file
        // (Presentation_tips.ppt).
        EMR_COMMENT_MSGR     = 0x5247534d // The string MSGR
    };
}


#endif
