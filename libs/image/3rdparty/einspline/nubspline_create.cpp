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

#include "nubspline_create.h"
#include <math.h>
#include <assert.h>
#ifndef _XOPEN_SOURCE
  #define _XOPEN_SOURCE 600
#endif
#ifndef __USE_XOPEN2K
  #define __USE_XOPEN2K
#endif
#include <stdlib.h>
#include <stdio.h>

////////////////////////////////////////////////////////
// Notes on conventions:                              //
// Below, M (and Mx, My, Mz) represent the number of  //
// data points to be interpolated.  With derivative   //
// boundary conditions, it is equal to the number of  //
// grid points.  With periodic boundary conditions,   //
// it is one less than the number of grid points.     //
// N (and Nx, Ny, Nz) is the number of B-spline       //
// coefficients, which is #(grid points)+2 for all    //
// boundary conditions.                               //
////////////////////////////////////////////////////////


////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//// Single-precision real creation routines        ////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
void
solve_NUB_deriv_interp_1d_s (NUBasis* restrict basis, 
			     float* restrict data, int datastride,
			     float* restrict    p, int pstride,
			     float abcdInitial[4], float abcdFinal[4])
{
  int M = basis->grid->num_points;
  int N = M+2;
  // Banded matrix storage.  The first three elements in the
  // tinyvector store the tridiagonal coefficients.  The last element
  // stores the RHS data.
#ifdef HAVE_C_VARARRAYS
  float bands[4*N];
#else
  float *bands = new float[4*N];
#endif
  

  // Fill up bands
  for (int i=0; i<4; i++) {
    bands[i]         = abcdInitial[i];
    bands[4*(N-1)+i] = abcdFinal[i];
  }
  for (int i=0; i<M; i++) {
    get_NUBasis_funcs_si (basis, i, &(bands[4*(i+1)]));
    bands[4*(i+1)+3] = data[datastride*i];
  }
    
  // Now solve:
  // First and last rows are different
  bands[4*0+1] /= bands[4*0+0];
  bands[4*0+2] /= bands[4*0+0];
  bands[4*0+3] /= bands[4*0+0];
  bands[4*0+0] = 1.0;
  bands[4*1+1] -= bands[4*1+0]*bands[4*0+1];
  bands[4*1+2] -= bands[4*1+0]*bands[4*0+2];
  bands[4*1+3] -= bands[4*1+0]*bands[4*0+3];
  bands[4*0+0] = 0.0;
  bands[4*1+2] /= bands[4*1+1];
  bands[4*1+3] /= bands[4*1+1];
  bands[4*1+1] = 1.0;
  
  // Now do rows 2 through M+1
  for (int row=2; row < N-1; row++) {
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

  p[pstride*(M+1)] = bands[4*(M+1)+3];
  // Now back substitute up
  for (int row=M; row>0; row--)
    p[pstride*(row)] = bands[4*(row)+3] - bands[4*(row)+2]*p[pstride*(row+1)];
  
  // Finish with first row
  p[0] = bands[4*(0)+3] - bands[4*(0)+1]*p[pstride*1] - bands[4*(0)+2]*p[pstride*2];

#ifndef HAVE_C_VARARRAYS
  delete[] bands;
#endif
}



// The number of elements in data should be one less than the number
// of grid points 
void
solve_NUB_periodic_interp_1d_s (NUBasis* restrict basis,
				float* restrict data, int datastride,
				float* restrict p, int pstride)
{
  int M = basis->grid->num_points-1;

  // Banded matrix storage.  The first three elements in each row
  // store the tridiagonal coefficients.  The last element
  // stores the RHS data.
#ifdef HAVE_C_VARARRAYS
  float bands[4*M], lastCol[M];
#else
  float *bands   = new float[4*M];
  float *lastCol = new float[  M];
#endif

  // Fill up bands
  for (int i=0; i<M; i++) {
    get_NUBasis_funcs_si (basis, i, &(bands[4*i])); 
    bands[4*(i)+3] = data[i*datastride];
  }
    
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
  p[pstride*M] = bands[4*(M-1)+3];
  for (int row=M-2; row>=0; row--) 
    p[pstride*(row+1)] = bands[4*(row)+3] - 
      bands[4*(row)+2]*p[pstride*(row+2)] - lastCol[row]*p[pstride*M];
  
  p[pstride*  0  ] = p[pstride*M];
  p[pstride*(M+1)] = p[pstride*1];
  p[pstride*(M+2)] = p[pstride*2];
#ifndef HAVE_C_VARARRAYS
  delete[] bands;
  delete[] lastCol;
#endif
}



void
find_NUBcoefs_1d_s (NUBasis* restrict basis, BCtype_s bc,
		    float *data,  int dstride,
		    float *coefs, int cstride)
{
  if (bc.lCode == PERIODIC) 
    solve_NUB_periodic_interp_1d_s (basis, data, dstride, coefs, cstride);
  else {
    int M = basis->grid->num_points;
    // Setup boundary conditions
    float bfuncs[4] = {};
    float dbfuncs[4] = {};
    float abcd_left[4] = {};
    float abcd_right[4] = {};
    // Left boundary
    if (bc.lCode == FLAT || bc.lCode == NATURAL)
      bc.lVal = 0.0;
    if (bc.lCode == FLAT || bc.lCode == DERIV1) {
      get_NUBasis_dfuncs_si (basis, 0, bfuncs, abcd_left);
      abcd_left[3] = bc.lVal;
    }
    if (bc.lCode == NATURAL || bc.lCode == DERIV2) {
      get_NUBasis_d2funcs_si (basis, 0, bfuncs, dbfuncs, abcd_left);
      abcd_left[3] = bc.lVal;
    }
    
    // Right boundary
    if (bc.rCode == FLAT || bc.rCode == NATURAL)
      bc.rVal = 0.0;
    if (bc.rCode == FLAT || bc.rCode == DERIV1) {
      get_NUBasis_dfuncs_si (basis, M-1, bfuncs, abcd_right);
      abcd_right[3] = bc.rVal;
    }
    if (bc.rCode == NATURAL || bc.rCode == DERIV2) {
      get_NUBasis_d2funcs_si (basis, M-1, bfuncs, dbfuncs, abcd_right);
      abcd_right[3] = bc.rVal;
    }
    // Now, solve for coefficients
    solve_NUB_deriv_interp_1d_s (basis, data, dstride, coefs, cstride,
				 abcd_left, abcd_right);
  }
}




NUBspline_1d_s *
create_NUBspline_1d_s (NUgrid* x_grid, BCtype_s xBC, float *data)
{
  // First, create the spline structure
  NUBspline_1d_s* spline = new NUBspline_1d_s;
  if (spline == NULL)
    return spline;
  spline->sp_code = NU1D;
  spline->t_code  = SINGLE_REAL;

  // Next, create the basis
  spline->x_basis = create_NUBasis (x_grid, xBC.lCode==PERIODIC);
  // M is the number of data points (but is unused)
//  int M;
//  if (xBC.lCode == PERIODIC) M = x_grid->num_points - 1;
//  else                       M = x_grid->num_points;
  int N = x_grid->num_points + 2;

  // Allocate coefficients and solve  
  spline->coefs = (float*)malloc (sizeof(float)*N);
  find_NUBcoefs_1d_s (spline->x_basis, xBC, data, 1, spline->coefs, 1);
    
  return spline;
}

NUBspline_2d_s *
create_NUBspline_2d_s (NUgrid* x_grid, NUgrid* y_grid,
		       BCtype_s xBC, BCtype_s yBC, float *data)
{
  // First, create the spline structure
  NUBspline_2d_s* spline = new NUBspline_2d_s;
  if (spline == NULL)
    return spline;
  spline->sp_code = NU2D;
  spline->t_code  = SINGLE_REAL;

  // Next, create the bases
  spline->x_basis = create_NUBasis (x_grid, xBC.lCode==PERIODIC);
  spline->y_basis = create_NUBasis (y_grid, yBC.lCode==PERIODIC);
  // set but unused
  //int Mx,
  int My, Nx, Ny;
//  if (xBC.lCode == PERIODIC) Mx = x_grid->num_points - 1;
//  else                       Mx = x_grid->num_points;
  if (yBC.lCode == PERIODIC) My = y_grid->num_points - 1;
  else                       My = y_grid->num_points;

  Nx = x_grid->num_points + 2;
  Ny = y_grid->num_points + 2;
    
  spline->x_stride = Ny;
#ifndef HAVE_SSE2
  spline->coefs = (float*)malloc (sizeof(float)*Nx*Ny);
#else
  posix_memalign ((void**)&spline->coefs, 16, sizeof(float)*Nx*Ny);
#endif

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) {
    int doffset = iy;
    int coffset = iy;
    find_NUBcoefs_1d_s (spline->x_basis, xBC, data+doffset, My,
			spline->coefs+coffset, Ny);
  }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) {
    int doffset = ix*Ny;
    int coffset = ix*Ny;
    find_NUBcoefs_1d_s (spline->y_basis, yBC, spline->coefs+doffset, 1, 
			spline->coefs+coffset, 1);
  }
    
  return spline;
}


