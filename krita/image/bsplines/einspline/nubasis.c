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

#include "nubasis.h"
#include <stdlib.h>

  

NUBasis*
create_NUBasis (NUgrid *grid, bool periodic)
{
  NUBasis* restrict basis = malloc (sizeof(NUBasis));
  basis->grid = grid;
  basis->periodic = periodic;
  int N = grid->num_points;
  basis->xVals = malloc ((N+5)*sizeof(double));
  basis->dxInv = malloc (3*(N+2)*sizeof(double));
  for (int i=0; i<N; i++)
    basis->xVals[i+2] = grid->points[i];
  double*  restrict g = grid->points;
  // Extend grid points on either end to provide enough points to
  // construct a full basis set
  if (!periodic) {
    basis->xVals[0]   = g[ 0 ] - 2.0*(g[1]-g[0]);
    basis->xVals[1]   = g[ 0 ] - 1.0*(g[1]-g[0]);
    basis->xVals[N+2] = g[N-1] + 1.0*(g[N-1]-g[N-2]);
    basis->xVals[N+3] = g[N-1] + 2.0*(g[N-1]-g[N-2]);
    basis->xVals[N+4] = g[N-1] + 3.0*(g[N-1]-g[N-2]);
  }
  else {
    basis->xVals[1]   = g[ 0 ] - (g[N-1] - g[N-2]);
    basis->xVals[0]   = g[ 0 ] - (g[N-1] - g[N-3]);
    basis->xVals[N+2] = g[N-1] + (g[ 1 ] - g[ 0 ]);
    basis->xVals[N+3] = g[N-1] + (g[ 2 ] - g[ 0 ]);
    basis->xVals[N+4] = g[N-1] + (g[ 3 ] - g[ 0 ]);
  }
  for (int i=0; i<N+2; i++) 
    for (int j=0; j<3; j++) 
      basis->dxInv[3*i+j] = 
	1.0/(basis->xVals[i+j+1]-basis->xVals[i]);
  return basis;
}

void
destroy_NUBasis (NUBasis *basis)
{
  free (basis->xVals);
  free (basis->dxInv);
  free (basis);
}


int
get_NUBasis_funcs_s (NUBasis* restrict basis, double x,
		     float bfuncs[4])
{
  double b1[2], b2[3];
  int i = (*basis->grid->reverse_map)(basis->grid, x);
  int i2 = i+2;
  double* restrict dxInv = basis->dxInv;
  double* restrict xVals = basis->xVals;

  b1[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+2)+0];
  b1[1]     = (x-xVals[i2])    * dxInv[3*(i+2)+0];
  b2[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+1)+1] * b1[0];
  b2[1]     = ((x-xVals[i2-1]) * dxInv[3*(i+1)+1] * b1[0]+
	       (xVals[i2+2]-x) * dxInv[3*(i+2)+1] * b1[1]);
  b2[2]     = (x-xVals[i2])    * dxInv[3*(i+2)+1] * b1[1];
  bfuncs[0] = (xVals[i2+1]-x)  * dxInv[3*(i  )+2] * b2[0];
  bfuncs[1] = ((x-xVals[i2-2]) * dxInv[3*(i  )+2] * b2[0] +
	       (xVals[i2+2]-x) * dxInv[3*(i+1)+2] * b2[1]);
  bfuncs[2] = ((x-xVals[i2-1]) * dxInv[3*(i+1)+2] * b2[1] +
	       (xVals[i2+3]-x) * dxInv[3*(i+2)+2] * b2[2]);
  bfuncs[3] = (x-xVals[i2])    * dxInv[3*(i+2)+2] * b2[2];
  return i;
}


void
get_NUBasis_funcs_si (NUBasis* restrict basis, int i,
		     float bfuncs[4])
{
  int i2 = i+2;
  double b1[2], b2[3];
  double x = basis->grid->points[i];
  double* restrict dxInv = basis->dxInv;
  double* restrict xVals = basis->xVals; 

  b1[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+2)+0];
  b1[1]     = (x-xVals[i2])    * dxInv[3*(i+2)+0];
  b2[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+1)+1] * b1[0];
  b2[1]     = ((x-xVals[i2-1]) * dxInv[3*(i+1)+1] * b1[0]+
	       (xVals[i2+2]-x) * dxInv[3*(i+2)+1] * b1[1]);
  b2[2]     = (x-xVals[i2])    * dxInv[3*(i+2)+1] * b1[1];
  bfuncs[0] = (xVals[i2+1]-x)  * dxInv[3*(i  )+2] * b2[0];
  bfuncs[1] = ((x-xVals[i2-2]) * dxInv[3*(i  )+2] * b2[0] +
	       (xVals[i2+2]-x) * dxInv[3*(i+1)+2] * b2[1]);
  bfuncs[2] = ((x-xVals[i2-1]) * dxInv[3*(i+1)+2] * b2[1] +
	       (xVals[i2+3]-x) * dxInv[3*(i+2)+2] * b2[2]);
  bfuncs[3] = (x-xVals[i2])    * dxInv[3*(i+2)+2] * b2[2];
}

