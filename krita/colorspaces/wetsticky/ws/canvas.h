/*
    FILE:        canvas.h
    PURPOSE:    Defines the public routines for manipulating the canvas.
    AUTHOR:        Kevin Waite 
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


extern int number_of_repaints_needed();
/*  Returns the number of cells needing to repainted.  */

extern void need_to_repaint (/* POINT */);
/*  Requests that the cell at the given point be repainted
    at the next update as it has been modified.  */

extern void next_cell_for_repaint (/* *CELL_PTR, POINT_PTR  */);
/*  Returns a pointer to a cell that needs to be updated as well
    as the location of that cell on the canvas.   If there are
    no more cells to be redrawn then the pointer will be NIL.  */

extern void next_cell_point (/* POINT_PTR */);
/*  Sets the POINT giving the coordinate location of the next
    cell on the canvas to be visited.  There is an even probability
    of each cell being visited.  */

extern CELL_PTR get_cell (/* POINT */);
/*  This function returns a pointer to the cell at the
    given address on the canvas.  */

extern DIRECTION anti_clockwise_from (/*  DIRECTION  */);
/*  Returns the direction found going clockwise from the
    given direction.  */ 

extern DIRECTION clockwise_from (/*  DIRECTION  */);
/*  Returns the direction found going clockwise from the
    given direction.  */ 

extern BOOLEAN neighbour (/* POINT, DIRECTION, POINT_PTR */);
/*  Set bPoint to the coordinate of the point that can
    be found by going one place in the given direction 
    from aPoint.  The direction can be NORTH, EAST, WEST
    or SOUTH.  If bPoint will be off the canvas then the
    function returns FALSE otherwise TRUE.  */

extern void initialise_canvas();
/*  Before it can be used the canvas needs to be initialised to 
    a default state.  This involves setting each of the cells to
    have no paint and for gravity to be uniformly down.  Each cell
    has the default absorbancy value.  */

extern void blob( /* int */);
/* This routine puts a square blob of black paint
   of the given side length centered on the canvas.
   This is used for test purposes.  */

extern void load_file(/*char *, int *, int * */);
/* Load a file from a portable pixmap into the canvas */
