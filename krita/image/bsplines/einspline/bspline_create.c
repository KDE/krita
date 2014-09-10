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

#include "bspline_create.h"
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif
#ifndef __USE_XOPEN2K
  #define __USE_XOPEN2K
#endif
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

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
			 int M, int cstride)
{
  // Solve interpolating equations
  // First and last rows are different
  bands[4*(0)+1] /= bands[4*(0)+0];
  bands[4*(0)+2] /= bands[4*(0)+0];
  bands[4*(0)+3] /= bands[4*(0)+0];
  bands[4*(0)+0] = 1.0;
  bands[4*(1)+1] -= bands[4*(1)+0]*bands[4*(0)+1];
  bands[4*(1)+2] -= bands[4*(1)+0]*bands[4*(0)+2];
  bands[4*(1)+3] -= bands[4*(1)+0]*bands[4*(0)+3];
  bands[4*(0)+0] = 0.0;
  bands[4*(1)+2] /= bands[4*(1)+1];
  bands[4*(1)+3] /= bands[4*(1)+1];
  bands[4*(1)+1] = 1.0;
  
  // Now do rows 2 through M+1
  for (int row=2; row < (M+1); row++) {
    bands[4*(row)+1] -= bands[4*(row)+0]*bands[4*(row-1)+2];
    bands[4*(row)+3] -= bands[4*(row)+0]*bands[4*(row-1)+3];
    bands[4*(row)+2] /= bands[4*(row)+1];
    bands[4*(row)+3] /= bands[4*(row)+1];
    bands[4*(row)+0] = 0.0;
    bands[4*(row)+1] = 1.0;
  }

  // Do last row
  bands[4*(M+1)+1] -= bands[4*(M+1)+0]*bands[4*(M-1)+2];
  bands[4*(M+1)+3] -= bands[4*(M+1)+0]*bands[4*(M-1)+3];
  bands[4*(M+1)+2] -= bands[4*(M+1)+1]*bands[4*(M)+2];
  bands[4*(M+1)+3] -= bands[4*(M+1)+1]*bands[4*(M)+3];
  bands[4*(M+1)+3] /= bands[4*(M+1)+2];
  bands[4*(M+1)+2] = 1.0;

  coefs[(M+1)*cstride] = bands[4*(M+1)+3];
  // Now back substitute up
  for (int row=M; row>0; row--)
    coefs[row*cstride] = bands[4*(row)+3] - bands[4*(row)+2]*coefs[cstride*(row+1)];
  
  // Finish with first row
  coefs[0] = bands[4*(0)+3] - bands[4*(0)+1]*coefs[1*cstride] - bands[4*(0)+2]*coefs[2*cstride];
}

// On input, bands should be filled with:
// row 0   :  abcdInitial from boundary conditions
// rows 1:M:  basis functions in first 3 cols, data in last
// row M+1 :  abcdFinal   from boundary conditions
// cstride gives the stride between values in coefs.
// On exit, coefs with contain interpolating B-spline coefs
void 
solve_periodic_interp_1d_s (float bands[], float coefs[],
			    int M, int cstride)
{
  float lastCol[M];
  // Now solve:
  // First and last rows are different
  bands[4*(0)+2] /= bands[4*(0)+1];
  bands[4*(0)+0] /= bands[4*(0)+1];
  bands[4*(0)+3] /= bands[4*(0)+1];
  bands[4*(0)+1]  = 1.0;
  bands[4*(M-1)+1] -= bands[4*(M-1)+2]*bands[4*(0)+0];
  bands[4*(M-1)+3] -= bands[4*(M-1)+2]*bands[4*(0)+3];
  bands[4*(M-1)+2]  = -bands[4*(M-1)+2]*bands[4*(0)+2];
  lastCol[0] = bands[4*(0)+0];
  
  for (int row=1; row < (M-1); row++) {
    bands[4*(row)+1] -= bands[4*(row)+0] * bands[4*(row-1)+2];
    bands[4*(row)+3] -= bands[4*(row)+0] * bands[4*(row-1)+3];
    lastCol[row]   = -bands[4*(row)+0] * lastCol[row-1];
    bands[4*(row)+0] = 0.0;
    bands[4*(row)+2] /= bands[4*(row)+1];
    bands[4*(row)+3] /= bands[4*(row)+1];
    lastCol[row]  /= bands[4*(row)+1];
    bands[4*(row)+1]  = 1.0;
    if (row < (M-2)) {
      bands[4*(M-1)+3] -= bands[4*(M-1)+2]*bands[4*(row)+3];
      bands[4*(M-1)+1] -= bands[4*(M-1)+2]*lastCol[row];
      bands[4*(M-1)+2] = -bands[4*(M-1)+2]*bands[4*(row)+2];
    }
  }

  // Now do last row
  // The [2] element and [0] element are now on top of each other 
  bands[4*(M-1)+0] += bands[4*(M-1)+2];
  bands[4*(M-1)+1] -= bands[4*(M-1)+0] * (bands[4*(M-2)+2]+lastCol[M-2]);
  bands[4*(M-1)+3] -= bands[4*(M-1)+0] *  bands[4*(M-2)+3];
  bands[4*(M-1)+3] /= bands[4*(M-1)+1];
  coefs[M*cstride] = bands[4*(M-1)+3];
  for (int row=M-2; row>=0; row--) 
    coefs[(row+1)*cstride] = 
      bands[4*(row)+3] - bands[4*(row)+2]*coefs[(row+2)*cstride] - lastCol[row]*coefs[M*cstride];
  
  coefs[0*cstride] = coefs[M*cstride];
  coefs[(M+1)*cstride] = coefs[1*cstride];
  coefs[(M+2)*cstride] = coefs[2*cstride];


}


// On input, bands should be filled with:
// row 0   :  abcdInitial from boundary conditions
// rows 1:M:  basis functions in first 3 cols, data in last
// row M+1 :  abcdFinal   from boundary conditions
// cstride gives the stride between values in coefs.
// On exit, coefs with contain interpolating B-spline coefs
void 
solve_antiperiodic_interp_1d_s (float bands[], float coefs[],
				int M, int cstride)
{
  bands[4*0+0]     *= -1.0;
  bands[4*(M-1)+2] *= -1.0;

  float lastCol[M];
  // Now solve:
  // First and last rows are different
  bands[4*(0)+2] /= bands[4*(0)+1];
  bands[4*(0)+0] /= bands[4*(0)+1];
  bands[4*(0)+3] /= bands[4*(0)+1];
  bands[4*(0)+1]  = 1.0;
  bands[4*(M-1)+1] -= bands[4*(M-1)+2]*bands[4*(0)+0];
  bands[4*(M-1)+3] -= bands[4*(M-1)+2]*bands[4*(0)+3];
  bands[4*(M-1)+2]  = -bands[4*(M-1)+2]*bands[4*(0)+2];
  lastCol[0] = bands[4*(0)+0];
  
  for (int row=1; row < (M-1); row++) {
    bands[4*(row)+1] -= bands[4*(row)+0] * bands[4*(row-1)+2];
    bands[4*(row)+3] -= bands[4*(row)+0] * bands[4*(row-1)+3];
    lastCol[row]   = -bands[4*(row)+0] * lastCol[row-1];
    bands[4*(row)+0] = 0.0;
    bands[4*(row)+2] /= bands[4*(row)+1];
    bands[4*(row)+3] /= bands[4*(row)+1];
    lastCol[row]  /= bands[4*(row)+1];
    bands[4*(row)+1]  = 1.0;
    if (row < (M-2)) {
      bands[4*(M-1)+3] -= bands[4*(M-1)+2]*bands[4*(row)+3];
      bands[4*(M-1)+1] -= bands[4*(M-1)+2]*lastCol[row];
      bands[4*(M-1)+2] = -bands[4*(M-1)+2]*bands[4*(row)+2];
    }
  }

  // Now do last row
  // The [2] element and [0] element are now on top of each other 
  bands[4*(M-1)+0] += bands[4*(M-1)+2];
  bands[4*(M-1)+1] -= bands[4*(M-1)+0] * (bands[4*(M-2)+2]+lastCol[M-2]);
  bands[4*(M-1)+3] -= bands[4*(M-1)+0] *  bands[4*(M-2)+3];
  bands[4*(M-1)+3] /= bands[4*(M-1)+1];
  coefs[M*cstride] = bands[4*(M-1)+3];
  for (int row=M-2; row>=0; row--) 
    coefs[(row+1)*cstride] = 
      bands[4*(row)+3] - bands[4*(row)+2]*coefs[(row+2)*cstride] - lastCol[row]*coefs[M*cstride];
  
  coefs[0*cstride]     = -coefs[M*cstride];
  coefs[(M+1)*cstride] = -coefs[1*cstride];
  coefs[(M+2)*cstride] = -coefs[2*cstride];


}