int
get_NUBasis_dfuncs_s (NUBasis* restrict basis, double x,
		      float bfuncs[4], float dbfuncs[4])
{
  double b1[2], b2[3];
  int i = (*basis->grid->reverse_map)(basis->grid, x);
  int i2 = i+2;
  double* restrict dxInv = basis->dxInv;
  double* restrict xVals = basis->xVals;

  b1[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+2)+0];
  b1[1]     = (x-xVals[i2])    * dxInv[3*(i+2)+0];
  b2[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+1)+1] * b1[0];
  b2[1]     = ((x-xVals[i2-1]) * dxInv[3*(i+1)+1] * b1[0]+
	       (xVals[i2+2]-x) * dxInv[3*(i+2)+1] * b1[1]);
  b2[2]     = (x-xVals[i2])    * dxInv[3*(i+2)+1] * b1[1];
  bfuncs[0] = (xVals[i2+1]-x)  * dxInv[3*(i  )+2] * b2[0];
  bfuncs[1] = ((x-xVals[i2-2]) * dxInv[3*(i  )+2] * b2[0] +
	       (xVals[i2+2]-x) * dxInv[3*(i+1)+2] * b2[1]);
  bfuncs[2] = ((x-xVals[i2-1]) * dxInv[3*(i+1)+2] * b2[1] +
	       (xVals[i2+3]-x) * dxInv[3*(i+2)+2] * b2[2]);
  bfuncs[3] = (x-xVals[i2])    * dxInv[3*(i+2)+2] * b2[2]; 

  dbfuncs[0] = -3.0 * (dxInv[3*(i  )+2] * b2[0]);
  dbfuncs[1] =  3.0 * (dxInv[3*(i  )+2] * b2[0] - dxInv[3*(i+1)+2] * b2[1]);
  dbfuncs[2] =  3.0 * (dxInv[3*(i+1)+2] * b2[1] - dxInv[3*(i+2)+2] * b2[2]);
  dbfuncs[3] =  3.0 * (dxInv[3*(i+2)+2] * b2[2]);

  return i;
}


void
get_NUBasis_dfuncs_si (NUBasis* restrict basis, int i,
		       float bfuncs[4], float dbfuncs[4])
{
  double b1[2], b2[3];
  double x = basis->grid->points[i];
  int i2 = i+2;
  double* restrict dxInv = basis->dxInv;
  double* restrict xVals = basis->xVals;

  b1[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+2)+0];
  b1[1]     = (x-xVals[i2])    * dxInv[3*(i+2)+0];
  b2[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+1)+1] * b1[0];
  b2[1]     = ((x-xVals[i2-1]) * dxInv[3*(i+1)+1] * b1[0]+
	       (xVals[i2+2]-x) * dxInv[3*(i+2)+1] * b1[1]);
  b2[2]     = (x-xVals[i2])    * dxInv[3*(i+2)+1] * b1[1];
  bfuncs[0] = (xVals[i2+1]-x)  * dxInv[3*(i  )+2] * b2[0];
  bfuncs[1] = ((x-xVals[i2-2]) * dxInv[3*(i  )+2] * b2[0] +
	       (xVals[i2+2]-x) * dxInv[3*(i+1)+2] * b2[1]);
  bfuncs[2] = ((x-xVals[i2-1]) * dxInv[3*(i+1)+2] * b2[1] +
	       (xVals[i2+3]-x) * dxInv[3*(i+2)+2] * b2[2]);
  bfuncs[3] = (x-xVals[i2])    * dxInv[3*(i+2)+2] * b2[2]; 

  dbfuncs[0] = -3.0 * (dxInv[3*(i  )+2] * b2[0]);
  dbfuncs[1] =  3.0 * (dxInv[3*(i  )+2] * b2[0] - dxInv[3*(i+1)+2] * b2[1]);
  dbfuncs[2] =  3.0 * (dxInv[3*(i+1)+2] * b2[1] - dxInv[3*(i+2)+2] * b2[2]);
  dbfuncs[3] =  3.0 * (dxInv[3*(i+2)+2] * b2[2]);
}


int
get_NUBasis_d2funcs_s (NUBasis* restrict basis, double x,
		       float bfuncs[4], float dbfuncs[4], float d2bfuncs[4])
{
  double b1[2], b2[3];
  int i = (*basis->grid->reverse_map)(basis->grid, x);
  int i2 = i+2;
  double* restrict dxInv = basis->dxInv;
  double* restrict xVals = basis->xVals;

  b1[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+2)+0];
  b1[1]     = (x-xVals[i2])    * dxInv[3*(i+2)+0];
  b2[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+1)+1] * b1[0];
  b2[1]     = ((x-xVals[i2-1]) * dxInv[3*(i+1)+1] * b1[0]+
	       (xVals[i2+2]-x) * dxInv[3*(i+2)+1] * b1[1]);
  b2[2]     = (x-xVals[i2])    * dxInv[3*(i+2)+1] * b1[1];
  bfuncs[0] = (xVals[i2+1]-x)  * dxInv[3*(i  )+2] * b2[0];
  bfuncs[1] = ((x-xVals[i2-2]) * dxInv[3*(i  )+2] * b2[0] +
	       (xVals[i2+2]-x) * dxInv[3*(i+1)+2] * b2[1]);
  bfuncs[2] = ((x-xVals[i2-1]) * dxInv[3*(i+1)+2] * b2[1] +
	       (xVals[i2+3]-x) * dxInv[3*(i+2)+2] * b2[2]);
  bfuncs[3] = (x-xVals[i2])    * dxInv[3*(i+2)+2] * b2[2]; 

  dbfuncs[0] = -3.0 * (dxInv[3*(i  )+2] * b2[0]);
  dbfuncs[1] =  3.0 * (dxInv[3*(i  )+2] * b2[0] - dxInv[3*(i+1)+2] * b2[1]);
  dbfuncs[2] =  3.0 * (dxInv[3*(i+1)+2] * b2[1] - dxInv[3*(i+2)+2] * b2[2]);
  dbfuncs[3] =  3.0 * (dxInv[3*(i+2)+2] * b2[2]);

  d2bfuncs[0] = 6.0 * (+dxInv[3*(i+0)+2]* dxInv[3*(i+1)+1]*b1[0]);
  d2bfuncs[1] = 6.0 * (-dxInv[3*(i+1)+1]*(dxInv[3*(i+0)+2]+dxInv[3*(i+1)+2])*b1[0] +
		        dxInv[3*(i+1)+2]* dxInv[3*(i+2)+1]*b1[1]);
  d2bfuncs[2] = 6.0 * (+dxInv[3*(i+1)+2]* dxInv[3*(i+1)+1]*b1[0] -
		        dxInv[3*(i+2)+1]*(dxInv[3*(i+1)+2] + dxInv[3*(i+2)+2])*b1[1]);
  d2bfuncs[3] = 6.0 * (+dxInv[3*(i+2)+2]* dxInv[3*(i+2)+1]*b1[1]);

  return i;
}


