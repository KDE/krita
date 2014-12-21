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

#ifndef NUBSPLINE_CREATE_H
#define NUBSPLINE_CREATE_H

#include "nubspline_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

NUgrid* 
create_center_grid (double start, double end, double ratio, int num_points);

NUgrid*
create_general_grid (double *points, int num_points);

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////       Nonuniform spline creation routines          ////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

////////////////////////////////////////
// Nonuniform, single precision, real //
////////////////////////////////////////
NUBspline_1d_s *
create_NUBspline_1d_s (NUgrid* x_grid, BCtype_s xBC, float *data);

NUBspline_2d_s *
create_NUBspline_2d_s (NUgrid* x_grid, NUgrid* y_grid,
		       BCtype_s xBC, BCtype_s yBC, float *data);

NUBspline_3d_s *
create_NUBspline_3d_s (NUgrid* x_grid, NUgrid* y_grid, NUgrid* z_grid,
		       BCtype_s xBC, BCtype_s yBC, BCtype_s zBC, float *data);

////////////////////////////////////////
// Nonuniform, double precision, real //
////////////////////////////////////////
NUBspline_1d_d *
create_NUBspline_1d_d (NUgrid* x_grid, BCtype_d xBC, double *data);

NUBspline_2d_d *
create_NUBspline_2d_d (NUgrid* x_grid, NUgrid* y_grid, 
		       BCtype_d xBC, BCtype_d yBC, double *data);

NUBspline_3d_d *
create_NUBspline_3d_d (NUgrid* x_grid, NUgrid* y_grid, NUgrid* z_grid, 
		       BCtype_d xBC, BCtype_d yBC, BCtype_d zBC, double *data);

///////////////////////////////////////////
// Nonuniform, single precision, complex //
///////////////////////////////////////////
NUBspline_1d_c *
create_NUBspline_1d_c (NUgrid* x_grid, BCtype_c xBC, 
		       complex_float *data);

NUBspline_2d_c *
create_NUBspline_2d_c (NUgrid* x_grid, NUgrid* y_grid, 
		       BCtype_c xBC, BCtype_c yBC, complex_float *data);

NUBspline_3d_c *
create_NUBspline_3d_c (NUgrid* x_grid, NUgrid* y_grid, NUgrid* z_grid, 
		       BCtype_c xBC, BCtype_c yBC, BCtype_c zBC, 
		       complex_float *data);

///////////////////////////////////////////
// Nonuniform, double precision, complex //
///////////////////////////////////////////
NUBspline_1d_z *
create_NUBspline_1d_z (NUgrid* x_grid, BCtype_z xBC, 
		       complex_double *data);
NUBspline_2d_z *
create_NUBspline_2d_z (NUgrid* x_grid, NUgrid* restrict y_grid, 
		       BCtype_z xBC, BCtype_z yBC, complex_double *data);

NUBspline_3d_z *
create_NUBspline_3d_z (NUgrid* x_grid, NUgrid* y_grid, NUgrid* z_grid, 
		       BCtype_z xBC, BCtype_z yBC, BCtype_z zBC, complex_double *data);

#ifdef __cplusplus
}
#endif
#endif
