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

#ifndef NUBASIS_H
#define NUBASIS_H

#include "nugrid.h"
//#include "config.h"

typedef struct
{
  NUgrid* restrict grid;
  // xVals is just the grid points, augmented by two extra points on
  // either side.  These are necessary to generate enough basis
  // functions. 
  double* restrict xVals;
  // dxInv[3*i+j] = 1.0/(grid(i+j-1)-grid(i-2))
  double* restrict dxInv;
  bool periodic;
} NUBasis;


#ifdef __cplusplus
extern "C" {
#endif

/////////////////
// Constructor //
/////////////////
NUBasis*
create_NUBasis (NUgrid *grid, bool periodic);

////////////////
// Destructor //
////////////////
void
destroy_NUBasis (NUBasis *basis);


////////////////////////////////////////////////
// Single-precision basis function evaluation //
////////////////////////////////////////////////
int
get_NUBasis_funcs_s (NUBasis* restrict basis, double x,
		     float bfuncs[4]);
void
get_NUBasis_funcs_si (NUBasis* restrict basis, int i,
		      float bfuncs[4]);

int
get_NUBasis_dfuncs_s (NUBasis* restrict basis, double x,
		      float bfuncs[4], float dbfuncs[4]);
void
get_NUBasis_dfuncs_si (NUBasis* restrict basis, int i,
		       float bfuncs[4], float dbfuncs[4]);

int
get_NUBasis_d2funcs_s (NUBasis* restrict basis, double x,
		       float bfuncs[4], float dbfuncs[4], float d2bfuncs[4]);
void
get_NUBasis_d2funcs_si (NUBasis* restrict basis, int i,
			float bfuncs[4], float dbfuncs[4], float d2bfuncs[4]);

////////////////////////////////////////////////
// Double-precision basis function evaluation //
////////////////////////////////////////////////
int
get_NUBasis_funcs_d (NUBasis* restrict basis, double x,
		     double bfuncs[4]);
void
get_NUBasis_funcs_di (NUBasis* restrict basis, int i,
		      double bfuncs[4]);
int
get_NUBasis_dfuncs_d (NUBasis* restrict basis, double x,
		      double bfuncs[4], double dbfuncs[4]);
void
get_NUBasis_dfuncs_di (NUBasis* restrict basis, int i,
		       double bfuncs[4], double dbfuncs[4]);
int
get_NUBasis_d2funcs_d (NUBasis* restrict basis, double x,
		       double bfuncs[4], double dbfuncs[4], 
		       double d2bfuncs[4]);
void
get_NUBasis_d2funcs_di (NUBasis* restrict basis, int i,
			double bfuncs[4], double dbfuncs[4], 
			double d2bfuncs[4]);
#ifdef __cplusplus
}
#endif

#ifdef HAVE_SSE2
#include <xmmintrin.h>
#include <emmintrin.h>

#ifdef __cplusplus
extern "C" {
#endif
int
get_NUBasis_funcs_sse_s (NUBasis* restrict basis, double x,
			 __m128 *restrict funcs);
int
get_NUBasis_dfuncs_sse_s (NUBasis* restrict basis, double x,
			  __m128 *restrict funcs, __m128 *restrict dfuncs);
int
get_NUBasis_d2funcs_sse_s (NUBasis* restrict basis, double x,
			   __m128 *restrict funcs, 
			   __m128 *restrict dfuncs, 
			   __m128 *restrict d2funcs);

int
get_NUBasis_funcs_sse_d (NUBasis* restrict basis, double x,
			 __m128d *restrict f01, __m128d *restrict f23);
int
get_NUBasis_dfuncs_sse_d (NUBasis* restrict basis, double x,
			  __m128d *restrict   f01, __m128d *restrict   f23,
			  __m128d *restrict  df01, __m128d *restrict  df23);
int
get_NUBasis_d2funcs_sse_d (NUBasis* restrict basis, double x,
			   __m128d *restrict   f01, __m128d *restrict   f23,
			   __m128d *restrict  df01, __m128d *restrict  df23,
			   __m128d *restrict d2f01, __m128d *restrict d2f23);
#ifdef __cplusplus
}
#endif
#endif // #ifdef HAVE_SSE2

#endif // #ifdef NUBASIS_H
