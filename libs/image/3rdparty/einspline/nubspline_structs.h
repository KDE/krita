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

#ifndef NUBSPLINE_STRUCTS_H
#define NUBSPLINE_STRUCTS_H

#include "bspline_base.h"
#include "nubasis.h"

typedef struct
{
  spline_code sp_code;
  type_code    t_code;
  void * restrict coefs;
  NUgrid *restrict  x_grid;
  NUBasis *restrict x_basis;
} NUBspline_1d;

typedef struct
{
  spline_code sp_code;
  type_code    t_code;
  void * restrict coefs;
  int x_stride;
  NUgrid *restrict  x_grid, *restrict y_grid;
  NUBasis *restrict x_basis, *restrict y_basis;
} NUBspline_2d;

typedef struct
{
  spline_code sp_code;
  type_code    t_code;
  void * restrict coefs;
  int x_stride, y_stride;
  NUgrid *restrict  x_grid, *restrict y_grid, *restrict z_grid;
  NUBasis *restrict x_basis, *restrict y_basis, *restrict z_basis;
} NUBspline_3d;


///////////////////////////
// Single precision real //
///////////////////////////
typedef struct
{
  spline_code sp_code;
  type_code    t_code;
  float* restrict coefs;
  NUgrid *restrict  x_grid;
  NUBasis *restrict x_basis;
  BCtype_s xBC;
} NUBspline_1d_s;

typedef struct
{
  spline_code sp_code;
  type_code    t_code;
  float* restrict coefs;
  int x_stride;
  NUgrid  *restrict x_grid,  *restrict y_grid;
  NUBasis *restrict x_basis, *restrict y_basis;
  BCtype_s xBC, yBC;
} NUBspline_2d_s;

typedef struct
{
  spline_code sp_code;
  type_code    t_code;
  float* restrict coefs;
  int x_stride, y_stride;
  NUgrid  *restrict x_grid,  *restrict y_grid,  *restrict z_grid;
  NUBasis *restrict x_basis, *restrict y_basis, *restrict z_basis;
  BCtype_s xBC, yBC, zBC;
} NUBspline_3d_s;

///////////////////////////
// Double precision real //
///////////////////////////
typedef struct
{
  spline_code sp_code;
  type_code    t_code;
  double* restrict coefs;
  NUgrid* restrict x_grid;
  NUBasis* restrict x_basis;
  BCtype_d xBC;
} NUBspline_1d_d;

typedef struct
{
  spline_code sp_code;
  type_code    t_code;
  double* restrict coefs;
  int x_stride;
  NUgrid * restrict x_grid, * restrict y_grid;
  NUBasis * restrict x_basis, * restrict y_basis;
  BCtype_d xBC, yBC;
} NUBspline_2d_d;

typedef struct
{
  spline_code sp_code;
  type_code    t_code;
  double* restrict coefs;
  int x_stride, y_stride;
  NUgrid  *restrict x_grid,  *restrict y_grid,  *restrict z_grid;
  NUBasis *restrict x_basis, *restrict y_basis, *restrict z_basis;
  BCtype_d xBC, yBC, zBC;
} NUBspline_3d_d;

//////////////////////////////
// Single precision complex //
//////////////////////////////
typedef struct
{
  spline_code sp_code;
  type_code    t_code;
  complex_float* restrict coefs;
  NUgrid* restrict x_grid;
  NUBasis* restrict x_basis;
  BCtype_c xBC;
} NUBspline_1d_c;

typedef struct
{
  spline_code sp_code;
  type_code    t_code;
  complex_float* restrict coefs;
  int x_stride;
  NUgrid* restrict x_grid, *restrict y_grid;
  NUBasis* restrict x_basis, *restrict y_basis;
  BCtype_c xBC, yBC;
} NUBspline_2d_c;

typedef struct
{
  spline_code sp_code;
  type_code    t_code;
  complex_float* restrict coefs;
  int x_stride, y_stride;
  NUgrid  *restrict x_grid,  *restrict y_grid,  *restrict z_grid;
  NUBasis *restrict x_basis, *restrict y_basis, *restrict z_basis;
  BCtype_c xBC, yBC, zBC;
} NUBspline_3d_c;

//////////////////////////////
// Double precision complex //
//////////////////////////////
typedef struct
{
  spline_code sp_code;
  type_code    t_code;
 complex_double* restrict coefs;
  NUgrid  *restrict x_grid;
  NUBasis *restrict x_basis;
  BCtype_z xBC;
} NUBspline_1d_z;

typedef struct
{
  spline_code sp_code;
  type_code    t_code;
  complex_double* restrict coefs;
  int x_stride;
  NUgrid  *restrict x_grid,  *restrict y_grid;
  NUBasis *restrict x_basis, *restrict y_basis;
  BCtype_z xBC, yBC;
} NUBspline_2d_z;

typedef struct
{
  spline_code sp_code;
  type_code    t_code;
  complex_double* restrict coefs;
  int x_stride, y_stride;
  NUgrid  *restrict x_grid,  *restrict y_grid,  *restrict z_grid;
  NUBasis *restrict x_basis, *restrict y_basis, *restrict z_basis;
  BCtype_z xBC, yBC, zBC;
} NUBspline_3d_z;

#endif
