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

#ifndef NUGRID_H
#define NUGRID_H

#include <stdbool.h>


typedef enum { LINEAR, GENERAL, CENTER, LOG } grid_type;

// Nonuniform grid base structure
typedef struct
{
  // public data
  grid_type code;
  double start, end;
  double* restrict points;
  int num_points;
  int (*reverse_map)(void *grid, double x);
} NUgrid;

#ifdef __cplusplus
extern "C" 
#endif


typedef struct
{
  // public data
  grid_type code;
  double start, end;
  double* restrict points;
  int num_points;
  int (*reverse_map)(void *grid, double x);

  // private data
  double a, aInv, b, bInv, center, even_half;
  int half_points, odd_one;
  bool odd;
} center_grid;


typedef struct
{
  // public data
  grid_type code;
  double start, end;
  double* restrict points;
  int num_points;
  int (*reverse_map)(void *grid, double x);

  // private data
  double a, ainv, startinv;
} log_grid;


#ifdef __cplusplus
extern "C" {
#endif

NUgrid*
create_center_grid (double start, double end, double ratio, 
		    int num_points);

NUgrid*
create_log_grid (double start, double end, int num_points);

NUgrid*
create_general_grid (double *points, int num_points);

void
destroy_grid (NUgrid *grid);

#ifdef __cplusplus
}
#endif
#endif