void
get_NUBasis_d2funcs_si (NUBasis* restrict basis, int i,
			float bfuncs[4], float dbfuncs[4], float d2bfuncs[4])
{
  double b1[2], b2[3];
  double x = basis->grid->points[i];
  int i2 = i+2;
  double* restrict dxInv = basis->dxInv;
  double* restrict xVals = basis->xVals;

  b1[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+2)+0];
  b1[1]     = (x-xVals[i2])    * dxInv[3*(i+2)+0];
  b2[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+1)+1] * b1[0];
  b2[1]     = ((x-xVals[i2-1]) * dxInv[3*(i+1)+1] * b1[0]+
	       (xVals[i2+2]-x) * dxInv[3*(i+2)+1] * b1[1]);
  b2[2]     = (x-xVals[i2])    * dxInv[3*(i+2)+1] * b1[1];
  bfuncs[0] = (xVals[i2+1]-x)  * dxInv[3*(i  )+2] * b2[0];
  bfuncs[1] = ((x-xVals[i2-2]) * dxInv[3*(i  )+2] * b2[0] +
	       (xVals[i2+2]-x) * dxInv[3*(i+1)+2] * b2[1]);
  bfuncs[2] = ((x-xVals[i2-1]) * dxInv[3*(i+1)+2] * b2[1] +
	       (xVals[i2+3]-x) * dxInv[3*(i+2)+2] * b2[2]);
  bfuncs[3] = (x-xVals[i2])    * dxInv[3*(i+2)+2] * b2[2]; 

  dbfuncs[0] = -3.0 * (dxInv[3*(i  )+2] * b2[0]);
  dbfuncs[1] =  3.0 * (dxInv[3*(i  )+2] * b2[0] - dxInv[3*(i+1)+2] * b2[1]);
  dbfuncs[2] =  3.0 * (dxInv[3*(i+1)+2] * b2[1] - dxInv[3*(i+2)+2] * b2[2]);
  dbfuncs[3] =  3.0 * (dxInv[3*(i+2)+2] * b2[2]);

  d2bfuncs[0] = 6.0 * (+dxInv[3*(i+0)+2]* dxInv[3*(i+1)+1]*b1[0]);
  d2bfuncs[1] = 6.0 * (-dxInv[3*(i+1)+1]*(dxInv[3*(i+0)+2]+dxInv[3*(i+1)+2])*b1[0] +
		        dxInv[3*(i+1)+2]* dxInv[3*(i+2)+1]*b1[1]);
  d2bfuncs[2] = 6.0 * (+dxInv[3*(i+1)+2]* dxInv[3*(i+1)+1]*b1[0] -
		        dxInv[3*(i+2)+1]*(dxInv[3*(i+1)+2] + dxInv[3*(i+2)+2])*b1[1]);
  d2bfuncs[3] = 6.0 * (+dxInv[3*(i+2)+2]* dxInv[3*(i+2)+1]*b1[1]);
}


//////////////////////////////
// Double-precision version //
//////////////////////////////
int
get_NUBasis_funcs_d (NUBasis* restrict basis, double x,
		     double bfuncs[4])
{
  double b1[2], b2[3];
  int i = (*basis->grid->reverse_map)(basis->grid, x);
  int i2 = i+2;
  double* restrict dxInv = basis->dxInv;
  double* restrict xVals = basis->xVals;

  b1[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+2)+0];
  b1[1]     = (x-xVals[i2])    * dxInv[3*(i+2)+0];
  b2[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+1)+1] * b1[0];
  b2[1]     = ((x-xVals[i2-1]) * dxInv[3*(i+1)+1] * b1[0]+
	       (xVals[i2+2]-x) * dxInv[3*(i+2)+1] * b1[1]);
  b2[2]     = (x-xVals[i2])    * dxInv[3*(i+2)+1] * b1[1];
  bfuncs[0] = (xVals[i2+1]-x)  * dxInv[3*(i  )+2] * b2[0];
  bfuncs[1] = ((x-xVals[i2-2]) * dxInv[3*(i  )+2] * b2[0] +
	       (xVals[i2+2]-x) * dxInv[3*(i+1)+2] * b2[1]);
  bfuncs[2] = ((x-xVals[i2-1]) * dxInv[3*(i+1)+2] * b2[1] +
	       (xVals[i2+3]-x) * dxInv[3*(i+2)+2] * b2[2]);
  bfuncs[3] = (x-xVals[i2])    * dxInv[3*(i+2)+2] * b2[2];
  return i;
}