NUBspline_3d_s *
create_NUBspline_3d_s (NUgrid* x_grid, NUgrid* y_grid, NUgrid* z_grid,
		       BCtype_s xBC, BCtype_s yBC, BCtype_s zBC, float *data)
{
  // First, create the spline structure
  NUBspline_3d_s* spline = new NUBspline_3d_s;
  if (spline == NULL)
    return spline;
  spline->sp_code = NU3D;
  spline->t_code  = SINGLE_REAL;

  // Next, create the bases
  spline->x_basis = create_NUBasis (x_grid, xBC.lCode==PERIODIC);
  spline->y_basis = create_NUBasis (y_grid, yBC.lCode==PERIODIC);
  spline->z_basis = create_NUBasis (z_grid, zBC.lCode==PERIODIC);
  // set but unused
  // int Mx,
  int My, Mz, Nx, Ny, Nz;
//  if (xBC.lCode == PERIODIC) Mx = x_grid->num_points - 1;
//  else                       Mx = x_grid->num_points;
  if (yBC.lCode == PERIODIC) My = y_grid->num_points - 1;
  else                       My = y_grid->num_points;
  if (zBC.lCode == PERIODIC) Mz = z_grid->num_points - 1;
  else                       Mz = z_grid->num_points;

  Nx = x_grid->num_points + 2;
  Ny = y_grid->num_points + 2;
  Nz = z_grid->num_points + 2;

  // Allocate coefficients and solve  
  spline->x_stride = Ny*Nz;
  spline->y_stride = Nz;
#ifndef HAVE_SSE2
  spline->coefs = (float*)malloc (sizeof(float)*Nx*Ny*Nz);
#else
  posix_memalign ((void**)&spline->coefs, 16, sizeof(float)*Nx*Ny*Nz);
#endif

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) 
    for (int iz=0; iz<Mz; iz++) {
      int doffset = iy*Mz+iz;
      int coffset = iy*Nz+iz;
      find_NUBcoefs_1d_s (spline->x_basis, xBC, data+doffset, My*Mz,
			  spline->coefs+coffset, Ny*Nz);
    }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iz=0; iz<Nz; iz++) {
      int doffset = ix*Ny*Nz + iz;
      int coffset = ix*Ny*Nz + iz;
      find_NUBcoefs_1d_s (spline->y_basis, yBC, spline->coefs+doffset, Nz, 
			  spline->coefs+coffset, Nz);
    }

  // Now, solve in the Z-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iy=0; iy<Ny; iy++) {
      int doffset = (ix*Ny+iy)*Nz;
      int coffset = (ix*Ny+iy)*Nz;
      find_NUBcoefs_1d_s (spline->z_basis, zBC, spline->coefs+doffset, 1, 
			  spline->coefs+coffset, 1);
    }
  return spline;
}

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//// Double-precision real creation routines        ////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
void
solve_NUB_deriv_interp_1d_d (NUBasis* restrict basis, 
			     double* restrict data, int datastride,
			     double* restrict    p, int pstride,
			     double abcdInitial[4], double abcdFinal[4])
{
  int M = basis->grid->num_points;
  int N = M+2;
  // Banded matrix storage.  The first three elements in the
  // tinyvector store the tridiagonal coefficients.  The last element
  // stores the RHS data.
#ifdef HAVE_C_VARARRAYS
  double bands[4*N];
#else
  double *bands = new double[4*N];
#endif

  // Fill up bands
  for (int i=0; i<4; i++) {
    bands[i]         = abcdInitial[i];
    bands[4*(N-1)+i] = abcdFinal[i];
  }
  for (int i=0; i<M; i++) {
    get_NUBasis_funcs_di (basis, i, &(bands[4*(i+1)]));
    bands[4*(i+1)+3] = data[datastride*i];
  }
  /* for (int i=0; i<4*N; i++)
     if (isnan(bands[i]))
     fprintf(stderr, "NAN at bands[%d].\n", i); */
    
  // Now solve:
  // First and last rows are different
  bands[4*0+1] /= bands[4*0+0];
  bands[4*0+2] /= bands[4*0+0];
  bands[4*0+3] /= bands[4*0+0];
  bands[4*0+0] = 1.0;
  bands[4*1+1] -= bands[4*1+0]*bands[4*0+1];
  bands[4*1+2] -= bands[4*1+0]*bands[4*0+2];
  bands[4*1+3] -= bands[4*1+0]*bands[4*0+3];
  bands[4*0+0] = 0.0;
  bands[4*1+2] /= bands[4*1+1];
  bands[4*1+3] /= bands[4*1+1];
  bands[4*1+1] = 1.0;
  
  // Now do rows 2 through M+1
  for (int row=2; row < N-1; row++) {
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

  p[pstride*(M+1)] = bands[4*(M+1)+3];
  // Now back substitute up
  for (int row=M; row>0; row--)
    p[pstride*(row)] = bands[4*(row)+3] - bands[4*(row)+2]*p[pstride*(row+1)];
  
  // Finish with first row
  p[0] = bands[4*(0)+3] - bands[4*(0)+1]*p[pstride*1] - bands[4*(0)+2]*p[pstride*2];
#ifndef HAVE_C_VARARRAYS
  delete[] bands;
#endif
}


void
solve_NUB_periodic_interp_1d_d (NUBasis* restrict basis,
				double* restrict data, int datastride,
				double* restrict p, int pstride)
{
  int M = basis->grid->num_points-1;

  // Banded matrix storage.  The first three elements in the
  // tinyvector store the tridiagonal coefficients.  The last element
  // stores the RHS data.
#ifdef HAVE_C_VARARRAYS
  double bands[4*M], lastCol[M];
#else
  double *bands   = new double[4*M];
  double *lastCol = new double[  M];
#endif

  // Fill up bands
  for (int i=0; i<M; i++) {
    get_NUBasis_funcs_di (basis, i, &(bands[4*i])); 
    bands[4*(i)+3] = data[i*datastride];
  }
    
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
  p[pstride*M] = bands[4*(M-1)+3];
  for (int row=M-2; row>=0; row--) 
    p[pstride*(row+1)] = bands[4*(row)+3] - 
      bands[4*(row)+2]*p[pstride*(row+2)] - lastCol[row]*p[pstride*M];
  
  p[pstride*  0  ] = p[pstride*M];
  p[pstride*(M+1)] = p[pstride*1];
  p[pstride*(M+2)] = p[pstride*2];
#ifndef HAVE_C_VARARRAYS
  delete[] bands;
  delete[] lastCol;
#endif
}



void
find_NUBcoefs_1d_d (NUBasis* restrict basis, BCtype_d bc,
		    double *data,  int dstride,
		    double *coefs, int cstride)
{
  if (bc.lCode == PERIODIC) 
    solve_NUB_periodic_interp_1d_d (basis, data, dstride, coefs, cstride);
  else {
    int M = basis->grid->num_points;
    // Setup boundary conditions
    double bfuncs[4], dbfuncs[4], abcd_left[4], abcd_right[4];
    // Left boundary
    if (bc.lCode == FLAT || bc.lCode == NATURAL)
      bc.lVal = 0.0;
    if (bc.lCode == FLAT || bc.lCode == DERIV1) {
      get_NUBasis_dfuncs_di (basis, 0, bfuncs, abcd_left);
      abcd_left[3] = bc.lVal;
    }
    if (bc.lCode == NATURAL || bc.lCode == DERIV2) {
      get_NUBasis_d2funcs_di (basis, 0, bfuncs, dbfuncs, abcd_left);
      abcd_left[3] = bc.lVal;
    }
    
    // Right boundary
    if (bc.rCode == FLAT || bc.rCode == NATURAL)
      bc.rVal = 0.0;
    if (bc.rCode == FLAT || bc.rCode == DERIV1) {
      get_NUBasis_dfuncs_di (basis, M-1, bfuncs, abcd_right);
      abcd_right[3] = bc.rVal;
    }
    if (bc.rCode == NATURAL || bc.rCode == DERIV2) {
      get_NUBasis_d2funcs_di (basis, M-1, bfuncs, dbfuncs, abcd_right);
      abcd_right[3] = bc.rVal;
    }

    // Now, solve for coefficients
    solve_NUB_deriv_interp_1d_d (basis, data, dstride, coefs, cstride,
				 abcd_left, abcd_right);
  }
}




NUBspline_1d_d *
create_NUBspline_1d_d (NUgrid* x_grid, BCtype_d xBC, double *data)
{
  // First, create the spline structure
  NUBspline_1d_d* spline = new NUBspline_1d_d;
  if (spline == NULL)
    return spline;
  spline->sp_code = NU1D;
  spline->t_code  = DOUBLE_REAL;

  // Next, create the basis
  spline->x_basis = create_NUBasis (x_grid, xBC.lCode==PERIODIC);
  // M is the number of data points (set but unused)
//  int M;
//  if (xBC.lCode == PERIODIC) M = x_grid->num_points - 1;
//  else                       M = x_grid->num_points;
  int N = x_grid->num_points + 2;

  // Allocate coefficients and solve
  spline->coefs = (double*)malloc (sizeof(double)*N);
  find_NUBcoefs_1d_d (spline->x_basis, xBC, data, 1, spline->coefs, 1);
    
  return spline;
}

NUBspline_2d_d *
create_NUBspline_2d_d (NUgrid* x_grid, NUgrid* y_grid,
		       BCtype_d xBC, BCtype_d yBC, double *data)
{
  // First, create the spline structure
  NUBspline_2d_d* spline = new NUBspline_2d_d;
  if (spline == NULL)
    return spline;
  spline->sp_code = NU2D;
  spline->t_code  = DOUBLE_REAL;

  // Next, create the bases
  spline->x_basis = create_NUBasis (x_grid, xBC.lCode==PERIODIC);
  spline->y_basis = create_NUBasis (y_grid, yBC.lCode==PERIODIC);

  // int Mx, (set but unused)
  int My, Nx, Ny;
//  if (xBC.lCode == PERIODIC) Mx = x_grid->num_points - 1;
//  else                       Mx = x_grid->num_points;
  if (yBC.lCode == PERIODIC) My = y_grid->num_points - 1;
  else                       My = y_grid->num_points;

  Nx = x_grid->num_points + 2;
  Ny = y_grid->num_points + 2;
  
  spline->x_stride = Ny;
#ifndef HAVE_SSE2
  spline->coefs = (double*)malloc (sizeof(double)*Nx*Ny);
#else
  posix_memalign ((void**)&spline->coefs, 16, sizeof(double)*Nx*Ny);
#endif

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) {
    int doffset = iy;
    int coffset = iy;
    find_NUBcoefs_1d_d (spline->x_basis, xBC, data+doffset, My,
			spline->coefs+coffset, Ny);
  }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) {
    int doffset = ix*Ny;
    int coffset = ix*Ny;
    find_NUBcoefs_1d_d (spline->y_basis, yBC, spline->coefs+doffset, 1, 
			spline->coefs+coffset, 1);
  }
    
  return spline;
}


