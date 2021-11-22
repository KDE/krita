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

#include "multi_bspline_create.h"
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif
#ifndef __USE_XOPEN2K
  #define __USE_XOPEN2K
#endif
#include <stdlib.h>
#include <stdio.h>

int posix_memalign(void **memptr, size_t alignment, size_t size);

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////       Helper functions for spline creation         ////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
void init_sse_data();

void
find_coefs_1d_d (Ugrid grid, BCtype_d bc, 
		 double *data,  intptr_t dstride,
		 double *coefs, intptr_t cstride);

void 
solve_deriv_interp_1d_s (float bands[], float coefs[],
			 int M, int cstride);

// On input, bands should be filled with:
// row 0   :  abcdInitial from boundary conditions
// rows 1:M:  basis functions in first 3 cols, data in last
// row M+1 :  abcdFinal   from boundary conditions
// cstride gives the stride between values in coefs.
// On exit, coefs with contain interpolating B-spline coefs
void 
solve_periodic_interp_1d_s (float bands[], float coefs[],
			    int M, int cstride);

// On input, bands should be filled with:
// row 0   :  abcdInitial from boundary conditions
// rows 1:M:  basis functions in first 3 cols, data in last
// row M+1 :  abcdFinal   from boundary conditions
// cstride gives the stride between values in coefs.
// On exit, coefs with contain interpolating B-spline coefs
void 
solve_antiperiodic_interp_1d_s (float bands[], float coefs[],
				int M, int cstride);

void
find_coefs_1d_s (Ugrid grid, BCtype_s bc, 
		 float *data,  intptr_t dstride,
		 float *coefs, intptr_t cstride);

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////     Single-Precision, Real Creation Routines       ////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

// On input, bands should be filled with:
// row 0   :  abcdInitial from boundary conditions
// rows 1:M:  basis functions in first 3 cols, data in last
// row M+1 :  abcdFinal   from boundary conditions
// cstride gives the stride between values in coefs.
// On exit, coefs with contain interpolating B-spline coefs
multi_UBspline_1d_s*
create_multi_UBspline_1d_s (Ugrid x_grid, BCtype_s xBC, int num_splines)
{
  // Create new spline
  multi_UBspline_1d_s* restrict spline = static_cast<multi_UBspline_1d_s*>(malloc(sizeof(multi_UBspline_1d_s)));
  if (!spline) {
    fprintf (stderr, "Out of memory allocating spline in create_multi_UBspline_1d_s.\n");
    abort();
  }
  spline->spcode = MULTI_U1D;
  spline->tcode  = SINGLE_REAL;
  spline->xBC = xBC; spline->x_grid = x_grid;
  spline->num_splines = num_splines;

  // Setup internal variables
  int Mx = x_grid.num;
  int Nx;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC) {
    x_grid.delta     = (x_grid.end-x_grid.start)/(double)(x_grid.num);
    Nx = Mx+3;
  }
  else {
    x_grid.delta     = (x_grid.end-x_grid.start)/(double)(x_grid.num-1);
    Nx = Mx+2;
  }

  int N = num_splines;
#ifdef HAVE_SSE
  if (N % 4) 
    N += 4 - (N % 4);
#endif 

  spline->x_stride = N;
  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;
#ifndef HAVE_POSIX_MEMALIGN
  spline->coefs = (float*)malloc (sizeof(float)*Nx*N);
#else
  posix_memalign ((void**)&spline->coefs, 64, (sizeof(float)*Nx*N));
#endif
#ifdef HAVE_SSE
  init_sse_data();    
#endif
  if (!spline->coefs) {
    fprintf (stderr, "Out of memory allocating spline coefficient in create_multi_UBspline_1d_s.\n");
    abort();
  }


  return spline;
}

void
set_multi_UBspline_1d_s (multi_UBspline_1d_s *spline, int num,
			 float *data)
{
  float *coefs = spline->coefs + num;
  int xs = spline->x_stride;
  find_coefs_1d_s (spline->x_grid, spline->xBC, data, 1, 
		   coefs, xs);
}


multi_UBspline_2d_s*
create_multi_UBspline_2d_s (Ugrid x_grid, Ugrid y_grid,
			    BCtype_s xBC, BCtype_s yBC, int num_splines)
{
  // Create new spline
  multi_UBspline_2d_s* restrict spline = static_cast<multi_UBspline_2d_s*>(malloc(sizeof(multi_UBspline_2d_s)));
  if (!spline) {
    fprintf (stderr, "Out of memory allocating spline in create_multi_UBspline_2d_s.\n");
    abort();
  }
  spline->spcode = MULTI_U2D;
  spline->tcode  = SINGLE_REAL;
  spline->xBC = xBC; 
  spline->yBC = yBC; 
  spline->num_splines = num_splines;
  // Setup internal variables
  int Mx = x_grid.num;
  int My = y_grid.num;
  int Nx, Ny;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC)     
    Nx = Mx+3;
  else                           
    Nx = Mx+2;
  x_grid.delta = (x_grid.end - x_grid.start)/(double)(Nx-3);

  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;

  if (yBC.lCode == PERIODIC || yBC.lCode == ANTIPERIODIC)     
    Ny = My+3;
  else                           
    Ny = My+2;
  y_grid.delta = (y_grid.end - y_grid.start)/(double)(Ny-3);
  y_grid.delta_inv = 1.0/y_grid.delta;
  spline->y_grid   = y_grid;

  int N = num_splines;
#ifdef HAVE_SSE
  if (N % 4) 
    N += 4 - (N % 4);
#endif


  spline->x_stride = Ny*N;
  spline->y_stride = N;

#ifndef HAVE_POSIX_MEMALIGN
  spline->coefs = (float*)malloc (sizeof(float)*Nx*Ny*N);
#else
  posix_memalign ((void**)&spline->coefs, 64, 
		  sizeof(float)*Nx*Ny*N);
#endif
#ifdef HAVE_SSE
  init_sse_data();
#endif
  if (!spline->coefs) {
    fprintf (stderr, "Out of memory allocating spline coefficients in create_multi_UBspline_2d_s.\n");
    abort();
  }

  return spline;
}

void
set_multi_UBspline_2d_s (multi_UBspline_2d_s* spline, int num, float *data)
{
  int Mx = spline->x_grid.num;
  int My = spline->y_grid.num;
  int Nx, Ny;

  if (spline->xBC.lCode == PERIODIC || spline->xBC.lCode == ANTIPERIODIC)     
    Nx = Mx+3;
  else                                   
    Nx = Mx+2;
  if (spline->yBC.lCode == PERIODIC || spline->yBC.lCode == ANTIPERIODIC)     
    Ny = My+3;
  else                                   
    Ny = My+2;

  float *coefs = spline->coefs + num;
  int ys = spline->y_stride;
  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) {
    intptr_t doffset = iy;
    intptr_t coffset = iy*ys;
    find_coefs_1d_s (spline->x_grid, spline->xBC, data+doffset, (intptr_t)My,
		     coefs+coffset, (intptr_t)Ny*ys);
  }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) {
    intptr_t doffset = ix*Ny*ys;
    intptr_t coffset = ix*Ny*ys;
    find_coefs_1d_s (spline->y_grid, spline->yBC, coefs+doffset, (intptr_t)ys, 
		     coefs+coffset, (intptr_t)ys);
  }
}