void
get_NUBasis_funcs_di (NUBasis* restrict basis, int i,
		      double bfuncs[4])
{
  int i2 = i+2;
  double b1[2], b2[3];
  double x = basis->grid->points[i];
  double* restrict dxInv = basis->dxInv;
  double* restrict xVals = basis->xVals; 

  b1[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+2)+0];
  b1[1]     = (x-xVals[i2])    * dxInv[3*(i+2)+0];
  b2[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+1)+1] * b1[0];
  b2[1]     = ((x-xVals[i2-1]) * dxInv[3*(i+1)+1] * b1[0]+
	       (xVals[i2+2]-x) * dxInv[3*(i+2)+1] * b1[1]);
  b2[2]     = (x-xVals[i2])    * dxInv[3*(i+2)+1] * b1[1];
  bfuncs[0] = (xVals[i2+1]-x)  * dxInv[3*(i  )+2] * b2[0];
  bfuncs[1] = ((x-xVals[i2-2]) * dxInv[3*(i  )+2] * b2[0] +
	       (xVals[i2+2]-x) * dxInv[3*(i+1)+2] * b2[1]);
  bfuncs[2] = ((x-xVals[i2-1]) * dxInv[3*(i+1)+2] * b2[1] +
	       (xVals[i2+3]-x) * dxInv[3*(i+2)+2] * b2[2]);
  bfuncs[3] = (x-xVals[i2])    * dxInv[3*(i+2)+2] * b2[2];
}

int
get_NUBasis_dfuncs_d (NUBasis* restrict basis, double x,
		      double bfuncs[4], double dbfuncs[4])
{
  double b1[2], b2[3];
  int i = (*basis->grid->reverse_map)(basis->grid, x);
  int i2 = i+2;
  double* restrict dxInv = basis->dxInv;
  double* restrict xVals = basis->xVals;

  b1[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+2)+0];
  b1[1]     = (x-xVals[i2])    * dxInv[3*(i+2)+0];
  b2[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+1)+1] * b1[0];
  b2[1]     = ((x-xVals[i2-1]) * dxInv[3*(i+1)+1] * b1[0]+
	       (xVals[i2+2]-x) * dxInv[3*(i+2)+1] * b1[1]);
  b2[2]     = (x-xVals[i2])    * dxInv[3*(i+2)+1] * b1[1];
  bfuncs[0] = (xVals[i2+1]-x)  * dxInv[3*(i  )+2] * b2[0];
  bfuncs[1] = ((x-xVals[i2-2]) * dxInv[3*(i  )+2] * b2[0] +
	       (xVals[i2+2]-x) * dxInv[3*(i+1)+2] * b2[1]);
  bfuncs[2] = ((x-xVals[i2-1]) * dxInv[3*(i+1)+2] * b2[1] +
	       (xVals[i2+3]-x) * dxInv[3*(i+2)+2] * b2[2]);
  bfuncs[3] = (x-xVals[i2])    * dxInv[3*(i+2)+2] * b2[2]; 

  dbfuncs[0] = -3.0 * (dxInv[3*(i  )+2] * b2[0]);
  dbfuncs[1] =  3.0 * (dxInv[3*(i  )+2] * b2[0] - dxInv[3*(i+1)+2] * b2[1]);
  dbfuncs[2] =  3.0 * (dxInv[3*(i+1)+2] * b2[1] - dxInv[3*(i+2)+2] * b2[2]);
  dbfuncs[3] =  3.0 * (dxInv[3*(i+2)+2] * b2[2]);

  return i;
}


void
get_NUBasis_dfuncs_di (NUBasis* restrict basis, int i,
		       double bfuncs[4], double dbfuncs[4])
{
  double b1[2], b2[3];
  double x = basis->grid->points[i];
  int i2 = i+2;
  double* restrict dxInv = basis->dxInv;
  double* restrict xVals = basis->xVals;

  b1[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+2)+0];
  b1[1]     = (x-xVals[i2])    * dxInv[3*(i+2)+0];
  b2[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+1)+1] * b1[0];
  b2[1]     = ((x-xVals[i2-1]) * dxInv[3*(i+1)+1] * b1[0]+
	       (xVals[i2+2]-x) * dxInv[3*(i+2)+1] * b1[1]);
  b2[2]     = (x-xVals[i2])    * dxInv[3*(i+2)+1] * b1[1];
  bfuncs[0] = (xVals[i2+1]-x)  * dxInv[3*(i  )+2] * b2[0];
  bfuncs[1] = ((x-xVals[i2-2]) * dxInv[3*(i  )+2] * b2[0] +
	       (xVals[i2+2]-x) * dxInv[3*(i+1)+2] * b2[1]);
  bfuncs[2] = ((x-xVals[i2-1]) * dxInv[3*(i+1)+2] * b2[1] +
	       (xVals[i2+3]-x) * dxInv[3*(i+2)+2] * b2[2]);
  bfuncs[3] = (x-xVals[i2])    * dxInv[3*(i+2)+2] * b2[2]; 

  dbfuncs[0] = -3.0 * (dxInv[3*(i  )+2] * b2[0]);
  dbfuncs[1] =  3.0 * (dxInv[3*(i  )+2] * b2[0] - dxInv[3*(i+1)+2] * b2[1]);
  dbfuncs[2] =  3.0 * (dxInv[3*(i+1)+2] * b2[1] - dxInv[3*(i+2)+2] * b2[2]);
  dbfuncs[3] =  3.0 * (dxInv[3*(i+2)+2] * b2[2]);
}