NUBspline_3d_d *
create_NUBspline_3d_d (NUgrid* x_grid, NUgrid* y_grid, NUgrid* z_grid,
		       BCtype_d xBC, BCtype_d yBC, BCtype_d zBC, double *data)
{
  // First, create the spline structure
  NUBspline_3d_d* spline = new NUBspline_3d_d;
  if (spline == NULL)
    return spline;
  spline->sp_code = NU3D;
  spline->t_code  = DOUBLE_REAL;

  // Next, create the bases
  spline->x_basis = create_NUBasis (x_grid, xBC.lCode==PERIODIC);
  spline->y_basis = create_NUBasis (y_grid, yBC.lCode==PERIODIC);
  spline->z_basis = create_NUBasis (z_grid, zBC.lCode==PERIODIC);

  // set but unused
  //int Mx,
  int My, Mz, Nx, Ny, Nz;
//  if (xBC.lCode == PERIODIC) Mx = x_grid->num_points - 1;
//  else                       Mx = x_grid->num_points;
  if (yBC.lCode == PERIODIC) My = y_grid->num_points - 1;
  else                       My = y_grid->num_points;
  if (zBC.lCode == PERIODIC) Mz = z_grid->num_points - 1;
  else                       Mz = z_grid->num_points;

  Nx = x_grid->num_points + 2;
  Ny = y_grid->num_points + 2;
  Nz = z_grid->num_points + 2;
  
  spline->x_stride = Ny*Nz;
  spline->y_stride = Nz;
#ifndef HAVE_SSE2
  spline->coefs = (double*)malloc (sizeof(double)*Nx*Ny*Nz);
#else
  posix_memalign ((void**)&spline->coefs, 16, sizeof(double)*Nx*Ny*Nz);
#endif

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) 
    for (int iz=0; iz<Mz; iz++) {
      int doffset = iy*Mz+iz;
      int coffset = iy*Nz+iz;
      find_NUBcoefs_1d_d (spline->x_basis, xBC, data+doffset, My*Mz,
			  spline->coefs+coffset, Ny*Nz);
    }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iz=0; iz<Nz; iz++) {
      int doffset = ix*Ny*Nz + iz;
      int coffset = ix*Ny*Nz + iz;
      find_NUBcoefs_1d_d (spline->y_basis, yBC, spline->coefs+doffset, Nz, 
			  spline->coefs+coffset, Nz);
    }

  // Now, solve in the Z-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iy=0; iy<Ny; iy++) {
      int doffset = (ix*Ny+iy)*Nz;
      int coffset = (ix*Ny+iy)*Nz;
      find_NUBcoefs_1d_d (spline->z_basis, zBC, spline->coefs+doffset, 1, 
			  spline->coefs+coffset, 1);
    }
  return spline;
}