multi_UBspline_3d_s*
create_multi_UBspline_3d_s (Ugrid x_grid, Ugrid y_grid, Ugrid z_grid,
			    BCtype_s xBC, BCtype_s yBC, BCtype_s zBC,
			    int num_splines)
{
  // Create new spline
  multi_UBspline_3d_s* restrict spline = static_cast<multi_UBspline_3d_s*>(malloc(sizeof(multi_UBspline_3d_s)));
  if (!spline) {
    fprintf (stderr, "Out of memory allocating spline in create_multi_UBspline_3d_s.\n");
    abort();
  }
  spline->spcode = MULTI_U3D;
  spline->tcode  = SINGLE_REAL;
  spline->xBC = xBC; 
  spline->yBC = yBC; 
  spline->zBC = zBC; 
  spline->num_splines = num_splines;
  // Setup internal variables
  int Mx = x_grid.num;  int My = y_grid.num; int Mz = z_grid.num;
  int Nx, Ny, Nz;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC)     
    Nx = Mx+3;
  else                           
    Nx = Mx+2;
  x_grid.delta = (x_grid.end - x_grid.start)/(double)(Nx-3);
  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;

  if (yBC.lCode == PERIODIC || yBC.lCode == ANTIPERIODIC)     
    Ny = My+3;
  else                           
    Ny = My+2;
  y_grid.delta = (y_grid.end - y_grid.start)/(double)(Ny-3);
  y_grid.delta_inv = 1.0/y_grid.delta;
  spline->y_grid   = y_grid;

  if (zBC.lCode == PERIODIC || zBC.lCode == ANTIPERIODIC)     
    Nz = Mz+3;
  else                           
    Nz = Mz+2;
  z_grid.delta = (z_grid.end - z_grid.start)/(double)(Nz-3);
  z_grid.delta_inv = 1.0/z_grid.delta;
  spline->z_grid   = z_grid;

  int N = num_splines;
#ifdef HAVE_SSE
  if (N % 4) 
    N += 4 - (N % 4);
#endif

  spline->x_stride      = Ny*Nz*N;
  spline->y_stride      = Nz*N;
  spline->z_stride      = N;

#ifndef HAVE_POSIX_MEMALIGN
  spline->coefs      = new float[Nx*Ny*Nz*N];
#else
  posix_memalign ((void**)&spline->coefs, 64, 
		  (sizeof(float)*Nx*Ny*Nz*N));
#endif
#ifdef HAVE_SSE
  init_sse_data();
#endif
  if (!spline->coefs) {
    fprintf (stderr, "Out of memory allocating spline coefficients in create_multi_UBspline_3d_s.\n");
    abort();
  }

  return spline;
}

void
set_multi_UBspline_3d_s (multi_UBspline_3d_s* spline, int num, float *data)
{
  int Mx = spline->x_grid.num;
  int My = spline->y_grid.num;
  int Mz = spline->z_grid.num;
  int Nx, Ny, Nz;

  if (spline->xBC.lCode == PERIODIC || spline->xBC.lCode == ANTIPERIODIC)     
    Nx = Mx+3;
  else                                   
    Nx = Mx+2;
  if (spline->yBC.lCode == PERIODIC || spline->yBC.lCode == ANTIPERIODIC)     
    Ny = My+3;
  else                                   
    Ny = My+2;
  if (spline->zBC.lCode == PERIODIC || spline->zBC.lCode == ANTIPERIODIC)     
    Nz = Mz+3;
  else                                   
    Nz = Mz+2;

  float *coefs = spline->coefs + num;

  int zs = spline->z_stride;
  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) 
    for (int iz=0; iz<Mz; iz++) {
      intptr_t doffset = iy*Mz+iz;
      intptr_t coffset = (iy*Nz+iz)*zs;
      find_coefs_1d_s (spline->x_grid, spline->xBC, data+doffset, (intptr_t)My*Mz,
		       coefs+coffset, (intptr_t)Ny*Nz*zs);
    }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iz=0; iz<Nz; iz++) {
      intptr_t doffset = (ix*Ny*Nz + iz)*zs;
      intptr_t coffset = (ix*Ny*Nz + iz)*zs;
      find_coefs_1d_s (spline->y_grid, spline->yBC, coefs+doffset, (intptr_t)Nz*zs, 
		       coefs+coffset, (intptr_t)Nz*zs);
    }

  // Now, solve in the Z-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iy=0; iy<Ny; iy++) {
      intptr_t doffset = ((ix*Ny+iy)*Nz)*zs;
      intptr_t coffset = ((ix*Ny+iy)*Nz)*zs;
      find_coefs_1d_s (spline->z_grid, spline->zBC, coefs+doffset, 
		       (intptr_t)zs, coefs+coffset, (intptr_t)zs);
    }
}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////    Single-Precision, Complex Creation Routines     ////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

// On input, bands should be filled with:
// row 0   :  abcdInitial from boundary conditions
// rows 1:M:  basis functions in first 3 cols, data in last
// row M+1 :  abcdFinal   from boundary conditions
// cstride gives the stride between values in coefs.
// On exit, coefs with contain interpolating B-spline coefs
multi_UBspline_1d_c*
create_multi_UBspline_1d_c (Ugrid x_grid, BCtype_c xBC, int num_splines)
{
  // Create new spline
  multi_UBspline_1d_c* restrict spline = static_cast<multi_UBspline_1d_c*>(malloc(sizeof(multi_UBspline_1d_c)));
  if (!spline) {
    fprintf (stderr, "Out of memory allocating spline in create_multi_UBspline_1d_c.\n");
    abort();
  }
  spline->spcode = MULTI_U1D;
  spline->tcode  = SINGLE_COMPLEX;
  spline->xBC = xBC; 
  spline->num_splines = num_splines;
  // Setup internal variables
  int M = x_grid.num;
  int N;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC) {
    x_grid.delta     = (x_grid.end-x_grid.start)/(double)(x_grid.num);
    N = M+3;
  }
  else {
    x_grid.delta     = (x_grid.end-x_grid.start)/(double)(x_grid.num-1);
    N = M+2;
  }

  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;
  spline->x_stride = num_splines;
#ifndef HAVE_POSIX_MEMALIGN
  spline->coefs = (complex_float*)malloc (2*sizeof(float)*N*num_splines);
#else
  posix_memalign ((void**)&spline->coefs, 64, 2*sizeof(float)*N*num_splines);
#endif
#ifdef HAVE_SSE
  init_sse_data();    
#endif
  if (!spline->coefs) {
    fprintf (stderr, "Out of memory allocating spline coefficients in create_multi_UBspline_1d_c.\n");
    abort();
  }

  return spline;
}

