/*
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

#ifndef SVMENUMS_H
#define SVMENUMS_H


/**
   \file

   Enumerations used in various parts of SVM files
*/

/**
   Namespace for StarView Metafile (SVM) classes
*/
namespace Libsvm
{

    /**
       Action types

       See the SPEC Section 2.1.1.1
    */
    enum ActionType {
        META_NULL_ACTION                  = 0,
        META_PIXEL_ACTION                 = 100,
        META_POINT_ACTION                 = 101,
        META_LINE_ACTION                  = 102,
        META_RECT_ACTION                  = 103,
        META_ROUNDRECT_ACTION             = 104,
        META_ELLIPSE_ACTION               = 105,
        META_ARC_ACTION                   = 106,
        META_PIE_ACTION                   = 107,
        META_CHORD_ACTION                 = 108,
        META_POLYLINE_ACTION              = 109,
        META_POLYGON_ACTION               = 110,
        META_POLYPOLYGON_ACTION           = 111,
        META_TEXT_ACTION                  = 112,
        META_TEXTARRAY_ACTION             = 113,
        META_STRETCHTEXT_ACTION           = 114,
        META_TEXTRECT_ACTION              = 115,
        META_BMP_ACTION                   = 116,
        META_BMPSCALE_ACTION              = 117,
        META_BMPSCALEPART_ACTION          = 118,
        META_BMPEX_ACTION                 = 119,
        META_BMPEXSCALE_ACTION            = 120,
        META_BMPEXSCALEPART_ACTION        = 121,
        META_MASK_ACTION                  = 122,
        META_MASKSCALE_ACTION             = 123,
        META_MASKSCALEPART_ACTION         = 124,
        META_GRADIENT_ACTION              = 125,
        META_HATCH_ACTION                 = 126,
        META_WALLPAPER_ACTION             = 127,
        META_CLIPREGION_ACTION            = 128,
        META_ISECTRECTCLIPREGION_ACTION   = 129,
        META_ISECTREGIONCLIPREGION_ACTION = 130,
        META_MOVECLIPREGION_ACTION        = 131,
        META_LINECOLOR_ACTION             = 132,
        META_FILLCOLOR_ACTION             = 133,
        META_TEXTCOLOR_ACTION             = 134,
        META_TEXTFILLCOLOR_ACTION         = 135,
        META_TEXTALIGN_ACTION             = 136,
        META_MAPMODE_ACTION               = 137,
        META_FONT_ACTION                  = 138,
        META_PUSH_ACTION                  = 139,
        META_POP_ACTION                   = 140,
        META_RASTEROP_ACTION              = 141,
        META_TRANSPARENT_ACTION           = 142,
        META_EPS_ACTION                   = 143,
        META_REFPOINT_ACTION              = 144,
        META_TEXTLINECOLOR_ACTION         = 145,
        META_TEXTLINE_ACTION              = 146,
        META_FLOATTRANSPARENT_ACTION      = 147,
        META_GRADIENTEX_ACTION            = 148,
        META_LAYOUTMODE_ACTION            = 149,
        META_TEXTLANGUAGE_ACTION          = 150,
        META_OVERLINECOLOR_ACTION         = 151,
        META_RENDERGRAPHIC_ACTION         = 152,
        META_COMMENT_ACTION               = 512
    };

#define META_LAST_ACTION  META_RENDERGRAPHIC_ACTION

    /** 
        Text align


        FIXME: Define this in the spec
    */

    enum TextAlign {
        ALIGN_TOP,
        ALIGN_BASELINE,
        ALIGN_BOTTOM
    };

    /**
       Mtf (FIXME)

       See the SPEC Section 2.1.1.2
    */
    enum MtfType {
        MTF_CONVERSION_NONE           = 0,
        MTF_CONVERSION_1BIT_THRESHOLD = 1,
        MTF_CONVERSION_8BIT_GREYS     = 2
    };


    /**
       Layout Mode

       See the SPEC, Section 2.2.2.4
    */
    enum LayoutMode {
        TEXT_LAYOUT_DEFAULT           = 0x0000,
        TEXT_LAYOUT_BIDI_LTR          = 0x0000,
        TEXT_LAYOUT_BIDI_RTL          = 0x0001,
        TEXT_LAYOUT_BIDI_STRONG       = 0x0002,
        TEXT_LAYOUT_TEXTORIGIN_LEFT   = 0x0004,
        TEXT_LAYOUT_TEXTORIGIN_RIGHT  = 0x0008,
        TEXT_LAYOUT_COMPLEX_DISABLED  = 0x0100,
        TEXT_LAYOUT_ENABLE_LIGATURES  = 0x0200,
        TEXT_LAYOUT_SUBSTITUTE_DIGITS = 0x0400
    };


    // ----------------------------------------------------------------
    //                             Flags
    // ----------------------------------------------------------------


    /**
       Mirror flags

       See the SPEC Section 2.1.2.1
    */
    enum MtfMirrorType {
        MTF_MIRROR_NONE = 0x00000000,
        MTF_MIRROR_HORZ = 0x00000001,
        MTF_MIRROR_VERT = 0x00000002
    };

}


#endif