////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//// Single-precision complex creation routines     ////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

void
find_NUBcoefs_1d_c (NUBasis* restrict basis, BCtype_c bc,
		    complex_float *data,  int dstride,
		    complex_float *coefs, int cstride)
{
  BCtype_s bc_r, bc_i;
  bc_r.lCode = bc.lCode;   bc_i.lCode = bc.lCode;
  bc_r.rCode = bc.rCode;   bc_i.rCode = bc.rCode;
  bc_r.lVal  = bc.lVal_r;  bc_r.rVal  = bc.rVal_r;
  bc_i.lVal  = bc.lVal_i;  bc_i.rVal  = bc.rVal_i;

  float *data_r  = ((float*)data );
  float *data_i  = ((float*)data )+1;
  float *coefs_r = ((float*)coefs);
  float *coefs_i = ((float*)coefs)+1;

  find_NUBcoefs_1d_s (basis, bc_r, data_r, 2*dstride, coefs_r, 2*cstride);
  find_NUBcoefs_1d_s (basis, bc_i, data_i, 2*dstride, coefs_i, 2*cstride);
}


NUBspline_1d_c *
create_NUBspline_1d_c (NUgrid* x_grid, BCtype_c xBC, complex_float *data)
{
  // First, create the spline structure
  NUBspline_1d_c* spline = new NUBspline_1d_c;
  if (spline == NULL)
    return spline;
  spline->sp_code = NU1D;
  spline->t_code  = SINGLE_COMPLEX;

  // Next, create the basis
  spline->x_basis = create_NUBasis (x_grid, xBC.lCode==PERIODIC);
  // M is the number of data points
//  int M;
//  if (xBC.lCode == PERIODIC) M = x_grid->num_points - 1;
//  else                       M = x_grid->num_points;
  int N = x_grid->num_points + 2;

  // Allocate coefficients and solve  
  spline->coefs = (complex_float*)malloc (sizeof(complex_float)*N);
  find_NUBcoefs_1d_c (spline->x_basis, xBC, data, 1, spline->coefs, 1);
    
  return spline;
}

