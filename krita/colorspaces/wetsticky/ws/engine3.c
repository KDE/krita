/*
	FILE:		engine.c
	PURPOSE:	Defines the routines for the Paint Engine.
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
#include <math.h>

#define HEIGHT_SCALE 1.0

/* *********************************************************************** */

int random_percent()
/*  This function returns a random number in the range [0,100].  */
{
   extern long random();

   return (random() % 101);
}

/* *********************************************************************** */

BOOLEAN allow_event_based_on(value)
/*  The given value is a percentage.  Compare this value
    with a randomly generated percentage and if it is larger
    then allow the event to happen (i.e. return TRUE) other-
    wise return FALSE.  */

int value;

{
   if (value > random_percent()) return(TRUE);
   return(FALSE);
}

/* *********************************************************************** */

BOOLEAN age_paint(cell)
/*  Make the paint in the given cell older, i.e. let
    if dry out a bit if it isn't already dry.  This
    function returns TRUE if the paint was already
    dry or becomes so, and FALSE otherwise.  */

CELL_PTR cell;

{
  if (cell->volume == 0) return(TRUE);
  if (cell->contents.liquid_content == 0) return(TRUE);
  if (allow_event_based_on(cell->contents.drying_rate) == TRUE) 
     cell->contents.liquid_content--;

  if (cell->contents.liquid_content == 0) return(TRUE);
  return(FALSE);
}

/* *********************************************************************** */

BOOLEAN similar_paint(aPaint, bPaint)
/* Determine whether the two paints are similar.  It is
   assumed that aPaint has come from the host cell (and
   so it is its miscibility value that is used).  The
   function returns TRUE if the paints are similar and
   FALSE otherwise.   */

PAINT aPaint, bPaint;

{
   int delta;

   delta = abs(aPaint.liquid_content - bPaint.liquid_content);
   if (delta <= aPaint.miscibility) return(TRUE);
   return(FALSE);
}

/* *********************************************************************** */

int surplus_paint(cell)
/*  Returns the amount of paint held by this cell greater than its
    absorbancy value.  This is the amount of paint that can flow.  */

CELL_PTR cell;

{
   return (MAX(cell->volume - cell->absorbancy, 0));
}

/* *********************************************************************** */

BOOLEAN has_surplus_paint(cell)
/*  Does the given cell have excess paint, i.e. can paint flow out
    of this cell.  Return TRUE if it can and FALSE otherwise.  */

CELL_PTR cell;

{
   if (surplus_paint(cell) > 0) return(TRUE);
   return(FALSE);
}

/* *********************************************************************** */

void stop() {  /*  Used for breakpointing.  */ }

void donate_paint(source, srcLocus, amount, dest, destLocus)
/* The source cell is donating the specified volume of its paint
   to the destination cell.  The destination cell must mix this 
   new paint with its existing paint to yield a new paint.
   This routine is also responsible for recording which cells
   have been updated and so need repainting.
   
   A special case is recognised where the destination has not yet
   had any paint applied.  This causes the donated paint to become
   to new contents of this cell.   

*/

CELL_PTR source, dest;
POINT    srcLocus, destLocus;
int      amount;