void
set_multi_UBspline_1d_c (multi_UBspline_1d_c* spline, int num, complex_float *data)
{
  complex_float *coefs = spline->coefs + num;

  BCtype_s xBC_r, xBC_i;
  xBC_r.lCode = spline->xBC.lCode;  xBC_r.rCode = spline->xBC.rCode;
  xBC_r.lVal  = spline->xBC.lVal_r; xBC_r.rVal  = spline->xBC.rVal_r;
  xBC_i.lCode = spline->xBC.lCode;  xBC_i.rCode = spline->xBC.rCode;
  xBC_i.lVal  = spline->xBC.lVal_i; xBC_i.rVal  = spline->xBC.rVal_i;

  int xs = spline->x_stride;
  // Real part
  find_coefs_1d_s (spline->x_grid, xBC_r, 
		   (float*)data, (intptr_t)2, (float*)coefs, (intptr_t)2*xs);
  // Imaginarty part
  find_coefs_1d_s (spline->x_grid, xBC_i, 
		   ((float*)data)+1, (intptr_t)2, ((float*)coefs+1), (intptr_t)2*xs);
}



multi_UBspline_2d_c*
create_multi_UBspline_2d_c (Ugrid x_grid, Ugrid y_grid,
			    BCtype_c xBC, BCtype_c yBC, int num_splines)
{
  // Create new spline
  multi_UBspline_2d_c* restrict spline = static_cast<multi_UBspline_2d_c*>(malloc(sizeof(multi_UBspline_2d_c)));
  if (!spline) {
    fprintf (stderr, "Out of memory allocating spline in create_multi_UBspline_2d_c.\n");
    abort();
  }
  spline->spcode = MULTI_U2D;
  spline->tcode  = SINGLE_COMPLEX;
  spline->xBC = xBC; 
  spline->yBC = yBC;
  spline->num_splines = num_splines;

  // Setup internal variables
  int Mx = x_grid.num;
  int My = y_grid.num;
  int Nx, Ny;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC)     
    Nx = Mx+3;
  else                           
    Nx = Mx+2;
  x_grid.delta = (x_grid.end - x_grid.start)/(double)(Nx-3);
  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;

  if (yBC.lCode == PERIODIC || yBC.lCode == ANTIPERIODIC)     
    Ny = My+3;
  else                           
    Ny = My+2;
  y_grid.delta = (y_grid.end - y_grid.start)/(double)(Ny-3);
  y_grid.delta_inv = 1.0/y_grid.delta;
  spline->y_grid   = y_grid;

  int N = num_splines;
#ifdef HAVE_SSE
  if (N % 2)
    N++;
#endif

  spline->x_stride = Ny*N;
  spline->y_stride = N;

#ifndef HAVE_POSIX_MEMALIGN
  spline->coefs = (complex_float*)malloc (2*sizeof(float)*Nx*Ny*N);
  spline->lapl2 = (complex_float*)malloc (4*sizeof(float)*N);
#else
  posix_memalign ((void**)&spline->coefs, 64, 
		  2*sizeof(float)*Nx*Ny*N);
  posix_memalign ((void**)&spline->lapl2, 64,
		  4*sizeof(float)*N);
#endif
#ifdef HAVE_SSE
  init_sse_data();
#endif
  if (!spline->coefs || !spline->lapl2) {
    fprintf (stderr, "Out of memory allocating spline coefficients in create_multi_UBspline_2d_c.\n");
    abort();
  }
  return spline;
}


void
set_multi_UBspline_2d_c (multi_UBspline_2d_c* spline, int num, complex_float *data)
{
  // Setup internal variables
  int Mx = spline->x_grid.num;
  int My = spline->y_grid.num;
  int Nx, Ny;

  complex_float* coefs = spline->coefs + num;

  if (spline->xBC.lCode == PERIODIC || spline->xBC.lCode == ANTIPERIODIC)     
    Nx = Mx+3;
  else                                   
    Nx = Mx+2;
  if (spline->yBC.lCode == PERIODIC || spline->yBC.lCode == ANTIPERIODIC)     
    Ny = My+3;
  else                                   
    Ny = My+2;

  BCtype_s xBC_r, xBC_i, yBC_r, yBC_i;
  xBC_r.lCode = spline->xBC.lCode;  xBC_r.rCode = spline->xBC.rCode;
  xBC_r.lVal  = spline->xBC.lVal_r; xBC_r.rVal  = spline->xBC.rVal_r;
  xBC_i.lCode = spline->xBC.lCode;  xBC_i.rCode = spline->xBC.rCode;
  xBC_i.lVal  = spline->xBC.lVal_i; xBC_i.rVal  = spline->xBC.rVal_i;
  yBC_r.lCode = spline->yBC.lCode;  yBC_r.rCode = spline->yBC.rCode;
  yBC_r.lVal  = spline->yBC.lVal_r; yBC_r.rVal  = spline->yBC.rVal_r;
  yBC_i.lCode = spline->yBC.lCode;  yBC_i.rCode = spline->yBC.rCode;
  yBC_i.lVal  = spline->yBC.lVal_i; yBC_i.rVal  = spline->yBC.rVal_i;
 
  int ys = spline->y_stride;
  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) {
    intptr_t doffset = (2*iy);
    intptr_t coffset = (2*iy)*ys;
    // Real part
    find_coefs_1d_s (spline->x_grid, xBC_r, ((float*)data)+doffset, (intptr_t)2*My,
		     (float*)coefs+coffset, (intptr_t)2*Ny*ys);
    // Imag part
    find_coefs_1d_s (spline->x_grid, xBC_i, ((float*)data)+doffset+1, (intptr_t)2*My,
		     ((float*)coefs)+coffset+1, (intptr_t)2*Ny*ys);
  }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) {
    intptr_t doffset = (2*ix*Ny)*ys;
    intptr_t coffset = (2*ix*Ny)*ys;
    // Real part
    find_coefs_1d_s (spline->y_grid, yBC_r, ((float*)coefs)+doffset, 
		     (intptr_t)2*ys, ((float*)coefs)+coffset, (intptr_t)2*ys);
    // Imag part
    find_coefs_1d_s (spline->y_grid, yBC_i, ((float*)coefs)+doffset+1, 
		     (intptr_t)2*ys, ((float*)coefs)+coffset+1, (intptr_t)2*ys);
  }  
}

multi_UBspline_3d_c*
create_multi_UBspline_3d_c (Ugrid x_grid, Ugrid y_grid, Ugrid z_grid,
		      BCtype_c xBC, BCtype_c yBC, BCtype_c zBC,
		      int num_splines)
{
  // Create new spline
  multi_UBspline_3d_c* restrict spline = static_cast<multi_UBspline_3d_c*>(malloc(sizeof(multi_UBspline_3d_c)));
  if (!spline) {
    fprintf (stderr, "Out of memory allocating spline in create_multi_UBspline_3d_c.\n");
    abort();
  }
  spline->spcode = MULTI_U3D;
  spline->tcode  = SINGLE_COMPLEX;
  spline->xBC = xBC; 
  spline->yBC = yBC; 
  spline->zBC = zBC; 
  spline->num_splines = num_splines;

  // Setup internal variables
  int Mx = x_grid.num;  int My = y_grid.num; int Mz = z_grid.num;
  int Nx, Ny, Nz;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC)     
    Nx = Mx+3;
  else                           
    Nx = Mx+2;
  x_grid.delta = (x_grid.end - x_grid.start)/(double)(Nx-3);
  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;

  if (yBC.lCode == PERIODIC || yBC.lCode == ANTIPERIODIC)     
    Ny = My+3;
  else                           
    Ny = My+2;
  y_grid.delta = (y_grid.end - y_grid.start)/(double)(Ny-3);
  y_grid.delta_inv = 1.0/y_grid.delta;
  spline->y_grid   = y_grid;

  if (zBC.lCode == PERIODIC || zBC.lCode == ANTIPERIODIC)     
    Nz = Mz+3;
  else                           
    Nz = Mz+2;
  z_grid.delta = (z_grid.end - z_grid.start)/(double)(Nz-3);
  z_grid.delta_inv = 1.0/z_grid.delta;
  spline->z_grid   = z_grid;

  int N = spline->num_splines;