NUBspline_2d_c *
create_NUBspline_2d_c (NUgrid* x_grid, NUgrid* y_grid,
		       BCtype_c xBC, BCtype_c yBC, complex_float *data)
{
  // First, create the spline structure
  NUBspline_2d_c* spline = new NUBspline_2d_c;
  if (spline == NULL)
    return spline;
  spline->sp_code = NU2D;
  spline->t_code  = SINGLE_COMPLEX;

  // Next, create the bases
  spline->x_basis = create_NUBasis (x_grid, xBC.lCode==PERIODIC);
  spline->y_basis = create_NUBasis (y_grid, yBC.lCode==PERIODIC);
//  int Mx,
  int My, Nx, Ny;
//  if (xBC.lCode == PERIODIC) Mx = x_grid->num_points - 1;
//  else                       Mx = x_grid->num_points;
  if (yBC.lCode == PERIODIC) My = y_grid->num_points - 1;
  else                       My = y_grid->num_points;

  Nx = x_grid->num_points + 2;
  Ny = y_grid->num_points + 2;
    
  spline->x_stride = Ny;
#ifndef HAVE_SSE2
  spline->coefs = (complex_float*)malloc (sizeof(complex_float)*Nx*Ny);
#else
  posix_memalign ((void**)&spline->coefs, 16, sizeof(complex_float)*Nx*Ny);
#endif

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) {
    int doffset = iy;
    int coffset = iy;
    find_NUBcoefs_1d_c (spline->x_basis, xBC, data+doffset, My,
			spline->coefs+coffset, Ny);
  }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) {
    int doffset = ix*Ny;
    int coffset = ix*Ny;
    find_NUBcoefs_1d_c (spline->y_basis, yBC, spline->coefs+doffset, 1, 
			spline->coefs+coffset, 1);
  }
    
  return spline;
}