{
   float delta, ratio;
   int   iDelta;

   source->volume -= amount;

   if (dest->volume == UNFILLED) {

      /*  The donated paint is going into an unfilled cell.  
          Copy the source's attributes into the destination.  */

      dest->volume = amount;
      dest->contents.colour.hue = source->contents.colour.hue;
      dest->contents.colour.lightness = source->contents.colour.lightness;
      dest->contents.colour.saturation = source->contents.colour.saturation;
      dest->contents.liquid_content = source->contents.liquid_content;
      dest->contents.miscibility = source->contents.miscibility;
      dest->contents.drying_rate = source->contents.drying_rate;

   } else {

   /*  Need to mix the existing paint in the dest with this amount
       of new paint from the source.  This is done using a linear
       interpolation mechanism using the relative amounts of the
       paint as the control.  */

      ratio = amount / (float)(dest->volume);
   
      iDelta = source->contents.colour.hue - dest->contents.colour.hue;
      if (iDelta != 0) {
         dest->contents.colour.hue += (int)(ratio * iDelta); 
         if (dest->contents.colour.hue >= 360)
            dest->contents.colour.hue -= 360; 
      }

      iDelta = source->contents.drying_rate - dest->contents.drying_rate;
      dest->contents.drying_rate += (int)(ratio * iDelta);
      dest->contents.drying_rate %= 101;

      iDelta = source->contents.liquid_content - dest->contents.liquid_content;
      dest->contents.liquid_content += (int)(ratio * iDelta);
      dest->contents.liquid_content %= 101;

      iDelta = source->contents.miscibility - dest->contents.miscibility;
      dest->contents.miscibility += (int)(ratio * iDelta);
      dest->contents.miscibility %= 101;

      delta = source->contents.colour.saturation - dest->contents.colour.saturation;
      dest->contents.colour.saturation += ratio * delta;

      delta = source->contents.colour.lightness - dest->contents.colour.lightness;
      dest->contents.colour.lightness += ratio * delta;

      dest->volume += amount;   /* The new volume of paint in dest. */

   }

   need_to_repaint(destLocus);
}

/* *********************************************************************** */

void handle_surface_tension(cell, locus)
/*  This routine handles the surface tension around the given cell.
*/

CELL_PTR cell;
POINT    locus;

{
   DIRECTION direction[3];
   POINT     loci[3];
   CELL_PTR  buddy[3];
   BOOLEAN   ok, similar[3];
   int       weakCount, weak[3], count[3], excess, chosen, side, start, finish, k, lowest;

   if (has_surplus_paint(cell) == FALSE) return;

   direction[0] = cell->gravity.direction;
   direction[1] = clockwise_from(direction[0]);
   direction[2] = anti_clockwise_from(direction[0]);

   for (k=0; k < 3; k++) {
      ok = neighbour(locus, direction[k], &loci[k]);
      if (ok == TRUE) {
         buddy[k] = get_cell(loci[k]);
         count[k] = 0;
      } else count[k] = -1;
   }

   for (k=0; k < 3; k++) 
      similar[k] = (count[k] == -1) 
                      ? FALSE 
                      : similar_paint(cell->contents, buddy[k]->contents); 

   for (k=0; k < 3; k++) {
      if ((count[k] != -1) && (similar[k] == FALSE)) {
         count[k] = 0;
         start = MAX(k-1, 0);
         finish = MIN(k+1, 2);
         for (side=start; side <= finish; side++) 
            if ((count[side] != -1) && (similar[side] == FALSE)) count[k]++;
          
      }
   }

   lowest = 4;
   for (k=0; k < 3; k++) if (count[k] >= 0) lowest = MIN(count[k], lowest);

   weakCount = 0;
   for (k=0; k < 3; k++) if (count[k] == lowest) weak[weakCount++] = k;

   /*  The weak array now holds weakCount indices of those sides that have
       the lowest surface tension and therefore where any paint would flow over.
       Now it is necessary to see whether paint will actually flow based on
       a probability level using the liquidity and volume of the paint in the
       cell as parameters.   Paint will flow over only one of the weakest sides
       with the side chosen at random.    */

   if (random_percent() > cell->contents.liquid_content) return;  /*  Too viscous.  */

   excess = surplus_paint(cell);
   if (random_percent() > excess * 3) return;   
   /*  The '3' in the previous statement is an empirically-derived multiplier.  */

   /*  The paint will flow.  Pick one of the weakest sides at random.  */

   chosen = weak[random_percent() % weakCount];
   donate_paint(cell, locus, (excess / 2), buddy[chosen], loci[chosen]);
}

/* *********************************************************************** */

BOOLEAN diffuse_paint(cell, locus)
/* Diffuse paint among the neighbours of the given cell.
   If this cell does not have surplus paint then return
   TRUE otherwise return FALSE.  */

CELL_PTR cell;
POINT locus;