#ifdef HIGH_PRECISION
void
find_coefs_1d_s (Ugrid grid, BCtype_s bc, 
		 float *data,  intptr_t dstride,
		 float *coefs, intptr_t cstride)
{
  BCtype_d d_bc;
  double *d_data, *d_coefs;

  d_bc.lCode = bc.lCode;   d_bc.rCode = bc.rCode;
  d_bc.lVal  = bc.lVal;    d_bc.rVal  = bc.rVal;
  int M = grid.num, N;
  if (bc.lCode == PERIODIC || bc.lCode == ANTIPERIODIC)    N = M+3;
  else                         N = M+2;

  d_data  = malloc (N*sizeof(double));
  d_coefs = malloc (N*sizeof(double));
  for (int i=0; i<M; i++)
    d_data[i] = data[i*dstride];
  find_coefs_1d_d (grid, d_bc, d_data, 1, d_coefs, 1);
  for (int i=0; i<N; i++)
    coefs[i*cstride] = d_coefs[i];
  free (d_data);
  free (d_coefs);
}

#else
void
find_coefs_1d_s (Ugrid grid, BCtype_s bc, 
		 float *data,  intptr_t dstride,
		 float *coefs, intptr_t cstride)
{
  int M = grid.num;
  float basis[4] = {1.0/6.0, 2.0/3.0, 1.0/6.0, 0.0};
  if (bc.lCode == PERIODIC || bc.lCode == ANTIPERIODIC) {
#ifdef HAVE_C_VARARRAYS
    float bands[4*M];
#else
    float *bands = malloc(4*M*sizeof(float));
#endif    
    for (int i=0; i<M; i++) {
      bands[4*i+0] = basis[0];
      bands[4*i+1] = basis[1];
      bands[4*i+2] = basis[2];
      bands[4*i+3] = data[i*dstride];
    }
    if (bc.lCode == PERIODIC)
      solve_periodic_interp_1d_s (bands, coefs, M, cstride);
    else
      solve_antiperiodic_interp_1d_s (bands, coefs, M, cstride);
#ifndef HAVE_C_VARARRAYS
    free (bands);
#endif
  }
  else {
    // Setup boundary conditions
    float abcd_left[4], abcd_right[4];
    // Left boundary
    if (bc.lCode == FLAT || bc.lCode == NATURAL)
      bc.lVal = 0.0;
    if (bc.lCode == FLAT || bc.lCode == DERIV1) {
      abcd_left[0] = -0.5 * grid.delta_inv;
      abcd_left[1] =  0.0 * grid.delta_inv; 
      abcd_left[2] =  0.5 * grid.delta_inv;
      abcd_left[3] =  bc.lVal;
    }
    if (bc.lCode == NATURAL || bc.lCode == DERIV2) {
      abcd_left[0] = 1.0 * grid.delta_inv * grid.delta_inv;
      abcd_left[1] =-2.0 * grid.delta_inv * grid.delta_inv;
      abcd_left[2] = 1.0 * grid.delta_inv * grid.delta_inv;
      abcd_left[3] = bc.lVal;
    }
    
    // Right boundary
    if (bc.rCode == FLAT || bc.rCode == NATURAL)
      bc.rVal = 0.0;
    if (bc.rCode == FLAT || bc.rCode == DERIV1) {
      abcd_right[0] = -0.5 * grid.delta_inv;
      abcd_right[1] =  0.0 * grid.delta_inv; 
      abcd_right[2] =  0.5 * grid.delta_inv;
      abcd_right[3] =  bc.rVal;
    }
    if (bc.rCode == NATURAL || bc.rCode == DERIV2) {
      abcd_right[0] = 1.0 *grid.delta_inv * grid.delta_inv;
      abcd_right[1] =-2.0 *grid.delta_inv * grid.delta_inv;
      abcd_right[2] = 1.0 *grid.delta_inv * grid.delta_inv;
      abcd_right[3] = bc.rVal;
    }
#ifdef HAVE_C_VARARRAYS
    float bands[4*(M+2)];
#else
    float *bands = malloc ((M+2)*4*sizeof(float));
#endif    
    for (int i=0; i<4; i++) {
      bands[4*( 0 )+i]   = abcd_left[i];
      bands[4*(M+1)+i] = abcd_right[i];
    }
    for (int i=0; i<M; i++) {
      for (int j=0; j<3; j++)
	bands[4*(i+1)+j] = basis[j];
      bands[4*(i+1)+3] = data[i*dstride];
    }   
    // Now, solve for coefficients
    solve_deriv_interp_1d_s (bands, coefs, M, cstride);
#ifndef HAVE_C_VARARRAYS
    free (bands);
#endif
  }
}

#endif

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
UBspline_1d_s*
create_UBspline_1d_s (Ugrid x_grid, BCtype_s xBC, float *data)
{
  // Create new spline
  UBspline_1d_s* restrict spline = malloc (sizeof(UBspline_1d_s));
  spline->spcode = U1D;
  spline->tcode  = SINGLE_REAL;
  spline->xBC = xBC; spline->x_grid = x_grid;

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
#ifndef HAVE_SSE2
  spline->coefs = malloc (sizeof(float)*N);
#else
  posix_memalign ((void**)&spline->coefs, 16, (sizeof(float)*N));
#endif
  find_coefs_1d_s (spline->x_grid, xBC, data, 1, spline->coefs, 1);

  init_sse_data();    
  return spline;
}

void
recompute_UBspline_1d_s (UBspline_1d_s* spline, float *data)
{
  find_coefs_1d_s (spline->x_grid, spline->xBC, data, 1, spline->coefs, 1);
}


UBspline_2d_s*
create_UBspline_2d_s (Ugrid x_grid, Ugrid y_grid,
		      BCtype_s xBC, BCtype_s yBC, float *data)
{
  // Create new spline
  UBspline_2d_s* restrict spline = malloc (sizeof(UBspline_2d_s));
  spline->spcode = U2D;
  spline->tcode  = SINGLE_REAL;
  spline->xBC = xBC; 
  spline->yBC = yBC; 
  // Setup internal variables
  int Mx = x_grid.num;
  int My = y_grid.num;
  int Nx, Ny;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC)     Nx = Mx+3;
  else                           Nx = Mx+2;
  x_grid.delta = (x_grid.end - x_grid.start)/(double)(Nx-3);
  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;

  if (yBC.lCode == PERIODIC || yBC.lCode == ANTIPERIODIC)     Ny = My+3;
  else                           Ny = My+2;
  y_grid.delta = (y_grid.end - y_grid.start)/(double)(Ny-3);
  y_grid.delta_inv = 1.0/y_grid.delta;
  spline->y_grid   = y_grid;
  spline->x_stride = Ny;
#ifndef HAVE_SSE2
  spline->coefs = malloc (sizeof(float)*Nx*Ny);
#else
  posix_memalign ((void**)&spline->coefs, 16, sizeof(float)*Nx*Ny);
#endif

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) {
    intptr_t doffset = iy;
    intptr_t coffset = iy;
    find_coefs_1d_s (spline->x_grid, spline->xBC, data+doffset, My,
		     spline->coefs+coffset, Ny);
  }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) {
    intptr_t doffset = ix*Ny;
    intptr_t coffset = ix*Ny;
    find_coefs_1d_s (spline->y_grid, spline->yBC, spline->coefs+doffset, 1, 
		     spline->coefs+coffset, 1);
  }
  init_sse_data();
  return spline;
}

void
recompute_UBspline_2d_s (UBspline_2d_s* spline, float *data)
{
  int Mx = spline->x_grid.num;
  int My = spline->y_grid.num;
  int Nx, Ny;

  if (spline->xBC.lCode == PERIODIC || spline->xBC.lCode == ANTIPERIODIC)     Nx = Mx+3;
  else                           Nx = Mx+2;
  if (spline->yBC.lCode == PERIODIC || spline->yBC.lCode == ANTIPERIODIC)     Ny = My+3;
  else                           Ny = My+2;

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) {
    intptr_t doffset = iy;
    intptr_t coffset = iy;
    find_coefs_1d_s (spline->x_grid, spline->xBC, data+doffset, My,
		     spline->coefs+coffset, Ny);
  }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) {
    intptr_t doffset = ix*Ny;
    intptr_t coffset = ix*Ny;
    find_coefs_1d_s (spline->y_grid, spline->yBC, spline->coefs+doffset, 1, 
		     spline->coefs+coffset, 1);
  }
}


