/////////////////////////////////////////////////////////////////////////////
//  einspline:  a library for creating and evaluating B-splines            //
//  Copyright (C) 2007 Kenneth P. Esler, Jr.                               //
//                                                                         //
//  This program is free software; you can redistribute it and/or modify   //
//  it under the terms of the GNU General Public License as published by   //
//  the Free Software Foundation; either version 2 of the License, or      //
//  (at your option) any later version.                                    //
//                                                                         //
//  This program is distributed in the hope that it will be useful,        //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of         //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          //
//  GNU General Public License for more details.                           //
//                                                                         //
//  You should have received a copy of the GNU General Public License      //
//  along with this program; if not, write to the Free Software            //
//  Foundation, Inc., 51 Franklin Street, Fifth Floor,                     //
//  Boston, MA  02110-1301  USA                                            //
/////////////////////////////////////////////////////////////////////////////

#include "nugrid.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>

int
center_grid_reverse_map (void* gridptr, double x)
{
  center_grid *grid = (center_grid *)gridptr;

  x -= grid->center;
  double index = 
    copysign (log1p(fabs(x)*grid->aInv)*grid->bInv, x);
  return (int)floor(grid->half_points + index - grid->even_half);
}

int
log_grid_reverse_map (void *gridptr, double x)
{
  log_grid *grid = (log_grid *)gridptr;
  
  int index = (int) floor(grid->ainv*log(x*grid->startinv));

  if (index < 0)
    return 0;
  else
    return index;
}


int
general_grid_reverse_map (void* gridptr, double x)
{
  NUgrid* grid = (NUgrid*) gridptr;
  int N = grid->num_points;
  double *points = grid->points;
  if (x <= points[0])
    return (0);
  else if (x >= points[N-1])
    return (N-1);
  else {
    int hi = N-1;
    int lo = 0;
    bool done = false;
    while (!done) {
      int i = (hi+lo)>>1;
      if (points[i] > x)
	hi = i;
      else
	lo = i;
      done = (hi-lo)<2;
    }
    return (lo);
  }
}

NUgrid*
create_center_grid (double start, double end, double ratio, 
		    int num_points)
{
  center_grid *grid = malloc (sizeof (center_grid));
  if (grid != NULL) {
    assert (ratio > 1.0);
    grid->start       = start;
    grid->end         = end;
    grid->center      = 0.5*(start + end);
    grid->num_points  = num_points;
    grid->half_points = num_points/2;
    grid->odd = ((num_points % 2) == 1);
    grid->b = log(ratio) / (double)(grid->half_points-1);
    grid->bInv = 1.0/grid->b;
    grid->points = malloc (num_points * sizeof(double));
    if (grid->odd) {
      grid->even_half = 0.0;  
      grid->odd_one   = 1;
      grid->a = 0.5*(end-start)/expm1(grid->b*grid->half_points);
      grid->aInv = 1.0/grid->a;
      for (int i=-grid->half_points; i<=grid->half_points; i++) {
	double sign;
	if (i<0) 
	  sign = -1.0;
	else
	  sign =  1.0;
	grid->points[i+grid->half_points] = 
	  sign * grid->a*expm1(grid->b*abs(i))+grid->center;
      }
    }
    else {
      grid->even_half = 0.5;  
      grid->odd_one   = 0;
      grid->a = 
	0.5*(end-start)/expm1(grid->b*(-0.5+grid->half_points));
      grid->aInv = 1.0/grid->a;
      for (int i=-grid->half_points; i<grid->half_points; i++) {
	double sign;
	if (i<0) sign = -1.0; 
	else     sign =  1.0;
	grid->points[i+grid->half_points] = 
	  sign * grid->a*expm1(grid->b*fabs(0.5+i)) + grid->center;
      }
    }
    grid->reverse_map = center_grid_reverse_map;
    grid->code = CENTER;
  }
  return (NUgrid*) grid;
}


NUgrid*
create_log_grid (double start, double end,
		 int num_points)
{
  log_grid *grid = malloc (sizeof (log_grid));
  grid->code = LOG;
  grid->start = start;
  grid->end = end;
  grid->num_points = num_points;
  grid->points = malloc(num_points*sizeof(double));
  grid->a = 1.0/(double)(num_points-1)*log(end/start);
  grid->ainv = 1.0/grid->a;
  grid->startinv = 1.0/start;
  for (int i=0; i<num_points; i++)
    grid->points[i] = start*exp(grid->a*(double)i);
  grid->reverse_map = log_grid_reverse_map;
  return (NUgrid*) grid;
}


NUgrid*
create_general_grid (double *points, int num_points)
{
  NUgrid* grid = malloc (sizeof(NUgrid));
  if (grid != NULL) {
    grid->reverse_map = general_grid_reverse_map;
    grid->code = GENERAL;
    grid->points = malloc (num_points*sizeof(double));
    grid->start = points[0];
    grid->end   = points[num_points-1];
    grid->num_points = num_points;
    for (int i=0; i<num_points; i++) 
      grid->points[i] = points[i];
    grid->code = GENERAL;
  }
  return grid;
}

void
destroy_grid (NUgrid *grid)
{
  free (grid->points);
  free (grid);
}