{
   extern long random();
   DIRECTION down, direction;
   CELL_PTR buddy;
   POINT nlocus;
   BOOLEAN ok;
   int excess;

   if (has_surplus_paint(cell) == FALSE) return(TRUE);

   down = cell->gravity.direction;
   direction = ((random() & 01) == 0) 
               ? clockwise_from(down) 
               : anti_clockwise_from(down);

   ok = neighbour(locus, direction, &nlocus);
   if (ok == FALSE) return(TRUE);

   buddy = get_cell(nlocus);

   if (similar_paint(cell->contents, buddy->contents) == FALSE) {
      handle_surface_tension(cell, locus);
      return(FALSE);
   }

   if (buddy->volume >= cell->volume) return(FALSE);

   if (allow_event_based_on(cell->contents.liquid_content) == FALSE)
      return(FALSE);

   /* Transfer one particle of paint from cell to its buddy. */

   excess = (cell->volume - buddy->volume) / 2;
   donate_paint(cell, locus, excess, buddy, nlocus);
   return(FALSE);
}

/* *********************************************************************** */

BOOLEAN apply_gravity(cell, locus)
/*  Subject the contents of the given cell to the effects
    of gravity.  Note that the direction of gravity is local
    to the given cell.   Locus is the address of this cell.
    This function returns TRUE if the paint in this cell
    cannot flow and FALSE otherwise.
*/

CELL_PTR cell;
POINT    locus;

{
   extern long random();
   POINT downhill;
   CELL_PTR down;
   BOOLEAN ok, can_flow;
   int barrier, excess;

   ok = neighbour(locus, cell->gravity.direction, &downhill);
   if (ok == FALSE) return(TRUE);  /* At bottom of canvas. */

   down = get_cell(downhill);

   can_flow = down->volume < (cell->volume + cell->gravity.strength)
	      ? TRUE : FALSE;

   if (can_flow == FALSE) return(TRUE);

   /*  Although this paint can flow introduce a random value that
       uses the viscosity of the paint to determine whether it does
       actually flow.  */

   barrier = random() % 10;
   if (cell->contents.liquid_content > barrier) { 
     /* Paint is actually moving.  Move half of the excess downward. */

     excess = (cell->volume - cell->absorbancy) / 2;
     donate_paint(cell, locus, excess, down, downhill);
   }

   return(FALSE);
}


float  lx, ly, lz;

void
compute_shade_vectors()
{
	extern  float lx, ly, lz;
	float D;
	
	lx = -1.0;  ly = 1.0;  lz = 2.0;

	D = sqrt ( lx * lx + ly * ly + lz * lz );

	lx = lx/D; ly = ly/D; lz = lz/D;

}

/* *********************************************************************** **									**
** 	new_intensity_value						**
**									**
**	calculates shade value for a pixel from surface characteristics ** **									**
**	Revision History						**
**									**
**	Rev	Date	By	Description				**
**	1.0 1/12/91	DE Original				**
**	1.1 1/04/92	DE Include Phong Shading			**
**	1.2 11/08/92	 JWP Parameterized Specular Component 	** **									**
*********************************************************************** */ 
float
normalize (x, y, z)
 float x, y, z; /*vector x, y, z components*/
{
 float result;

        /* function calculates the amount to divide each vector component
           to normalize it to a unit vector. The parameters are the x, y,
z
           components and the result is the amount to divide by */

 result = sqrt (x*x + y*y + z*z);
 return (result);

 }



float Newnormalize(V, W)
float *V;
float *W;
{
float temp;

temp = normalize(V[0], V[1], V[2]);

W[0] = V[0]/temp;
W[1] = V[1]/temp;
W[2] = V[2]/temp;

return temp;
}

float dot(V, W)
float V[3];
float W[3];
{

return ( (V[0])*(W[0]) + (V[1])*(W[1]) + (V[2])*(W[2]) );

}

float Phong (Nv, Lv, Ev, shine)
float Nv[3];
float Lv[3];
float Ev[3];
float shine;
{
float Hv[3];
 
Newnormalize(Ev, Ev);
 
Hv[0] = Ev[0] + Lv[0];
Hv[1] = Ev[1] + Lv[1];
Hv[2] = Ev[2] + Lv[2];
 
Newnormalize (Hv, Hv);
 
shine = abs(shine);
return( pow(dot(Nv, Hv), shine) );
}