int
get_NUBasis_d2funcs_d (NUBasis* restrict basis, double x,
		       double bfuncs[4], double dbfuncs[4], double d2bfuncs[4])
{
  double b1[2], b2[3];
  int i = (*basis->grid->reverse_map)(basis->grid, x);
  int i2 = i+2;
  double* restrict dxInv = basis->dxInv;
  double* restrict xVals = basis->xVals;

  b1[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+2)+0];
  b1[1]     = (x-xVals[i2])    * dxInv[3*(i+2)+0];
  b2[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+1)+1] * b1[0];
  b2[1]     = ((x-xVals[i2-1]) * dxInv[3*(i+1)+1] * b1[0]+
	       (xVals[i2+2]-x) * dxInv[3*(i+2)+1] * b1[1]);
  b2[2]     = (x-xVals[i2])    * dxInv[3*(i+2)+1] * b1[1];
  bfuncs[0] = (xVals[i2+1]-x)  * dxInv[3*(i  )+2] * b2[0];
  bfuncs[1] = ((x-xVals[i2-2]) * dxInv[3*(i  )+2] * b2[0] +
	       (xVals[i2+2]-x) * dxInv[3*(i+1)+2] * b2[1]);
  bfuncs[2] = ((x-xVals[i2-1]) * dxInv[3*(i+1)+2] * b2[1] +
	       (xVals[i2+3]-x) * dxInv[3*(i+2)+2] * b2[2]);
  bfuncs[3] = (x-xVals[i2])    * dxInv[3*(i+2)+2] * b2[2]; 

  dbfuncs[0] = -3.0 * (dxInv[3*(i  )+2] * b2[0]);
  dbfuncs[1] =  3.0 * (dxInv[3*(i  )+2] * b2[0] - dxInv[3*(i+1)+2] * b2[1]);
  dbfuncs[2] =  3.0 * (dxInv[3*(i+1)+2] * b2[1] - dxInv[3*(i+2)+2] * b2[2]);
  dbfuncs[3] =  3.0 * (dxInv[3*(i+2)+2] * b2[2]);

  d2bfuncs[0] = 6.0 * (+dxInv[3*(i+0)+2]* dxInv[3*(i+1)+1]*b1[0]);
  d2bfuncs[1] = 6.0 * (-dxInv[3*(i+1)+1]*(dxInv[3*(i+0)+2]+dxInv[3*(i+1)+2])*b1[0] +
		        dxInv[3*(i+1)+2]* dxInv[3*(i+2)+1]*b1[1]);
  d2bfuncs[2] = 6.0 * (+dxInv[3*(i+1)+2]* dxInv[3*(i+1)+1]*b1[0] -
		        dxInv[3*(i+2)+1]*(dxInv[3*(i+1)+2] + dxInv[3*(i+2)+2])*b1[1]);
  d2bfuncs[3] = 6.0 * (+dxInv[3*(i+2)+2]* dxInv[3*(i+2)+1]*b1[1]);

  return i;
}


void
get_NUBasis_d2funcs_di (NUBasis* restrict basis, int i,
			double bfuncs[4], double dbfuncs[4], 
			double d2bfuncs[4])
{
  double b1[2], b2[3];
  double x = basis->grid->points[i];
  int i2 = i+2;
  double* restrict dxInv = basis->dxInv;
  double* restrict xVals = basis->xVals;

  b1[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+2)+0];
  b1[1]     = (x-xVals[i2])    * dxInv[3*(i+2)+0];
  b2[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+1)+1] * b1[0];
  b2[1]     = ((x-xVals[i2-1]) * dxInv[3*(i+1)+1] * b1[0]+
	       (xVals[i2+2]-x) * dxInv[3*(i+2)+1] * b1[1]);
  b2[2]     = (x-xVals[i2])    * dxInv[3*(i+2)+1] * b1[1];
  bfuncs[0] = (xVals[i2+1]-x)  * dxInv[3*(i  )+2] * b2[0];
  bfuncs[1] = ((x-xVals[i2-2]) * dxInv[3*(i  )+2] * b2[0] +
	       (xVals[i2+2]-x) * dxInv[3*(i+1)+2] * b2[1]);
  bfuncs[2] = ((x-xVals[i2-1]) * dxInv[3*(i+1)+2] * b2[1] +
	       (xVals[i2+3]-x) * dxInv[3*(i+2)+2] * b2[2]);
  bfuncs[3] = (x-xVals[i2])    * dxInv[3*(i+2)+2] * b2[2]; 

  dbfuncs[0] = -3.0 * (dxInv[3*(i  )+2] * b2[0]);
  dbfuncs[1] =  3.0 * (dxInv[3*(i  )+2] * b2[0] - dxInv[3*(i+1)+2] * b2[1]);
  dbfuncs[2] =  3.0 * (dxInv[3*(i+1)+2] * b2[1] - dxInv[3*(i+2)+2] * b2[2]);
  dbfuncs[3] =  3.0 * (dxInv[3*(i+2)+2] * b2[2]);

  d2bfuncs[0] = 6.0 * (+dxInv[3*(i+0)+2]* dxInv[3*(i+1)+1]*b1[0]);
  d2bfuncs[1] = 6.0 * (-dxInv[3*(i+1)+1]*(dxInv[3*(i+0)+2]+dxInv[3*(i+1)+2])*b1[0] +
		        dxInv[3*(i+1)+2]* dxInv[3*(i+2)+1]*b1[1]);
  d2bfuncs[2] = 6.0 * (+dxInv[3*(i+1)+2]* dxInv[3*(i+1)+1]*b1[0] -
		        dxInv[3*(i+2)+1]*(dxInv[3*(i+1)+2] + dxInv[3*(i+2)+2])*b1[1]);
  d2bfuncs[3] = 6.0 * (+dxInv[3*(i+2)+2]* dxInv[3*(i+2)+1]*b1[1]);
}


