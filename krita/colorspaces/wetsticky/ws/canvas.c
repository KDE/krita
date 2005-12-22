/*
	FILE:		canvas.c
	PURPOSE:	Hides the canvas and provides its access and 
			manipuation routines.
	AUTHOR:		Kevin Waite 
	VERSION:	1.00  (10-May-91)

Copyright 1991, 1992, 2002, 2003 Tunde Cockshott, Kevin Waite, David England. 

Contact David England d.england@livjm.ac.uk
School of Computing and Maths Sciences,
Liverpool John Moores University 
Liverpool L3 3AF
United Kingdom
Phone +44 151 231 2271

Wet and Sticky is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version. Wet and Sticky is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with Wet and Sticky; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA 

*/

#include "constants.h"
#include "types.h"
#include "canvas.h"
#include <stdio.h>

/*  Declare the canvas data structure local to this module.  It can
    only be accessed via routines given in the header file for this module.  
    The (0,0) location for the canvas is at the top-left.  */

CELL canvas[CANVAS_WIDTH][CANVAS_HEIGHT];

/*  This module maintains a list of the addresses of cells that have
    been modified since the last redraw and therefore need updating.
    Points are added to this list by the need_to_repaint() routine
    and are removed by the next_cell_for_repaint() function.  The
    pointer to the current tail of the list is updated by side-effect. */

static POINT need_repainting[REDRAW_LIMIT];
static int   next_free = 0;
static int   next_to_repaint = 0;

/* *********************************************************************** */

int number_of_repaints_needed()
/*  Returns the number of cells that need to be repainted. */

{
   return (next_free);
}

/* *********************************************************************** */

void need_to_repaint(point)
/*  The cell at this location needs to be redrawn since it has
    been altered.   Scan the list to see if it is already
    scheduled for a repainting operation and only add it if
    it is not there.   */

POINT point;

{
   int k;


   /*  If the list is already full then simply ignore the repaint
       request - it will get done eventually anyway.  */

   if (next_free == REDRAW_LIMIT) return;

   /* Check whether this point is already on the list. */

   for (k=0; k < next_free; k++) {
      if ((need_repainting[k].x == point.x) &&
          (need_repainting[k].y == point.y)) break;
   }

   if (k < next_free) return; /* Already in the list. */

   /*  Add this new cell address to the end of the list. */

   need_repainting[next_free].x = point.x;
   need_repainting[next_free].y = point.y;
   next_free++;
}

/* *********************************************************************** */

void next_cell_for_repaint(cell, locus)
/*  This routine returns the next cell to be repainted, together with its
    location on the canvas.  This is determined by taking the next point 
    from the need_repainting list and accessing its cell.  If the list is 
    empty then return NIL. 
    Note that the repainting operation will clear out the list before
    any other new positions are added.  */ 

   CELL_PTR *cell;
   POINT_PTR locus;

{
   if (next_to_repaint >= next_free) {
      next_to_repaint = next_free = 0;
      *(cell) = NIL;
      return;
   }

   *(cell) = get_cell(need_repainting[next_to_repaint]);
   locus->x = need_repainting[next_to_repaint].x;
   locus->y = need_repainting[next_to_repaint].y;
   next_to_repaint++;
}

/* *********************************************************************** */

void next_cell_point (address)
/*  Sets the POINT giving the coordinate location of the next
    cell on the canvas to be visited.  There is an even probability
    of each cell being visited.  */

POINT_PTR address;
{
   extern long random();

   address->x = random() % CANVAS_WIDTH;
   address->y = random() % CANVAS_HEIGHT;
}


/* *********************************************************************** */

CELL_PTR get_cell (point)
/*  This function returns a pointer to the cell at the
    given address on the canvas.  */

POINT point;

{
   return (&canvas[point.x][point.y]);
}

/* *********************************************************************** */

DIRECTION anti_clockwise_from (arrow)
/*  Returns the direction found going anti-clockwise from the
    given direction.  */

DIRECTION arrow;

{
   switch (arrow) {
  
      case NORTH:  return(WEST);
      case EAST:   return(NORTH);
      case SOUTH:  return(EAST);
      case WEST:   ;

   }
   return(SOUTH);
}

/* *********************************************************************** */

DIRECTION clockwise_from (arrow)
/*  Returns the direction found going clockwise from the
    given direction.  */

DIRECTION arrow;

{
   switch (arrow) {
  
      case NORTH:  return(EAST);
      case EAST:   return(SOUTH);
      case SOUTH:  return(WEST);
      case WEST:   ;

   }
   return(NORTH);
}

/* *********************************************************************** */

