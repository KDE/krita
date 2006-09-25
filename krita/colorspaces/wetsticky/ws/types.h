/*
    FILE:        types.h
    PURPOSE:    Defines all the main types used in Wet&Sticky.
    AUTHORS:    Kevin Waite and David England
    VERSION:    1.00  (10-May-91)

Copyright 1991, 1992, 2002, 2003 Tunde Cockshott, Kevin Waite, David England. 

Contact David England d.england@livjm.ac.uk
School of Computing and Maths Sciences,
Liverpool John Moores University 
Liverpool L3 3AF
United Kingdom
Phone +44 151 231 2271

Wet and Sticky is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version. Wet and Sticky is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with Wet and Sticky; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA 


*/


/*  A color is specified as a vector in HLS space.  Hue is a value
    in the range 0..360 degrees with 0 degrees being red.  Saturation
    and Lightness are both in the range [0,1].  A lightness of 0 means
    black, with 1 being white.  A totally saturated color has saturation
    of 1.
*/

typedef struct hls_color { short int hue; float saturation, lightness; }
HLS_COLOUR;

typedef struct rgb_color {float r; float g; float b;}
RGB_COLOUR;


/* The address of a cell on the canvas. */

typedef struct point { int x, y; } POINT, *POINT_PTR;  


/*  A direction can be NORTH, EAST, SOUTH or WEST. */

typedef short int DIRECTION;  

typedef short int BOOLEAN;  /*  FALSE or TRUE  */


typedef struct paint {
   HLS_COLOUR color;
   int        liquid_content;  /*  [0,100].  */
   int        drying_rate;     /*  [0,100].  */
   int        miscibility;     /*  [0,inf].  */
} PAINT, *PAINT_PTR;


/*  Defines the strength and direction of gravity for a cell.  */

typedef struct gravity {
   DIRECTION  direction;    
   int        strength;     /*  [0,Infinity). */
} GRAVITY, *GRAVITY_PTR;


/*  Defines the contents and attributes of a cell on the canvas. */

typedef struct cell {
   PAINT       contents;    /* The paint in this cell. */
   GRAVITY     gravity;     /* This cell's gravity.  */
   short int   absorbancy;  /* How much paint can this cell hold? */
   short int   volume;      /* The volume of paint. */
} CELL, *CELL_PTR;
