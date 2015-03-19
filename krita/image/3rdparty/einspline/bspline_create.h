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

#ifndef BSPLINE_CREATE_H
#define BSPLINE_CREATE_H

#include "bspline_base.h"
#include "bspline_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////              Spline creation functions             ////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/////////////////////////////////////
// Uniform, single precision, real //
/////////////////////////////////////
// Create 1D uniform single-precision, real Bspline
UBspline_1d_s *
create_UBspline_1d_s (Ugrid x_grid, BCtype_s xBC, float *data);

// Create 2D uniform single-precision, real Bspline
UBspline_2d_s *
create_UBspline_2d_s (Ugrid x_grid,   Ugrid y_grid,
		      BCtype_s   xBC, BCtype_s   yBC,
		      float *data);

// Create 3D uniform single-precision, real Bspline
UBspline_3d_s *
create_UBspline_3d_s (Ugrid x_grid,   Ugrid y_grid,   Ugrid z_grid,
		      BCtype_s  xBC,  BCtype_s   yBC, BCtype_s   zBC,
		      float *data);

void
recompute_UBspline_1d_s (UBspline_1d_s* spline, float *data);

void
recompute_UBspline_2d_s (UBspline_2d_s* spline, float *data);

void
recompute_UBspline_3d_s (UBspline_3d_s* spline, float *data);

/////////////////////////////////////
// Uniform, double precision, real //
/////////////////////////////////////
// Create 1D uniform single-precision, real Bspline
UBspline_1d_d *
create_UBspline_1d_d (Ugrid x_grid, BCtype_d xBC, double *data);

// Create 2D uniform single-precision, real Bspline
UBspline_2d_d *
create_UBspline_2d_d (Ugrid x_grid,   Ugrid y_grid,
		      BCtype_d   xBC, BCtype_d   yBC,
		      double *data);

// Create 3D uniform single-precision, real Bspline
UBspline_3d_d *
create_UBspline_3d_d (Ugrid x_grid,   Ugrid   y_grid,   Ugrid z_grid,
		      BCtype_d  xBC,  BCtype_d   yBC, BCtype_d   zBC,
		      double *data);

void
recompute_UBspline_1d_d (UBspline_1d_d* spline, double *data);

void
recompute_UBspline_2d_d (UBspline_2d_d* spline, double *data);

void
recompute_UBspline_3d_d (UBspline_3d_d* spline, double *data);

///////////////////////////////////////
// Uniform, single precision, complex//
///////////////////////////////////////
// Create 1D uniform single-precision, real Bspline
UBspline_1d_c *
create_UBspline_1d_c (Ugrid x_grid, BCtype_c xBC, complex_float *data);

// Create 2D uniform single-precision, real Bspline
UBspline_2d_c *
create_UBspline_2d_c (Ugrid   x_grid, Ugrid   y_grid,
		      BCtype_c   xBC, BCtype_c   yBC,
		      complex_float *data);

// Create 3D uniform single-precision, real Bspline
UBspline_3d_c *
create_UBspline_3d_c (Ugrid  x_grid, Ugrid y_grid, Ugrid z_grid,
		      BCtype_c  xBC, BCtype_c yBC, BCtype_c zBC,
		      complex_float *data);

void
recompute_UBspline_1d_c (UBspline_1d_c* spline, complex_float *data);

void
recompute_UBspline_2d_c (UBspline_2d_c* spline, complex_float *data);

void
recompute_UBspline_3d_c (UBspline_3d_c* spline, complex_float *data);
 
///////////////////////////////////////
// Uniform, double precision, complex//
///////////////////////////////////////
// Create 1D uniform double-precision, complex Bspline
UBspline_1d_z *
create_UBspline_1d_z (Ugrid x_grid, BCtype_z xBC, complex_double *data);

// Create 2D uniform double-precision, complex Bspline
UBspline_2d_z *
create_UBspline_2d_z (Ugrid x_grid, Ugrid y_grid,
		      BCtype_z   xBC, BCtype_z   yBC,
		      complex_double *data);

// Create 3D uniform double-precision, complex Bspline
UBspline_3d_z *
create_UBspline_3d_z (Ugrid  x_grid, Ugrid   y_grid, Ugrid z_grid,
		      BCtype_z  xBC, BCtype_z   yBC, BCtype_z zBC,
		      complex_double *data);

void
recompute_UBspline_1d_z (UBspline_1d_z* spline, complex_double *data);

void
recompute_UBspline_2d_z (UBspline_2d_z* spline, complex_double *data);

void
recompute_UBspline_3d_z (UBspline_3d_z* spline, complex_double *data);

#ifdef __cplusplus
}
#endif

#endif