UBspline_3d_s*
create_UBspline_3d_s (Ugrid x_grid, Ugrid y_grid, Ugrid z_grid,
		      BCtype_s xBC, BCtype_s yBC, BCtype_s zBC,
		      float *data)
{
  // Create new spline
  UBspline_3d_s* restrict spline = malloc (sizeof(UBspline_3d_s));
  spline->spcode = U3D;
  spline->tcode  = SINGLE_REAL;
  spline->xBC = xBC; 
  spline->yBC = yBC; 
  spline->zBC = zBC; 
  // Setup internal variables
  int Mx = x_grid.num;  int My = y_grid.num; int Mz = z_grid.num;
  int Nx, Ny, Nz;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC)     Nx = Mx+3;
  else                           Nx = Mx+2;
  x_grid.delta = (x_grid.end - x_grid.start)/(double)(Nx-3);
  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;

  if (yBC.lCode == PERIODIC || yBC.lCode == ANTIPERIODIC)     Ny = My+3;
  else                           Ny = My+2;
  y_grid.delta = (y_grid.end - y_grid.start)/(double)(Ny-3);
  y_grid.delta_inv = 1.0/y_grid.delta;
  spline->y_grid   = y_grid;

  if (zBC.lCode == PERIODIC || zBC.lCode == ANTIPERIODIC)     Nz = Mz+3;
  else                           Nz = Mz+2;
  z_grid.delta = (z_grid.end - z_grid.start)/(double)(Nz-3);
  z_grid.delta_inv = 1.0/z_grid.delta;
  spline->z_grid   = z_grid;

  spline->x_stride = Ny*Nz;
  spline->y_stride = Nz;

#ifndef HAVE_SSE2
  spline->coefs      = malloc (sizeof(float)*Nx*Ny*Nz);
#else
  posix_memalign ((void**)&spline->coefs, 16, (sizeof(float)*Nx*Ny*Nz));
#endif

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) 
    for (int iz=0; iz<Mz; iz++) {
      intptr_t doffset = iy*Mz+iz;
      intptr_t coffset = iy*Nz+iz;
      find_coefs_1d_s (spline->x_grid, xBC, data+doffset, My*Mz,
		       spline->coefs+coffset, Ny*Nz);
    }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iz=0; iz<Nz; iz++) {
      intptr_t doffset = ix*Ny*Nz + iz;
      intptr_t coffset = ix*Ny*Nz + iz;
      find_coefs_1d_s (spline->y_grid, yBC, spline->coefs+doffset, Nz, 
		       spline->coefs+coffset, Nz);
    }

  // Now, solve in the Z-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iy=0; iy<Ny; iy++) {
      intptr_t doffset = (ix*Ny+iy)*Nz;
      intptr_t coffset = (ix*Ny+iy)*Nz;
      find_coefs_1d_s (spline->z_grid, zBC, spline->coefs+doffset, 1, 
		       spline->coefs+coffset, 1);
    }
  init_sse_data();
  return spline;
}

void
recompute_UBspline_3d_s (UBspline_3d_s* spline, float *data)
{
  int Mx = spline->x_grid.num;
  int My = spline->y_grid.num;
  int Mz = spline->z_grid.num;
  int Nx, Ny, Nz;

  if (spline->xBC.lCode == PERIODIC || spline->xBC.lCode == ANTIPERIODIC)     Nx = Mx+3;
  else                           Nx = Mx+2;
  if (spline->yBC.lCode == PERIODIC || spline->yBC.lCode == ANTIPERIODIC)     Ny = My+3;
  else                           Ny = My+2;
  if (spline->zBC.lCode == PERIODIC || spline->zBC.lCode == ANTIPERIODIC)     Nz = Mz+3;
  else                           Nz = Mz+2;

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) 
    for (int iz=0; iz<Mz; iz++) {
      intptr_t doffset = iy*Mz+iz;
      intptr_t coffset = iy*Nz+iz;
      find_coefs_1d_s (spline->x_grid, spline->xBC, data+doffset, My*Mz,
		       spline->coefs+coffset, Ny*Nz);
    }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iz=0; iz<Nz; iz++) {
      intptr_t doffset = ix*Ny*Nz + iz;
      intptr_t coffset = ix*Ny*Nz + iz;
      find_coefs_1d_s (spline->y_grid, spline->yBC, spline->coefs+doffset, Nz, 
		       spline->coefs+coffset, Nz);
    }

  // Now, solve in the Z-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iy=0; iy<Ny; iy++) {
      intptr_t doffset = (ix*Ny+iy)*Nz;
      intptr_t coffset = (ix*Ny+iy)*Nz;
      find_coefs_1d_s (spline->z_grid, spline->zBC, spline->coefs+doffset, 1, 
		       spline->coefs+coffset, 1);
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
UBspline_1d_c*
create_UBspline_1d_c (Ugrid x_grid, BCtype_c xBC, complex_float *data)
{
  // Create new spline
  UBspline_1d_c* restrict spline = malloc (sizeof(UBspline_1d_c));
  spline->spcode = U1D;
  spline->tcode  = SINGLE_COMPLEX;
  spline->xBC = xBC; 
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
#ifndef HAVE_SSE2
  spline->coefs = malloc (2*sizeof(float)*N);
#else
  posix_memalign ((void**)&spline->coefs, 16, 2*sizeof(float)*N);
#endif

  BCtype_s xBC_r, xBC_i;
  xBC_r.lCode = xBC.lCode;  xBC_r.rCode = xBC.rCode;
  xBC_r.lVal  = xBC.lVal_r; xBC_r.rVal  = xBC.rVal_r;
  xBC_i.lCode = xBC.lCode;  xBC_i.rCode = xBC.rCode;
  xBC_i.lVal  = xBC.lVal_i; xBC_i.rVal  = xBC.rVal_i;
  // Real part
  find_coefs_1d_s (spline->x_grid, xBC_r, 
		   (float*)data, 2, (float*)spline->coefs, 2);
  // Imaginarty part
  find_coefs_1d_s (spline->x_grid, xBC_i, 
		   ((float*)data)+1, 2, ((float*)spline->coefs+1), 2);

  init_sse_data();    
  return spline;
}

void
recompute_UBspline_1d_c (UBspline_1d_c* spline, complex_float *data)
{
  
  BCtype_s xBC_r, xBC_i;
  xBC_r.lCode = spline->xBC.lCode;  xBC_r.rCode = spline->xBC.rCode;
  xBC_r.lVal  = spline->xBC.lVal_r; xBC_r.rVal  = spline->xBC.rVal_r;
  xBC_i.lCode = spline->xBC.lCode;  xBC_i.rCode = spline->xBC.rCode;
  xBC_i.lVal  = spline->xBC.lVal_i; xBC_i.rVal  = spline->xBC.rVal_i;

  // Real part
  find_coefs_1d_s (spline->x_grid, xBC_r, 
		   (float*)data, 2, (float*)spline->coefs, 2);
  // Imaginarty part
  find_coefs_1d_s (spline->x_grid, xBC_i, 
		   ((float*)data)+1, 2, ((float*)spline->coefs+1), 2);
}



UBspline_2d_c*
create_UBspline_2d_c (Ugrid x_grid, Ugrid y_grid,
		      BCtype_c xBC, BCtype_c yBC, complex_float *data)
{
  // Create new spline
  UBspline_2d_c* restrict spline = malloc (sizeof(UBspline_2d_c));
  spline->spcode = U2D;
  spline->tcode  = SINGLE_COMPLEX;
  spline->xBC = xBC; 
  spline->yBC = yBC; 

  // Setup internal variables
  int Mx = x_grid.num;
  int My = y_grid.num;
  int Nx, Ny;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC)     Nx = Mx+3;
  else                           Nx = Mx+2;
  x_grid.delta = (x_grid.end - x_grid.start)/(double)(Nx-3);
  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;

  if (yBC.lCode == PERIODIC || yBC.lCode == ANTIPERIODIC)     Ny = My+3;
  else                           Ny = My+2;
  y_grid.delta = (y_grid.end - y_grid.start)/(double)(Ny-3);
  y_grid.delta_inv = 1.0/y_grid.delta;
  spline->y_grid   = y_grid;
  spline->x_stride = Ny;

#ifndef HAVE_SSE2
  spline->coefs = malloc (2*sizeof(float)*Nx*Ny);
#else
  posix_memalign ((void**)&spline->coefs, 16, 2*sizeof(float)*Nx*Ny);
#endif

  BCtype_s xBC_r, xBC_i, yBC_r, yBC_i;
  xBC_r.lCode = xBC.lCode;  xBC_r.rCode = xBC.rCode;
  xBC_r.lVal  = xBC.lVal_r; xBC_r.rVal  = xBC.rVal_r;
  xBC_i.lCode = xBC.lCode;  xBC_i.rCode = xBC.rCode;
  xBC_i.lVal  = xBC.lVal_i; xBC_i.rVal  = xBC.rVal_i;
  yBC_r.lCode = yBC.lCode;  yBC_r.rCode = yBC.rCode;
  yBC_r.lVal  = yBC.lVal_r; yBC_r.rVal  = yBC.rVal_r;
  yBC_i.lCode = yBC.lCode;  yBC_i.rCode = yBC.rCode;
  yBC_i.lVal  = yBC.lVal_i; yBC_i.rVal  = yBC.rVal_i;
  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) {
    intptr_t doffset = 2*iy;
    intptr_t coffset = 2*iy;
    // Real part
    find_coefs_1d_s (spline->x_grid, xBC_r, ((float*)data)+doffset, 2*My,
		     (float*)spline->coefs+coffset, 2*Ny);
    // Imag part
    find_coefs_1d_s (spline->x_grid, xBC_i, ((float*)data)+doffset+1, 2*My,
		     ((float*)spline->coefs)+coffset+1, 2*Ny);
  }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) {
    intptr_t doffset = 2*ix*Ny;
    intptr_t coffset = 2*ix*Ny;
    // Real part
    find_coefs_1d_s (spline->y_grid, yBC_r, ((float*)spline->coefs)+doffset, 2, 
		     ((float*)spline->coefs)+coffset, 2);
    // Imag part
    find_coefs_1d_s (spline->y_grid, yBC_i, ((float*)spline->coefs)+doffset+1, 2, 
		     ((float*)spline->coefs)+coffset+1, 2);
  }
  init_sse_data();
  return spline;
}


