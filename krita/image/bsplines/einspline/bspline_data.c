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

//#include "config.h"

/*****************
/*   SSE Data    */
/*****************/

#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif 

#define _XOPEN_SOURCE 600

#ifndef __USE_XOPEN2K
  #define __USE_XOPEN2K
#endif
#include <stdlib.h>

#ifdef HAVE_SSE
#include <xmmintrin.h>

// Single-precision version of matrices
__m128 *restrict A_s = (__m128 *)0;
// There is a problem with alignment of global variables in shared
// libraries on 32-bit machines.
// __m128  A0, A1, A2, A3, dA0, dA1, dA2, dA3, d2A0, d2A1, d2A2, d2A3;
#endif

#ifdef HAVE_SSE2
// Double-precision version of matrices
#include <emmintrin.h>
__m128d *restrict A_d = (__m128d *)0;

// There is a problem with alignment of global variables in shared
// libraries on 32-bit machines.
//__m128d A0_01, A0_23, A1_01, A1_23, A2_01, A2_23, A3_01, A3_23,
//  dA0_01, dA0_23, dA1_01, dA1_23, dA2_01, dA2_23, dA3_01, dA3_23,
//  d2A0_01, d2A0_23, d2A1_01, d2A1_23, d2A2_01, d2A2_23, d2A3_01, d2A3_23;
#endif 

void init_sse_data()
{
#ifdef HAVE_SSE
  if (A_s == 0) {
    posix_memalign ((void**)&A_s, 16, (sizeof(__m128)*12));
    A_s[0]  = _mm_setr_ps ( 1.0/6.0, -3.0/6.0,  3.0/6.0, -1.0/6.0 );
    A_s[0]  = _mm_setr_ps ( 1.0/6.0, -3.0/6.0,  3.0/6.0, -1.0/6.0 );	  
    A_s[1]  = _mm_setr_ps ( 4.0/6.0,  0.0/6.0, -6.0/6.0,  3.0/6.0 );	  
    A_s[2]  = _mm_setr_ps ( 1.0/6.0,  3.0/6.0,  3.0/6.0, -3.0/6.0 );	  
    A_s[3]  = _mm_setr_ps ( 0.0/6.0,  0.0/6.0,  0.0/6.0,  1.0/6.0 );	  
    A_s[4]  = _mm_setr_ps ( -0.5,  1.0, -0.5, 0.0  );		  
    A_s[5]  = _mm_setr_ps (  0.0, -2.0,  1.5, 0.0  );		  
    A_s[6]  = _mm_setr_ps (  0.5,  1.0, -1.5, 0.0  );		  
    A_s[7]  = _mm_setr_ps (  0.0,  0.0,  0.5, 0.0  );		  
    A_s[8]  = _mm_setr_ps (  1.0, -1.0,  0.0, 0.0  );		  
    A_s[9]  = _mm_setr_ps ( -2.0,  3.0,  0.0, 0.0  );		  
    A_s[10] = _mm_setr_ps (  1.0, -3.0,  0.0, 0.0  );		  
    A_s[11] = _mm_setr_ps (  0.0,  1.0,  0.0, 0.0  );                  
  }
                 
#endif
#ifdef HAVE_SSE2
  if (A_d == 0) {
    posix_memalign ((void**)&A_d, 16, (sizeof(__m128d)*24));
    A_d[ 0] = _mm_setr_pd (  3.0/6.0, -1.0/6.0 );	   
    A_d[ 1] = _mm_setr_pd (  1.0/6.0, -3.0/6.0 );	   
    A_d[ 2] = _mm_setr_pd ( -6.0/6.0,  3.0/6.0 );	   
    A_d[ 3] = _mm_setr_pd (  4.0/6.0,  0.0/6.0 );	   
    A_d[ 4] = _mm_setr_pd (  3.0/6.0, -3.0/6.0 );	   
    A_d[ 5] = _mm_setr_pd (  1.0/6.0,  3.0/6.0 );	   
    A_d[ 6] = _mm_setr_pd (  0.0/6.0,  1.0/6.0 );	   
    A_d[ 7] = _mm_setr_pd (  0.0/6.0,  0.0/6.0 );	   
    A_d[ 8] = _mm_setr_pd ( -0.5,  0.0 );		   
    A_d[ 9] = _mm_setr_pd ( -0.5,  1.0 );		   
    A_d[10] = _mm_setr_pd (  1.5,  0.0 );		   
    A_d[11] = _mm_setr_pd (  0.0, -2.0 );		   
    A_d[12] = _mm_setr_pd ( -1.5,  0.0 );		   
    A_d[13] = _mm_setr_pd (  0.5,  1.0 );		   
    A_d[14] = _mm_setr_pd (  0.5,  0.0 );		   
    A_d[15] = _mm_setr_pd (  0.0,  0.0 );		   
    A_d[16] = _mm_setr_pd (  0.0,  0.0 );		   
    A_d[17] = _mm_setr_pd (  1.0, -1.0 );		   
    A_d[18] = _mm_setr_pd (  0.0,  0.0 );		   
    A_d[19] = _mm_setr_pd ( -2.0,  3.0 );		   
    A_d[20] = _mm_setr_pd (  0.0,  0.0 );		   
    A_d[21] = _mm_setr_pd (  1.0, -3.0 );		   
    A_d[22] = _mm_setr_pd (  0.0,  0.0 );		   
    A_d[23] = _mm_setr_pd (  0.0,  1.0 );   
  }                
#endif
}


