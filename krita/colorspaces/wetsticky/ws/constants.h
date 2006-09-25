/*
    FILE:        constants.h
    PURPOSE:    Constains all the #DEFINES for Wet&Sticky.
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

/*  Some utility constants.  */

#define  TRUE    1
#define  FALSE   0
#define  YES     1
#define  NO      0
#define  NIL     0
#define  DEBUG   1
#define  VERSION "1.0" 

/*  Define the constants for colors in the HLS space.  */

#define  UNFILLED           -1
#define  MAX_COLOUR_INDEX  255


/*  Define the dimensions of the intelligent canvas. */

#define  CANVAS_WIDTH  300
#define  CANVAS_HEIGHT 300
#define  SCALE_WIDTH    30


/*  Define constants that control the evolution of the paint. */

#define  STEP_LIMIT    200
#define  REDRAW_LIMIT  500


/*  Define some constants used in testing the system. */

#define  DEFAULT_BLOB_SIZE (CANVAS_WIDTH / 3)
#define  BLOB_NAME         "-blob"


/*  Constants used in modelling gravity.  */

#define NORTH 0
#define EAST  1
#define SOUTH 2
#define WEST  3

#define DEFAULT_GRAVITY_STRENGTH  10


/*  Define some macros.  */

#define MAX(A,B) ((A) > (B) ? (A) : (B))
#define MIN(A,B) ((A) < (B) ? (A) : (B))