void
recompute_UBspline_2d_c (UBspline_2d_c* spline, complex_float *data)
{
  // Setup internal variables
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

  BCtype_s xBC_r, xBC_i, yBC_r, yBC_i;
  xBC_r.lCode = spline->xBC.lCode;  xBC_r.rCode = spline->xBC.rCode;
  xBC_r.lVal  = spline->xBC.lVal_r; xBC_r.rVal  = spline->xBC.rVal_r;
  xBC_i.lCode = spline->xBC.lCode;  xBC_i.rCode = spline->xBC.rCode;
  xBC_i.lVal  = spline->xBC.lVal_i; xBC_i.rVal  = spline->xBC.rVal_i;
  yBC_r.lCode = spline->yBC.lCode;  yBC_r.rCode = spline->yBC.rCode;
  yBC_r.lVal  = spline->yBC.lVal_r; yBC_r.rVal  = spline->yBC.rVal_r;
  yBC_i.lCode = spline->yBC.lCode;  yBC_i.rCode = spline->yBC.rCode;
  yBC_i.lVal  = spline->yBC.lVal_i; yBC_i.rVal  = spline->yBC.rVal_i;
 
  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) {
    intptr_t doffset = 2*iy;
    intptr_t coffset = 2*iy;
    // Real part
    find_coefs_1d_s (spline->x_grid, xBC_r, ((float*)data)+doffset, 2*My,
		     (float*)spline->coefs+coffset, 2*Ny);
    // Imag part
    find_coefs_1d_s (spline->x_grid, xBC_i, ((float*)data)+doffset+1, 2*My,
		     ((float*)spline->coefs)+coffset+1, 2*Ny);
  }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) {
    intptr_t doffset = 2*ix*Ny;
    intptr_t coffset = 2*ix*Ny;
    // Real part
    find_coefs_1d_s (spline->y_grid, yBC_r, ((float*)spline->coefs)+doffset, 2, 
		     ((float*)spline->coefs)+coffset, 2);
    // Imag part
    find_coefs_1d_s (spline->y_grid, yBC_i, ((float*)spline->coefs)+doffset+1, 2, 
		     ((float*)spline->coefs)+coffset+1, 2);
  }  
}

UBspline_3d_c*
create_UBspline_3d_c (Ugrid x_grid, Ugrid y_grid, Ugrid z_grid,
		      BCtype_c xBC, BCtype_c yBC, BCtype_c zBC,
		      complex_float *data)
{
  // Create new spline
  UBspline_3d_c* restrict spline = malloc (sizeof(UBspline_3d_c));
  spline->spcode = U3D;
  spline->tcode  = SINGLE_COMPLEX;
  spline->xBC = xBC; 
  spline->yBC = yBC; 
  spline->zBC = zBC; 

  // Setup internal variables
  int Mx = x_grid.num;  int My = y_grid.num; int Mz = z_grid.num;
  int Nx, Ny, Nz;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC)     Nx = Mx+3;
  else                           Nx = Mx+2;
  x_grid.delta = (x_grid.end - x_grid.start)/(double)(Nx-3);
  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;

  if (yBC.lCode == PERIODIC || yBC.lCode == ANTIPERIODIC)     Ny = My+3;
  else                           Ny = My+2;
  y_grid.delta = (y_grid.end - y_grid.start)/(double)(Ny-3);
  y_grid.delta_inv = 1.0/y_grid.delta;
  spline->y_grid   = y_grid;

  if (zBC.lCode == PERIODIC || zBC.lCode == ANTIPERIODIC)     Nz = Mz+3;
  else                           Nz = Mz+2;
  z_grid.delta = (z_grid.end - z_grid.start)/(double)(Nz-3);
  z_grid.delta_inv = 1.0/z_grid.delta;
  spline->z_grid   = z_grid;

  spline->x_stride = Ny*Nz;
  spline->y_stride = Nz;

#ifndef HAVE_SSE2
  spline->coefs      = malloc (2*sizeof(float)*Nx*Ny*Nz);
#else
  posix_memalign ((void**)&spline->coefs, 16, 2*sizeof(float)*Nx*Ny*Nz);
#endif

  BCtype_s xBC_r, xBC_i, yBC_r, yBC_i, zBC_r, zBC_i;
  xBC_r.lCode = xBC.lCode;  xBC_r.rCode = xBC.rCode;
  xBC_r.lVal  = xBC.lVal_r; xBC_r.rVal  = xBC.rVal_r;
  xBC_i.lCode = xBC.lCode;  xBC_i.rCode = xBC.rCode;
  xBC_i.lVal  = xBC.lVal_i; xBC_i.rVal  = xBC.rVal_i;
  yBC_r.lCode = yBC.lCode;  yBC_r.rCode = yBC.rCode;
  yBC_r.lVal  = yBC.lVal_r; yBC_r.rVal  = yBC.rVal_r;
  yBC_i.lCode = yBC.lCode;  yBC_i.rCode = yBC.rCode;
  yBC_i.lVal  = yBC.lVal_i; yBC_i.rVal  = yBC.rVal_i;
  zBC_r.lCode = zBC.lCode;  zBC_r.rCode = zBC.rCode;
  zBC_r.lVal  = zBC.lVal_r; zBC_r.rVal  = zBC.rVal_r;
  zBC_i.lCode = zBC.lCode;  zBC_i.rCode = zBC.rCode;
  zBC_i.lVal  = zBC.lVal_i; zBC_i.rVal  = zBC.rVal_i;
  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) 
    for (int iz=0; iz<Mz; iz++) {
      intptr_t doffset = 2*(iy*Mz+iz);
      intptr_t coffset = 2*(iy*Nz+iz);
      // Real part
      find_coefs_1d_s (spline->x_grid, xBC_r, ((float*)data)+doffset, 2*My*Mz,
		       ((float*)spline->coefs)+coffset, 2*Ny*Nz);
      // Imag part
      find_coefs_1d_s (spline->x_grid, xBC_i, ((float*)data)+doffset+1, 2*My*Mz,
		       ((float*)spline->coefs)+coffset+1, 2*Ny*Nz);
    }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iz=0; iz<Nz; iz++) {
      intptr_t doffset = 2*(ix*Ny*Nz + iz);
      intptr_t coffset = 2*(ix*Ny*Nz + iz);
      // Real part
      find_coefs_1d_s (spline->y_grid, yBC_r, ((float*)spline->coefs)+doffset, 2*Nz, 
		       ((float*)spline->coefs)+coffset, 2*Nz);
      // Imag part
      find_coefs_1d_s (spline->y_grid, yBC_i, ((float*)spline->coefs)+doffset+1, 2*Nz, 
		       ((float*)spline->coefs)+coffset+1, 2*Nz);
    }

  // Now, solve in the Z-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iy=0; iy<Ny; iy++) {
      intptr_t doffset = 2*((ix*Ny+iy)*Nz);
      intptr_t coffset = 2*((ix*Ny+iy)*Nz);
      // Real part
      find_coefs_1d_s (spline->z_grid, zBC_r, ((float*)spline->coefs)+doffset, 2, 
		       ((float*)spline->coefs)+coffset, 2);
      // Imag part
      find_coefs_1d_s (spline->z_grid, zBC_i, ((float*)spline->coefs)+doffset+1, 2, 
		       ((float*)spline->coefs)+coffset+1, 2);
    }

  init_sse_data();
  return spline;
}