#ifdef HAVE_SSE
  if (N % 2)
    N++;
#endif

  spline->x_stride = Ny*Nz*N;
  spline->y_stride = Nz*N;
  spline->z_stride = N;

#ifndef HAVE_POSIX_MEMALIGN
  spline->coefs = (complex_float*)malloc (2*sizeof(float)*Nx*Ny*Nz*N);
  spline->lapl3 = (complex_float*)malloc (6*sizeof(float)*N);
#else
  posix_memalign ((void**)&spline->coefs, 64, 2*sizeof(float)*Nx*Ny*Nz*N);
  posix_memalign ((void**)&spline->lapl3, 64, 6*sizeof(float)*N);  
#endif
#ifdef HAVE_SSE
  init_sse_data();
#endif
  if (!spline->coefs || !spline->lapl3) {
    fprintf (stderr, "Out of memory allocating spline coefficients in create_multi_UBspline_3d_c.\n");
    abort();
  }

  return spline;
}

void
set_multi_UBspline_3d_c (multi_UBspline_3d_c* spline, int num, complex_float *data)
{
  // Setup internal variables
  int Mx = spline->x_grid.num;
  int My = spline->y_grid.num;
  int Mz = spline->z_grid.num;
  int Nx, Ny, Nz;

  if (spline->xBC.lCode == PERIODIC || spline->xBC.lCode == ANTIPERIODIC)     
    Nx = Mx+3;
  else                           
    Nx = Mx+2;
  if (spline->yBC.lCode == PERIODIC || spline->yBC.lCode == ANTIPERIODIC)     
    Ny = My+3;
  else                           
    Ny = My+2;
  if (spline->zBC.lCode == PERIODIC || spline->zBC.lCode == ANTIPERIODIC)     
    Nz = Mz+3;
  else                          
    Nz = Mz+2;

  BCtype_s xBC_r, xBC_i, yBC_r, yBC_i, zBC_r, zBC_i;
  xBC_r.lCode = spline->xBC.lCode;  xBC_r.rCode = spline->xBC.rCode;
  xBC_r.lVal  = spline->xBC.lVal_r; xBC_r.rVal  = spline->xBC.rVal_r;
  xBC_i.lCode = spline->xBC.lCode;  xBC_i.rCode = spline->xBC.rCode;
  xBC_i.lVal  = spline->xBC.lVal_i; xBC_i.rVal  = spline->xBC.rVal_i;
  yBC_r.lCode = spline->yBC.lCode;  yBC_r.rCode = spline->yBC.rCode;
  yBC_r.lVal  = spline->yBC.lVal_r; yBC_r.rVal  = spline->yBC.rVal_r;
  yBC_i.lCode = spline->yBC.lCode;  yBC_i.rCode = spline->yBC.rCode;
  yBC_i.lVal  = spline->yBC.lVal_i; yBC_i.rVal  = spline->yBC.rVal_i;
  zBC_r.lCode = spline->zBC.lCode;  zBC_r.rCode = spline->zBC.rCode;
  zBC_r.lVal  = spline->zBC.lVal_r; zBC_r.rVal  = spline->zBC.rVal_r;
  zBC_i.lCode = spline->zBC.lCode;  zBC_i.rCode = spline->zBC.rCode;
  zBC_i.lVal  = spline->zBC.lVal_i; zBC_i.rVal  = spline->zBC.rVal_i;

  complex_float *coefs = spline->coefs + num;
  int zs = spline->z_stride;
  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) 
    for (int iz=0; iz<Mz; iz++) {
      intptr_t doffset = 2*(iy*Mz+iz);
      intptr_t coffset = 2*(iy*Nz+iz)*zs;
      // Real part
      find_coefs_1d_s (spline->x_grid, xBC_r, 
		       ((float*)data)+doffset,  (intptr_t)2*My*Mz,
		       ((float*)coefs)+coffset, (intptr_t)2*Ny*Nz*zs);
      // Imag part
      find_coefs_1d_s (spline->x_grid, xBC_i, 
		       ((float*)data)+doffset+1,  (intptr_t)2*My*Mz,
		       ((float*)coefs)+coffset+1, (intptr_t)2*Ny*Nz*zs);
    }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iz=0; iz<Nz; iz++) {
      intptr_t doffset = 2*(ix*Ny*Nz + iz)*zs;
      intptr_t coffset = 2*(ix*Ny*Nz + iz)*zs;
      // Real part
      find_coefs_1d_s (spline->y_grid, yBC_r, 
		       ((float*)coefs)+doffset, (intptr_t)2*Nz*zs, 
		       ((float*)coefs)+coffset, (intptr_t)2*Nz*zs);
      // Imag part
      find_coefs_1d_s (spline->y_grid, yBC_i, 
		       ((float*)coefs)+doffset+1, (intptr_t)2*Nz*zs, 
		       ((float*)coefs)+coffset+1, (intptr_t)2*Nz*zs);
    }

  // Now, solve in the Z-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iy=0; iy<Ny; iy++) {
      intptr_t doffset = 2*((ix*Ny+iy)*Nz)*zs;
      intptr_t coffset = 2*((ix*Ny+iy)*Nz)*zs;
      // Real part
      find_coefs_1d_s (spline->z_grid, zBC_r, 
		       ((float*)coefs)+doffset, (intptr_t)2*zs, 
		       ((float*)coefs)+coffset, (intptr_t)2*zs);
      // Imag part
      find_coefs_1d_s (spline->z_grid, zBC_i, 
		       ((float*)coefs)+doffset+1, (intptr_t)2*zs, 
		       ((float*)coefs)+coffset+1, (intptr_t)2*zs);
    }
}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////     Double-Precision, Real Creation Routines       ////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

// On input, bands should be filled with:
// row 0   :  abcdInitial from boundary conditions
// rows 1:M:  basis functions in first 3 cols, data in last
// row M+1 :  abcdFinal   from boundary conditions
// cstride gives the stride between values in coefs.
// On exit, coefs with contain interpolating B-spline coefs
void 
solve_deriv_interp_1d_d (double bands[], double coefs[],
			 int M, int cstride);

// On input, bands should be filled with:
// row 0   :  abcdInitial from boundary conditions
// rows 1:M:  basis functions in first 3 cols, data in last
// row M+1 :  abcdFinal   from boundary conditions
// cstride gives the stride between values in coefs.
// On exit, coefs with contain interpolating B-spline coefs
void 
solve_periodic_interp_1d_d (double bands[], double coefs[],
			    int M, int cstride);