float
new_intensity_value(a_pnt)
POINT a_pnt;
/* Calculate the new intensity value of a pixel 
in order to construct a bump map of the paint surface
*/
{
float h, h1, h2, h3, h4;
	int shininess;
float Ka, Kd, Ks;
	float wetmax, degree, norm, distance;
	float g;
float Nv[3];
	float Ev[3];
	float Hv[3];
	float Lv[3];
	extern float lx, ly, lz;
float intensity, light_intensity;
POINT b_pnt;
CELL_PTR cell;
CELL_PTR next_cell;
	int x_cntr, y_cntr;

Ka = 0.1;
Kd = 0.6;
	Ks = 0.4;

	wetmax = 100.0;
	distance = 2500.0;
	light_intensity = 2.0;
	shininess = 200;

cell = get_cell(a_pnt);

h = (float)cell->volume;

if (neighbour(a_pnt, NORTH, &b_pnt)) {
next_cell = get_cell(b_pnt);
h1 = (float)next_cell->volume;
} else
h1 = h;

if (neighbour(a_pnt, EAST, &b_pnt)) {
next_cell = get_cell(b_pnt);
h2 = (float)next_cell->volume;
} else
h2 = h;

if (neighbour(a_pnt, SOUTH, &b_pnt)) {
next_cell = get_cell(b_pnt);
h3 = (float)next_cell->volume;
} else
h3 = h;

if (neighbour(a_pnt, WEST, &b_pnt)) {
next_cell = get_cell(b_pnt);
h4 = (float)next_cell->volume;
} else
h4 = h;

h1 = h1/HEIGHT_SCALE;
h2 = h2/HEIGHT_SCALE;
h3 = h3/HEIGHT_SCALE;
h4 = h4/HEIGHT_SCALE;

	degree = (float)abs(cell->contents.liquid_content)/wetmax; 

	x_cntr= 150 - a_pnt.x;
	y_cntr= 150 - a_pnt.y;

	Ks = light_intensity * Ks * degree;

	Kd = light_intensity * Kd;

	shininess = (int)degree * shininess + 1; 


	Nv[1] = h3 - h1;
	Nv[0] = h4 - h2;
Nv[2] = 4.0;

	Newnormalize (Nv, Nv);

	Lv[0] = lx;
Lv[1] = ly;
	Lv[2] = lz;

	g = dot(Lv, Nv)*Kd + Ka;

        g = g * (float)cell->contents.colour.hue;

Ev[0] = (float)x_cntr;
Ev[1] = (float)y_cntr;
Ev[2] = distance;

        intensity = g + Ks*Phong(Nv, Lv, Ev, (float)shininess);

        if ( intensity > 255.0 ) {
                intensity = 0.0;
        } else {
                if (intensity < 0.0)
                        intensity = 255.0;
                else
                        intensity = 255.0 - intensity;
        }
        
        
        /*printf("wetness %d colour %d intensity %f guraud %f phong %f\n",
                        cell->contents.liquid_content,
                                cell->contents.colour.hue,
                                intensity,
                                g,
                                intensity - g);*/
 
 
return (intensity);
}


/* *********************************************************************** */

void single_step()
/*  This routine defines the paint steps involved in the
    basic cycle of the painting engine.  */

{
   POINT     locus;
   CELL_PTR  cell;
   BOOLEAN   done;

   next_cell_point(&locus);
   cell = get_cell(locus);
   
   done = age_paint(cell);
   if (done == TRUE) return;

   done = diffuse_paint(cell, locus);
   if (done == TRUE) return;

   done = apply_gravity(cell, locus);
}

brush_stroke(x,y)
int x;
int y;
{
   POINT pnt;
   CELL_PTR cell;


	pnt.x = x;
	pnt.y = y;

	cell = get_cell(pnt);

         cell->contents.liquid_content = 100;
	   cell->contents.drying_rate = 10;
	   cell->contents.miscibility = 80;

	   cell->contents.colour.hue = 128;
	   cell->contents.colour.saturation = 1.0;
	   cell->contents.colour.lightness = 0.0;

	   cell->volume = 50;
	
}