#ifdef HAVE_SSE2
typedef union
{
  float s[4];
  __m128 v;
} uvec4;

typedef union
{
  double s[2];
  __m128d v;
} uvec2;

int
get_NUBasis_funcs_sse_s (NUBasis* restrict basis, double x,
			 __m128 *restrict funcs)
{
  double b1[2], b2[3];
  int i = (*basis->grid->reverse_map)(basis->grid, x);
  int i2 = i+2;
  double* restrict dxInv = basis->dxInv;
  double* restrict xVals = basis->xVals;
  
  uvec4 bfuncs;
  

  b1[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+2)+0];
  b1[1]     = (x-xVals[i2])    * dxInv[3*(i+2)+0];
  b2[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+1)+1] * b1[0];
  b2[1]     = ((x-xVals[i2-1]) * dxInv[3*(i+1)+1] * b1[0]+
	       (xVals[i2+2]-x) * dxInv[3*(i+2)+1] * b1[1]);
  b2[2]     = (x-xVals[i2])    * dxInv[3*(i+2)+1] * b1[1];
  bfuncs.s[0] = (xVals[i2+1]-x)  * dxInv[3*(i  )+2] * b2[0];
  bfuncs.s[1] = ((x-xVals[i2-2]) * dxInv[3*(i  )+2] * b2[0] +
	       (xVals[i2+2]-x) * dxInv[3*(i+1)+2] * b2[1]);
  bfuncs.s[2] = ((x-xVals[i2-1]) * dxInv[3*(i+1)+2] * b2[1] +
	       (xVals[i2+3]-x) * dxInv[3*(i+2)+2] * b2[2]);
  bfuncs.s[3] = (x-xVals[i2])    * dxInv[3*(i+2)+2] * b2[2];
  *funcs = bfuncs.v;
  return i;
}

int
get_NUBasis_dfuncs_sse_s (NUBasis* restrict basis, double x,
			  __m128 *restrict funcs, __m128 *restrict dfuncs)
{
  double b1[2], b2[3];
  int i = (*basis->grid->reverse_map)(basis->grid, x);
  int i2 = i+2;
  double* restrict dxInv = basis->dxInv;
  double* restrict xVals = basis->xVals;
  uvec4 bfuncs, dbfuncs;


  b1[0]       = (xVals[i2+1]-x)  * dxInv[3*(i+2)+0];
  b1[1]       = (x-xVals[i2])    * dxInv[3*(i+2)+0];
  b2[0]       = (xVals[i2+1]-x)  * dxInv[3*(i+1)+1] * b1[0];
  b2[1]       = ((x-xVals[i2-1]) * dxInv[3*(i+1)+1] * b1[0]+
		 (xVals[i2+2]-x) * dxInv[3*(i+2)+1] * b1[1]);
  b2[2]       = (x-xVals[i2])    * dxInv[3*(i+2)+1] * b1[1];
  bfuncs.s[0] = (xVals[i2+1]-x)  * dxInv[3*(i  )+2] * b2[0];
  bfuncs.s[1] = ((x-xVals[i2-2]) * dxInv[3*(i  )+2] * b2[0] +
		 (xVals[i2+2]-x) * dxInv[3*(i+1)+2] * b2[1]);
  bfuncs.s[2] = ((x-xVals[i2-1]) * dxInv[3*(i+1)+2] * b2[1] +
		 (xVals[i2+3]-x) * dxInv[3*(i+2)+2] * b2[2]);
  bfuncs.s[3] = (x-xVals[i2])    * dxInv[3*(i+2)+2] * b2[2]; 

  dbfuncs.s[0] = -3.0 * (dxInv[3*(i  )+2] * b2[0]);
  dbfuncs.s[1] =  3.0 * (dxInv[3*(i  )+2] * b2[0] - dxInv[3*(i+1)+2] * b2[1]);
  dbfuncs.s[2] =  3.0 * (dxInv[3*(i+1)+2] * b2[1] - dxInv[3*(i+2)+2] * b2[2]);
  dbfuncs.s[3] =  3.0 * (dxInv[3*(i+2)+2] * b2[2]);

  *funcs  =  bfuncs.v;
  *dfuncs = dbfuncs.v;

  return i;
}