void
find_coefs_1d_d (Ugrid grid, BCtype_d bc, 
		 double *data,  intptr_t dstride,
		 double *coefs, intptr_t cstride);

multi_UBspline_1d_d*
create_multi_UBspline_1d_d (Ugrid x_grid, BCtype_d xBC, int num_splines)
{
  // Create new spline
  multi_UBspline_1d_d* restrict spline = static_cast<multi_UBspline_1d_d*>(malloc(sizeof(multi_UBspline_1d_d)));
  if (!spline) {
    fprintf (stderr, "Out of memory allocating spline in create_multi_UBspline_1d_d.\n");
    abort();
  }
  spline->spcode = MULTI_U1D;
  spline->tcode  = DOUBLE_REAL;
  spline->xBC = xBC; 
  spline->num_splines = num_splines;

  // Setup internal variables
  int Mx = x_grid.num;
  int Nx;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC) {
    x_grid.delta     = (x_grid.end-x_grid.start)/(double)(x_grid.num);
    Nx = Mx+3;
  }
  else {
    x_grid.delta     = (x_grid.end-x_grid.start)/(double)(x_grid.num-1);
    Nx = Mx+2;
  }

  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;

  int N = num_splines;
#ifdef HAVE_SSE2
  // We must pad to keep data aligned for SSE operations
  if (N & 1)
    N++;
#endif
  spline->x_stride = N;

#ifndef HAVE_POSIX_MEMALIGN
  spline->coefs = (double*)malloc (sizeof(double)*Nx*N);

#else
  posix_memalign ((void**)&spline->coefs, 64, sizeof(double)*Nx*N);
#endif
#ifdef HAVE_SSE2
  init_sse_data();
#endif
  if (!spline->coefs) {
    fprintf (stderr, "Out of memory allocating spline coefficients in create_multi_UBspline_1d_d.\n");
    abort();
  }

  return spline;
}

void
set_multi_UBspline_1d_d (multi_UBspline_1d_d* spline, int num, double *data)
{
  double *coefs = spline->coefs + num;
  int xs = spline->x_stride;
  find_coefs_1d_d (spline->x_grid, spline->xBC, data, 1, coefs, xs);
}

void
set_multi_UBspline_1d_d_BC (multi_UBspline_1d_d* spline, int num, double *data,
			    BCtype_d xBC)
{
  double *coefs = spline->coefs + num;
  int xs = spline->x_stride;
  find_coefs_1d_d (spline->x_grid, xBC, data, 1, coefs, xs);
}


multi_UBspline_2d_d*
create_multi_UBspline_2d_d (Ugrid x_grid, Ugrid y_grid,
			    BCtype_d xBC, BCtype_d yBC, int num_splines)
{
  // Create new spline
  multi_UBspline_2d_d* restrict spline = static_cast<multi_UBspline_2d_d*>(malloc(sizeof(multi_UBspline_2d_d)));
  if (!spline) {
    fprintf (stderr, "Out of memory allocating spline in create_multi_UBspline_2d_d.\n");
    abort();
  }
  spline->spcode = MULTI_U2D;
  spline->tcode  = DOUBLE_REAL;
  spline->xBC = xBC; 
  spline->yBC = yBC; 
  spline->num_splines = num_splines;
 
  // Setup internal variables
  int Mx = x_grid.num;
  int My = y_grid.num;
  int Nx, Ny;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC)     
    Nx = Mx+3;
  else                           
    Nx = Mx+2;
  x_grid.delta = (x_grid.end - x_grid.start)/(double)(Nx-3);
  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;

  if (yBC.lCode == PERIODIC || yBC.lCode == ANTIPERIODIC)     
    Ny = My+3;
  else                           
    Ny = My+2;
  y_grid.delta = (y_grid.end - y_grid.start)/(double)(Ny-3);
  y_grid.delta_inv = 1.0/y_grid.delta;
  spline->y_grid   = y_grid;

  int N = num_splines;

#ifdef HAVE_SSE2
  // We must pad to keep data align for SSE operations
  if (num_splines & 1)
    N++;
#endif
  spline->x_stride = Ny*N;
  spline->y_stride = N;

#ifndef HAVE_POSIX_MEMALIGN
  spline->coefs = (double*)malloc (sizeof(double)*Nx*Ny*N);
#else
  posix_memalign ((void**)&spline->coefs, 64, (sizeof(double)*Nx*Ny*N));
#endif
#ifdef HAVE_SSE2
  init_sse_data();
#endif
  if (!spline->coefs) {
    fprintf (stderr, "Out of memory allocating spline coefficients in create_multi_UBspline_2d_d.\n");
    abort();
  }

  return spline;
}

void
set_multi_UBspline_2d_d (multi_UBspline_2d_d* spline, int num, double *data)
{
  int Mx = spline->x_grid.num;
  int My = spline->y_grid.num;
  int Nx, Ny;
  double *coefs = spline->coefs + num;

  if (spline->xBC.lCode == PERIODIC || spline->xBC.lCode == ANTIPERIODIC)     
    Nx = Mx+3;
  else                                   
    Nx = Mx+2;

  if (spline->yBC.lCode == PERIODIC || spline->yBC.lCode == ANTIPERIODIC)     
    Ny = My+3;
  else                                   
    Ny = My+2;

  int ys = spline->y_stride;
  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) {
    intptr_t doffset = iy;
    intptr_t coffset = iy*ys;
    find_coefs_1d_d (spline->x_grid, spline->xBC, 
		     data+doffset,  (intptr_t)My,
		     coefs+coffset, (intptr_t)Ny*ys);
  }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) {
    intptr_t doffset = ix*Ny*ys;
    intptr_t coffset = ix*Ny*ys;
    find_coefs_1d_d (spline->y_grid, spline->yBC, 
		     coefs+doffset, (intptr_t)ys, 
		     coefs+coffset, (intptr_t)ys);
  }
}


multi_UBspline_3d_d*
create_multi_UBspline_3d_d (Ugrid x_grid, Ugrid y_grid, Ugrid z_grid,
			    BCtype_d xBC, BCtype_d yBC, BCtype_d zBC,
			    int num_splines)
{
  // Create new spline
  multi_UBspline_3d_d* restrict spline;
#ifdef HAVE_POSIX_MEMALIGN
  posix_memalign ((void**)&spline, 64, (size_t)sizeof(multi_UBspline_3d_d));
#else
  spline = static_cast<multi_UBspline_3d_d*>(malloc(sizeof(multi_UBspline_3d_d)));
#endif
  if (!spline) {
    fprintf (stderr, "Out of memory allocating spline in create_multi_UBspline_3d_d.\n");
    abort();
  }
  spline->spcode = MULTI_U3D;
  spline->tcode  = DOUBLE_REAL;
  spline->xBC = xBC; 
  spline->yBC = yBC; 
  spline->zBC = zBC; 
  spline->num_splines = num_splines;

  // Setup internal variables
  int Mx = x_grid.num;  int My = y_grid.num; int Mz = z_grid.num;
  int Nx, Ny, Nz;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC)     
    Nx = Mx+3;
  else                           
    Nx = Mx+2;
  x_grid.delta = (x_grid.end - x_grid.start)/(double)(Nx-3);
  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;

  if (yBC.lCode == PERIODIC || yBC.lCode == ANTIPERIODIC)     
    Ny = My+3;
  else                           
    Ny = My+2;
  y_grid.delta = (y_grid.end - y_grid.start)/(double)(Ny-3);
  y_grid.delta_inv = 1.0/y_grid.delta;
  spline->y_grid   = y_grid;

  if (zBC.lCode == PERIODIC || zBC.lCode == ANTIPERIODIC)     
    Nz = Mz+3;
  else                           
    Nz = Mz+2;
  z_grid.delta = (z_grid.end - z_grid.start)/(double)(Nz-3);
  z_grid.delta_inv = 1.0/z_grid.delta;
  spline->z_grid   = z_grid;


  int N = num_splines;
