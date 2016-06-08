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

#ifndef BSPLINE_BASE_H
#define BSPLINE_BASE_H

//#include "config.h"
#include "local_definitions.h"

#ifdef __cplusplus
#include <complex>
typedef std::complex<float>  complex_float;
typedef std::complex<double> complex_double;
#else
#include <complex.h>
typedef complex float  complex_float;
typedef complex double complex_double;
#endif

// Conventions:
// Postfixes:  
// s:  single precision real
// d:  double precision real
// c:  single precision complex
// z:  double precision complex

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////              Basic type declarations               ////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

typedef enum { PERIODIC, DERIV1, DERIV2, FLAT, NATURAL, ANTIPERIODIC } bc_code;
typedef enum { U1D       , U2D       , U3D      , 
	       NU1D      , NU2D      , NU3D     ,
               MULTI_U1D , MULTI_U2D , MULTI_U3D,
               MULTI_NU1D, MULTI_NU2D, MULTI_NU3D } spline_code;
typedef enum { SINGLE_REAL, DOUBLE_REAL, SINGLE_COMPLEX, DOUBLE_COMPLEX }
  type_code;

typedef struct 
{
  bc_code lCode, rCode;
  float lVal, rVal;
} BCtype_s;

typedef struct 
{
  bc_code lCode, rCode;
  double lVal, rVal;
} BCtype_d;

typedef struct 
{
  bc_code lCode, rCode;
  float lVal_r, lVal_i, rVal_r, rVal_i;
} BCtype_c;

typedef struct 
{
  bc_code lCode, rCode;
  double lVal_r, lVal_i, rVal_r, rVal_i;
} BCtype_z;


typedef struct
{
  double start, end;
  int num;

  // private
  double delta, delta_inv;
} Ugrid;

typedef struct
{
  spline_code sp_code;
  type_code   t_code;
  void *restrict coefs;
} Bspline;

#ifdef __cplusplus 
extern "C" 
#endif
void
destroy_Bspline (void *spline);

#endif