NUBspline_3d_c *
create_NUBspline_3d_c (NUgrid* x_grid, NUgrid* y_grid, NUgrid* z_grid,
		       BCtype_c xBC, BCtype_c yBC, BCtype_c zBC, complex_float *data)
{
  // First, create the spline structure
  NUBspline_3d_c* spline = new NUBspline_3d_c;
  if (spline == NULL)
    return spline;
  spline->sp_code = NU3D;
  spline->t_code  = SINGLE_COMPLEX;

  // Next, create the bases
  spline->x_basis = create_NUBasis (x_grid, xBC.lCode==PERIODIC);
  spline->y_basis = create_NUBasis (y_grid, yBC.lCode==PERIODIC);
  spline->z_basis = create_NUBasis (z_grid, zBC.lCode==PERIODIC);
//  int Mx,
  int My, Mz, Nx, Ny, Nz;
//  if (xBC.lCode == PERIODIC) Mx = x_grid->num_points - 1;
//  else                       Mx = x_grid->num_points;
  if (yBC.lCode == PERIODIC) My = y_grid->num_points - 1;
  else                       My = y_grid->num_points;
  if (zBC.lCode == PERIODIC) Mz = z_grid->num_points - 1;
  else                       Mz = z_grid->num_points;

  Nx = x_grid->num_points + 2;
  Ny = y_grid->num_points + 2;
  Nz = z_grid->num_points + 2;

  // Allocate coefficients and solve  
  spline->x_stride = Ny*Nz;
  spline->y_stride = Nz;
#ifndef HAVE_SSE2
  spline->coefs = (complex_float*)malloc (sizeof(complex_float)*Nx*Ny*Nz);
#else
  posix_memalign ((void**)&spline->coefs, 16, sizeof(complex_float)*Nx*Ny*Nz);
#endif

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) 
    for (int iz=0; iz<Mz; iz++) {
      int doffset = iy*Mz+iz;
      int coffset = iy*Nz+iz;
      find_NUBcoefs_1d_c (spline->x_basis, xBC, data+doffset, My*Mz,
			  spline->coefs+coffset, Ny*Nz);
    }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iz=0; iz<Nz; iz++) {
      int doffset = ix*Ny*Nz + iz;
      int coffset = ix*Ny*Nz + iz;
      find_NUBcoefs_1d_c (spline->y_basis, yBC, spline->coefs+doffset, Nz, 
			  spline->coefs+coffset, Nz);
    }

  // Now, solve in the Z-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iy=0; iy<Ny; iy++) {
      int doffset = (ix*Ny+iy)*Nz;
      int coffset = (ix*Ny+iy)*Nz;
      find_NUBcoefs_1d_c (spline->z_basis, zBC, spline->coefs+doffset, 1, 
			  spline->coefs+coffset, 1);
    }
  return spline;
}

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//// Double-precision complex creation routines     ////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