void
recompute_UBspline_3d_c (UBspline_3d_c* spline, complex_float *data)
{
  // Setup internal variables
  int Mx = spline->x_grid.num;
  int My = spline->y_grid.num;
  int Mz = spline->z_grid.num;
  int Nx, Ny, Nz;

  if (spline->xBC.lCode == PERIODIC || spline->xBC.lCode == ANTIPERIODIC)     Nx = Mx+3;
  else                           Nx = Mx+2;
  if (spline->yBC.lCode == PERIODIC || spline->yBC.lCode == ANTIPERIODIC)     Ny = My+3;
  else                           Ny = My+2;
  if (spline->zBC.lCode == PERIODIC || spline->zBC.lCode == ANTIPERIODIC)     Nz = Mz+3;
  else                           Nz = Mz+2;

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
  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) 
    for (int iz=0; iz<Mz; iz++) {
      intptr_t doffset = 2*(iy*Mz+iz);
      intptr_t coffset = 2*(iy*Nz+iz);
      // Real part
      find_coefs_1d_s (spline->x_grid, xBC_r, ((float*)data)+doffset, 2*My*Mz,
		       ((float*)spline->coefs)+coffset, 2*Ny*Nz);
      // Imag part
      find_coefs_1d_s (spline->x_grid, xBC_i, ((float*)data)+doffset+1, 2*My*Mz,
		       ((float*)spline->coefs)+coffset+1, 2*Ny*Nz);
    }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iz=0; iz<Nz; iz++) {
      intptr_t doffset = 2*(ix*Ny*Nz + iz);
      intptr_t coffset = 2*(ix*Ny*Nz + iz);
      // Real part
      find_coefs_1d_s (spline->y_grid, yBC_r, ((float*)spline->coefs)+doffset, 2*Nz, 
		       ((float*)spline->coefs)+coffset, 2*Nz);
      // Imag part
      find_coefs_1d_s (spline->y_grid, yBC_i, ((float*)spline->coefs)+doffset+1, 2*Nz, 
		       ((float*)spline->coefs)+coffset+1, 2*Nz);
    }

  // Now, solve in the Z-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iy=0; iy<Ny; iy++) {
      intptr_t doffset = 2*((ix*Ny+iy)*Nz);
      intptr_t coffset = 2*((ix*Ny+iy)*Nz);
      // Real part
      find_coefs_1d_s (spline->z_grid, zBC_r, ((float*)spline->coefs)+doffset, 2, 
		       ((float*)spline->coefs)+coffset, 2);
      // Imag part
      find_coefs_1d_s (spline->z_grid, zBC_i, ((float*)spline->coefs)+doffset+1, 2, 
		       ((float*)spline->coefs)+coffset+1, 2);
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
			 int M, int cstride)
{
  // Solve interpolating equations
  // First and last rows are different
  bands[4*(0)+1] /= bands[4*(0)+0];
  bands[4*(0)+2] /= bands[4*(0)+0];
  bands[4*(0)+3] /= bands[4*(0)+0];
  bands[4*(0)+0] = 1.0;
  bands[4*(1)+1] -= bands[4*(1)+0]*bands[4*(0)+1];
  bands[4*(1)+2] -= bands[4*(1)+0]*bands[4*(0)+2];
  bands[4*(1)+3] -= bands[4*(1)+0]*bands[4*(0)+3];
  bands[4*(0)+0] = 0.0;
  bands[4*(1)+2] /= bands[4*(1)+1];
  bands[4*(1)+3] /= bands[4*(1)+1];
  bands[4*(1)+1] = 1.0;
  
  // Now do rows 2 through M+1
  for (int row=2; row < (M+1); row++) {
    bands[4*(row)+1] -= bands[4*(row)+0]*bands[4*(row-1)+2];
    bands[4*(row)+3] -= bands[4*(row)+0]*bands[4*(row-1)+3];
    bands[4*(row)+2] /= bands[4*(row)+1];
    bands[4*(row)+3] /= bands[4*(row)+1];
    bands[4*(row)+0] = 0.0;
    bands[4*(row)+1] = 1.0;
  }

  // Do last row
  bands[4*(M+1)+1] -= bands[4*(M+1)+0]*bands[4*(M-1)+2];
  bands[4*(M+1)+3] -= bands[4*(M+1)+0]*bands[4*(M-1)+3];
  bands[4*(M+1)+2] -= bands[4*(M+1)+1]*bands[4*(M)+2];
  bands[4*(M+1)+3] -= bands[4*(M+1)+1]*bands[4*(M)+3];
  bands[4*(M+1)+3] /= bands[4*(M+1)+2];
  bands[4*(M+1)+2] = 1.0;

  coefs[(M+1)*cstride] = bands[4*(M+1)+3];
  // Now back substitute up
  for (int row=M; row>0; row--)
    coefs[row*cstride] = bands[4*(row)+3] - bands[4*(row)+2]*coefs[cstride*(row+1)];
  
  // Finish with first row
  coefs[0] = bands[4*(0)+3] - bands[4*(0)+1]*coefs[1*cstride] - bands[4*(0)+2]*coefs[2*cstride];
}

// On input, bands should be filled with:
// row 0   :  abcdInitial from boundary conditions
// rows 1:M:  basis functions in first 3 cols, data in last
// row M+1 :  abcdFinal   from boundary conditions
// cstride gives the stride between values in coefs.
// On exit, coefs with contain interpolating B-spline coefs
void 
solve_periodic_interp_1d_d (double bands[], double coefs[],
			    int M, int cstride)
{
  double lastCol[M];
  // Now solve:
  // First and last rows are different
  bands[4*(0)+2] /= bands[4*(0)+1];
  bands[4*(0)+0] /= bands[4*(0)+1];
  bands[4*(0)+3] /= bands[4*(0)+1];
  bands[4*(0)+1]  = 1.0;
  bands[4*(M-1)+1] -= bands[4*(M-1)+2]*bands[4*(0)+0];
  bands[4*(M-1)+3] -= bands[4*(M-1)+2]*bands[4*(0)+3];
  bands[4*(M-1)+2]  = -bands[4*(M-1)+2]*bands[4*(0)+2];
  lastCol[0] = bands[4*(0)+0];
  
  for (int row=1; row < (M-1); row++) {
    bands[4*(row)+1] -= bands[4*(row)+0] * bands[4*(row-1)+2];
    bands[4*(row)+3] -= bands[4*(row)+0] * bands[4*(row-1)+3];
    lastCol[row]   = -bands[4*(row)+0] * lastCol[row-1];
    bands[4*(row)+0] = 0.0;
    bands[4*(row)+2] /= bands[4*(row)+1];
    bands[4*(row)+3] /= bands[4*(row)+1];
    lastCol[row]  /= bands[4*(row)+1];
    bands[4*(row)+1]  = 1.0;
    if (row < (M-2)) {
      bands[4*(M-1)+3] -= bands[4*(M-1)+2]*bands[4*(row)+3];
      bands[4*(M-1)+1] -= bands[4*(M-1)+2]*lastCol[row];
      bands[4*(M-1)+2] = -bands[4*(M-1)+2]*bands[4*(row)+2];
    }
  }

  // Now do last row
  // The [2] element and [0] element are now on top of each other 
  bands[4*(M-1)+0] += bands[4*(M-1)+2];
  bands[4*(M-1)+1] -= bands[4*(M-1)+0] * (bands[4*(M-2)+2]+lastCol[M-2]);
  bands[4*(M-1)+3] -= bands[4*(M-1)+0] *  bands[4*(M-2)+3];
  bands[4*(M-1)+3] /= bands[4*(M-1)+1];
  coefs[M*cstride] = bands[4*(M-1)+3];
  for (int row=M-2; row>=0; row--) 
    coefs[(row+1)*cstride] = 
      bands[4*(row)+3] - bands[4*(row)+2]*coefs[(row+2)*cstride] - lastCol[row]*coefs[M*cstride];
  
  coefs[0*cstride] = coefs[M*cstride];
  coefs[(M+1)*cstride] = coefs[1*cstride];
  coefs[(M+2)*cstride] = coefs[2*cstride];
}

// On input, bands should be filled with:
// row 0   :  abcdInitial from boundary conditions
// rows 1:M:  basis functions in first 3 cols, data in last
// row M+1 :  abcdFinal   from boundary conditions
// cstride gives the stride between values in coefs.
// On exit, coefs with contain interpolating B-spline coefs
void 
solve_antiperiodic_interp_1d_d (double bands[], double coefs[],
			    int M, int cstride)
{
  double lastCol[M];
  bands[4*0+0]     *= -1.0;
  bands[4*(M-1)+2] *= -1.0;
  // Now solve:
  // First and last rows are different
  bands[4*(0)+2] /= bands[4*(0)+1];
  bands[4*(0)+0] /= bands[4*(0)+1];
  bands[4*(0)+3] /= bands[4*(0)+1];
  bands[4*(0)+1]  = 1.0;
  bands[4*(M-1)+1] -= bands[4*(M-1)+2]*bands[4*(0)+0];
  bands[4*(M-1)+3] -= bands[4*(M-1)+2]*bands[4*(0)+3];
  bands[4*(M-1)+2]  = -bands[4*(M-1)+2]*bands[4*(0)+2];
  lastCol[0] = bands[4*(0)+0];
  
  for (int row=1; row < (M-1); row++) {
    bands[4*(row)+1] -= bands[4*(row)+0] * bands[4*(row-1)+2];
    bands[4*(row)+3] -= bands[4*(row)+0] * bands[4*(row-1)+3];
    lastCol[row]   = -bands[4*(row)+0] * lastCol[row-1];
    bands[4*(row)+0] = 0.0;
    bands[4*(row)+2] /= bands[4*(row)+1];
    bands[4*(row)+3] /= bands[4*(row)+1];
    lastCol[row]  /= bands[4*(row)+1];
    bands[4*(row)+1]  = 1.0;
    if (row < (M-2)) {
      bands[4*(M-1)+3] -= bands[4*(M-1)+2]*bands[4*(row)+3];
      bands[4*(M-1)+1] -= bands[4*(M-1)+2]*lastCol[row];
      bands[4*(M-1)+2] = -bands[4*(M-1)+2]*bands[4*(row)+2];
    }
  }

  // Now do last row
  // The [2] element and [0] element are now on top of each other 
  bands[4*(M-1)+0] += bands[4*(M-1)+2];
  bands[4*(M-1)+1] -= bands[4*(M-1)+0] * (bands[4*(M-2)+2]+lastCol[M-2]);
  bands[4*(M-1)+3] -= bands[4*(M-1)+0] *  bands[4*(M-2)+3];
  bands[4*(M-1)+3] /= bands[4*(M-1)+1];
  coefs[M*cstride] = bands[4*(M-1)+3];
  for (int row=M-2; row>=0; row--) 
    coefs[(row+1)*cstride] = 
      bands[4*(row)+3] - bands[4*(row)+2]*coefs[(row+2)*cstride] - lastCol[row]*coefs[M*cstride];
  
  coefs[0*cstride]     = -coefs[M*cstride];
  coefs[(M+1)*cstride] = -coefs[1*cstride];
  coefs[(M+2)*cstride] = -coefs[2*cstride];
}