int
get_NUBasis_d2funcs_sse_s (NUBasis* restrict basis, double x,
			   __m128 *restrict funcs, __m128 *restrict dfuncs, __m128 *restrict d2funcs)
{
  double b1[2], b2[3];
  int i = (*basis->grid->reverse_map)(basis->grid, x);
  int i2 = i+2;
  double* restrict dxInv = basis->dxInv;
  double* restrict xVals = basis->xVals;
  uvec4 bfuncs, dbfuncs, d2bfuncs;

  b1[0]       = (xVals[i2+1]-x)  * dxInv[3*(i+2)+0];
  b1[1]       = (x-xVals[i2])    * dxInv[3*(i+2)+0];
  b2[0]       = (xVals[i2+1]-x)  * dxInv[3*(i+1)+1] * b1[0];
  b2[1]       = ((x-xVals[i2-1]) * dxInv[3*(i+1)+1] * b1[0]+
		 (xVals[i2+2]-x) * dxInv[3*(i+2)+1] * b1[1]);
  b2[2]       = (x-xVals[i2])    * dxInv[3*(i+2)+1] * b1[1];
  bfuncs.s[0] = (xVals[i2+1]-x)  * dxInv[3*(i  )+2] * b2[0];
  bfuncs.s[1] = ((x-xVals[i2-2]) * dxInv[3*(i  )+2] * b2[0] +
		 (xVals[i2+2]-x) * dxInv[3*(i+1)+2] * b2[1]);
  bfuncs.s[2] = ((x-xVals[i2-1]) * dxInv[3*(i+1)+2] * b2[1] +
		 (xVals[i2+3]-x) * dxInv[3*(i+2)+2] * b2[2]);
  bfuncs.s[3] = (x-xVals[i2])    * dxInv[3*(i+2)+2] * b2[2]; 

  dbfuncs.s[0]  = -3.0 * (dxInv[3*(i  )+2] * b2[0]);
  dbfuncs.s[1]  =  3.0 * (dxInv[3*(i  )+2] * b2[0] - dxInv[3*(i+1)+2] * b2[1]);
  dbfuncs.s[2]  =  3.0 * (dxInv[3*(i+1)+2] * b2[1] - dxInv[3*(i+2)+2] * b2[2]);
  dbfuncs.s[3]  =  3.0 * (dxInv[3*(i+2)+2] * b2[2]);

  d2bfuncs.s[0] = 6.0 * (+dxInv[3*(i+0)+2]* dxInv[3*(i+1)+1]*b1[0]);
  d2bfuncs.s[1] = 6.0 * (-dxInv[3*(i+1)+1]*(dxInv[3*(i+0)+2]+dxInv[3*(i+1)+2])*b1[0] +
			 dxInv[3*(i+1)+2]* dxInv[3*(i+2)+1]*b1[1]);
  d2bfuncs.s[2] = 6.0 * (+dxInv[3*(i+1)+2]* dxInv[3*(i+1)+1]*b1[0] -
			 dxInv[3*(i+2)+1]*(dxInv[3*(i+1)+2] + dxInv[3*(i+2)+2])*b1[1]);
  d2bfuncs.s[3] = 6.0 * (+dxInv[3*(i+2)+2]* dxInv[3*(i+2)+1]*b1[1]);

  *funcs   =   bfuncs.v;
  *dfuncs  =  dbfuncs.v;
  *d2funcs = d2bfuncs.v;

  return i;
}


//////////////////////////////
// Double-precision version //
//////////////////////////////
int
get_NUBasis_funcs_sse_d (NUBasis* restrict basis, double x,
			  __m128d *restrict   f01, __m128d *restrict   f23)
{
  double b1[2], b2[3];
  int i = (*basis->grid->reverse_map)(basis->grid, x);
  int i2 = i+2;
  double* restrict dxInv = basis->dxInv;
  double* restrict xVals = basis->xVals;
  uvec2 bf01, bf23, dbf01, dbf23;

  b1[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+2)+0];
  b1[1]     = (x-xVals[i2])    * dxInv[3*(i+2)+0];
  b2[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+1)+1] * b1[0];
  b2[1]     = ((x-xVals[i2-1]) * dxInv[3*(i+1)+1] * b1[0]+
	       (xVals[i2+2]-x) * dxInv[3*(i+2)+1] * b1[1]);
  b2[2]     = (x-xVals[i2])    * dxInv[3*(i+2)+1] * b1[1];
  bf01.s[0] = (xVals[i2+1]-x)  * dxInv[3*(i  )+2] * b2[0];
  bf01.s[1] = ((x-xVals[i2-2]) * dxInv[3*(i  )+2] * b2[0] +
	       (xVals[i2+2]-x) * dxInv[3*(i+1)+2] * b2[1]);
  bf23.s[0] = ((x-xVals[i2-1]) * dxInv[3*(i+1)+2] * b2[1] +
	       (xVals[i2+3]-x) * dxInv[3*(i+2)+2] * b2[2]);
  bf23.s[1] = (x-xVals[i2])    * dxInv[3*(i+2)+2] * b2[2]; 

  *f01   =   bf01.v;  *f23 =   bf23.v;
  return i;
}

int
get_NUBasis_dfuncs_sse_d (NUBasis* restrict basis, double x,
			  __m128d *restrict   f01, __m128d *restrict   f23,
			  __m128d *restrict  df01, __m128d *restrict  df23)