void
find_NUBcoefs_1d_z (NUBasis* restrict basis, BCtype_z bc,
		    complex_double *data,  int dstride,
		    complex_double *coefs, int cstride)
{
  BCtype_d bc_r, bc_i;
  bc_r.lCode = bc.lCode;   bc_i.lCode = bc.lCode;
  bc_r.rCode = bc.rCode;   bc_i.rCode = bc.rCode;
  bc_r.lVal  = bc.lVal_r;  bc_r.rVal  = bc.rVal_r;
  bc_i.lVal  = bc.lVal_i;  bc_i.rVal  = bc.rVal_i;

  double *data_r  = ((double*)data );
  double *data_i  = ((double*)data )+1;
  double *coefs_r = ((double*)coefs);
  double *coefs_i = ((double*)coefs)+1;

  find_NUBcoefs_1d_d (basis, bc_r, data_r, 2*dstride, coefs_r, 2*cstride);
  find_NUBcoefs_1d_d (basis, bc_i, data_i, 2*dstride, coefs_i, 2*cstride);
}


NUBspline_1d_z *
create_NUBspline_1d_z (NUgrid* x_grid, BCtype_z xBC, complex_double *data)
{
  // First, create the spline structure
  NUBspline_1d_z* spline = new NUBspline_1d_z;
  if (spline == NULL)
    return spline;
  spline->sp_code = NU1D;
  spline->t_code  = DOUBLE_COMPLEX;

  // Next, create the basis
  spline->x_basis = create_NUBasis (x_grid, xBC.lCode==PERIODIC);
  // M is the number of data points
//  int M;
//  if (xBC.lCode == PERIODIC) M = x_grid->num_points - 1;
//  else                       M = x_grid->num_points;
  int N = x_grid->num_points + 2;

  // Allocate coefficients and solve  
  spline->coefs = (complex_double*)malloc (sizeof(complex_double)*N);
  find_NUBcoefs_1d_z (spline->x_basis, xBC, data, 1, spline->coefs, 1);
    
  return spline;
}

NUBspline_2d_z *
create_NUBspline_2d_z (NUgrid* x_grid, NUgrid* y_grid,
		       BCtype_z xBC, BCtype_z yBC, complex_double *data)
{
  // First, create the spline structure
  NUBspline_2d_z* spline = new NUBspline_2d_z;
  if (spline == NULL)
    return spline;
  spline->sp_code = NU2D;
  spline->t_code  = DOUBLE_COMPLEX;

  // Next, create the bases
  spline->x_basis = create_NUBasis (x_grid, xBC.lCode==PERIODIC);
  spline->y_basis = create_NUBasis (y_grid, yBC.lCode==PERIODIC);
//  int Mx,
  int My, Nx, Ny;
//  if (xBC.lCode == PERIODIC) Mx = x_grid->num_points - 1;
//  else                       Mx = x_grid->num_points;
  if (yBC.lCode == PERIODIC) My = y_grid->num_points - 1;
  else                       My = y_grid->num_points;

  Nx = x_grid->num_points + 2;
  Ny = y_grid->num_points + 2;
    
  spline->x_stride = Ny;
#ifndef HAVE_SSE2
  spline->coefs = (complex_double*)malloc (sizeof(complex_double)*Nx*Ny);
#else
  posix_memalign ((void**)&spline->coefs, 16, sizeof(complex_double)*Nx*Ny);
#endif

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) {
    int doffset = iy;
    int coffset = iy;
    find_NUBcoefs_1d_z (spline->x_basis, xBC, data+doffset, My,
			spline->coefs+coffset, Ny);
  }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) {
    int doffset = ix*Ny;
    int coffset = ix*Ny;
    find_NUBcoefs_1d_z (spline->y_basis, yBC, spline->coefs+doffset, 1, 
			spline->coefs+coffset, 1);
  }
    
  return spline;
}


