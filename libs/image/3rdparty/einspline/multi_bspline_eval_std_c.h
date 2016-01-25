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

#ifndef MULTI_BSPLINE_EVAL_STD_C_H
#define MULTI_BSPLINE_EVAL_STD_C_H

#include <math.h>
#include <stdio.h>
#include "multi_bspline_structs.h"

extern const float* restrict   Af;
extern const float* restrict  dAf;
extern const float* restrict d2Af;
extern const float* restrict d3Af;

/************************************************************/
/* 1D float-precision, complex evaulation functions        */
/************************************************************/
inline void
eval_multi_UBspline_1d_c (multi_UBspline_1d_c *spline,
			  double x,
			  complex_float* restrict vals)
{
  x -= spline->x_grid.start;
  float ux = x*spline->x_grid.delta_inv;
  float ipartx, tx;
  tx = modff (ux, &ipartx);  int ix = (int) ipartx;
  
  float tpx[4], a[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  complex_float* restrict coefs = spline->coefs;
  
  a[0]  = (Af[ 0]*tpx[0] + Af[ 1]*tpx[1] + Af[ 2]*tpx[2] + Af[ 3]*tpx[3]);
  a[1]  = (Af[ 4]*tpx[0] + Af[ 5]*tpx[1] + Af[ 6]*tpx[2] + Af[ 7]*tpx[3]);
  a[2]  = (Af[ 8]*tpx[0] + Af[ 9]*tpx[1] + Af[10]*tpx[2] + Af[11]*tpx[3]);
  a[3]  = (Af[12]*tpx[0] + Af[13]*tpx[1] + Af[14]*tpx[2] + Af[15]*tpx[3]);

  int xs = spline->x_stride;

  for (int n=0; n<spline->num_splines; n++) 
    vals[n]  = 0.0;

  for (int i=0; i<4; i++) {
    complex_float* restrict coefs = spline->coefs + ((ix+i)*xs);
    for (int n=0; n<spline->num_splines; n++) 
      vals[n]  +=   a[i] * coefs[n];
  }
}



inline void
eval_multi_UBspline_1d_c_vg (multi_UBspline_1d_c *spline,
			     double x,
			     complex_float* restrict vals,
			     complex_float* restrict grads)
{
  x -= spline->x_grid.start;
  float ux = x*spline->x_grid.delta_inv;
  float ipartx, tx;
  tx = modff (ux, &ipartx);  int ix = (int) ipartx;
  
  float tpx[4], a[4], da[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  complex_float* restrict coefs = spline->coefs;

  a[0]  = (Af[ 0]*tpx[0] + Af[ 1]*tpx[1] + Af[ 2]*tpx[2] + Af[ 3]*tpx[3]);
  a[1]  = (Af[ 4]*tpx[0] + Af[ 5]*tpx[1] + Af[ 6]*tpx[2] + Af[ 7]*tpx[3]);
  a[2]  = (Af[ 8]*tpx[0] + Af[ 9]*tpx[1] + Af[10]*tpx[2] + Af[11]*tpx[3]);
  a[3]  = (Af[12]*tpx[0] + Af[13]*tpx[1] + Af[14]*tpx[2] + Af[15]*tpx[3]);
  da[0] = (dAf[ 0]*tpx[0] + dAf[ 1]*tpx[1] + dAf[ 2]*tpx[2] + dAf[ 3]*tpx[3]);
  da[1] = (dAf[ 4]*tpx[0] + dAf[ 5]*tpx[1] + dAf[ 6]*tpx[2] + dAf[ 7]*tpx[3]);
  da[2] = (dAf[ 8]*tpx[0] + dAf[ 9]*tpx[1] + dAf[10]*tpx[2] + dAf[11]*tpx[3]);
  da[3] = (dAf[12]*tpx[0] + dAf[13]*tpx[1] + dAf[14]*tpx[2] + dAf[15]*tpx[3]);

  int xs = spline->x_stride;

  for (int n=0; n<spline->num_splines; n++) {
    vals[n]  = 0.0;
    grads[n] = 0.0;
  }

  for (int i=0; i<4; i++) { 
    complex_float* restrict coefs = spline->coefs + ((ix+i)*xs);
    for (int n=0; n<spline->num_splines; n++) {
      vals[n]  +=   a[i] * coefs[n];
      grads[n] +=  da[i] * coefs[n];
    }
  }
  
  float dxInv = spline->x_grid.delta_inv;
  for (int n=0; n<spline->num_splines; n++) 
    grads[n] *= dxInv;
}


inline void
eval_multi_UBspline_1d_c_vgl (multi_UBspline_1d_c *spline,
			      double x,
			      complex_float* restrict vals,
			      complex_float* restrict grads,
			      complex_float* restrict lapl)	  
{
  x -= spline->x_grid.start;
  float ux = x*spline->x_grid.delta_inv;
  float ipartx, tx;
  tx = modff (ux, &ipartx);  int ix = (int) ipartx;
  
  float tpx[4], a[4], da[4], d2a[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  complex_float* restrict coefs = spline->coefs;

  a[0]  = (Af[ 0]*tpx[0] + Af[ 1]*tpx[1] + Af[ 2]*tpx[2] + Af[ 3]*tpx[3]);
  a[1]  = (Af[ 4]*tpx[0] + Af[ 5]*tpx[1] + Af[ 6]*tpx[2] + Af[ 7]*tpx[3]);
  a[2]  = (Af[ 8]*tpx[0] + Af[ 9]*tpx[1] + Af[10]*tpx[2] + Af[11]*tpx[3]);
  a[3]  = (Af[12]*tpx[0] + Af[13]*tpx[1] + Af[14]*tpx[2] + Af[15]*tpx[3]);
  da[0] = (dAf[ 0]*tpx[0] + dAf[ 1]*tpx[1] + dAf[ 2]*tpx[2] + dAf[ 3]*tpx[3]);
  da[1] = (dAf[ 4]*tpx[0] + dAf[ 5]*tpx[1] + dAf[ 6]*tpx[2] + dAf[ 7]*tpx[3]);
  da[2] = (dAf[ 8]*tpx[0] + dAf[ 9]*tpx[1] + dAf[10]*tpx[2] + dAf[11]*tpx[3]);
  da[3] = (dAf[12]*tpx[0] + dAf[13]*tpx[1] + dAf[14]*tpx[2] + dAf[15]*tpx[3]);
  d2a[0] = (d2Af[ 0]*tpx[0] + d2Af[ 1]*tpx[1] + d2Af[ 2]*tpx[2] + d2Af[ 3]*tpx[3]);
  d2a[1] = (d2Af[ 4]*tpx[0] + d2Af[ 5]*tpx[1] + d2Af[ 6]*tpx[2] + d2Af[ 7]*tpx[3]);
  d2a[2] = (d2Af[ 8]*tpx[0] + d2Af[ 9]*tpx[1] + d2Af[10]*tpx[2] + d2Af[11]*tpx[3]);
  d2a[3] = (d2Af[12]*tpx[0] + d2Af[13]*tpx[1] + d2Af[14]*tpx[2] + d2Af[15]*tpx[3]);

  int xs = spline->x_stride;

  for (int n=0; n<spline->num_splines; n++) {
    vals[n]  = 0.0;
    grads[n] = 0.0;
    lapl[n]  = 0.0;
  }

  for (int i=0; i<4; i++) {      
    complex_float* restrict coefs = spline->coefs + ((ix+i)*xs);
    for (int n=0; n<spline->num_splines; n++) {
      vals[n]  +=   a[i] * coefs[n];
      grads[n] +=  da[i] * coefs[n];
      lapl[n]  += d2a[i] * coefs[n];
    }
  }

  float dxInv = spline->x_grid.delta_inv;
  for (int n=0; n<spline->num_splines; n++) {
    grads[n] *= dxInv;
    lapl [n] *= dxInv*dxInv;
  }
}


inline void
eval_multi_UBspline_1d_c_vgh (multi_UBspline_1d_c *spline,
			      double x,
			      complex_float* restrict vals,
			      complex_float* restrict grads,
			      complex_float* restrict hess)
{
  eval_multi_UBspline_1d_c_vgl (spline, x, vals, grads, hess);
}


/************************************************************/
/* 2D float-precision, complex evaulation functions        */
/************************************************************/
inline void
eval_multi_UBspline_2d_c (multi_UBspline_2d_c *spline,
			  double x, double y,
			  complex_float* restrict vals)
{
  x -= spline->x_grid.start;
  y -= spline->y_grid.start;
  float ux = x*spline->x_grid.delta_inv;
  float uy = y*spline->y_grid.delta_inv;
  float ipartx, iparty, tx, ty;
  tx = modff (ux, &ipartx);  int ix = (int) ipartx;
  ty = modff (uy, &iparty);  int iy = (int) iparty;
  
  float tpx[4], tpy[4], a[4], b[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  tpy[0] = ty*ty*ty;  tpy[1] = ty*ty;  tpy[2] = ty;  tpy[3] = 1.0;
  complex_float* restrict coefs = spline->coefs;

  a[0] = (Af[ 0]*tpx[0] + Af[ 1]*tpx[1] + Af[ 2]*tpx[2] + Af[ 3]*tpx[3]);
  a[1] = (Af[ 4]*tpx[0] + Af[ 5]*tpx[1] + Af[ 6]*tpx[2] + Af[ 7]*tpx[3]);
  a[2] = (Af[ 8]*tpx[0] + Af[ 9]*tpx[1] + Af[10]*tpx[2] + Af[11]*tpx[3]);
  a[3] = (Af[12]*tpx[0] + Af[13]*tpx[1] + Af[14]*tpx[2] + Af[15]*tpx[3]);

  b[0] = (Af[ 0]*tpy[0] + Af[ 1]*tpy[1] + Af[ 2]*tpy[2] + Af[ 3]*tpy[3]);
  b[1] = (Af[ 4]*tpy[0] + Af[ 5]*tpy[1] + Af[ 6]*tpy[2] + Af[ 7]*tpy[3]);
  b[2] = (Af[ 8]*tpy[0] + Af[ 9]*tpy[1] + Af[10]*tpy[2] + Af[11]*tpy[3]);
  b[3] = (Af[12]*tpy[0] + Af[13]*tpy[1] + Af[14]*tpy[2] + Af[15]*tpy[3]);

  int xs = spline->x_stride;
  int ys = spline->y_stride;

  for (int n=0; n<spline->num_splines; n++)
    vals[n] = 0.0;

  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++) {
      float prefactor = a[i]*b[j];
      complex_float* restrict coefs = spline->coefs + ((ix+i)*xs + (iy+j)*ys);
      for (int n=0; n<spline->num_splines; n++) 
	vals[n] += prefactor*coefs[n];
    }
}


inline void
eval_multi_UBspline_2d_c_vg (multi_UBspline_2d_c *spline,
			     double x, double y,
			     complex_float* restrict vals,
			     complex_float* restrict grads)
{
  x -= spline->x_grid.start;
  y -= spline->y_grid.start;
  float ux = x*spline->x_grid.delta_inv;
  float uy = y*spline->y_grid.delta_inv;
  float ipartx, iparty, tx, ty;
  tx = modff (ux, &ipartx);  int ix = (int) ipartx;
  ty = modff (uy, &iparty);  int iy = (int) iparty;
  
  float tpx[4], tpy[4], a[4], b[4], da[4], db[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  tpy[0] = ty*ty*ty;  tpy[1] = ty*ty;  tpy[2] = ty;  tpy[3] = 1.0;
  complex_float* restrict coefs = spline->coefs;

  a[0]  = (Af[ 0]*tpx[0] + Af[ 1]*tpx[1] + Af[ 2]*tpx[2] + Af[ 3]*tpx[3]);
  a[1]  = (Af[ 4]*tpx[0] + Af[ 5]*tpx[1] + Af[ 6]*tpx[2] + Af[ 7]*tpx[3]);
  a[2]  = (Af[ 8]*tpx[0] + Af[ 9]*tpx[1] + Af[10]*tpx[2] + Af[11]*tpx[3]);
  a[3]  = (Af[12]*tpx[0] + Af[13]*tpx[1] + Af[14]*tpx[2] + Af[15]*tpx[3]);
  da[0] = (dAf[ 0]*tpx[0] + dAf[ 1]*tpx[1] + dAf[ 2]*tpx[2] + dAf[ 3]*tpx[3]);
  da[1] = (dAf[ 4]*tpx[0] + dAf[ 5]*tpx[1] + dAf[ 6]*tpx[2] + dAf[ 7]*tpx[3]);
  da[2] = (dAf[ 8]*tpx[0] + dAf[ 9]*tpx[1] + dAf[10]*tpx[2] + dAf[11]*tpx[3]);
  da[3] = (dAf[12]*tpx[0] + dAf[13]*tpx[1] + dAf[14]*tpx[2] + dAf[15]*tpx[3]);

  b[0] = (Af[ 0]*tpy[0] + Af[ 1]*tpy[1] + Af[ 2]*tpy[2] + Af[ 3]*tpy[3]);
  b[1] = (Af[ 4]*tpy[0] + Af[ 5]*tpy[1] + Af[ 6]*tpy[2] + Af[ 7]*tpy[3]);
  b[2] = (Af[ 8]*tpy[0] + Af[ 9]*tpy[1] + Af[10]*tpy[2] + Af[11]*tpy[3]);
  b[3] = (Af[12]*tpy[0] + Af[13]*tpy[1] + Af[14]*tpy[2] + Af[15]*tpy[3]);
  db[0] = (dAf[ 0]*tpy[0] + dAf[ 1]*tpy[1] + dAf[ 2]*tpy[2] + dAf[ 3]*tpy[3]);
  db[1] = (dAf[ 4]*tpy[0] + dAf[ 5]*tpy[1] + dAf[ 6]*tpy[2] + dAf[ 7]*tpy[3]);
  db[2] = (dAf[ 8]*tpy[0] + dAf[ 9]*tpy[1] + dAf[10]*tpy[2] + dAf[11]*tpy[3]);
  db[3] = (dAf[12]*tpy[0] + dAf[13]*tpy[1] + dAf[14]*tpy[2] + dAf[15]*tpy[3]);

  int xs = spline->x_stride;
  int ys = spline->y_stride;

  for (int n=0; n<spline->num_splines; n++) {
    vals[n] = 0.0;
    grads[2*n+0] = grads[2*n+1] = grads[2*n+2] = 0.0;
  }

  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++) {
      float ab = a[i]*b[j];
      float dab[2];
      dab[0] = da[i]* b[j];
      dab[1] =  a[i]*db[j];
      
      complex_float* restrict coefs = spline->coefs + ((ix+i)*xs + (iy+j)*ys);
      for (int n=0; n<spline->num_splines; n++) {
	vals [n]     +=   ab   *coefs[n];
	grads[2*n+0] +=  dab[0]*coefs[n];
	grads[2*n+1] +=  dab[1]*coefs[n];
      }
    }

  float dxInv = spline->x_grid.delta_inv;
  float dyInv = spline->y_grid.delta_inv;
  for (int n=0; n<spline->num_splines; n++) {
    grads[2*n+0] *= dxInv;
    grads[2*n+1] *= dyInv;
  }
}

inline void
eval_multi_UBspline_2d_c_vgl (multi_UBspline_2d_c *spline,
			      double x, double y,
			      complex_float* restrict vals,
			      complex_float* restrict grads,
			      complex_float* restrict lapl)	  
{
  x -= spline->x_grid.start;
  y -= spline->y_grid.start;
  float ux = x*spline->x_grid.delta_inv;
  float uy = y*spline->y_grid.delta_inv;
  float ipartx, iparty, tx, ty;
  tx = modff (ux, &ipartx);  int ix = (int) ipartx;
  ty = modff (uy, &iparty);  int iy = (int) iparty;
  
  float tpx[4], tpy[4], a[4], b[4], da[4], db[4], d2a[4], d2b[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  tpy[0] = ty*ty*ty;  tpy[1] = ty*ty;  tpy[2] = ty;  tpy[3] = 1.0;
  complex_float* restrict coefs = spline->coefs;

  a[0]  = (Af[ 0]*tpx[0] + Af[ 1]*tpx[1] + Af[ 2]*tpx[2] + Af[ 3]*tpx[3]);
  a[1]  = (Af[ 4]*tpx[0] + Af[ 5]*tpx[1] + Af[ 6]*tpx[2] + Af[ 7]*tpx[3]);
  a[2]  = (Af[ 8]*tpx[0] + Af[ 9]*tpx[1] + Af[10]*tpx[2] + Af[11]*tpx[3]);
  a[3]  = (Af[12]*tpx[0] + Af[13]*tpx[1] + Af[14]*tpx[2] + Af[15]*tpx[3]);
  da[0] = (dAf[ 0]*tpx[0] + dAf[ 1]*tpx[1] + dAf[ 2]*tpx[2] + dAf[ 3]*tpx[3]);
  da[1] = (dAf[ 4]*tpx[0] + dAf[ 5]*tpx[1] + dAf[ 6]*tpx[2] + dAf[ 7]*tpx[3]);
  da[2] = (dAf[ 8]*tpx[0] + dAf[ 9]*tpx[1] + dAf[10]*tpx[2] + dAf[11]*tpx[3]);
  da[3] = (dAf[12]*tpx[0] + dAf[13]*tpx[1] + dAf[14]*tpx[2] + dAf[15]*tpx[3]);
  d2a[0] = (d2Af[ 0]*tpx[0] + d2Af[ 1]*tpx[1] + d2Af[ 2]*tpx[2] + d2Af[ 3]*tpx[3]);
  d2a[1] = (d2Af[ 4]*tpx[0] + d2Af[ 5]*tpx[1] + d2Af[ 6]*tpx[2] + d2Af[ 7]*tpx[3]);
  d2a[2] = (d2Af[ 8]*tpx[0] + d2Af[ 9]*tpx[1] + d2Af[10]*tpx[2] + d2Af[11]*tpx[3]);
  d2a[3] = (d2Af[12]*tpx[0] + d2Af[13]*tpx[1] + d2Af[14]*tpx[2] + d2Af[15]*tpx[3]);

  b[0] = (Af[ 0]*tpy[0] + Af[ 1]*tpy[1] + Af[ 2]*tpy[2] + Af[ 3]*tpy[3]);
  b[1] = (Af[ 4]*tpy[0] + Af[ 5]*tpy[1] + Af[ 6]*tpy[2] + Af[ 7]*tpy[3]);
  b[2] = (Af[ 8]*tpy[0] + Af[ 9]*tpy[1] + Af[10]*tpy[2] + Af[11]*tpy[3]);
  b[3] = (Af[12]*tpy[0] + Af[13]*tpy[1] + Af[14]*tpy[2] + Af[15]*tpy[3]);
  db[0] = (dAf[ 0]*tpy[0] + dAf[ 1]*tpy[1] + dAf[ 2]*tpy[2] + dAf[ 3]*tpy[3]);
  db[1] = (dAf[ 4]*tpy[0] + dAf[ 5]*tpy[1] + dAf[ 6]*tpy[2] + dAf[ 7]*tpy[3]);
  db[2] = (dAf[ 8]*tpy[0] + dAf[ 9]*tpy[1] + dAf[10]*tpy[2] + dAf[11]*tpy[3]);
  db[3] = (dAf[12]*tpy[0] + dAf[13]*tpy[1] + dAf[14]*tpy[2] + dAf[15]*tpy[3]);
  d2b[0] = (d2Af[ 0]*tpy[0] + d2Af[ 1]*tpy[1] + d2Af[ 2]*tpy[2] + d2Af[ 3]*tpy[3]);
  d2b[1] = (d2Af[ 4]*tpy[0] + d2Af[ 5]*tpy[1] + d2Af[ 6]*tpy[2] + d2Af[ 7]*tpy[3]);
  d2b[2] = (d2Af[ 8]*tpy[0] + d2Af[ 9]*tpy[1] + d2Af[10]*tpy[2] + d2Af[11]*tpy[3]);
  d2b[3] = (d2Af[12]*tpy[0] + d2Af[13]*tpy[1] + d2Af[14]*tpy[2] + d2Af[15]*tpy[3]);

  int xs = spline->x_stride;
  int ys = spline->y_stride;

  complex_float lapl2[2*spline->num_splines];
  for (int n=0; n<spline->num_splines; n++) {
    vals[n] = 0.0;
    grads[2*n+0] = grads[2*n+1] = 0.0;
    lapl2[2*n+0] = lapl2[2*n+1] = 0.0;
  }

  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++) {
      float ab = a[i]*b[j];
      float dab[2], d2ab[2];
      dab[0] = da[i]* b[j];
      dab[1] =  a[i]*db[j];
      d2ab[0] = d2a[i]*  b[j];
      d2ab[1] =   a[i]*d2b[j];
      
      complex_float* restrict coefs = spline->coefs + ((ix+i)*xs + (iy+j)*ys);
      for (int n=0; n<spline->num_splines; n++) {
	vals[n]      +=   ab   *coefs[n];
	grads[2*n+0] +=  dab[0]*coefs[n];
	grads[2*n+1] +=  dab[1]*coefs[n];
	lapl2[2*n+0] += d2ab[0]*coefs[n];
	lapl2[2*n+1] += d2ab[1]*coefs[n];
      }
    }

  float dxInv = spline->x_grid.delta_inv;
  float dyInv = spline->y_grid.delta_inv;
  for (int n=0; n<spline->num_splines; n++) {
    grads[2*n+0] *= dxInv;
    grads[2*n+1] *= dyInv;
    lapl2[2*n+0] *= dxInv*dxInv;
    lapl2[2*n+1] *= dyInv*dyInv;
    lapl[n] = lapl2[2*n+0] + lapl2[2*n+1];
  }
}

inline void
eval_multi_UBspline_2d_c_vgh (multi_UBspline_2d_c *spline,
			      double x, double y,
			      complex_float* restrict vals,
			      complex_float* restrict grads,
			      complex_float* restrict hess)	  
{
  x -= spline->x_grid.start;
  y -= spline->y_grid.start;
  float ux = x*spline->x_grid.delta_inv;
  float uy = y*spline->y_grid.delta_inv;
  float ipartx, iparty, tx, ty;
  tx = modff (ux, &ipartx);  int ix = (int) ipartx;
  ty = modff (uy, &iparty);  int iy = (int) iparty;
  
  float tpx[4], tpy[4], a[4], b[4], da[4], db[4], d2a[4], d2b[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  tpy[0] = ty*ty*ty;  tpy[1] = ty*ty;  tpy[2] = ty;  tpy[3] = 1.0;
  complex_float* restrict coefs = spline->coefs;

  a[0]  = (Af[ 0]*tpx[0] + Af[ 1]*tpx[1] + Af[ 2]*tpx[2] + Af[ 3]*tpx[3]);
  a[1]  = (Af[ 4]*tpx[0] + Af[ 5]*tpx[1] + Af[ 6]*tpx[2] + Af[ 7]*tpx[3]);
  a[2]  = (Af[ 8]*tpx[0] + Af[ 9]*tpx[1] + Af[10]*tpx[2] + Af[11]*tpx[3]);
  a[3]  = (Af[12]*tpx[0] + Af[13]*tpx[1] + Af[14]*tpx[2] + Af[15]*tpx[3]);
  da[0] = (dAf[ 0]*tpx[0] + dAf[ 1]*tpx[1] + dAf[ 2]*tpx[2] + dAf[ 3]*tpx[3]);
  da[1] = (dAf[ 4]*tpx[0] + dAf[ 5]*tpx[1] + dAf[ 6]*tpx[2] + dAf[ 7]*tpx[3]);
  da[2] = (dAf[ 8]*tpx[0] + dAf[ 9]*tpx[1] + dAf[10]*tpx[2] + dAf[11]*tpx[3]);
  da[3] = (dAf[12]*tpx[0] + dAf[13]*tpx[1] + dAf[14]*tpx[2] + dAf[15]*tpx[3]);
  d2a[0] = (d2Af[ 0]*tpx[0] + d2Af[ 1]*tpx[1] + d2Af[ 2]*tpx[2] + d2Af[ 3]*tpx[3]);
  d2a[1] = (d2Af[ 4]*tpx[0] + d2Af[ 5]*tpx[1] + d2Af[ 6]*tpx[2] + d2Af[ 7]*tpx[3]);
  d2a[2] = (d2Af[ 8]*tpx[0] + d2Af[ 9]*tpx[1] + d2Af[10]*tpx[2] + d2Af[11]*tpx[3]);
  d2a[3] = (d2Af[12]*tpx[0] + d2Af[13]*tpx[1] + d2Af[14]*tpx[2] + d2Af[15]*tpx[3]);

  b[0] = (Af[ 0]*tpy[0] + Af[ 1]*tpy[1] + Af[ 2]*tpy[2] + Af[ 3]*tpy[3]);
  b[1] = (Af[ 4]*tpy[0] + Af[ 5]*tpy[1] + Af[ 6]*tpy[2] + Af[ 7]*tpy[3]);
  b[2] = (Af[ 8]*tpy[0] + Af[ 9]*tpy[1] + Af[10]*tpy[2] + Af[11]*tpy[3]);
  b[3] = (Af[12]*tpy[0] + Af[13]*tpy[1] + Af[14]*tpy[2] + Af[15]*tpy[3]);
  db[0] = (dAf[ 0]*tpy[0] + dAf[ 1]*tpy[1] + dAf[ 2]*tpy[2] + dAf[ 3]*tpy[3]);
  db[1] = (dAf[ 4]*tpy[0] + dAf[ 5]*tpy[1] + dAf[ 6]*tpy[2] + dAf[ 7]*tpy[3]);
  db[2] = (dAf[ 8]*tpy[0] + dAf[ 9]*tpy[1] + dAf[10]*tpy[2] + dAf[11]*tpy[3]);
  db[3] = (dAf[12]*tpy[0] + dAf[13]*tpy[1] + dAf[14]*tpy[2] + dAf[15]*tpy[3]);
  d2b[0] = (d2Af[ 0]*tpy[0] + d2Af[ 1]*tpy[1] + d2Af[ 2]*tpy[2] + d2Af[ 3]*tpy[3]);
  d2b[1] = (d2Af[ 4]*tpy[0] + d2Af[ 5]*tpy[1] + d2Af[ 6]*tpy[2] + d2Af[ 7]*tpy[3]);
  d2b[2] = (d2Af[ 8]*tpy[0] + d2Af[ 9]*tpy[1] + d2Af[10]*tpy[2] + d2Af[11]*tpy[3]);
  d2b[3] = (d2Af[12]*tpy[0] + d2Af[13]*tpy[1] + d2Af[14]*tpy[2] + d2Af[15]*tpy[3]);

  int xs = spline->x_stride;
  int ys = spline->y_stride;

  for (int n=0; n<spline->num_splines; n++) {
    vals[n] = 0.0;
    grads[2*n+0] = grads[2*n+1] = 0.0;
    for (int i=0; i<4; i++)
      hess[4*n+i] = 0.0;
  }

  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++){
      float ab = a[i]*b[j];
      float dab[2], d2ab[3];
      dab[0] = da[i]* b[j];
      dab[1] =  a[i]*db[j];
      d2ab[0] = d2a[i] *   b[j];
      d2ab[1] =  da[i] *  db[j];
      d2ab[2] =   a[i] * d2b[j];

      complex_float* restrict coefs = spline->coefs + ((ix+i)*xs + (iy+j)*ys);
      for (int n=0; n<spline->num_splines; n++) {
	vals[n]      +=   ab   *coefs[n];
	grads[2*n+0] +=  dab[0]*coefs[n];
	grads[2*n+1] +=  dab[1]*coefs[n];
	hess [4*n+0] += d2ab[0]*coefs[n];
	hess [4*n+1] += d2ab[1]*coefs[n];
	hess [4*n+3] += d2ab[2]*coefs[n];
      }
    }

  float dxInv = spline->x_grid.delta_inv;
  float dyInv = spline->y_grid.delta_inv;
  for (int n=0; n<spline->num_splines; n++) {
    grads[2*n+0] *= dxInv;
    grads[2*n+1] *= dyInv;
    hess[4*n+0] *= dxInv*dxInv;
    hess[4*n+1] *= dxInv*dyInv;
    hess[4*n+3] *= dyInv*dyInv;
    // Copy hessian elements into lower half of 3x3 matrix
    hess[4*n+2] = hess[4*n+1];
  }
}


/************************************************************/
/* 3D float-precision, complex evaulation functions        */
/************************************************************/
inline void
eval_multi_UBspline_3d_c (multi_UBspline_3d_c *spline,
			  double x, double y, double z,
			  complex_float* restrict vals)
{
  x -= spline->x_grid.start;
  y -= spline->y_grid.start;
  z -= spline->z_grid.start;
  float ux = x*spline->x_grid.delta_inv;
  float uy = y*spline->y_grid.delta_inv;
  float uz = z*spline->z_grid.delta_inv;
  float ipartx, iparty, ipartz, tx, ty, tz;
  tx = modff (ux, &ipartx);  int ix = (int) ipartx;
  ty = modff (uy, &iparty);  int iy = (int) iparty;
  tz = modff (uz, &ipartz);  int iz = (int) ipartz;
  
  float tpx[4], tpy[4], tpz[4], a[4], b[4], c[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  tpy[0] = ty*ty*ty;  tpy[1] = ty*ty;  tpy[2] = ty;  tpy[3] = 1.0;
  tpz[0] = tz*tz*tz;  tpz[1] = tz*tz;  tpz[2] = tz;  tpz[3] = 1.0;
  complex_float* restrict coefs = spline->coefs;

  a[0] = (Af[ 0]*tpx[0] + Af[ 1]*tpx[1] + Af[ 2]*tpx[2] + Af[ 3]*tpx[3]);
  a[1] = (Af[ 4]*tpx[0] + Af[ 5]*tpx[1] + Af[ 6]*tpx[2] + Af[ 7]*tpx[3]);
  a[2] = (Af[ 8]*tpx[0] + Af[ 9]*tpx[1] + Af[10]*tpx[2] + Af[11]*tpx[3]);
  a[3] = (Af[12]*tpx[0] + Af[13]*tpx[1] + Af[14]*tpx[2] + Af[15]*tpx[3]);

  b[0] = (Af[ 0]*tpy[0] + Af[ 1]*tpy[1] + Af[ 2]*tpy[2] + Af[ 3]*tpy[3]);
  b[1] = (Af[ 4]*tpy[0] + Af[ 5]*tpy[1] + Af[ 6]*tpy[2] + Af[ 7]*tpy[3]);
  b[2] = (Af[ 8]*tpy[0] + Af[ 9]*tpy[1] + Af[10]*tpy[2] + Af[11]*tpy[3]);
  b[3] = (Af[12]*tpy[0] + Af[13]*tpy[1] + Af[14]*tpy[2] + Af[15]*tpy[3]);

  c[0] = (Af[ 0]*tpz[0] + Af[ 1]*tpz[1] + Af[ 2]*tpz[2] + Af[ 3]*tpz[3]);
  c[1] = (Af[ 4]*tpz[0] + Af[ 5]*tpz[1] + Af[ 6]*tpz[2] + Af[ 7]*tpz[3]);
  c[2] = (Af[ 8]*tpz[0] + Af[ 9]*tpz[1] + Af[10]*tpz[2] + Af[11]*tpz[3]);
  c[3] = (Af[12]*tpz[0] + Af[13]*tpz[1] + Af[14]*tpz[2] + Af[15]*tpz[3]);

  int xs = spline->x_stride;
  int ys = spline->y_stride;
  int zs = spline->z_stride;

  for (int n=0; n<spline->num_splines; n++)
    vals[n] = 0.0;

  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++) 
      for (int k=0; k<4; k++) {
	float prefactor = a[i]*b[j]*c[k];
	complex_float* restrict coefs = spline->coefs + ((ix+i)*xs + (iy+j)*ys + (iz+k)*zs);
	for (int n=0; n<spline->num_splines; n++) 
	  vals[n] += prefactor*coefs[n];
      }
}


inline void
eval_multi_UBspline_3d_c_vg (multi_UBspline_3d_c *spline,
			      double x, double y, double z,
			      complex_float* restrict vals,
			     complex_float* restrict grads)
{
  x -= spline->x_grid.start;
  y -= spline->y_grid.start;
  z -= spline->z_grid.start;
  float ux = x*spline->x_grid.delta_inv;
  float uy = y*spline->y_grid.delta_inv;
  float uz = z*spline->z_grid.delta_inv;
  float ipartx, iparty, ipartz, tx, ty, tz;
  tx = modff (ux, &ipartx);  int ix = (int) ipartx;
  ty = modff (uy, &iparty);  int iy = (int) iparty;
  tz = modff (uz, &ipartz);  int iz = (int) ipartz;
  
  float tpx[4], tpy[4], tpz[4], a[4], b[4], c[4], 
    da[4], db[4], dc[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  tpy[0] = ty*ty*ty;  tpy[1] = ty*ty;  tpy[2] = ty;  tpy[3] = 1.0;
  tpz[0] = tz*tz*tz;  tpz[1] = tz*tz;  tpz[2] = tz;  tpz[3] = 1.0;
  complex_float* restrict coefs = spline->coefs;

  a[0]  = (Af[ 0]*tpx[0] + Af[ 1]*tpx[1] + Af[ 2]*tpx[2] + Af[ 3]*tpx[3]);
  a[1]  = (Af[ 4]*tpx[0] + Af[ 5]*tpx[1] + Af[ 6]*tpx[2] + Af[ 7]*tpx[3]);
  a[2]  = (Af[ 8]*tpx[0] + Af[ 9]*tpx[1] + Af[10]*tpx[2] + Af[11]*tpx[3]);
  a[3]  = (Af[12]*tpx[0] + Af[13]*tpx[1] + Af[14]*tpx[2] + Af[15]*tpx[3]);
  da[0] = (dAf[ 0]*tpx[0] + dAf[ 1]*tpx[1] + dAf[ 2]*tpx[2] + dAf[ 3]*tpx[3]);
  da[1] = (dAf[ 4]*tpx[0] + dAf[ 5]*tpx[1] + dAf[ 6]*tpx[2] + dAf[ 7]*tpx[3]);
  da[2] = (dAf[ 8]*tpx[0] + dAf[ 9]*tpx[1] + dAf[10]*tpx[2] + dAf[11]*tpx[3]);
  da[3] = (dAf[12]*tpx[0] + dAf[13]*tpx[1] + dAf[14]*tpx[2] + dAf[15]*tpx[3]);

  b[0] = (Af[ 0]*tpy[0] + Af[ 1]*tpy[1] + Af[ 2]*tpy[2] + Af[ 3]*tpy[3]);
  b[1] = (Af[ 4]*tpy[0] + Af[ 5]*tpy[1] + Af[ 6]*tpy[2] + Af[ 7]*tpy[3]);
  b[2] = (Af[ 8]*tpy[0] + Af[ 9]*tpy[1] + Af[10]*tpy[2] + Af[11]*tpy[3]);
  b[3] = (Af[12]*tpy[0] + Af[13]*tpy[1] + Af[14]*tpy[2] + Af[15]*tpy[3]);
  db[0] = (dAf[ 0]*tpy[0] + dAf[ 1]*tpy[1] + dAf[ 2]*tpy[2] + dAf[ 3]*tpy[3]);
  db[1] = (dAf[ 4]*tpy[0] + dAf[ 5]*tpy[1] + dAf[ 6]*tpy[2] + dAf[ 7]*tpy[3]);
  db[2] = (dAf[ 8]*tpy[0] + dAf[ 9]*tpy[1] + dAf[10]*tpy[2] + dAf[11]*tpy[3]);
  db[3] = (dAf[12]*tpy[0] + dAf[13]*tpy[1] + dAf[14]*tpy[2] + dAf[15]*tpy[3]);

  c[0] = (Af[ 0]*tpz[0] + Af[ 1]*tpz[1] + Af[ 2]*tpz[2] + Af[ 3]*tpz[3]);
  c[1] = (Af[ 4]*tpz[0] + Af[ 5]*tpz[1] + Af[ 6]*tpz[2] + Af[ 7]*tpz[3]);
  c[2] = (Af[ 8]*tpz[0] + Af[ 9]*tpz[1] + Af[10]*tpz[2] + Af[11]*tpz[3]);
  c[3] = (Af[12]*tpz[0] + Af[13]*tpz[1] + Af[14]*tpz[2] + Af[15]*tpz[3]);
  dc[0] = (dAf[ 0]*tpz[0] + dAf[ 1]*tpz[1] + dAf[ 2]*tpz[2] + dAf[ 3]*tpz[3]);
  dc[1] = (dAf[ 4]*tpz[0] + dAf[ 5]*tpz[1] + dAf[ 6]*tpz[2] + dAf[ 7]*tpz[3]);
  dc[2] = (dAf[ 8]*tpz[0] + dAf[ 9]*tpz[1] + dAf[10]*tpz[2] + dAf[11]*tpz[3]);
  dc[3] = (dAf[12]*tpz[0] + dAf[13]*tpz[1] + dAf[14]*tpz[2] + dAf[15]*tpz[3]);

  int xs = spline->x_stride;
  int ys = spline->y_stride;
  int zs = spline->z_stride;

  for (int n=0; n<spline->num_splines; n++) {
    vals[n] = 0.0;
    grads[3*n+0] = grads[3*n+1] = grads[3*n+2] = 0.0;
  }

  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++) 
      for (int k=0; k<4; k++) {
	float abc = a[i]*b[j]*c[k];
	float dabc[3];
	dabc[0] = da[i]* b[j]* c[k];
	dabc[1] =  a[i]*db[j]* c[k];
	dabc[2] =  a[i]* b[j]*dc[k];

	complex_float* restrict coefs = spline->coefs + ((ix+i)*xs + (iy+j)*ys + (iz+k)*zs);
	for (int n=0; n<spline->num_splines; n++) {
	  vals[n]      +=   abc   *coefs[n];
	  grads[3*n+0] +=  dabc[0]*coefs[n];
	  grads[3*n+1] +=  dabc[1]*coefs[n];
	  grads[3*n+2] +=  dabc[2]*coefs[n];
	}
      }

  float dxInv = spline->x_grid.delta_inv;
  float dyInv = spline->y_grid.delta_inv;
  float dzInv = spline->z_grid.delta_inv; 
  for (int n=0; n<spline->num_splines; n++) {
    grads[3*n+0] *= dxInv;
    grads[3*n+1] *= dyInv;
    grads[3*n+2] *= dzInv;
  }
}



inline void
eval_multi_UBspline_3d_c_vgl (multi_UBspline_3d_c *spline,
			      double x, double y, double z,
			      complex_float* restrict vals,
			      complex_float* restrict grads,
			      complex_float* restrict lapl)	  
{
  x -= spline->x_grid.start;
  y -= spline->y_grid.start;
  z -= spline->z_grid.start;
  float ux = x*spline->x_grid.delta_inv;
  float uy = y*spline->y_grid.delta_inv;
  float uz = z*spline->z_grid.delta_inv;
  float ipartx, iparty, ipartz, tx, ty, tz;
  tx = modff (ux, &ipartx);  int ix = (int) ipartx;
  ty = modff (uy, &iparty);  int iy = (int) iparty;
  tz = modff (uz, &ipartz);  int iz = (int) ipartz;
  
  float tpx[4], tpy[4], tpz[4], a[4], b[4], c[4], 
    da[4], db[4], dc[4], d2a[4], d2b[4], d2c[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  tpy[0] = ty*ty*ty;  tpy[1] = ty*ty;  tpy[2] = ty;  tpy[3] = 1.0;
  tpz[0] = tz*tz*tz;  tpz[1] = tz*tz;  tpz[2] = tz;  tpz[3] = 1.0;
  complex_float* restrict coefs = spline->coefs;

  a[0]  = (Af[ 0]*tpx[0] + Af[ 1]*tpx[1] + Af[ 2]*tpx[2] + Af[ 3]*tpx[3]);
  a[1]  = (Af[ 4]*tpx[0] + Af[ 5]*tpx[1] + Af[ 6]*tpx[2] + Af[ 7]*tpx[3]);
  a[2]  = (Af[ 8]*tpx[0] + Af[ 9]*tpx[1] + Af[10]*tpx[2] + Af[11]*tpx[3]);
  a[3]  = (Af[12]*tpx[0] + Af[13]*tpx[1] + Af[14]*tpx[2] + Af[15]*tpx[3]);
  da[0] = (dAf[ 0]*tpx[0] + dAf[ 1]*tpx[1] + dAf[ 2]*tpx[2] + dAf[ 3]*tpx[3]);
  da[1] = (dAf[ 4]*tpx[0] + dAf[ 5]*tpx[1] + dAf[ 6]*tpx[2] + dAf[ 7]*tpx[3]);
  da[2] = (dAf[ 8]*tpx[0] + dAf[ 9]*tpx[1] + dAf[10]*tpx[2] + dAf[11]*tpx[3]);
  da[3] = (dAf[12]*tpx[0] + dAf[13]*tpx[1] + dAf[14]*tpx[2] + dAf[15]*tpx[3]);
  d2a[0] = (d2Af[ 0]*tpx[0] + d2Af[ 1]*tpx[1] + d2Af[ 2]*tpx[2] + d2Af[ 3]*tpx[3]);
  d2a[1] = (d2Af[ 4]*tpx[0] + d2Af[ 5]*tpx[1] + d2Af[ 6]*tpx[2] + d2Af[ 7]*tpx[3]);
  d2a[2] = (d2Af[ 8]*tpx[0] + d2Af[ 9]*tpx[1] + d2Af[10]*tpx[2] + d2Af[11]*tpx[3]);
  d2a[3] = (d2Af[12]*tpx[0] + d2Af[13]*tpx[1] + d2Af[14]*tpx[2] + d2Af[15]*tpx[3]);

  b[0] = (Af[ 0]*tpy[0] + Af[ 1]*tpy[1] + Af[ 2]*tpy[2] + Af[ 3]*tpy[3]);
  b[1] = (Af[ 4]*tpy[0] + Af[ 5]*tpy[1] + Af[ 6]*tpy[2] + Af[ 7]*tpy[3]);
  b[2] = (Af[ 8]*tpy[0] + Af[ 9]*tpy[1] + Af[10]*tpy[2] + Af[11]*tpy[3]);
  b[3] = (Af[12]*tpy[0] + Af[13]*tpy[1] + Af[14]*tpy[2] + Af[15]*tpy[3]);
  db[0] = (dAf[ 0]*tpy[0] + dAf[ 1]*tpy[1] + dAf[ 2]*tpy[2] + dAf[ 3]*tpy[3]);
  db[1] = (dAf[ 4]*tpy[0] + dAf[ 5]*tpy[1] + dAf[ 6]*tpy[2] + dAf[ 7]*tpy[3]);
  db[2] = (dAf[ 8]*tpy[0] + dAf[ 9]*tpy[1] + dAf[10]*tpy[2] + dAf[11]*tpy[3]);
  db[3] = (dAf[12]*tpy[0] + dAf[13]*tpy[1] + dAf[14]*tpy[2] + dAf[15]*tpy[3]);
  d2b[0] = (d2Af[ 0]*tpy[0] + d2Af[ 1]*tpy[1] + d2Af[ 2]*tpy[2] + d2Af[ 3]*tpy[3]);
  d2b[1] = (d2Af[ 4]*tpy[0] + d2Af[ 5]*tpy[1] + d2Af[ 6]*tpy[2] + d2Af[ 7]*tpy[3]);
  d2b[2] = (d2Af[ 8]*tpy[0] + d2Af[ 9]*tpy[1] + d2Af[10]*tpy[2] + d2Af[11]*tpy[3]);
  d2b[3] = (d2Af[12]*tpy[0] + d2Af[13]*tpy[1] + d2Af[14]*tpy[2] + d2Af[15]*tpy[3]);

  c[0] = (Af[ 0]*tpz[0] + Af[ 1]*tpz[1] + Af[ 2]*tpz[2] + Af[ 3]*tpz[3]);
  c[1] = (Af[ 4]*tpz[0] + Af[ 5]*tpz[1] + Af[ 6]*tpz[2] + Af[ 7]*tpz[3]);
  c[2] = (Af[ 8]*tpz[0] + Af[ 9]*tpz[1] + Af[10]*tpz[2] + Af[11]*tpz[3]);
  c[3] = (Af[12]*tpz[0] + Af[13]*tpz[1] + Af[14]*tpz[2] + Af[15]*tpz[3]);
  dc[0] = (dAf[ 0]*tpz[0] + dAf[ 1]*tpz[1] + dAf[ 2]*tpz[2] + dAf[ 3]*tpz[3]);
  dc[1] = (dAf[ 4]*tpz[0] + dAf[ 5]*tpz[1] + dAf[ 6]*tpz[2] + dAf[ 7]*tpz[3]);
  dc[2] = (dAf[ 8]*tpz[0] + dAf[ 9]*tpz[1] + dAf[10]*tpz[2] + dAf[11]*tpz[3]);
  dc[3] = (dAf[12]*tpz[0] + dAf[13]*tpz[1] + dAf[14]*tpz[2] + dAf[15]*tpz[3]);
  d2c[0] = (d2Af[ 0]*tpz[0] + d2Af[ 1]*tpz[1] + d2Af[ 2]*tpz[2] + d2Af[ 3]*tpz[3]);
  d2c[1] = (d2Af[ 4]*tpz[0] + d2Af[ 5]*tpz[1] + d2Af[ 6]*tpz[2] + d2Af[ 7]*tpz[3]);
  d2c[2] = (d2Af[ 8]*tpz[0] + d2Af[ 9]*tpz[1] + d2Af[10]*tpz[2] + d2Af[11]*tpz[3]);
  d2c[3] = (d2Af[12]*tpz[0] + d2Af[13]*tpz[1] + d2Af[14]*tpz[2] + d2Af[15]*tpz[3]);

  int xs = spline->x_stride;
  int ys = spline->y_stride;
  int zs = spline->z_stride;

  complex_float lapl3[3*spline->num_splines];
  for (int n=0; n<spline->num_splines; n++) {
    vals[n] = 0.0;
    grads[3*n+0] = grads[3*n+1] = grads[3*n+2] = 0.0;
    lapl3[3*n+0] = lapl3[3*n+1] = lapl3[3*n+2] = 0.0;
  }

  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++) 
      for (int k=0; k<4; k++) {
	float abc = a[i]*b[j]*c[k];
	float dabc[3], d2abc[3];
	dabc[0] = da[i]* b[j]* c[k];
	dabc[1] =  a[i]*db[j]* c[k];
	dabc[2] =  a[i]* b[j]*dc[k];
	d2abc[0] = d2a[i]*  b[j]*  c[k];
	d2abc[1] =   a[i]*d2b[j]*  c[k];
	d2abc[2] =   a[i]*  b[j]*d2c[k];

	complex_float* restrict coefs = spline->coefs + ((ix+i)*xs + (iy+j)*ys + (iz+k)*zs);
	for (int n=0; n<spline->num_splines; n++) {
	  vals[n]      +=   abc   *coefs[n];
	  grads[3*n+0] +=  dabc[0]*coefs[n];
	  grads[3*n+1] +=  dabc[1]*coefs[n];
	  grads[3*n+2] +=  dabc[2]*coefs[n];
	  lapl3[3*n+0] += d2abc[0]*coefs[n];
	  lapl3[3*n+1] += d2abc[1]*coefs[n];
	  lapl3[3*n+2] += d2abc[2]*coefs[n];
	}
      }

  float dxInv = spline->x_grid.delta_inv;
  float dyInv = spline->y_grid.delta_inv;
  float dzInv = spline->z_grid.delta_inv; 
  for (int n=0; n<spline->num_splines; n++) {
    grads[3*n+0] *= dxInv;
    grads[3*n+1] *= dyInv;
    grads[3*n+2] *= dzInv;
    lapl3[3*n+0] *= dxInv*dxInv;
    lapl3[3*n+1] *= dyInv*dyInv;
    lapl3[3*n+2] *= dzInv*dzInv;
    lapl[n] = lapl3[3*n+0] + lapl3[3*n+1] + lapl3[3*n+2];
  }
}



inline void
eval_multi_UBspline_3d_c_vgh (multi_UBspline_3d_c *spline,
			      double x, double y, double z,
			      complex_float* restrict vals,
			      complex_float* restrict grads,
			      complex_float* restrict hess)	  
{
  x -= spline->x_grid.start;
  y -= spline->y_grid.start;
  z -= spline->z_grid.start;
  float ux = x*spline->x_grid.delta_inv;
  float uy = y*spline->y_grid.delta_inv;
  float uz = z*spline->z_grid.delta_inv;
  float ipartx, iparty, ipartz, tx, ty, tz;
  tx = modff (ux, &ipartx);  int ix = (int) ipartx;
  ty = modff (uy, &iparty);  int iy = (int) iparty;
  tz = modff (uz, &ipartz);  int iz = (int) ipartz;
  
  float tpx[4], tpy[4], tpz[4], a[4], b[4], c[4], 
    da[4], db[4], dc[4], d2a[4], d2b[4], d2c[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  tpy[0] = ty*ty*ty;  tpy[1] = ty*ty;  tpy[2] = ty;  tpy[3] = 1.0;
  tpz[0] = tz*tz*tz;  tpz[1] = tz*tz;  tpz[2] = tz;  tpz[3] = 1.0;
  complex_float* restrict coefs = spline->coefs;

  a[0]  = (Af[ 0]*tpx[0] + Af[ 1]*tpx[1] + Af[ 2]*tpx[2] + Af[ 3]*tpx[3]);
  a[1]  = (Af[ 4]*tpx[0] + Af[ 5]*tpx[1] + Af[ 6]*tpx[2] + Af[ 7]*tpx[3]);
  a[2]  = (Af[ 8]*tpx[0] + Af[ 9]*tpx[1] + Af[10]*tpx[2] + Af[11]*tpx[3]);
  a[3]  = (Af[12]*tpx[0] + Af[13]*tpx[1] + Af[14]*tpx[2] + Af[15]*tpx[3]);
  da[0] = (dAf[ 1]*tpx[1] + dAf[ 2]*tpx[2] + dAf[ 3]*tpx[3]);
  da[1] = (dAf[ 5]*tpx[1] + dAf[ 6]*tpx[2] + dAf[ 7]*tpx[3]);
  da[2] = (dAf[ 9]*tpx[1] + dAf[10]*tpx[2] + dAf[11]*tpx[3]);
  da[3] = (dAf[13]*tpx[1] + dAf[14]*tpx[2] + dAf[15]*tpx[3]);
  d2a[0] = (d2Af[ 2]*tpx[2] + d2Af[ 3]*tpx[3]);
  d2a[1] = (d2Af[ 6]*tpx[2] + d2Af[ 7]*tpx[3]);
  d2a[2] = (d2Af[10]*tpx[2] + d2Af[11]*tpx[3]);
  d2a[3] = (d2Af[14]*tpx[2] + d2Af[15]*tpx[3]);

  b[0] = (Af[ 0]*tpy[0] + Af[ 1]*tpy[1] + Af[ 2]*tpy[2] + Af[ 3]*tpy[3]);
  b[1] = (Af[ 4]*tpy[0] + Af[ 5]*tpy[1] + Af[ 6]*tpy[2] + Af[ 7]*tpy[3]);
  b[2] = (Af[ 8]*tpy[0] + Af[ 9]*tpy[1] + Af[10]*tpy[2] + Af[11]*tpy[3]);
  b[3] = (Af[12]*tpy[0] + Af[13]*tpy[1] + Af[14]*tpy[2] + Af[15]*tpy[3]);
  db[0] = (dAf[ 1]*tpy[1] + dAf[ 2]*tpy[2] + dAf[ 3]*tpy[3]);
  db[1] = (dAf[ 5]*tpy[1] + dAf[ 6]*tpy[2] + dAf[ 7]*tpy[3]);
  db[2] = (dAf[ 9]*tpy[1] + dAf[10]*tpy[2] + dAf[11]*tpy[3]);
  db[3] = (dAf[13]*tpy[1] + dAf[14]*tpy[2] + dAf[15]*tpy[3]);
  d2b[0] = (d2Af[ 2]*tpy[2] + d2Af[ 3]*tpy[3]);
  d2b[1] = (d2Af[ 6]*tpy[2] + d2Af[ 7]*tpy[3]);
  d2b[2] = (d2Af[10]*tpy[2] + d2Af[11]*tpy[3]);
  d2b[3] = (d2Af[14]*tpy[2] + d2Af[15]*tpy[3]);

  c[0] = (Af[ 0]*tpz[0] + Af[ 1]*tpz[1] + Af[ 2]*tpz[2] + Af[ 3]*tpz[3]);
  c[1] = (Af[ 4]*tpz[0] + Af[ 5]*tpz[1] + Af[ 6]*tpz[2] + Af[ 7]*tpz[3]);
  c[2] = (Af[ 8]*tpz[0] + Af[ 9]*tpz[1] + Af[10]*tpz[2] + Af[11]*tpz[3]);
  c[3] = (Af[12]*tpz[0] + Af[13]*tpz[1] + Af[14]*tpz[2] + Af[15]*tpz[3]);
  dc[0] = (dAf[ 1]*tpz[1] + dAf[ 2]*tpz[2] + dAf[ 3]*tpz[3]);
  dc[1] = (dAf[ 5]*tpz[1] + dAf[ 6]*tpz[2] + dAf[ 7]*tpz[3]);
  dc[2] = (dAf[ 9]*tpz[1] + dAf[10]*tpz[2] + dAf[11]*tpz[3]);
  dc[3] = (dAf[13]*tpz[1] + dAf[14]*tpz[2] + dAf[15]*tpz[3]);
  d2c[0] = (d2Af[ 2]*tpz[2] + d2Af[ 3]*tpz[3]);
  d2c[1] = (d2Af[ 6]*tpz[2] + d2Af[ 7]*tpz[3]);
  d2c[2] = (d2Af[10]*tpz[2] + d2Af[11]*tpz[3]);
  d2c[3] = (d2Af[14]*tpz[2] + d2Af[15]*tpz[3]);

  int xs = spline->x_stride;
  int ys = spline->y_stride;
  int zs = spline->z_stride;

  for (int n=0; n<spline->num_splines; n++) {
    vals[n] = 0.0;
    grads[3*n+0] = grads[3*n+1] = grads[3*n+2] = 0.0;
    for (int i=0; i<9; i++)
      hess[9*n+i] = 0.0;
  }

  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++) 
      for (int k=0; k<4; k++) {
	float abc = a[i]*b[j]*c[k];
	float dabc[3], d2abc[6];
	dabc[0] = da[i]* b[j]* c[k];
	dabc[1] =  a[i]*db[j]* c[k];
	dabc[2] =  a[i]* b[j]*dc[k];
	d2abc[0] = d2a[i]*  b[j]*  c[k];
	d2abc[1] =  da[i]* db[j]*  c[k];
	d2abc[2] =  da[i]*  b[j]* dc[k];
	d2abc[3] =   a[i]*d2b[j]*  c[k];
	d2abc[4] =   a[i]* db[j]* dc[k];
	d2abc[5] =   a[i]*  b[j]*d2c[k];

	complex_float* restrict coefs = spline->coefs + ((ix+i)*xs + (iy+j)*ys + (iz+k)*zs);
	for (int n=0; n<spline->num_splines; n++) {
	  vals[n]      +=   abc   *coefs[n];
	  grads[3*n+0] +=  dabc[0]*coefs[n];
	  grads[3*n+1] +=  dabc[1]*coefs[n];
	  grads[3*n+2] +=  dabc[2]*coefs[n];
	  hess [9*n+0] += d2abc[0]*coefs[n];
	  hess [9*n+1] += d2abc[1]*coefs[n];
	  hess [9*n+2] += d2abc[2]*coefs[n];
	  hess [9*n+4] += d2abc[3]*coefs[n];
	  hess [9*n+5] += d2abc[4]*coefs[n];
	  hess [9*n+8] += d2abc[5]*coefs[n];
	}
      }

  float dxInv = spline->x_grid.delta_inv;
  float dyInv = spline->y_grid.delta_inv;
  float dzInv = spline->z_grid.delta_inv; 
  for (int n=0; n<spline->num_splines; n++) {
    grads[3*n+0] *= dxInv;
    grads[3*n+1] *= dyInv;
    grads[3*n+2] *= dzInv;
    hess[9*n+0] *= dxInv*dxInv;
    hess[9*n+4] *= dyInv*dyInv;
    hess[9*n+8] *= dzInv*dzInv;
    hess[9*n+1] *= dxInv*dyInv;
    hess[9*n+2] *= dxInv*dzInv;
    hess[9*n+5] *= dyInv*dzInv;
    // Copy hessian elements into lower half of 3x3 matrix
    hess[9*n+3] = hess[9*n+1];
    hess[9*n+6] = hess[9*n+2];
    hess[9*n+7] = hess[9*n+5];
  }
}


inline void
eval_multi_UBspline_3d_s_vghgh (multi_UBspline_3d_c *spline,
               double x, double y, double z,
               complex_float* restrict vals,
               complex_float* restrict grads,
               complex_float* restrict hess,
               complex_float* restrict gradhess)    
{
  x -= spline->x_grid.start;
  y -= spline->y_grid.start;
  z -= spline->z_grid.start;
  float ux = x*spline->x_grid.delta_inv;
  float uy = y*spline->y_grid.delta_inv;
  float uz = z*spline->z_grid.delta_inv;
  float ipartx, iparty, ipartz, tx, ty, tz;
  tx = modff (ux, &ipartx);  int ix = (int) ipartx;
  ty = modff (uy, &iparty);  int iy = (int) iparty;
  tz = modff (uz, &ipartz);  int iz = (int) ipartz;
  
  float tpx[4], tpy[4], tpz[4], a[4], b[4], c[4], 
    da[4], db[4], dc[4], d2a[4], d2b[4], d2c[4],
    d3a[4], d3b[4], d3c[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  tpy[0] = ty*ty*ty;  tpy[1] = ty*ty;  tpy[2] = ty;  tpy[3] = 1.0;
  tpz[0] = tz*tz*tz;  tpz[1] = tz*tz;  tpz[2] = tz;  tpz[3] = 1.0;
  complex_float* restrict coefs = spline->coefs;

  a[0]  = (Af[ 0]*tpx[0] + Af[ 1]*tpx[1] + Af[ 2]*tpx[2] + Af[ 3]*tpx[3]);
  a[1]  = (Af[ 4]*tpx[0] + Af[ 5]*tpx[1] + Af[ 6]*tpx[2] + Af[ 7]*tpx[3]);
  a[2]  = (Af[ 8]*tpx[0] + Af[ 9]*tpx[1] + Af[10]*tpx[2] + Af[11]*tpx[3]);
  a[3]  = (Af[12]*tpx[0] + Af[13]*tpx[1] + Af[14]*tpx[2] + Af[15]*tpx[3]);
  da[0] = (dAf[ 0]*tpx[0] + dAf[ 1]*tpx[1] + dAf[ 2]*tpx[2] + dAf[ 3]*tpx[3]);
  da[1] = (dAf[ 4]*tpx[0] + dAf[ 5]*tpx[1] + dAf[ 6]*tpx[2] + dAf[ 7]*tpx[3]);
  da[2] = (dAf[ 8]*tpx[0] + dAf[ 9]*tpx[1] + dAf[10]*tpx[2] + dAf[11]*tpx[3]);
  da[3] = (dAf[12]*tpx[0] + dAf[13]*tpx[1] + dAf[14]*tpx[2] + dAf[15]*tpx[3]);
  d2a[0] = (d2Af[ 0]*tpx[0] + d2Af[ 1]*tpx[1] + d2Af[ 2]*tpx[2] + d2Af[ 3]*tpx[3]);
  d2a[1] = (d2Af[ 4]*tpx[0] + d2Af[ 5]*tpx[1] + d2Af[ 6]*tpx[2] + d2Af[ 7]*tpx[3]);
  d2a[2] = (d2Af[ 8]*tpx[0] + d2Af[ 9]*tpx[1] + d2Af[10]*tpx[2] + d2Af[11]*tpx[3]);
  d2a[3] = (d2Af[12]*tpx[0] + d2Af[13]*tpx[1] + d2Af[14]*tpx[2] + d2Af[15]*tpx[3]);
  d3a[0] = (/*d2Af[ 0]*tpx[0] + d2Af[ 1]*tpx[1] + d2Af[ 2]*tpx[2] +*/ d3Af[ 3]*tpx[3]);
  d3a[1] = (/*d2Af[ 4]*tpx[0] + d2Af[ 5]*tpx[1] + d2Af[ 6]*tpx[2] +*/ d3Af[ 7]*tpx[3]);
  d3a[2] = (/*d2Af[ 8]*tpx[0] + d2Af[ 9]*tpx[1] + d2Af[10]*tpx[2] +*/ d3Af[11]*tpx[3]);
  d3a[3] = (/*d2Af[12]*tpx[0] + d2Af[13]*tpx[1] + d2Af[14]*tpx[2] +*/ d3Af[15]*tpx[3]);
  
  b[0] = (Af[ 0]*tpy[0] + Af[ 1]*tpy[1] + Af[ 2]*tpy[2] + Af[ 3]*tpy[3]);
  b[1] = (Af[ 4]*tpy[0] + Af[ 5]*tpy[1] + Af[ 6]*tpy[2] + Af[ 7]*tpy[3]);
  b[2] = (Af[ 8]*tpy[0] + Af[ 9]*tpy[1] + Af[10]*tpy[2] + Af[11]*tpy[3]);
  b[3] = (Af[12]*tpy[0] + Af[13]*tpy[1] + Af[14]*tpy[2] + Af[15]*tpy[3]);
  db[0] = (dAf[ 0]*tpy[0] + dAf[ 1]*tpy[1] + dAf[ 2]*tpy[2] + dAf[ 3]*tpy[3]);
  db[1] = (dAf[ 4]*tpy[0] + dAf[ 5]*tpy[1] + dAf[ 6]*tpy[2] + dAf[ 7]*tpy[3]);
  db[2] = (dAf[ 8]*tpy[0] + dAf[ 9]*tpy[1] + dAf[10]*tpy[2] + dAf[11]*tpy[3]);
  db[3] = (dAf[12]*tpy[0] + dAf[13]*tpy[1] + dAf[14]*tpy[2] + dAf[15]*tpy[3]);
  d2b[0] = (d2Af[ 0]*tpy[0] + d2Af[ 1]*tpy[1] + d2Af[ 2]*tpy[2] + d2Af[ 3]*tpy[3]);
  d2b[1] = (d2Af[ 4]*tpy[0] + d2Af[ 5]*tpy[1] + d2Af[ 6]*tpy[2] + d2Af[ 7]*tpy[3]);
  d2b[2] = (d2Af[ 8]*tpy[0] + d2Af[ 9]*tpy[1] + d2Af[10]*tpy[2] + d2Af[11]*tpy[3]);
  d2b[3] = (d2Af[12]*tpy[0] + d2Af[13]*tpy[1] + d2Af[14]*tpy[2] + d2Af[15]*tpy[3]);
  d3b[0] = (/*d2Af[ 0]*tpx[0] + d2Af[ 1]*tpx[1] + d2Af[ 2]*tpx[2] +*/ d3Af[ 3]*tpy[3]);
  d3b[1] = (/*d2Af[ 4]*tpx[0] + d2Af[ 5]*tpx[1] + d2Af[ 6]*tpx[2] +*/ d3Af[ 7]*tpy[3]);
  d3b[2] = (/*d2Af[ 8]*tpx[0] + d2Af[ 9]*tpx[1] + d2Af[10]*tpx[2] +*/ d3Af[11]*tpy[3]);
  d3b[3] = (/*d2Af[12]*tpx[0] + d2Af[13]*tpx[1] + d2Af[14]*tpx[2] +*/ d3Af[15]*tpy[3]);

  c[0] = (Af[ 0]*tpz[0] + Af[ 1]*tpz[1] + Af[ 2]*tpz[2] + Af[ 3]*tpz[3]);
  c[1] = (Af[ 4]*tpz[0] + Af[ 5]*tpz[1] + Af[ 6]*tpz[2] + Af[ 7]*tpz[3]);
  c[2] = (Af[ 8]*tpz[0] + Af[ 9]*tpz[1] + Af[10]*tpz[2] + Af[11]*tpz[3]);
  c[3] = (Af[12]*tpz[0] + Af[13]*tpz[1] + Af[14]*tpz[2] + Af[15]*tpz[3]);
  dc[0] = (dAf[ 0]*tpz[0] + dAf[ 1]*tpz[1] + dAf[ 2]*tpz[2] + dAf[ 3]*tpz[3]);
  dc[1] = (dAf[ 4]*tpz[0] + dAf[ 5]*tpz[1] + dAf[ 6]*tpz[2] + dAf[ 7]*tpz[3]);
  dc[2] = (dAf[ 8]*tpz[0] + dAf[ 9]*tpz[1] + dAf[10]*tpz[2] + dAf[11]*tpz[3]);
  dc[3] = (dAf[12]*tpz[0] + dAf[13]*tpz[1] + dAf[14]*tpz[2] + dAf[15]*tpz[3]);
  d2c[0] = (d2Af[ 0]*tpz[0] + d2Af[ 1]*tpz[1] + d2Af[ 2]*tpz[2] + d2Af[ 3]*tpz[3]);
  d2c[1] = (d2Af[ 4]*tpz[0] + d2Af[ 5]*tpz[1] + d2Af[ 6]*tpz[2] + d2Af[ 7]*tpz[3]);
  d2c[2] = (d2Af[ 8]*tpz[0] + d2Af[ 9]*tpz[1] + d2Af[10]*tpz[2] + d2Af[11]*tpz[3]);
  d2c[3] = (d2Af[12]*tpz[0] + d2Af[13]*tpz[1] + d2Af[14]*tpz[2] + d2Af[15]*tpz[3]);
  d3c[0] = (/*d2Af[ 0]*tpx[0] + d2Af[ 1]*tpx[1] + d2Af[ 2]*tpx[2] +*/ d3Af[ 3]*tpz[3]);
  d3c[1] = (/*d2Af[ 4]*tpx[0] + d2Af[ 5]*tpx[1] + d2Af[ 6]*tpx[2] +*/ d3Af[ 7]*tpz[3]);
  d3c[2] = (/*d2Af[ 8]*tpx[0] + d2Af[ 9]*tpx[1] + d2Af[10]*tpx[2] +*/ d3Af[11]*tpz[3]);
  d3c[3] = (/*d2Af[12]*tpx[0] + d2Af[13]*tpx[1] + d2Af[14]*tpx[2] +*/ d3Af[15]*tpz[3]);

  int xs = spline->x_stride;
  int ys = spline->y_stride;
  int zs = spline->z_stride;

  for (int n=0; n<spline->num_splines; n++) {
    vals[n] = 0.0;
    grads[3*n+0] = grads[3*n+1] = grads[3*n+2] = 0.0;
    for (int i=0; i<9; i++)
      hess[9*n+i] = 0.0;
    for (int i=0; i<27; i++)
      gradhess[27*n+i] = 0.0;
  }

  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++) 
      for (int k=0; k<4; k++) {
   float abc = a[i]*b[j]*c[k];
   float dabc[3], d2abc[6], d3abc[10];
   dabc[0] = da[i]* b[j]* c[k];
   dabc[1] =  a[i]*db[j]* c[k];
   dabc[2] =  a[i]* b[j]*dc[k];
   d2abc[0] = d2a[i]*  b[j]*  c[k];
   d2abc[1] =  da[i]* db[j]*  c[k];
   d2abc[2] =  da[i]*  b[j]* dc[k];
   d2abc[3] =   a[i]*d2b[j]*  c[k];
   d2abc[4] =   a[i]* db[j]* dc[k];
   d2abc[5] =   a[i]*  b[j]*d2c[k];
   
   d3abc[0] = d3a[i]*  b[j]*  c[k];
   d3abc[1] = d2a[i]* db[j]*  c[k];
   d3abc[2] = d2a[i]*  b[j]* dc[k];
   d3abc[3] =  da[i]*d2b[j]*  c[k];
   d3abc[4] =  da[i]* db[j]* dc[k];
   d3abc[5] =  da[i]*  b[j]*d2c[k];   
   d3abc[6] =   a[i]*d3b[j]*  c[k];
   d3abc[7] =   a[i]*d2b[j]* dc[k];
   d3abc[8] =   a[i]* db[j]*d2c[k];     
   d3abc[9] =   a[i]*  b[j]*d3c[k];

   complex_float* restrict coefs = spline->coefs + ((ix+i)*xs + (iy+j)*ys + (iz+k)*zs);
   for (int n=0; n<spline->num_splines; n++) {
     vals[n]      +=   abc   *coefs[n];
     grads[3*n+0] +=  dabc[0]*coefs[n];
     grads[3*n+1] +=  dabc[1]*coefs[n];
     grads[3*n+2] +=  dabc[2]*coefs[n];
     hess [9*n+0] += d2abc[0]*coefs[n];
     hess [9*n+1] += d2abc[1]*coefs[n];
     hess [9*n+2] += d2abc[2]*coefs[n];
     hess [9*n+4] += d2abc[3]*coefs[n];
     hess [9*n+5] += d2abc[4]*coefs[n];
     hess [9*n+8] += d2abc[5]*coefs[n];
     
     gradhess [27*n+0 ] += d3abc[0]*coefs[n];
     gradhess [27*n+1 ] += d3abc[1]*coefs[n];
     gradhess [27*n+2 ] += d3abc[2]*coefs[n];
     gradhess [27*n+4 ] += d3abc[3]*coefs[n];
     gradhess [27*n+5 ] += d3abc[4]*coefs[n];
     gradhess [27*n+8 ] += d3abc[5]*coefs[n];
     gradhess [27*n+13] += d3abc[6]*coefs[n];
     gradhess [27*n+14] += d3abc[7]*coefs[n];
     gradhess [27*n+17] += d3abc[8]*coefs[n];
     gradhess [27*n+26] += d3abc[9]*coefs[n];
   }
      }

  float dxInv = spline->x_grid.delta_inv;
  float dyInv = spline->y_grid.delta_inv;
  float dzInv = spline->z_grid.delta_inv; 
  for (int n=0; n<spline->num_splines; n++) {
    grads[3*n+0] *= dxInv;
    grads[3*n+1] *= dyInv;
    grads[3*n+2] *= dzInv;
    hess [9*n+0] *= dxInv*dxInv;
    hess [9*n+4] *= dyInv*dyInv;
    hess [9*n+8] *= dzInv*dzInv;
    hess [9*n+1] *= dxInv*dyInv;
    hess [9*n+2] *= dxInv*dzInv;
    hess [9*n+5] *= dyInv*dzInv;
    // Copy hessian elements into lower half of 3x3 matrix
    hess [9*n+3] = hess[9*n+1];
    hess [9*n+6] = hess[9*n+2];
    hess [9*n+7] = hess[9*n+5];
    
    gradhess [27*n+0 ] *= dxInv*dxInv*dxInv;
    gradhess [27*n+1 ] *= dxInv*dxInv*dyInv;
    gradhess [27*n+2 ] *= dxInv*dxInv*dzInv;
    gradhess [27*n+4 ] *= dxInv*dyInv*dyInv;
    gradhess [27*n+5 ] *= dxInv*dyInv*dzInv;
    gradhess [27*n+8 ] *= dxInv*dzInv*dzInv;
    gradhess [27*n+13] *= dyInv*dyInv*dyInv;
    gradhess [27*n+14] *= dyInv*dyInv*dzInv; 
    gradhess [27*n+17] *= dyInv*dzInv*dzInv;
    gradhess [27*n+26] *= dzInv*dzInv*dzInv;
    
    // Copy gradhess elements into rest of tensor
    gradhess [27*n+9  ] = gradhess [27*n+3  ] = gradhess [27*n+1 ];
    gradhess [27*n+18 ] = gradhess [27*n+6  ] = gradhess [27*n+2 ];
    gradhess [27*n+22 ] = gradhess [27*n+16 ] = gradhess [27*n+14];
    gradhess [27*n+12 ] = gradhess [27*n+10 ] = gradhess [27*n+4 ];
    gradhess [27*n+24 ] = gradhess [27*n+20 ] = gradhess [27*n+8 ];
    gradhess [27*n+25 ] = gradhess [27*n+23 ] = gradhess [27*n+17];
    gradhess [27*n+21 ] = gradhess [27*n+19 ] = gradhess [27*n+15] = gradhess [27*n+11 ] = gradhess [27*n+7 ] = gradhess [27*n+5];

  }
}

#endif