void
find_coefs_1d_d (Ugrid grid, BCtype_d bc, 
		 double *data,  intptr_t dstride,
		 double *coefs, intptr_t cstride)
{
  int M = grid.num;
  double basis[4] = {1.0/6.0, 2.0/3.0, 1.0/6.0, 0.0};
  if (bc.lCode == PERIODIC || bc.lCode == ANTIPERIODIC) {
#ifdef HAVE_C_VARARRAYS
    double bands[M*4];
#else
    double *bands = malloc (4*M*sizeof(double));
#endif
    for (int i=0; i<M; i++) {
      bands[4*i+0] = basis[0];
      bands[4*i+1] = basis[1];
      bands[4*i+2] = basis[2];
      bands[4*i+3] = data[i*dstride];
    }
    if (bc.lCode == ANTIPERIODIC) 
      solve_antiperiodic_interp_1d_d (bands, coefs, M, cstride);
    else
      solve_periodic_interp_1d_d (bands, coefs, M, cstride);


#ifndef HAVE_C_VARARRAYS
    free (bands);
#endif
  }
  else {
    // Setup boundary conditions
    double abcd_left[4], abcd_right[4];
    // Left boundary
    if (bc.lCode == FLAT || bc.lCode == NATURAL)
      bc.lVal = 0.0;
    if (bc.lCode == FLAT || bc.lCode == DERIV1) {
      abcd_left[0] = -0.5 * grid.delta_inv;
      abcd_left[1] =  0.0 * grid.delta_inv; 
      abcd_left[2] =  0.5 * grid.delta_inv;
      abcd_left[3] =  bc.lVal;
    }
    if (bc.lCode == NATURAL || bc.lCode == DERIV2) {
      abcd_left[0] = 1.0 * grid.delta_inv * grid.delta_inv;
      abcd_left[1] =-2.0 * grid.delta_inv * grid.delta_inv;
      abcd_left[2] = 1.0 * grid.delta_inv * grid.delta_inv;
      abcd_left[3] = bc.lVal;
    }
    
    // Right boundary
    if (bc.rCode == FLAT || bc.rCode == NATURAL)
      bc.rVal = 0.0;
    if (bc.rCode == FLAT || bc.rCode == DERIV1) {
      abcd_right[0] = -0.5 * grid.delta_inv;
      abcd_right[1] =  0.0 * grid.delta_inv; 
      abcd_right[2] =  0.5 * grid.delta_inv;
      abcd_right[3] =  bc.rVal;
    }
    if (bc.rCode == NATURAL || bc.rCode == DERIV2) {
      abcd_right[0] = 1.0 *grid.delta_inv * grid.delta_inv;
      abcd_right[1] =-2.0 *grid.delta_inv * grid.delta_inv;
      abcd_right[2] = 1.0 *grid.delta_inv * grid.delta_inv;
      abcd_right[3] = bc.rVal;
    }
#ifdef HAVE_C_VARARRAYS
    double bands[(M+2)*4];
#else
    double *bands = malloc ((M+2)*4*sizeof(double));
#endif
    for (int i=0; i<4; i++) {
      bands[4*( 0 )+i] = abcd_left[i];
      bands[4*(M+1)+i] = abcd_right[i];
    }
    for (int i=0; i<M; i++) {
      for (int j=0; j<3; j++)
	bands[4*(i+1)+j] = basis[j];
      bands[4*(i+1)+3] = data[i*dstride];
    }   
    // Now, solve for coefficients
    solve_deriv_interp_1d_d (bands, coefs, M, cstride);
#ifndef HAVE_C_VARARRAYS
    free (bands);
#endif
  }
}

	       

UBspline_1d_d*
create_UBspline_1d_d (Ugrid x_grid, BCtype_d xBC, double *data)
{
  // Create new spline
  UBspline_1d_d* restrict spline = malloc (sizeof(UBspline_1d_d));
  spline->spcode = U1D;
  spline->tcode  = DOUBLE_REAL;
  spline->xBC = xBC; 

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

#ifndef HAVE_SSE2
  spline->coefs = malloc (sizeof(double)*N);
#else
  posix_memalign ((void**)&spline->coefs, 16, sizeof(double)*N);
#endif
  find_coefs_1d_d (spline->x_grid, xBC, data, 1, spline->coefs, 1);
    
  init_sse_data();
  return spline;
}

void
recompute_UBspline_1d_d (UBspline_1d_d* spline, double *data)
{
  find_coefs_1d_d (spline->x_grid, spline->xBC, data, 1, spline->coefs, 1);
}


UBspline_2d_d*
create_UBspline_2d_d (Ugrid x_grid, Ugrid y_grid,
		      BCtype_d xBC, BCtype_d yBC, double *data)
{
  // Create new spline
  UBspline_2d_d* restrict spline = malloc (sizeof(UBspline_2d_d));
  spline->spcode = U2D;
  spline->tcode  = DOUBLE_REAL;
  spline->xBC = xBC; 
  spline->yBC = yBC; 
 
  // Setup internal variables
  int Mx = x_grid.num;
  int My = y_grid.num;
  int Nx, Ny;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC)     Nx = Mx+3;
  else                           Nx = Mx+2;
  x_grid.delta = (x_grid.end - x_grid.start)/(double)(Nx-3);
  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;

  if (yBC.lCode == PERIODIC || yBC.lCode == ANTIPERIODIC)     Ny = My+3;
  else                           Ny = My+2;
  y_grid.delta = (y_grid.end - y_grid.start)/(double)(Ny-3);
  y_grid.delta_inv = 1.0/y_grid.delta;
  spline->y_grid   = y_grid;
  spline->x_stride = Ny;

#ifndef HAVE_SSE2
  spline->coefs = malloc (sizeof(double)*Nx*Ny);
#else
  posix_memalign ((void**)&spline->coefs, 16, (sizeof(double)*Nx*Ny));
#endif

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) {
    intptr_t doffset = iy;
    intptr_t coffset = iy;
    find_coefs_1d_d (spline->x_grid, xBC, data+doffset, My,
		     spline->coefs+coffset, Ny);
  }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) {
    intptr_t doffset = ix*Ny;
    intptr_t coffset = ix*Ny;
    find_coefs_1d_d (spline->y_grid, yBC, spline->coefs+doffset, 1, 
		     spline->coefs+coffset, 1);
  }

  init_sse_data();
  return spline;
}

void
recompute_UBspline_2d_d (UBspline_2d_d* spline, double *data)
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

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) {
    intptr_t doffset = iy;
    intptr_t coffset = iy;
    find_coefs_1d_d (spline->x_grid, spline->xBC, data+doffset, My,
		     spline->coefs+coffset, Ny);
  }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) {
    intptr_t doffset = ix*Ny;
    intptr_t coffset = ix*Ny;
    find_coefs_1d_d (spline->y_grid, spline->yBC, spline->coefs+doffset, 1, 
		     spline->coefs+coffset, 1);
  }
}