{
  double b1[2], b2[3];
  int i = (*basis->grid->reverse_map)(basis->grid, x);
  int i2 = i+2;
  double* restrict dxInv = basis->dxInv;
  double* restrict xVals = basis->xVals;
  uvec2 bf01, bf23, dbf01, dbf23;

  b1[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+2)+0];
  b1[1]     = (x-xVals[i2])    * dxInv[3*(i+2)+0];
  b2[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+1)+1] * b1[0];
  b2[1]     = ((x-xVals[i2-1]) * dxInv[3*(i+1)+1] * b1[0]+
	       (xVals[i2+2]-x) * dxInv[3*(i+2)+1] * b1[1]);
  b2[2]     = (x-xVals[i2])    * dxInv[3*(i+2)+1] * b1[1];
  bf01.s[0] = (xVals[i2+1]-x)  * dxInv[3*(i  )+2] * b2[0];
  bf01.s[1] = ((x-xVals[i2-2]) * dxInv[3*(i  )+2] * b2[0] +
	       (xVals[i2+2]-x) * dxInv[3*(i+1)+2] * b2[1]);
  bf23.s[0] = ((x-xVals[i2-1]) * dxInv[3*(i+1)+2] * b2[1] +
	       (xVals[i2+3]-x) * dxInv[3*(i+2)+2] * b2[2]);
  bf23.s[1] = (x-xVals[i2])    * dxInv[3*(i+2)+2] * b2[2]; 

  dbf01.s[0] = -3.0 * (dxInv[3*(i  )+2] * b2[0]);
  dbf01.s[1] =  3.0 * (dxInv[3*(i  )+2] * b2[0] - dxInv[3*(i+1)+2] * b2[1]);
  dbf23.s[0] =  3.0 * (dxInv[3*(i+1)+2] * b2[1] - dxInv[3*(i+2)+2] * b2[2]);
  dbf23.s[1] =  3.0 * (dxInv[3*(i+2)+2] * b2[2]);

  *f01   =   bf01.v;   *f23 =   bf23.v;
  *df01  =  dbf01.v;  *df23 =  dbf23.v;

  return i;
}

int
get_NUBasis_d2funcs_sse_d (NUBasis* restrict basis, double x,
			   __m128d *restrict   f01, __m128d *restrict   f23,
			   __m128d *restrict  df01, __m128d *restrict  df23,
			   __m128d *restrict d2f01, __m128d *restrict d2f23)
{
  double b1[2], b2[3];
  int i = (*basis->grid->reverse_map)(basis->grid, x);
  int i2 = i+2;
  double* restrict dxInv = basis->dxInv;
  double* restrict xVals = basis->xVals;
  uvec2 bf01, bf23, dbf01, dbf23, d2bf01, d2bf23;
  
  b1[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+2)+0];
  b1[1]     = (x-xVals[i2])    * dxInv[3*(i+2)+0];
  b2[0]     = (xVals[i2+1]-x)  * dxInv[3*(i+1)+1] * b1[0];
  b2[1]     = ((x-xVals[i2-1]) * dxInv[3*(i+1)+1] * b1[0]+
	       (xVals[i2+2]-x) * dxInv[3*(i+2)+1] * b1[1]);
  b2[2]     = (x-xVals[i2])    * dxInv[3*(i+2)+1] * b1[1];
  bf01.s[0] = (xVals[i2+1]-x)  * dxInv[3*(i  )+2] * b2[0];
  bf01.s[1] = ((x-xVals[i2-2]) * dxInv[3*(i  )+2] * b2[0] +
	       (xVals[i2+2]-x) * dxInv[3*(i+1)+2] * b2[1]);
  bf23.s[0] = ((x-xVals[i2-1]) * dxInv[3*(i+1)+2] * b2[1] +
	       (xVals[i2+3]-x) * dxInv[3*(i+2)+2] * b2[2]);
  bf23.s[1] = (x-xVals[i2])    * dxInv[3*(i+2)+2] * b2[2]; 
  
  dbf01.s[0] = -3.0 * (dxInv[3*(i  )+2] * b2[0]);
  dbf01.s[1] =  3.0 * (dxInv[3*(i  )+2] * b2[0] - dxInv[3*(i+1)+2] * b2[1]);
  dbf23.s[0] =  3.0 * (dxInv[3*(i+1)+2] * b2[1] - dxInv[3*(i+2)+2] * b2[2]);
  dbf23.s[1] =  3.0 * (dxInv[3*(i+2)+2] * b2[2]);
  
  d2bf01.s[0] = 6.0 * (+dxInv[3*(i+0)+2]* dxInv[3*(i+1)+1]*b1[0]);
  d2bf01.s[1] = 6.0 * (-dxInv[3*(i+1)+1]*(dxInv[3*(i+0)+2]+dxInv[3*(i+1)+2])*b1[0] +
		       dxInv[3*(i+1)+2]* dxInv[3*(i+2)+1]*b1[1]);
  d2bf23.s[0] = 6.0 * (+dxInv[3*(i+1)+2]* dxInv[3*(i+1)+1]*b1[0] -
		       dxInv[3*(i+2)+1]*(dxInv[3*(i+1)+2] + dxInv[3*(i+2)+2])*b1[1]);
  d2bf23.s[1] = 6.0 * (+dxInv[3*(i+2)+2]* dxInv[3*(i+2)+1]*b1[1]);
  
  *f01   =   bf01.v;    *f23 =   bf23.v;
  *df01  =  dbf01.v;   *df23 =  dbf23.v;
  *d2f01 = d2bf01.v;  *d2f23 = d2bf23.v;
  
  return i;
}

#endif