#if defined HAVE_SSE2 || defined HAVE_VSX
  // We must pad to keep data align for SSE operations
  if (N & 1)
    N++;
#endif
  
  spline->x_stride = Ny*Nz*N;
  spline->y_stride = Nz*N;
  spline->z_stride = N;
  
#ifdef HAVE_POSIX_MEMALIGN
  posix_memalign ((void**)&spline->coefs, 64, ((size_t)sizeof(double)*Nx*Ny*Nz*N));
#else
  spline->coefs      = new double[Nx*Ny*Nz*N];
#endif
#ifdef HAVE_SSE2
  init_sse_data();
#endif
  if (!spline->coefs) {
    fprintf (stderr, "Out of memory allocating spline coefficients in create_multi_UBspline_3d_d.\n");
    abort();
  }

  return spline;
}

void
set_multi_UBspline_3d_d (multi_UBspline_3d_d* spline, int num, double *data)
{
  int Mx = spline->x_grid.num;  
  int My = spline->y_grid.num; 
  int Mz = spline->z_grid.num;
  int Nx, Ny, Nz;

  if (spline->xBC.lCode == PERIODIC || spline->xBC.lCode == ANTIPERIODIC)     
    Nx = Mx+3;
  else                                   
    Nx = Mx+2;
  if (spline->yBC.lCode == PERIODIC || spline->yBC.lCode == ANTIPERIODIC)     
    Ny = My+3;
  else                                   
    Ny = My+2;
  if (spline->zBC.lCode == PERIODIC || spline->zBC.lCode == ANTIPERIODIC)     
    Nz = Mz+3;
  else                                   
    Nz = Mz+2;

  double *coefs = spline->coefs + num;
  intptr_t zs = spline->z_stride;

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) 
    for (int iz=0; iz<Mz; iz++) {
      intptr_t doffset = iy*Mz+iz;
      intptr_t coffset = (iy*Nz+iz)*zs;
      find_coefs_1d_d (spline->x_grid, spline->xBC, 
		       data+doffset,  (intptr_t)My*Mz,
		       coefs+coffset, (intptr_t)Ny*Nz*zs);
    }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iz=0; iz<Nz; iz++) {
      intptr_t doffset = (ix*Ny*Nz + iz)*zs;
      intptr_t coffset = (ix*Ny*Nz + iz)*zs;
      find_coefs_1d_d (spline->y_grid, spline->yBC, 
		       coefs+doffset, (intptr_t)Nz*zs, 
		       coefs+coffset, (intptr_t)Nz*zs);
    }

  // Now, solve in the Z-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iy=0; iy<Ny; iy++) {
      intptr_t doffset = (ix*Ny+iy)*Nz*zs;
      intptr_t coffset = (ix*Ny+iy)*Nz*zs;
      find_coefs_1d_d (spline->z_grid, spline->zBC, 
		       coefs+doffset, (intptr_t)zs, 
		       coefs+coffset, (intptr_t)zs);
    }
}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////    Double-Precision, Complex Creation Routines     ////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

// On input, bands should be filled with:
// row 0   :  abcdInitial from boundary conditions
// rows 1:M:  basis functions in first 3 cols, data in last
// row M+1 :  abcdFinal   from boundary conditions
// cstride gives the stride between values in coefs.
// On exit, coefs with contain interpolating B-spline coefs


multi_UBspline_1d_z*
create_multi_UBspline_1d_z (Ugrid x_grid, BCtype_z xBC, int num_splines)
{
  // Create new spline
  multi_UBspline_1d_z* restrict spline = static_cast<multi_UBspline_1d_z*>(malloc(sizeof(multi_UBspline_1d_z)));
  if (!spline) {
    fprintf (stderr, "Out of memory allocating spline in create_multi_UBspline_1d_z.\n");
    abort();
  }
  spline->spcode = MULTI_U1D;
  spline->tcode  = DOUBLE_COMPLEX;
  spline->xBC = xBC; 
  spline->num_splines = num_splines;

  // Setup internal variables
  int Mx = x_grid.num;
  int Nx;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC) {
    x_grid.delta     = (x_grid.end-x_grid.start)/(double)(x_grid.num);
    Nx = Mx+3;
  }
  else {
    x_grid.delta     = (x_grid.end-x_grid.start)/(double)(x_grid.num-1);
    Nx = Mx+2;
  }

  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;
  spline->x_stride = num_splines;

#ifndef HAVE_POSIX_MEMALIGN
  spline->coefs = (complex_double*)malloc (2*sizeof(double)*Nx*num_splines);
#else
  posix_memalign ((void**)&spline->coefs, 64, 2*sizeof(double)*Nx*num_splines);
#endif
#ifdef HAVE_SSE2
  init_sse_data();   
#endif
   if (!spline->coefs) {
    fprintf (stderr, "Out of memory allocating spline coefficients in create_multi_UBspline_1d_z.\n");
    abort();
  }

  return spline;
}

void
set_multi_UBspline_1d_z (multi_UBspline_1d_z* spline, int num, complex_double *data)
{
//  int Mx = spline->x_grid.num;
// Set but not used
//  int Nx;

  complex_double *coefs = spline->coefs + num;

//  if (spline->xBC.lCode == PERIODIC || spline->xBC.lCode == ANTIPERIODIC)
//    Nx = Mx+3;
//  else
//    Nx = Mx+2;

  BCtype_d xBC_r, xBC_i;
  xBC_r.lCode = spline->xBC.lCode;  xBC_r.rCode = spline->xBC.rCode;
  xBC_r.lVal  = spline->xBC.lVal_r; xBC_r.rVal  = spline->xBC.rVal_r;
  xBC_i.lCode = spline->xBC.lCode;  xBC_i.rCode = spline->xBC.rCode;
  xBC_i.lVal  = spline->xBC.lVal_i; xBC_i.rVal  = spline->xBC.rVal_i;
  int xs = spline->x_stride;
  // Real part
  find_coefs_1d_d (spline->x_grid, xBC_r, 
		   (double*)data,      (intptr_t)2, 
		   ((double*)coefs),   (intptr_t)2*xs);
  // Imaginary part
  find_coefs_1d_d (spline->x_grid, xBC_i, 
		   ((double*)data)+1,  (intptr_t)2, 
		   ((double*)coefs)+1, (intptr_t)2*xs);
 
}