NUBspline_3d_z *
create_NUBspline_3d_z (NUgrid* x_grid, NUgrid* y_grid, NUgrid* z_grid,
		       BCtype_z xBC, BCtype_z yBC, BCtype_z zBC, complex_double *data)
{
  // First, create the spline structure
  NUBspline_3d_z* spline = new NUBspline_3d_z;
  if (spline == NULL)
    return spline;
  spline->sp_code = NU3D;
  spline->t_code  = DOUBLE_COMPLEX;
  spline->x_grid = x_grid;
  spline->y_grid = y_grid;
  spline->z_grid = z_grid;

  // Next, create the bases
  spline->x_basis = create_NUBasis (x_grid, xBC.lCode==PERIODIC);
  spline->y_basis = create_NUBasis (y_grid, yBC.lCode==PERIODIC);
  spline->z_basis = create_NUBasis (z_grid, zBC.lCode==PERIODIC);
//  int Mx,
  int My, Mz, Nx, Ny, Nz;
//  if (xBC.lCode == PERIODIC) Mx = x_grid->num_points - 1;
//  else                       Mx = x_grid->num_points;
  if (yBC.lCode == PERIODIC) My = y_grid->num_points - 1;
  else                       My = y_grid->num_points;
  if (zBC.lCode == PERIODIC) Mz = z_grid->num_points - 1;
  else                       Mz = z_grid->num_points;

  Nx = x_grid->num_points + 2;
  Ny = y_grid->num_points + 2;
  Nz = z_grid->num_points + 2;

  // Allocate coefficients and solve  
  spline->x_stride = Ny*Nz;
  spline->y_stride = Nz;
#ifndef HAVE_SSE2
  spline->coefs = (complex_double*)malloc (sizeof(complex_double)*Nx*Ny*Nz);
#else
  posix_memalign ((void**)&spline->coefs, 16, sizeof(complex_double)*Nx*Ny*Nz);
#endif

  // First, solve in the X-direction 
  for (int iy=0; iy<My; iy++) 
    for (int iz=0; iz<Mz; iz++) {
      int doffset = iy*Mz+iz;
      int coffset = iy*Nz+iz;
      find_NUBcoefs_1d_z (spline->x_basis, xBC, data+doffset, My*Mz,
			  spline->coefs+coffset, Ny*Nz);
      /* for (int ix=0; ix<Nx; ix++) {
	complex_double z = spline->coefs[coffset+ix*spline->x_stride];
	if (isnan(creal(z)))
	  fprintf (stderr, "NAN encountered in create_NUBspline_3d_z at real part of (%d,%d,%d)\n",
		   ix,iy,iz);
	if (isnan(cimag(z)))
	  fprintf (stderr, "NAN encountered in create_NUBspline_3d_z at imag part of (%d,%d,%d)\n",
		   ix,iy,iz);
       } */
    }
  
  // Now, solve in the Y-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iz=0; iz<Nz; iz++) {
      int doffset = ix*Ny*Nz + iz;
      int coffset = ix*Ny*Nz + iz;
      find_NUBcoefs_1d_z (spline->y_basis, yBC, spline->coefs+doffset, Nz, 
			  spline->coefs+coffset, Nz);
    }

  // Now, solve in the Z-direction
  for (int ix=0; ix<Nx; ix++) 
    for (int iy=0; iy<Ny; iy++) {
      int doffset = (ix*Ny+iy)*Nz;
      int coffset = (ix*Ny+iy)*Nz;
      find_NUBcoefs_1d_z (spline->z_basis, zBC, spline->coefs+doffset, 1, 
			  spline->coefs+coffset, 1);
    }
  return spline;
}


void
destroy_NUBspline(Bspline *spline)
{
    free(spline->coefs);
  switch (spline->sp_code) {
  case NU1D:
    destroy_NUBasis (((NUBspline_1d*)spline)->x_basis);
    break;
  case NU2D:
    destroy_NUBasis (((NUBspline_2d*)spline)->x_basis);
    destroy_NUBasis (((NUBspline_2d*)spline)->y_basis);
    break;
    
  case NU3D:
    destroy_NUBasis (((NUBspline_3d*)spline)->x_basis);
    destroy_NUBasis (((NUBspline_3d*)spline)->y_basis);
    destroy_NUBasis (((NUBspline_3d*)spline)->z_basis);
    break;
  case U1D:
  case U2D:
  case MULTI_U1D:
  case MULTI_U2D:
  case MULTI_U3D:
  case MULTI_NU1D:
  case MULTI_NU2D:
  case MULTI_NU3D:
  default:
      ;
  }
  delete spline;
}
    
