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

#ifndef MULTI_BSPLINE_CREATE_H
#define MULTI_BSPLINE_CREATE_H

#include "bspline_base.h"
#include "multi_bspline_structs.h"

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
multi_UBspline_1d_s *
create_multi_UBspline_1d_s (Ugrid x_grid, BCtype_s xBC, int num_splines);

// Create 2D uniform single-precision, real Bspline
multi_UBspline_2d_s *
create_multi_UBspline_2d_s (Ugrid x_grid,   Ugrid y_grid,
			    BCtype_s   xBC, BCtype_s   yBC,
			    int num_splines);

// Create 3D uniform single-precision, real Bspline
multi_UBspline_3d_s *
create_multi_UBspline_3d_s (Ugrid x_grid,   Ugrid y_grid,   Ugrid z_grid,
			    BCtype_s  xBC,  BCtype_s   yBC, BCtype_s   zBC,
			    int num_splines);
  
// Set the data for the splines, and compute spline coefficients
void
set_multi_UBspline_1d_s (multi_UBspline_1d_s *spline, 
			 int spline_num, float *data);

void
set_multi_UBspline_2d_s (multi_UBspline_2d_s *spline, 
			 int spline_num, float *data);

void
set_multi_UBspline_3d_s (multi_UBspline_3d_s *spline, 
			 int spline_num, float *data);


/////////////////////////////////////
// Uniform, double precision, real //
/////////////////////////////////////
// Create 1D uniform single-precision, real Bspline
multi_UBspline_1d_d *
create_multi_UBspline_1d_d (Ugrid x_grid, BCtype_d xBC, int num_splines);

// Create 2D uniform single-precision, real Bspline
multi_UBspline_2d_d *
create_multi_UBspline_2d_d (Ugrid x_grid,   Ugrid y_grid,
			    BCtype_d   xBC, BCtype_d   yBC,
			    int num_splines);

// Create 3D uniform single-precision, real Bspline
multi_UBspline_3d_d *
create_multi_UBspline_3d_d (Ugrid x_grid,   Ugrid   y_grid,   Ugrid z_grid,
			    BCtype_d  xBC,  BCtype_d   yBC, BCtype_d   zBC,
			    int num_splines);

// Set the data for the splines, and compute spline coefficients
void
set_multi_UBspline_1d_d (multi_UBspline_1d_d *spline, 
			 int spline_num, double *data);
void
set_multi_UBspline_1d_d_BC (multi_UBspline_1d_d *spline, 
			    int spline_num, double *data, BCtype_d xBC);

void
set_multi_UBspline_2d_d (multi_UBspline_2d_d *spline, 
			 int spline_num, double *data);

void
set_multi_UBspline_3d_d (multi_UBspline_3d_d *spline, 
			 int spline_num, double *data);

///////////////////////////////////////
// Uniform, single precision, complex//
///////////////////////////////////////
// Create 1D uniform single-precision, real Bspline
multi_UBspline_1d_c *
create_multi_UBspline_1d_c (Ugrid x_grid, BCtype_c xBC, int num_splines);

// Create 2D uniform single-precision, real Bspline
multi_UBspline_2d_c *
create_multi_UBspline_2d_c (Ugrid   x_grid, Ugrid   y_grid,
			    BCtype_c   xBC, BCtype_c   yBC,
			    int num_splines);
  
// Create 3D uniform single-precision, real Bspline
multi_UBspline_3d_c *
create_multi_UBspline_3d_c (Ugrid  x_grid, Ugrid y_grid, Ugrid z_grid,
			    BCtype_c  xBC, BCtype_c yBC, BCtype_c zBC,
			    int num_splines);

// Set the data for the splines, and compute spline coefficients
void
set_multi_UBspline_1d_c (multi_UBspline_1d_c *spline, int spline_num, 
			 complex_float *data);

void
set_multi_UBspline_2d_c (multi_UBspline_2d_c *spline, int spline_num, 
			 complex_float *data);

void
set_multi_UBspline_3d_c (multi_UBspline_3d_c *spline, int spline_num, 
			 complex_float *data);

///////////////////////////////////////
// Uniform, double precision, complex//
///////////////////////////////////////
// Create 1D uniform double-precision, complex Bspline
multi_UBspline_1d_z *
create_multi_UBspline_1d_z (Ugrid x_grid, BCtype_z xBC, int num_splines);

// Create 2D uniform double-precision, complex Bspline
multi_UBspline_2d_z *
create_multi_UBspline_2d_z (Ugrid x_grid, Ugrid y_grid,
			    BCtype_z   xBC, BCtype_z   yBC,
			    int num_splines);

// Create 3D uniform double-precision, complex Bspline
multi_UBspline_3d_z *
create_multi_UBspline_3d_z (Ugrid  x_grid, Ugrid   y_grid, Ugrid z_grid,
			    BCtype_z  xBC, BCtype_z   yBC, BCtype_z zBC,
			    int num_splines);

// Set the data for the splines, and compute spline coefficients
void
set_multi_UBspline_1d_z (multi_UBspline_1d_z *spline, int spline_num, 
			 complex_double *data);
void
set_multi_UBspline_1d_z_BC (multi_UBspline_1d_z *spline, int spline_num, 
			    complex_double *data, BCtype_z xBC);


void
set_multi_UBspline_2d_z (multi_UBspline_2d_z *spline, int spline_num, 
			 complex_double *data);

void
set_multi_UBspline_3d_z (multi_UBspline_3d_z *spline, int spline_num, 
			 complex_double *data);

#ifdef __cplusplus
}
#endif

#endif