void
set_multi_UBspline_1d_z_BC (multi_UBspline_1d_z *spline, int num, 
			    complex_double *data, BCtype_z xBC)
{
//  int Mx = spline->x_grid.num;
//  int Nx;

  complex_double *coefs = spline->coefs + num;

//  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC)
//    Nx = Mx+3;
//  else
//    Nx = Mx+2;

  BCtype_d xBC_r, xBC_i;
  xBC_r.lCode = xBC.lCode;  xBC_r.rCode = xBC.rCode;
  xBC_r.lVal  = xBC.lVal_r; xBC_r.rVal  = xBC.rVal_r;
  xBC_i.lCode = xBC.lCode;  xBC_i.rCode = xBC.rCode;
  xBC_i.lVal  = xBC.lVal_i; xBC_i.rVal  = xBC.rVal_i;
  int xs = spline->x_stride;
  // Real part
  find_coefs_1d_d (spline->x_grid, xBC_r, 
		   (double*)data,    (intptr_t)2, 
		   ((double*)coefs), (intptr_t)2*xs);
  // Imaginary part
  find_coefs_1d_d (spline->x_grid, xBC_i, 
		   ((double*)data)+1,  (intptr_t)2, 
		   ((double*)coefs)+1, (intptr_t)2*xs);
}


multi_UBspline_2d_z*
create_multi_UBspline_2d_z (Ugrid x_grid, Ugrid y_grid,
		      BCtype_z xBC, BCtype_z yBC, int num_splines)
{
  // Create new spline
  multi_UBspline_2d_z* restrict spline = static_cast<multi_UBspline_2d_z*>(malloc(sizeof(multi_UBspline_2d_z)));
  if (!spline) {
    fprintf (stderr, "Out of memory allocating spline in create_multi_UBspline_2d_z.\n");
    abort();
  }
  spline->spcode = MULTI_U2D;
  spline->tcode  = DOUBLE_COMPLEX;
  spline->xBC = xBC; 
  spline->yBC = yBC;
  spline->num_splines = num_splines;

  // Setup internal variables
  int Mx = x_grid.num;
  int My = y_grid.num;
  int Nx, Ny;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC)     
    Nx = Mx+3;
  else                           
    Nx = Mx+2;
  x_grid.delta = (x_grid.end - x_grid.start)/(double)(Nx-3);
  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;

  if (yBC.lCode == PERIODIC || yBC.lCode == ANTIPERIODIC)     
    Ny = My+3;
  else                           
    Ny = My+2;
  y_grid.delta = (y_grid.end - y_grid.start)/(double)(Ny-3);
  y_grid.delta_inv = 1.0/y_grid.delta;
  spline->y_grid   = y_grid;
  spline->x_stride = Ny*num_splines;
  spline->y_stride = num_splines;

#ifndef HAVE_POSIX_MEMALIGN
  spline->coefs = (complex_double*)malloc (2*sizeof(double)*Nx*Ny*num_splines);
  spline->lapl2 = (complex_double*)malloc (4*sizeof(double)*num_splines);
#else
  posix_memalign ((void**)&spline->coefs, 64, 2*sizeof(double)*Nx*Ny*num_splines);
  posix_memalign ((void**)&spline->lapl2, 64, 4*sizeof(double)*num_splines);
#endif
#ifdef HAVE_SSE2
  init_sse_data();
#endif
  if (!spline->coefs || !spline->lapl2) {
    fprintf (stderr, "Out of memory allocating spline coefficients in create_multi_UBspline_2d_z.\n");
    abort();
  }

  return spline;
}


void
set_multi_UBspline_2d_z (multi_UBspline_2d_z* spline, int num,
			 complex_double *data)
{
  int Mx = spline->x_grid.num;
  int My = spline->y_grid.num;
  int Nx, Ny;

  if (spline->xBC.lCode == PERIODIC || spline->xBC.lCode == ANTIPERIODIC)     
    Nx = Mx+3;
  else                           
    Nx = Mx+2;
  if (spline->yBC.lCode == PERIODIC || spline->yBC.lCode == ANTIPERIODIC)     
    Ny = My+3;
  else                           
    Ny = My+2;

  BCtype_d xBC_r, xBC_i, yBC_r, yBC_i;
  xBC_r.lCode = spline->xBC.lCode;  xBC_r.rCode = spline->xBC.rCode;
  xBC_r.lVal  = spline->xBC.lVal_r; xBC_r.rVal  = spline->xBC.rVal_r;
  xBC_i.lCode = spline->xBC.lCode;  xBC_i.rCode = spline->xBC.rCode;
  xBC_i.lVal  = spline->xBC.lVal_i; xBC_i.rVal  = spline->xBC.rVal_i;
  yBC_r.lCode = spline->yBC.lCode;  yBC_r.rCode = spline->yBC.rCode;
  yBC_r.lVal  = spline->yBC.lVal_r; yBC_r.rVal  = spline->yBC.rVal_r;
  yBC_i.lCode = spline->yBC.lCode;  yBC_i.rCode = spline->yBC.rCode;
  yBC_i.lVal  = spline->yBC.lVal_i; yBC_i.rVal  = spline->yBC.rVal_i;

  complex_double *coefs = spline->coefs + num;
  int ys = spline->y_stride;

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) {
    intptr_t doffset = 2*iy;
    intptr_t coffset = 2*iy*ys;
    // Real part
    find_coefs_1d_d (spline->x_grid, xBC_r, 
		     ((double*)data+doffset), (intptr_t)2*My, 
		     (double*)coefs+coffset,  (intptr_t)2*Ny*ys);
    // Imag part
    find_coefs_1d_d (spline->x_grid, xBC_i, 
		     ((double*)data)+doffset+1,  (intptr_t)2*My, 
		     ((double*)coefs)+coffset+1, (intptr_t)2*Ny*ys);
  }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) {
    intptr_t doffset = 2*ix*Ny*ys;
    intptr_t coffset = 2*ix*Ny*ys;
    // Real part
    find_coefs_1d_d (spline->y_grid, yBC_r, 
		     ((double*)coefs)+doffset, (intptr_t)2*ys, 
		     (double*)coefs+coffset,   (intptr_t)2*ys);
    // Imag part
    find_coefs_1d_d (spline->y_grid, yBC_i, 
		     (double*)coefs+doffset+1,   (intptr_t)2*ys, 
		     ((double*)coefs)+coffset+1, (intptr_t)2*ys);
  }
}