UBspline_3d_d*
create_UBspline_3d_d (Ugrid x_grid, Ugrid y_grid, Ugrid z_grid,
		      BCtype_d xBC, BCtype_d yBC, BCtype_d zBC,
		      double *data)
{
  // Create new spline
  UBspline_3d_d* restrict spline = malloc (sizeof(UBspline_3d_d));
  spline->spcode = U3D;
  spline->tcode  = DOUBLE_REAL;
  spline->xBC = xBC; 
  spline->yBC = yBC; 
  spline->zBC = zBC; 

  // Setup internal variables
  int Mx = x_grid.num;  int My = y_grid.num; int Mz = z_grid.num;
  int Nx, Ny, Nz;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC)     Nx = Mx+3;
  else                           Nx = Mx+2;
  x_grid.delta = (x_grid.end - x_grid.start)/(double)(Nx-3);
  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;

  if (yBC.lCode == PERIODIC || yBC.lCode == ANTIPERIODIC)     Ny = My+3;
  else                           Ny = My+2;
  y_grid.delta = (y_grid.end - y_grid.start)/(double)(Ny-3);
  y_grid.delta_inv = 1.0/y_grid.delta;
  spline->y_grid   = y_grid;

  if (zBC.lCode == PERIODIC || zBC.lCode == ANTIPERIODIC)     Nz = Mz+3;
  else                           Nz = Mz+2;
  z_grid.delta = (z_grid.end - z_grid.start)/(double)(Nz-3);
  z_grid.delta_inv = 1.0/z_grid.delta;
  spline->z_grid   = z_grid;

  spline->x_stride = Ny*Nz;
  spline->y_stride = Nz;

#ifndef HAVE_SSE2
  spline->coefs      = malloc (sizeof(double)*Nx*Ny*Nz);
#else
  posix_memalign ((void**)&spline->coefs, 16, (sizeof(double)*Nx*Ny*Nz));
#endif

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) 
    for (int iz=0; iz<Mz; iz++) {
      intptr_t doffset = iy*Mz+iz;
      intptr_t coffset = iy*Nz+iz;
      find_coefs_1d_d (spline->x_grid, xBC, data+doffset, My*Mz,
		       spline->coefs+coffset, Ny*Nz);
    }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iz=0; iz<Nz; iz++) {
      intptr_t doffset = ix*Ny*Nz + iz;
      intptr_t coffset = ix*Ny*Nz + iz;
      find_coefs_1d_d (spline->y_grid, yBC, spline->coefs+doffset, Nz, 
		       spline->coefs+coffset, Nz);
    }

  // Now, solve in the Z-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iy=0; iy<Ny; iy++) {
      intptr_t doffset = (ix*Ny+iy)*Nz;
      intptr_t coffset = (ix*Ny+iy)*Nz;
      find_coefs_1d_d (spline->z_grid, zBC, spline->coefs+doffset, 1, 
		       spline->coefs+coffset, 1);
    }

  init_sse_data();
  return spline;
}

void
recompute_UBspline_3d_d (UBspline_3d_d* spline, double *data)
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

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) 
    for (int iz=0; iz<Mz; iz++) {
      intptr_t doffset = iy*Mz+iz;
      intptr_t coffset = iy*Nz+iz;
      find_coefs_1d_d (spline->x_grid, spline->xBC, data+doffset, My*Mz,
		       spline->coefs+coffset, Ny*Nz);
    }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iz=0; iz<Nz; iz++) {
      intptr_t doffset = ix*Ny*Nz + iz;
      intptr_t coffset = ix*Ny*Nz + iz;
      find_coefs_1d_d (spline->y_grid, spline->yBC, spline->coefs+doffset, Nz, 
		       spline->coefs+coffset, Nz);
    }

  // Now, solve in the Z-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iy=0; iy<Ny; iy++) {
      intptr_t doffset = (ix*Ny+iy)*Nz;
      intptr_t coffset = (ix*Ny+iy)*Nz;
      find_coefs_1d_d (spline->z_grid, spline->zBC, spline->coefs+doffset, 1, 
		       spline->coefs+coffset, 1);
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


UBspline_1d_z*
create_UBspline_1d_z (Ugrid x_grid, BCtype_z xBC, complex_double *data)
{
  // Create new spline
  UBspline_1d_z* restrict spline = malloc (sizeof(UBspline_1d_z));
  spline->spcode = U1D;
  spline->tcode  = DOUBLE_COMPLEX;
  spline->xBC = xBC; 

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
#ifndef HAVE_SSE2
  spline->coefs = malloc (2*sizeof(double)*N);
#else
  posix_memalign ((void**)&spline->coefs, 16, 2*sizeof(double)*N);
#endif

  BCtype_d xBC_r, xBC_i;
  xBC_r.lCode = xBC.lCode;  xBC_r.rCode = xBC.rCode;
  xBC_r.lVal  = xBC.lVal_r; xBC_r.rVal  = xBC.rVal_r;
  xBC_i.lCode = xBC.lCode;  xBC_i.rCode = xBC.rCode;
  xBC_i.lVal  = xBC.lVal_i; xBC_i.rVal  = xBC.rVal_i;
  // Real part
  find_coefs_1d_d (spline->x_grid, xBC_r, (double*)data, 2, 
		   (double*)spline->coefs, 2);
  // Imaginarty part
  find_coefs_1d_d (spline->x_grid, xBC_i, ((double*)data)+1, 2, 
		   ((double*)spline->coefs)+1, 2);
 
  init_sse_data();   
  return spline;
}

void
recompute_UBspline_1d_z (UBspline_1d_z* spline, complex_double *data)
{
  int M = spline->x_grid.num;
  int N;

  if (spline->xBC.lCode == PERIODIC || spline->xBC.lCode == ANTIPERIODIC)   
    N = M+3;
  else                                 
    N = M+2;

  BCtype_d xBC_r, xBC_i;
  xBC_r.lCode = spline->xBC.lCode;  xBC_r.rCode = spline->xBC.rCode;
  xBC_r.lVal  = spline->xBC.lVal_r; xBC_r.rVal  = spline->xBC.rVal_r;
  xBC_i.lCode = spline->xBC.lCode;  xBC_i.rCode = spline->xBC.rCode;
  xBC_i.lVal  = spline->xBC.lVal_i; xBC_i.rVal  = spline->xBC.rVal_i;
  // Real part
  find_coefs_1d_d (spline->x_grid, xBC_r, (double*)data, 2, 
		   (double*)spline->coefs, 2);
  // Imaginarty part
  find_coefs_1d_d (spline->x_grid, xBC_i, ((double*)data)+1, 2, 
		   ((double*)spline->coefs)+1, 2);
 
}


UBspline_2d_z*
create_UBspline_2d_z (Ugrid x_grid, Ugrid y_grid,
		      BCtype_z xBC, BCtype_z yBC, complex_double *data)
{
  // Create new spline
  UBspline_2d_z* restrict spline = malloc (sizeof(UBspline_2d_z));
  spline->spcode = U2D;
  spline->tcode  = DOUBLE_COMPLEX;
  spline->xBC = xBC; 
  spline->yBC = yBC; 

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
  spline->x_stride = Ny;

#ifndef HAVE_SSE2
  spline->coefs = malloc (2*sizeof(double)*Nx*Ny);
#else
  posix_memalign ((void**)&spline->coefs, 16, 2*sizeof(double)*Nx*Ny);
#endif

  BCtype_d xBC_r, xBC_i, yBC_r, yBC_i;
  xBC_r.lCode = xBC.lCode;  xBC_r.rCode = xBC.rCode;
  xBC_r.lVal  = xBC.lVal_r; xBC_r.rVal  = xBC.rVal_r;
  xBC_i.lCode = xBC.lCode;  xBC_i.rCode = xBC.rCode;
  xBC_i.lVal  = xBC.lVal_i; xBC_i.rVal  = xBC.rVal_i;
  yBC_r.lCode = yBC.lCode;  yBC_r.rCode = yBC.rCode;
  yBC_r.lVal  = yBC.lVal_r; yBC_r.rVal  = yBC.rVal_r;
  yBC_i.lCode = yBC.lCode;  yBC_i.rCode = yBC.rCode;
  yBC_i.lVal  = yBC.lVal_i; yBC_i.rVal  = yBC.rVal_i;
  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) {
    intptr_t doffset = 2*iy;
    intptr_t coffset = 2*iy;
    // Real part
    find_coefs_1d_d (spline->x_grid, xBC_r, ((double*)data+doffset), 2*My,
		     (double*)spline->coefs+coffset, 2*Ny);
    // Imag part
    find_coefs_1d_d (spline->x_grid, xBC_i, ((double*)data)+doffset+1, 2*My,
		     ((double*)spline->coefs)+coffset+1, 2*Ny);
  }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) {
    intptr_t doffset = 2*ix*Ny;
    intptr_t coffset = 2*ix*Ny;
    // Real part
    find_coefs_1d_d (spline->y_grid, yBC_r, ((double*)spline->coefs)+doffset, 2, 
		     (double*)spline->coefs+coffset, 2);
    // Imag part
    find_coefs_1d_d (spline->y_grid, yBC_i, (double*)spline->coefs+doffset+1, 2, 
		     ((double*)spline->coefs)+coffset+1, 2);
  }

  init_sse_data();
  return spline;
}