BOOLEAN neighbour (aPoint, direction, bPoint)
/*  Set bPoint to the coordinate of the point that can
    be found by going one place in the given direction 
    from aPoint.  The direction can be NORTH, EAST, WEST
    or SOUTH.  If bPoint will be off the canvas then the
    function returns FALSE otherwise TRUE.  */

POINT     aPoint;
POINT_PTR bPoint;
DIRECTION direction;

{
    int x, y;

    switch (direction) {

       case NORTH:  x = 0; y = -1; break;
       case EAST:   x = 1; y = 0;  break;
       case SOUTH:  x = 0; y = 1;  break;
       case WEST:   x = -1; y = 0; break;
    }

    bPoint->x = aPoint.x + x;
    bPoint->y = aPoint.y + y;

    if ((bPoint->x >= CANVAS_WIDTH) || (bPoint->x < 0) ||
       (bPoint->y >= CANVAS_HEIGHT) || (bPoint->y < 0)) return(FALSE);

    return(TRUE);
}

/* *********************************************************************** */

void initialise_paint (paint)
/*  Set this paint to be a dry, unmixing white. */

PAINT_PTR paint;

{
   paint->colour.hue = 255;
   paint->colour.saturation = 0.0;
   paint->colour.lightness = 1.0;
   paint->liquid_content = 0;
   paint->drying_rate = 0;
   paint->miscibility = 0;
}

/* *********************************************************************** */

void initialise_cell(cell)
/*  Reset the given cell to a default value.  */

CELL_PTR cell;

{
   initialise_paint (&cell->contents);

   cell->volume = UNFILLED;  /* Indicates that no paint has yet been applied.  
*/
   cell->absorbancy = 10;
   cell->gravity.direction = SOUTH;
   cell->gravity.strength = DEFAULT_GRAVITY_STRENGTH;
}


/* *********************************************************************** */

void split_gravity()
/*  This routine is for test purposes only.  It causes the right
    half of the canvas to have gravity going NORTH with the left
    half having gravity going SOUTH.   */

{
   POINT p;
   CELL_PTR cell;

   /*for (p.x=CANVAS_WIDTH/2; p.x < CANVAS_WIDTH; p.x++) {*/
   for (p.x=165; p.x < CANVAS_WIDTH; p.x++) {
      for (p.y=0; p.y < CANVAS_HEIGHT; p.y++) {
	 cell = get_cell (p);
         cell->gravity.direction = NORTH;
      }
   }
}

/* *********************************************************************** */

void initialise_canvas()
/*  Before it can be used the canvas needs to be initialised to 
    a default state.  This involves setting each of the cells to
    have no paint and for gravity to be uniformly down.  Each cell
    has the default absorbancy value.  */

{
   POINT p;
   CELL_PTR cell;

   for (p.x=0; p.x < CANVAS_WIDTH; p.x++) {
      for (p.y=0; p.y < CANVAS_HEIGHT; p.y++) {
	 cell = get_cell (p);
         initialise_cell (cell);
      }
   }
}

/* *********************************************************************** */

void print_cell_attributes(cell)
CELL_PTR cell;
{
   printf("Volume         = %d\n", cell->volume);

   printf("Liquid content = %d%%\n", cell->contents.liquid_content);
   printf("Drying rate    = %d%%\n", cell->contents.drying_rate);
   printf("Miscibility    = %d%%\n\n", cell->contents.miscibility);

   printf("Saturation = %2f\n", cell->contents.colour.saturation);
   printf("Lightness  = %2f\n", cell->contents.colour.lightness);
   printf("Hue        = %d\n", cell->contents.colour.hue);
   printf ("------------------------------\n\n");
}

/* *********************************************************************** */

void blob_cell_alpha (cell)
CELL_PTR cell;

{
   cell->contents.liquid_content = 100;
   cell->contents.drying_rate = 10;
   cell->contents.miscibility = 80;
   
   cell->contents.colour.hue = 20;
   cell->contents.colour.saturation = 1.0;
   cell->contents.colour.lightness = 0.0;

   cell->volume = 50;

}

/* *********************************************************************** */

void blob_cell_beta (cell)
CELL_PTR cell;

{
   cell->contents.liquid_content = 80;
   cell->contents.drying_rate = 20;
   cell->contents.miscibility = 90;
   
   cell->contents.colour.hue = 70;
   cell->contents.colour.saturation = 1.0;
   cell->contents.colour.lightness = 0.7;

   cell->volume = 30;

}
/* *********************************************************************** */

void blob_cell_gamma (cell)
CELL_PTR cell;

{
   cell->contents.liquid_content = 80;
   cell->contents.drying_rate = 40;
   cell->contents.miscibility = 80;
   
   cell->contents.colour.hue = 100;
   cell->contents.colour.saturation = 0.5;
   cell->contents.colour.lightness = 0.4;

   cell->volume = 50;

}