multi_UBspline_3d_z*
create_multi_UBspline_3d_z (Ugrid x_grid, Ugrid y_grid, Ugrid z_grid,
			    BCtype_z xBC, BCtype_z yBC, BCtype_z zBC,
			    int num_splines)
{
  // Create new spline
  multi_UBspline_3d_z* restrict spline = static_cast<multi_UBspline_3d_z*>(malloc(sizeof(multi_UBspline_3d_z)));
  if (!spline) {
    fprintf (stderr, "Out of memory allocating spline in create_multi_UBspline_3d_z.\n");
    abort();
  }
  spline->spcode = MULTI_U3D;
  spline->tcode  = DOUBLE_COMPLEX;
  spline->xBC = xBC; 
  spline->yBC = yBC; 
  spline->zBC = zBC;
  spline->num_splines = num_splines;

  // Setup internal variables
  int Mx = x_grid.num;  int My = y_grid.num; int Mz = z_grid.num;
  int Nx, Ny, Nz;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC)     
    Nx = Mx+3;
  else                           
    Nx = Mx+2;
  x_grid.delta = (x_grid.end - x_grid.start)/(double)(Nx-3);
  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;

  if (yBC.lCode == PERIODIC || yBC.lCode == ANTIPERIODIC)     
    Ny = My+3;
  else                           
    Ny = My+2;
  y_grid.delta = (y_grid.end - y_grid.start)/(double)(Ny-3);
  y_grid.delta_inv = 1.0/y_grid.delta;
  spline->y_grid   = y_grid;

  if (zBC.lCode == PERIODIC || zBC.lCode == ANTIPERIODIC)     
    Nz = Mz+3;
  else                           
    Nz = Mz+2;
  z_grid.delta = (z_grid.end - z_grid.start)/(double)(Nz-3);
  z_grid.delta_inv = 1.0/z_grid.delta;
  spline->z_grid   = z_grid;

  int N = num_splines;
#ifdef HAVE_SSE2
  if (N & 3)
    N += 4-(N & 3);
#endif

  spline->x_stride = (intptr_t)Ny*(intptr_t)Nz*N;
  spline->y_stride = Nz*N;
  spline->z_stride = N;

#ifndef HAVE_POSIX_MEMALIGN
  spline->coefs      = new complex_double[Nx*Ny*Nz*N];
  spline->lapl3 = new complex_double[3*N];
#else
  posix_memalign ((void**)&spline->coefs, 64, (size_t)2*sizeof(double)*Nx*Ny*Nz*N);
  posix_memalign ((void**)&spline->lapl3, 64, 6*sizeof(double)*N);
#endif

#ifdef HAVE_SSE2
  init_sse_data();
#endif
  if (!spline->coefs || !spline->lapl3) {
    fprintf (stderr, "Out of memory allocating spline coefficients in create_multi_UBspline_3d_z.\n");
    abort();
  }

  return spline;
}

void
set_multi_UBspline_3d_z (multi_UBspline_3d_z* spline, int num, complex_double *data)
{
  // Setup internal variables
  int Mx = spline->x_grid.num;  
  int My = spline->y_grid.num; 
  int Mz = spline->z_grid.num;
  int Nx, Ny, Nz;

  if (spline->xBC.lCode == PERIODIC || spline->xBC.lCode == ANTIPERIODIC)     
    Nx = Mx+3;
  else                                   
    Nx = Mx+2;
  if (spline->yBC.lCode == PERIODIC || spline->yBC.lCode == ANTIPERIODIC)     
    Ny = My+3;
  else                                   
    Ny = My+2;
  if (spline->zBC.lCode == PERIODIC || spline->zBC.lCode == ANTIPERIODIC)     
    Nz = Mz+3;
  else                                   
    Nz = Mz+2;

  BCtype_d xBC_r, xBC_i, yBC_r, yBC_i, zBC_r, zBC_i;
  xBC_r.lCode = spline->xBC.lCode;  xBC_r.rCode = spline->xBC.rCode;
  xBC_r.lVal  = spline->xBC.lVal_r; xBC_r.rVal  = spline->xBC.rVal_r;
  xBC_i.lCode = spline->xBC.lCode;  xBC_i.rCode = spline->xBC.rCode;
  xBC_i.lVal  = spline->xBC.lVal_i; xBC_i.rVal  = spline->xBC.rVal_i;
  yBC_r.lCode = spline->yBC.lCode;  yBC_r.rCode = spline->yBC.rCode;
  yBC_r.lVal  = spline->yBC.lVal_r; yBC_r.rVal  = spline->yBC.rVal_r;
  yBC_i.lCode = spline->yBC.lCode;  yBC_i.rCode = spline->yBC.rCode;
  yBC_i.lVal  = spline->yBC.lVal_i; yBC_i.rVal  = spline->yBC.rVal_i;
  zBC_r.lCode = spline->zBC.lCode;  zBC_r.rCode = spline->zBC.rCode;
  zBC_r.lVal  = spline->zBC.lVal_r; zBC_r.rVal  = spline->zBC.rVal_r;
  zBC_i.lCode = spline->zBC.lCode;  zBC_i.rCode = spline->zBC.rCode;
  zBC_i.lVal  = spline->zBC.lVal_i; zBC_i.rVal  = spline->zBC.rVal_i;

  complex_double *coefs = spline->coefs + num;

// unused variable
//  int N = spline->num_splines;
  int zs = spline->z_stride;
  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) 
    for (int iz=0; iz<Mz; iz++) {
      intptr_t doffset = 2*(iy*Mz+iz);
      intptr_t coffset = 2*(iy*Nz+iz)*zs;
      // Real part
      find_coefs_1d_d (spline->x_grid, xBC_r, 
		       ((double*)data)+doffset,  (intptr_t)2*My*Mz,
		       ((double*)coefs)+coffset, (intptr_t)2*Ny*Nz*zs);
      // Imag part
      find_coefs_1d_d (spline->x_grid, xBC_i, 
		       ((double*)data)+doffset+1,  (intptr_t)2*My*Mz,
		       ((double*)coefs)+coffset+1, (intptr_t)2*Ny*Nz*zs);
    }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iz=0; iz<Nz; iz++) {
      intptr_t doffset = 2*(ix*Ny*Nz + iz)*zs;
      intptr_t coffset = 2*(ix*Ny*Nz + iz)*zs;
      // Real part
      find_coefs_1d_d (spline->y_grid, yBC_r, 
		       ((double*)coefs)+doffset, (intptr_t)2*Nz*zs, 
		       ((double*)coefs)+coffset, (intptr_t)2*Nz*zs);
      // Imag part
      find_coefs_1d_d (spline->y_grid, yBC_i, 
		       ((double*)coefs)+doffset+1, (intptr_t)2*Nz*zs, 
		       ((double*)coefs)+coffset+1, (intptr_t)2*Nz*zs);
    }

  // Now, solve in the Z-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iy=0; iy<Ny; iy++) {
      intptr_t doffset = 2*((ix*Ny+iy)*Nz)*zs;
      intptr_t coffset = 2*((ix*Ny+iy)*Nz)*zs;
      // Real part
      find_coefs_1d_d (spline->z_grid, zBC_r, 
		       ((double*)coefs)+doffset, (intptr_t)2*zs, 
		       ((double*)coefs)+coffset, (intptr_t)2*zs);
      // Imag part
      find_coefs_1d_d (spline->z_grid, zBC_i, 
		       ((double*)coefs)+doffset+1, (intptr_t)2*zs, 
		       ((double*)coefs)+coffset+1, (intptr_t)2*zs);
    }
}


void
destroy_multi_UBspline (Bspline *spline)
{
  free(spline->coefs);
  free(spline);
}