void
recompute_UBspline_2d_z (UBspline_2d_z* spline, complex_double *data)
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

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) {
    intptr_t doffset = 2*iy;
    intptr_t coffset = 2*iy;
    // Real part
    find_coefs_1d_d (spline->x_grid, xBC_r, ((double*)data+doffset), 2*My,
		     (double*)spline->coefs+coffset, 2*Ny);
    // Imag part
    find_coefs_1d_d (spline->x_grid, xBC_i, ((double*)data)+doffset+1, 2*My,
		     ((double*)spline->coefs)+coffset+1, 2*Ny);
  }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) {
    intptr_t doffset = 2*ix*Ny;
    intptr_t coffset = 2*ix*Ny;
    // Real part
    find_coefs_1d_d (spline->y_grid, yBC_r, ((double*)spline->coefs)+doffset, 2, 
		     (double*)spline->coefs+coffset, 2);
    // Imag part
    find_coefs_1d_d (spline->y_grid, yBC_i, (double*)spline->coefs+doffset+1, 2, 
		     ((double*)spline->coefs)+coffset+1, 2);
  }
}



UBspline_3d_z*
create_UBspline_3d_z (Ugrid x_grid, Ugrid y_grid, Ugrid z_grid,
		      BCtype_z xBC, BCtype_z yBC, BCtype_z zBC,
		      complex_double *data)
{
  // Create new spline
  UBspline_3d_z* restrict spline = malloc (sizeof(UBspline_3d_z));
  spline->spcode = U3D;
  spline->tcode  = DOUBLE_COMPLEX;
  spline->xBC = xBC; 
  spline->yBC = yBC; 
  spline->zBC = zBC;

  // Setup internal variables
  int Mx = x_grid.num;  int My = y_grid.num; int Mz = z_grid.num;
  int Nx, Ny, Nz;

  if (xBC.lCode == PERIODIC || xBC.lCode == ANTIPERIODIC)     Nx = Mx+3;
  else                           Nx = Mx+2;
  x_grid.delta = (x_grid.end - x_grid.start)/(double)(Nx-3);
  x_grid.delta_inv = 1.0/x_grid.delta;
  spline->x_grid   = x_grid;

  if (yBC.lCode == PERIODIC || yBC.lCode == ANTIPERIODIC)     Ny = My+3;
  else                           Ny = My+2;
  y_grid.delta = (y_grid.end - y_grid.start)/(double)(Ny-3);
  y_grid.delta_inv = 1.0/y_grid.delta;
  spline->y_grid   = y_grid;

  if (zBC.lCode == PERIODIC || zBC.lCode == ANTIPERIODIC)     Nz = Mz+3;
  else                           Nz = Mz+2;
  z_grid.delta = (z_grid.end - z_grid.start)/(double)(Nz-3);
  z_grid.delta_inv = 1.0/z_grid.delta;
  spline->z_grid   = z_grid;

  spline->x_stride = Ny*Nz;
  spline->y_stride = Nz;

#ifndef HAVE_SSE2
  spline->coefs      = malloc (2*sizeof(double)*Nx*Ny*Nz);
#else
  posix_memalign ((void**)&spline->coefs, 16, 2*sizeof(double)*Nx*Ny*Nz);
#endif

  BCtype_d xBC_r, xBC_i, yBC_r, yBC_i, zBC_r, zBC_i;
  xBC_r.lCode = xBC.lCode;  xBC_r.rCode = xBC.rCode;
  xBC_r.lVal  = xBC.lVal_r; xBC_r.rVal  = xBC.rVal_r;
  xBC_i.lCode = xBC.lCode;  xBC_i.rCode = xBC.rCode;
  xBC_i.lVal  = xBC.lVal_i; xBC_i.rVal  = xBC.rVal_i;
  yBC_r.lCode = yBC.lCode;  yBC_r.rCode = yBC.rCode;
  yBC_r.lVal  = yBC.lVal_r; yBC_r.rVal  = yBC.rVal_r;
  yBC_i.lCode = yBC.lCode;  yBC_i.rCode = yBC.rCode;
  yBC_i.lVal  = yBC.lVal_i; yBC_i.rVal  = yBC.rVal_i;
  zBC_r.lCode = zBC.lCode;  zBC_r.rCode = zBC.rCode;
  zBC_r.lVal  = zBC.lVal_r; zBC_r.rVal  = zBC.rVal_r;
  zBC_i.lCode = zBC.lCode;  zBC_i.rCode = zBC.rCode;
  zBC_i.lVal  = zBC.lVal_i; zBC_i.rVal  = zBC.rVal_i;
  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) 
    for (int iz=0; iz<Mz; iz++) {
      intptr_t doffset = 2*(iy*Mz+iz);
      intptr_t coffset = 2*(iy*Nz+iz);
      // Real part
      find_coefs_1d_d (spline->x_grid, xBC_r, ((double*)data)+doffset, 2*My*Mz,
		       ((double*)spline->coefs)+coffset, 2*Ny*Nz);
      // Imag part
      find_coefs_1d_d (spline->x_grid, xBC_i, ((double*)data)+doffset+1, 2*My*Mz,
		       ((double*)spline->coefs)+coffset+1, 2*Ny*Nz);
    }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iz=0; iz<Nz; iz++) {
      intptr_t doffset = 2*(ix*Ny*Nz + iz);
      intptr_t coffset = 2*(ix*Ny*Nz + iz);
      // Real part
      find_coefs_1d_d (spline->y_grid, yBC_r, ((double*)spline->coefs)+doffset, 2*Nz, 
		       ((double*)spline->coefs)+coffset, 2*Nz);
      // Imag part
      find_coefs_1d_d (spline->y_grid, yBC_i, ((double*)spline->coefs)+doffset+1, 2*Nz, 
		       ((double*)spline->coefs)+coffset+1, 2*Nz);
    }

  // Now, solve in the Z-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iy=0; iy<Ny; iy++) {
      intptr_t doffset = 2*((ix*Ny+iy)*Nz);
      intptr_t coffset = 2*((ix*Ny+iy)*Nz);
      // Real part
      find_coefs_1d_d (spline->z_grid, zBC_r, ((double*)spline->coefs)+doffset, 2, 
		       ((double*)spline->coefs)+coffset, 2);
      // Imag part
      find_coefs_1d_d (spline->z_grid, zBC_i, ((double*)spline->coefs)+doffset+1, 2, 
		       ((double*)spline->coefs)+coffset+1, 2);
    }
  init_sse_data();
  return spline;
}

void
recompute_UBspline_3d_z (UBspline_3d_z* spline, complex_double *data)
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
  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) 
    for (int iz=0; iz<Mz; iz++) {
      intptr_t doffset = 2*(iy*Mz+iz);
      intptr_t coffset = 2*(iy*Nz+iz);
      // Real part
      find_coefs_1d_d (spline->x_grid, xBC_r, ((double*)data)+doffset, 2*My*Mz,
		       ((double*)spline->coefs)+coffset, 2*Ny*Nz);
      // Imag part
      find_coefs_1d_d (spline->x_grid, xBC_i, ((double*)data)+doffset+1, 2*My*Mz,
		       ((double*)spline->coefs)+coffset+1, 2*Ny*Nz);
    }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iz=0; iz<Nz; iz++) {
      intptr_t doffset = 2*(ix*Ny*Nz + iz);
      intptr_t coffset = 2*(ix*Ny*Nz + iz);
      // Real part
      find_coefs_1d_d (spline->y_grid, yBC_r, ((double*)spline->coefs)+doffset, 2*Nz, 
		       ((double*)spline->coefs)+coffset, 2*Nz);
      // Imag part
      find_coefs_1d_d (spline->y_grid, yBC_i, ((double*)spline->coefs)+doffset+1, 2*Nz, 
		       ((double*)spline->coefs)+coffset+1, 2*Nz);
    }

  // Now, solve in the Z-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iy=0; iy<Ny; iy++) {
      intptr_t doffset = 2*((ix*Ny+iy)*Nz);
      intptr_t coffset = 2*((ix*Ny+iy)*Nz);
      // Real part
      find_coefs_1d_d (spline->z_grid, zBC_r, ((double*)spline->coefs)+doffset, 2, 
		       ((double*)spline->coefs)+coffset, 2);
      // Imag part
      find_coefs_1d_d (spline->z_grid, zBC_i, ((double*)spline->coefs)+doffset+1, 2, 
		       ((double*)spline->coefs)+coffset+1, 2);
    }
}


void
destroy_UBspline (Bspline *spline)
{
  free (spline->coefs);
  free (spline);
}

void 
destroy_NUBspline (Bspline *spline);

void
destroy_multi_UBspline (Bspline *spline);

void
destroy_Bspline (void *spline)
{
  Bspline *sp = (Bspline *)spline;
  if (sp->sp_code <= U3D) 
    destroy_UBspline (sp);
  else if (sp->sp_code <= NU3D) 
    destroy_NUBspline (sp);
  else if (sp->sp_code <= MULTI_U3D)
    destroy_multi_UBspline (sp);
  else
    fprintf (stderr, "Error in destroy_Bspline:  invalide spline code %d.\n",
	     sp->sp_code);
}