/* *********************************************************************** */

void old_blob(width)
/* This routine puts a square blob of various paints
   of the given side length centred on the canvas.
   This is used for test purposes.  */

int width;

{
   int count, lump, startx, starty, x, y;

   width = (width > CANVAS_WIDTH) ? CANVAS_WIDTH : width;
   width = (width > CANVAS_HEIGHT) ? CANVAS_HEIGHT : width;

   printf("This run used a square blob of side %d pixels\n", width);
   printf ("centred on the canvas.  The blob was split into three equal\n");
   printf ("vertical strips with the following paint attributes:\n\n");

   startx = (CANVAS_WIDTH - width) / 2;
   starty = (CANVAS_HEIGHT - width) / 2;
   lump = width / 3;

   count=0;
   for (x = startx; x < startx + width; x++) {
      for (y = starty; y < starty + width; y++) {
         switch (count / lump) {
    
             case 0:  blob_cell_alpha (&canvas[x][y]);  break;
             case 1:  blob_cell_beta (&canvas[x][y]);   break;
             default: blob_cell_gamma (&canvas[x][y]);  break;

         }
      }
      count++;
   }
   split_gravity();

   print_cell_attributes (&canvas[startx][starty]);
   print_cell_attributes (&canvas[startx + lump][starty]);
   print_cell_attributes (&canvas[startx + (2*lump)][starty]);
}

gravity_set(x,y,x1,y1,attr)
int x;
int y;
int x1;
int y1;
int attr;
{
	/* set the canavs absorbancy and gravity to various test values
		in the region (x,y),(x1,y1)
	*/

	int i,j;

	for (i=x; i < x1; i++ )
            for (j=y; j < y1; j++) {
		canvas[i][j].absorbancy = 10; /* default 10 */
		   canvas[i][j].gravity.direction = SOUTH;
		   canvas[i][j].gravity.strength =  DEFAULT_GRAVITY_STRENGTH;
					/*DEFAULT_GRAVITY_STRENGTH*/
	    }
}

blob_set(x, y, x1, y1, attr)
int x;
int y;
int x1;
int y1;
int attr;
{
	/* Set a blob of paint in the rectangle (x,y),(x1,y1) with
		the attribute, attr (can be volume, liquidity or dryness*/
	int i,j;
	float colour;

	colour = (3.6 * 122);
	colour = 64;
	fprintf(stderr, "attribute value %d %d\n", attr, (int)colour);


	for (i=x; i < x1; i++ )
	    for (j=y; j < y1; j++) {
		canvas[i][j].contents.liquid_content = 80;
		   canvas[i][j].contents.drying_rate = 50;
		   canvas[i][j].contents.miscibility = 80;

		   canvas[i][j].contents.colour.hue = (int)colour;
		   canvas[i][j].contents.colour.saturation = 1.0;
		   canvas[i][j].contents.colour.lightness = 0.7;

		   canvas[i][j].volume = attr;
	}
		

}

void blob(type)
int type;
{

	/* paint nine test blobs on the canvas */

	/* X Example 1 All attributes at 80% except vol 0 - 100%*/

	/* X Example 2 All attributes at 80% except liq 0 - 100% */

        /* X Example 3 All attributes at 80% except dry 0 - 100% */

	/* X Example 4 All attributes at 80% except absorp 100 - 0% */

	/* X Example 5 All attributes at 80% except gravity 0 - 100% */

	/* X Example 6 All attributes at 80% except direction N,S,E & W */

	blob_set(20, 20, 80, 80, 0);
	gravity_set(0, 0, 100, 100, SOUTH);

	blob_set(120,20, 180, 80, 22);
	gravity_set(150, 0, 300, 150, EAST);
	
	blob_set(220,20,280,80,33);
	gravity_set(0, 150, 150, 300, NORTH);

	blob_set(20,120,80,180,44);
	gravity_set(150, 150, 300, 300, WEST);
	
	blob_set(120,120,180,180,55);
	gravity_set(100, 100, 200, 200, 55);

	blob_set(220,120, 280,180, 66);
	gravity_set(200, 100, 300, 200, 66);
	
	blob_set(20,220,80,280,77);
	gravity_set(0, 200, 100, 300, 77);

	blob_set(120,220,180,280,88);
	gravity_set(100, 200, 200, 300, 88);

	blob_set(220,220,280,280, 100);
	gravity_set(200, 200, 300, 300, 100);
	
}

void
load_file(filename, width, height)
char *filename;
int *width;
int *height;
{

	/* Load in a file using the load_ppm_format() function
	 This loads in a file in Portable Pixmap format 
	*/

	load_ppm_format(filename, canvas, width, height);
}	