#ifdef USE_ALTIVEC
vector float A0   = (vector float) ( -1.0/6.0,  3.0/6.0, -3.0/6.0, 1.0/6.0);
vector float A1   = (vector float) (  3.0/6.0, -6.0/6.0,  0.0/6.0, 4.0/6.0);
vector float A2   = (vector float) ( -3.0/6.0,  3.0/6.0,  3.0/6.0, 1.0/6.0);
vector float A3   = (vector float) (  1.0/6.0,  0.0/6.0,  0.0/6.0, 0.0/6.0);
/* vector float A0   = (vector float) ( -1.0/6.0,  3.0/6.0, -3.0/6.0, 1.0/6.0); */
/* vector float A1   = (vector float) (  3.0/6.0, -6.0/6.0,  3.0/6.0, 0.0/6.0); */
/* vector float A2   = (vector float) ( -3.0/6.0,  0.0/6.0,  3.0/6.0, 0.0/6.0); */
/* vector float A3   = (vector float) (  1.0/6.0,  4.0/6.0,  1.0/6.0, 0.0/6.0); */
/* vector float A0   = (vector float) ( 1.0/6.0, -3.0/6.0,  3.0/6.0, -1.0/6.0); */
/* vector float A1   = (vector float) ( 4.0/6.0,  0.0/6.0, -6.0/6.0,  3.0/6.0); */
/* vector float A2   = (vector float) ( 1.0/6.0,  3.0/6.0,  3.0/6.0, -3.0/6.0); */
/* vector float A3   = (vector float) ( 0.0/6.0,  0.0/6.0,  0.0/6.0,  1.0/6.0); */
vector float dA0  = (vector float) ( 0.0, -0.5,  1.0, -0.5 );
vector float dA1  = (vector float) ( 0.0,  1.5, -2.0,  0.0 );
vector float dA2  = (vector float) ( 0.0, -1.5,  1.0,  0.5 );
vector float dA3  = (vector float) ( 0.0,  0.5,  0.0,  0.0 );
vector float d2A0 = (vector float) ( 0.0,  0.0, -1.0,  1.0 );
vector float d2A1 = (vector float) ( 0.0,  0.0,  3.0, -2.0 );
vector float d2A2 = (vector float) ( 0.0,  0.0, -3.0,  1.0 );
vector float d2A3 = (vector float) ( 0.0,  0.0,  1.0,  0.0 );
#endif

/*****************/
/* Standard Data */
/*****************/

//////////////////////
// Single precision //
//////////////////////
const float A44f[16] = 
  { -1.0/6.0,  3.0/6.0, -3.0/6.0, 1.0/6.0,
     3.0/6.0, -6.0/6.0,  0.0/6.0, 4.0/6.0,
    -3.0/6.0,  3.0/6.0,  3.0/6.0, 1.0/6.0,
     1.0/6.0,  0.0/6.0,  0.0/6.0, 0.0/6.0 };
const float* restrict Af = A44f;

const float dA44f[16] =
  {  0.0, -0.5,  1.0, -0.5,
     0.0,  1.5, -2.0,  0.0,
     0.0, -1.5,  1.0,  0.5,
     0.0,  0.5,  0.0,  0.0 };
const float* restrict dAf = dA44f;

const float d2A44f[16] = 
  {  0.0, 0.0, -1.0,  1.0,
     0.0, 0.0,  3.0, -2.0,
     0.0, 0.0, -3.0,  1.0,
     0.0, 0.0,  1.0,  0.0 };
const float* restrict d2Af = d2A44f;

const float d3A44f[16] =
  {  0.0, 0.0,  0.0, -1.0,
     0.0, 0.0,  0.0,  3.0,
     0.0, 0.0,  0.0, -3.0,
     0.0, 0.0,  0.0,  1.0};
const float* restrict d3Af = d3A44f;

//////////////////////
// Double precision //
//////////////////////
const double A44d[16] = 
  { -1.0/6.0,  3.0/6.0, -3.0/6.0, 1.0/6.0,
     3.0/6.0, -6.0/6.0,  0.0/6.0, 4.0/6.0,
    -3.0/6.0,  3.0/6.0,  3.0/6.0, 1.0/6.0,
     1.0/6.0,  0.0/6.0,  0.0/6.0, 0.0/6.0 };
const double* restrict Ad = A44d;

const double dA44d[16] =
  {  0.0, -0.5,  1.0, -0.5,
     0.0,  1.5, -2.0,  0.0,
     0.0, -1.5,  1.0,  0.5,
     0.0,  0.5,  0.0,  0.0 };
const double* restrict dAd = dA44d;

const double d2A44d[16] = 
  {  0.0, 0.0, -1.0,  1.0,
     0.0, 0.0,  3.0, -2.0,
     0.0, 0.0, -3.0,  1.0,
     0.0, 0.0,  1.0,  0.0 };
const double* restrict d2Ad = d2A44d;

const double d3A44d[16] =
  {  0.0, 0.0,  0.0, -1.0,
     0.0, 0.0,  0.0,  3.0,
     0.0, 0.0,  0.0, -3.0,
     0.0, 0.0,  0.0,  1.0};
const double* restrict d3Ad = d3A44d;
