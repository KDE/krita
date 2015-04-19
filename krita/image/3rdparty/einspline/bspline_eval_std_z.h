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

#ifndef BSPLINE_EVAL_STD_Z_H
#define BSPLINE_EVAL_STD_Z_H

#include <math.h>
#include <stdio.h>

extern const double* restrict   Ad;
extern const double* restrict  dAd;
extern const double* restrict d2Ad;

/************************************************************/
/* 1D double-precision, complex evaulation functions        */
/************************************************************/

/* Value only */
inline void
eval_UBspline_1d_z (UBspline_1d_z * restrict spline, 
		    double x, complex_double* restrict val)
{
  x -= spline->x_grid.start;
  double u = x*spline->x_grid.delta_inv;
  double ipart, t;
  t = modf (u, &ipart);
  int i = (int) ipart;
  
  double tp[4];
  tp[0] = t*t*t;  tp[1] = t*t;  tp[2] = t;  tp[3] = 1.0;
  complex_double* restrict coefs = spline->coefs;

  *val = 
    (coefs[i+0]*(Ad[ 0]*tp[0] + Ad[ 1]*tp[1] + Ad[ 2]*tp[2] + Ad[ 3]*tp[3])+
     coefs[i+1]*(Ad[ 4]*tp[0] + Ad[ 5]*tp[1] + Ad[ 6]*tp[2] + Ad[ 7]*tp[3])+
     coefs[i+2]*(Ad[ 8]*tp[0] + Ad[ 9]*tp[1] + Ad[10]*tp[2] + Ad[11]*tp[3])+
     coefs[i+3]*(Ad[12]*tp[0] + Ad[13]*tp[1] + Ad[14]*tp[2] + Ad[15]*tp[3]));
}

/* Value and first derivative */
inline void
eval_UBspline_1d_z_vg (UBspline_1d_z * restrict spline, double x, 
		       complex_double* restrict val, 
		       complex_double* restrict grad)
{
  x -= spline->x_grid.start;
  double u = x*spline->x_grid.delta_inv;
  double ipart, t;
  t = modf (u, &ipart);
  int i = (int) ipart;
  
  double tp[4];
  tp[0] = t*t*t;  tp[1] = t*t;  tp[2] = t;  tp[3] = 1.0;
  complex_double* restrict coefs = spline->coefs;

  *val = 
    (coefs[i+0]*(Ad[ 0]*tp[0] + Ad[ 1]*tp[1] + Ad[ 2]*tp[2] + Ad[ 3]*tp[3])+
     coefs[i+1]*(Ad[ 4]*tp[0] + Ad[ 5]*tp[1] + Ad[ 6]*tp[2] + Ad[ 7]*tp[3])+
     coefs[i+2]*(Ad[ 8]*tp[0] + Ad[ 9]*tp[1] + Ad[10]*tp[2] + Ad[11]*tp[3])+
     coefs[i+3]*(Ad[12]*tp[0] + Ad[13]*tp[1] + Ad[14]*tp[2] + Ad[15]*tp[3]));
  *grad = spline->x_grid.delta_inv * 
    (coefs[i+0]*(dAd[ 1]*tp[1] + dAd[ 2]*tp[2] + dAd[ 3]*tp[3])+
     coefs[i+1]*(dAd[ 5]*tp[1] + dAd[ 6]*tp[2] + dAd[ 7]*tp[3])+
     coefs[i+2]*(dAd[ 9]*tp[1] + dAd[10]*tp[2] + dAd[11]*tp[3])+
     coefs[i+3]*(dAd[13]*tp[1] + dAd[14]*tp[2] + dAd[15]*tp[3]));
}
/* Value, first derivative, and second derivative */
inline void
eval_UBspline_1d_z_vgl (UBspline_1d_z * restrict spline, double x, 
			complex_double* restrict val, complex_double* restrict grad,
			complex_double* restrict lapl)
{
  x -= spline->x_grid.start;
  double u = x*spline->x_grid.delta_inv;
  double ipart, t;
  t = modf (u, &ipart);
  int i = (int) ipart;
  
  double tp[4];
  tp[0] = t*t*t;  tp[1] = t*t;  tp[2] = t;  tp[3] = 1.0;
  complex_double* restrict coefs = spline->coefs;

  *val = 
    (coefs[i+0]*(Ad[ 0]*tp[0] + Ad[ 1]*tp[1] + Ad[ 2]*tp[2] + Ad[ 3]*tp[3])+
     coefs[i+1]*(Ad[ 4]*tp[0] + Ad[ 5]*tp[1] + Ad[ 6]*tp[2] + Ad[ 7]*tp[3])+
     coefs[i+2]*(Ad[ 8]*tp[0] + Ad[ 9]*tp[1] + Ad[10]*tp[2] + Ad[11]*tp[3])+
     coefs[i+3]*(Ad[12]*tp[0] + Ad[13]*tp[1] + Ad[14]*tp[2] + Ad[15]*tp[3]));
  *grad = spline->x_grid.delta_inv * 
    (coefs[i+0]*(dAd[ 1]*tp[1] + dAd[ 2]*tp[2] + dAd[ 3]*tp[3])+
     coefs[i+1]*(dAd[ 5]*tp[1] + dAd[ 6]*tp[2] + dAd[ 7]*tp[3])+
     coefs[i+2]*(dAd[ 9]*tp[1] + dAd[10]*tp[2] + dAd[11]*tp[3])+
     coefs[i+3]*(dAd[13]*tp[1] + dAd[14]*tp[2] + dAd[15]*tp[3]));
  *lapl = spline->x_grid.delta_inv * spline->x_grid.delta_inv * 
    (coefs[i+0]*(d2Ad[ 2]*tp[2] + d2Ad[ 3]*tp[3])+
     coefs[i+1]*(d2Ad[ 6]*tp[2] + d2Ad[ 7]*tp[3])+
     coefs[i+2]*(d2Ad[10]*tp[2] + d2Ad[11]*tp[3])+
     coefs[i+3]*(d2Ad[14]*tp[2] + d2Ad[15]*tp[3]));
}

inline void
eval_UBspline_1d_z_vgh (UBspline_1d_z * restrict spline, double x, 
			complex_double* restrict val, 
			complex_double* restrict grad,
			complex_double* restrict hess)
{
  eval_UBspline_1d_z_vgh (spline, x, val, grad, hess);
}
/************************************************************/
/* 2D double-precision, complex evaulation functions        */
/************************************************************/

/* Value only */
inline void
eval_UBspline_2d_z (UBspline_2d_z * restrict spline, 
		    double x, double y, complex_double* restrict val)
{
  x -= spline->x_grid.start;
  y -= spline->y_grid.start;
  double ux = x*spline->x_grid.delta_inv;
  double uy = y*spline->y_grid.delta_inv;
  double ipartx, iparty, tx, ty;
  tx = modf (ux, &ipartx);
  ty = modf (uy, &iparty);
  int ix = (int) ipartx;
  int iy = (int) iparty;
  
  double tpx[4], tpy[4], a[4], b[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  tpy[0] = ty*ty*ty;  tpy[1] = ty*ty;  tpy[2] = ty;  tpy[3] = 1.0;
  complex_double* restrict coefs = spline->coefs;

  a[0] = (Ad[ 0]*tpx[0] + Ad[ 1]*tpx[1] + Ad[ 2]*tpx[2] + Ad[ 3]*tpx[3]);
  a[1] = (Ad[ 4]*tpx[0] + Ad[ 5]*tpx[1] + Ad[ 6]*tpx[2] + Ad[ 7]*tpx[3]);
  a[2] = (Ad[ 8]*tpx[0] + Ad[ 9]*tpx[1] + Ad[10]*tpx[2] + Ad[11]*tpx[3]);
  a[3] = (Ad[12]*tpx[0] + Ad[13]*tpx[1] + Ad[14]*tpx[2] + Ad[15]*tpx[3]);

  b[0] = (Ad[ 0]*tpy[0] + Ad[ 1]*tpy[1] + Ad[ 2]*tpy[2] + Ad[ 3]*tpy[3]);
  b[1] = (Ad[ 4]*tpy[0] + Ad[ 5]*tpy[1] + Ad[ 6]*tpy[2] + Ad[ 7]*tpy[3]);
  b[2] = (Ad[ 8]*tpy[0] + Ad[ 9]*tpy[1] + Ad[10]*tpy[2] + Ad[11]*tpy[3]);
  b[3] = (Ad[12]*tpy[0] + Ad[13]*tpy[1] + Ad[14]*tpy[2] + Ad[15]*tpy[3]);
  
  int xs = spline->x_stride;
#define C(i,j) coefs[(ix+(i))*xs+iy+(j)]
  *val = (a[0]*(C(0,0)*b[0]+C(0,1)*b[1]+C(0,2)*b[2]+C(0,3)*b[3])+
	  a[1]*(C(1,0)*b[0]+C(1,1)*b[1]+C(1,2)*b[2]+C(1,3)*b[3])+
	  a[2]*(C(2,0)*b[0]+C(2,1)*b[1]+C(2,2)*b[2]+C(2,3)*b[3])+
	  a[3]*(C(3,0)*b[0]+C(3,1)*b[1]+C(3,2)*b[2]+C(3,3)*b[3]));
#undef C

}


/* Value and gradient */
inline void
eval_UBspline_2d_z_vg (UBspline_2d_z * restrict spline, 
		       double x, double y, 
		       complex_double* restrict val, 
		       complex_double* restrict grad)
{
  x -= spline->x_grid.start;
  y -= spline->y_grid.start;
  double ux = x*spline->x_grid.delta_inv;
  double uy = y*spline->y_grid.delta_inv;
  double ipartx, iparty, tx, ty;
  tx = modf (ux, &ipartx);
  ty = modf (uy, &iparty);
  int ix = (int) ipartx;
  int iy = (int) iparty;
  
  double tpx[4], tpy[4], a[4], b[4], da[4], db[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  tpy[0] = ty*ty*ty;  tpy[1] = ty*ty;  tpy[2] = ty;  tpy[3] = 1.0;
  complex_double* restrict coefs = spline->coefs;

  a[0]  = (Ad[ 0]*tpx[0] + Ad[ 1]*tpx[1] + Ad[ 2]*tpx[2] + Ad[ 3]*tpx[3]);
  a[1]  = (Ad[ 4]*tpx[0] + Ad[ 5]*tpx[1] + Ad[ 6]*tpx[2] + Ad[ 7]*tpx[3]);
  a[2]  = (Ad[ 8]*tpx[0] + Ad[ 9]*tpx[1] + Ad[10]*tpx[2] + Ad[11]*tpx[3]);
  a[3]  = (Ad[12]*tpx[0] + Ad[13]*tpx[1] + Ad[14]*tpx[2] + Ad[15]*tpx[3]);
  da[0] = (dAd[ 1]*tpx[1] + dAd[ 2]*tpx[2] + dAd[ 3]*tpx[3]);
  da[1] = (dAd[ 5]*tpx[1] + dAd[ 6]*tpx[2] + dAd[ 7]*tpx[3]);
  da[2] = (dAd[ 9]*tpx[1] + dAd[10]*tpx[2] + dAd[11]*tpx[3]);
  da[3] = (dAd[13]*tpx[1] + dAd[14]*tpx[2] + dAd[15]*tpx[3]);

  b[0]  = (Ad[ 0]*tpy[0] + Ad[ 1]*tpy[1] + Ad[ 2]*tpy[2] + Ad[ 3]*tpy[3]);
  b[1]  = (Ad[ 4]*tpy[0] + Ad[ 5]*tpy[1] + Ad[ 6]*tpy[2] + Ad[ 7]*tpy[3]);
  b[2]  = (Ad[ 8]*tpy[0] + Ad[ 9]*tpy[1] + Ad[10]*tpy[2] + Ad[11]*tpy[3]);
  b[3]  = (Ad[12]*tpy[0] + Ad[13]*tpy[1] + Ad[14]*tpy[2] + Ad[15]*tpy[3]);
  db[0] = (dAd[ 1]*tpy[1] + dAd[ 2]*tpy[2] + dAd[ 3]*tpy[3]);
  db[1] = (dAd[ 5]*tpy[1] + dAd[ 6]*tpy[2] + dAd[ 7]*tpy[3]);
  db[2] = (dAd[ 9]*tpy[1] + dAd[10]*tpy[2] + dAd[11]*tpy[3]);
  db[3] = (dAd[13]*tpy[1] + dAd[14]*tpy[2] + dAd[15]*tpy[3]);
  
  int xs = spline->x_stride;
#define C(i,j) coefs[(ix+(i))*xs+iy+(j)]
  *val =    
    (a[0]*(C(0,0)*b[0]+C(0,1)*b[1]+C(0,2)*b[2]+C(0,3)*b[3])+
     a[1]*(C(1,0)*b[0]+C(1,1)*b[1]+C(1,2)*b[2]+C(1,3)*b[3])+
     a[2]*(C(2,0)*b[0]+C(2,1)*b[1]+C(2,2)*b[2]+C(2,3)*b[3])+
     a[3]*(C(3,0)*b[0]+C(3,1)*b[1]+C(3,2)*b[2]+C(3,3)*b[3]));
  grad[0] = spline->x_grid.delta_inv *
    (da[0]*(C(0,0)*b[0]+C(0,1)*b[1]+C(0,2)*b[2]+C(0,3)*b[3])+
     da[1]*(C(1,0)*b[0]+C(1,1)*b[1]+C(1,2)*b[2]+C(1,3)*b[3])+
     da[2]*(C(2,0)*b[0]+C(2,1)*b[1]+C(2,2)*b[2]+C(2,3)*b[3])+
     da[3]*(C(3,0)*b[0]+C(3,1)*b[1]+C(3,2)*b[2]+C(3,3)*b[3]));
  grad[1] = spline->y_grid.delta_inv * 
    (a[0]*(C(0,0)*db[0]+C(0,1)*db[1]+C(0,2)*db[2]+C(0,3)*db[3])+
     a[1]*(C(1,0)*db[0]+C(1,1)*db[1]+C(1,2)*db[2]+C(1,3)*db[3])+
     a[2]*(C(2,0)*db[0]+C(2,1)*db[1]+C(2,2)*db[2]+C(2,3)*db[3])+
     a[3]*(C(3,0)*db[0]+C(3,1)*db[1]+C(3,2)*db[2]+C(3,3)*db[3]));
#undef C

}

/* Value, gradient, and laplacian */
inline void
eval_UBspline_2d_z_vgl (UBspline_2d_z * restrict spline, 
			double x, double y, complex_double* restrict val, 
			complex_double* restrict grad, 
			complex_double* restrict lapl)
{
  x -= spline->x_grid.start;
  y -= spline->y_grid.start;
  double ux = x*spline->x_grid.delta_inv;
  double uy = y*spline->y_grid.delta_inv;
  double ipartx, iparty, tx, ty;
  tx = modf (ux, &ipartx);
  ty = modf (uy, &iparty);
  int ix = (int) ipartx;
  int iy = (int) iparty;
  
  double tpx[4], tpy[4], a[4], b[4], da[4], db[4], d2a[4], d2b[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  tpy[0] = ty*ty*ty;  tpy[1] = ty*ty;  tpy[2] = ty;  tpy[3] = 1.0;
  complex_double* restrict coefs = spline->coefs;

  a[0]   = (  Ad[ 0]*tpx[0] +   Ad[ 1]*tpx[1] +  Ad[ 2]*tpx[2] + Ad[ 3]*tpx[3]);
  a[1]   = (  Ad[ 4]*tpx[0] +   Ad[ 5]*tpx[1] +  Ad[ 6]*tpx[2] + Ad[ 7]*tpx[3]);
  a[2]   = (  Ad[ 8]*tpx[0] +   Ad[ 9]*tpx[1] +  Ad[10]*tpx[2] + Ad[11]*tpx[3]);
  a[3]   = (  Ad[12]*tpx[0] +   Ad[13]*tpx[1] +  Ad[14]*tpx[2] + Ad[15]*tpx[3]);
  da[0]  = ( dAd[ 1]*tpx[1] +  dAd[ 2]*tpx[2] + dAd[ 3]*tpx[3]);
  da[1]  = ( dAd[ 5]*tpx[1] +  dAd[ 6]*tpx[2] + dAd[ 7]*tpx[3]);
  da[2]  = ( dAd[ 9]*tpx[1] +  dAd[10]*tpx[2] + dAd[11]*tpx[3]);
  da[3]  = ( dAd[13]*tpx[1] +  dAd[14]*tpx[2] + dAd[15]*tpx[3]);
  d2a[0] = (d2Ad[ 2]*tpx[2] + d2Ad[ 3]*tpx[3]);
  d2a[1] = (d2Ad[ 6]*tpx[2] + d2Ad[ 7]*tpx[3]);
  d2a[2] = (d2Ad[10]*tpx[2] + d2Ad[11]*tpx[3]);
  d2a[3] = (d2Ad[14]*tpx[2] + d2Ad[15]*tpx[3]);

  b[0]  = ( Ad[ 0]*tpy[0] + Ad[ 1]*tpy[1] + Ad[ 2]*tpy[2] + Ad[ 3]*tpy[3]);
  b[1]  = ( Ad[ 4]*tpy[0] + Ad[ 5]*tpy[1] + Ad[ 6]*tpy[2] + Ad[ 7]*tpy[3]);
  b[2]  = ( Ad[ 8]*tpy[0] + Ad[ 9]*tpy[1] + Ad[10]*tpy[2] + Ad[11]*tpy[3]);
  b[3]  = ( Ad[12]*tpy[0] + Ad[13]*tpy[1] + Ad[14]*tpy[2] + Ad[15]*tpy[3]);
  db[0] = (dAd[ 1]*tpy[1] + dAd[ 2]*tpy[2] + dAd[ 3]*tpy[3]);
  db[1] = (dAd[ 5]*tpy[1] + dAd[ 6]*tpy[2] + dAd[ 7]*tpy[3]);
  db[2] = (dAd[ 9]*tpy[1] + dAd[10]*tpy[2] + dAd[11]*tpy[3]);
  db[3] = (dAd[13]*tpy[1] + dAd[14]*tpy[2] + dAd[15]*tpy[3]);
  d2b[0] = (d2Ad[ 2]*tpy[2] + d2Ad[ 3]*tpy[3]);
  d2b[1] = (d2Ad[ 6]*tpy[2] + d2Ad[ 7]*tpy[3]);
  d2b[2] = (d2Ad[10]*tpy[2] + d2Ad[11]*tpy[3]);
  d2b[3] = (d2Ad[14]*tpy[2] + d2Ad[15]*tpy[3]);
  
  int xs = spline->x_stride;
#define C(i,j) coefs[(ix+(i))*xs+iy+(j)]
  *val =    
    (a[0]*(C(0,0)*b[0]+C(0,1)*b[1]+C(0,2)*b[2]+C(0,3)*b[3])+
     a[1]*(C(1,0)*b[0]+C(1,1)*b[1]+C(1,2)*b[2]+C(1,3)*b[3])+
     a[2]*(C(2,0)*b[0]+C(2,1)*b[1]+C(2,2)*b[2]+C(2,3)*b[3])+
     a[3]*(C(3,0)*b[0]+C(3,1)*b[1]+C(3,2)*b[2]+C(3,3)*b[3]));
  grad[0] = spline->x_grid.delta_inv *
    (da[0]*(C(0,0)*b[0]+C(0,1)*b[1]+C(0,2)*b[2]+C(0,3)*b[3])+
     da[1]*(C(1,0)*b[0]+C(1,1)*b[1]+C(1,2)*b[2]+C(1,3)*b[3])+
     da[2]*(C(2,0)*b[0]+C(2,1)*b[1]+C(2,2)*b[2]+C(2,3)*b[3])+
     da[3]*(C(3,0)*b[0]+C(3,1)*b[1]+C(3,2)*b[2]+C(3,3)*b[3]));
  grad[1] = spline->y_grid.delta_inv *
    (a[0]*(C(0,0)*db[0]+C(0,1)*db[1]+C(0,2)*db[2]+C(0,3)*db[3])+
     a[1]*(C(1,0)*db[0]+C(1,1)*db[1]+C(1,2)*db[2]+C(1,3)*db[3])+
     a[2]*(C(2,0)*db[0]+C(2,1)*db[1]+C(2,2)*db[2]+C(2,3)*db[3])+
     a[3]*(C(3,0)*db[0]+C(3,1)*db[1]+C(3,2)*db[2]+C(3,3)*db[3]));
  *lapl   = 
    spline->y_grid.delta_inv * spline->y_grid.delta_inv *
    (a[0]*(C(0,0)*d2b[0]+C(0,1)*d2b[1]+C(0,2)*d2b[2]+C(0,3)*d2b[3])+
     a[1]*(C(1,0)*d2b[0]+C(1,1)*d2b[1]+C(1,2)*d2b[2]+C(1,3)*d2b[3])+
     a[2]*(C(2,0)*d2b[0]+C(2,1)*d2b[1]+C(2,2)*d2b[2]+C(2,3)*d2b[3])+
     a[3]*(C(3,0)*d2b[0]+C(3,1)*d2b[1]+C(3,2)*d2b[2]+C(3,3)*d2b[3])) + 
    spline->x_grid.delta_inv * spline->x_grid.delta_inv *
     (d2a[0]*(C(0,0)*b[0]+C(0,1)*b[1]+C(0,2)*b[2]+C(0,3)*b[3])+
      d2a[1]*(C(1,0)*b[0]+C(1,1)*b[1]+C(1,2)*b[2]+C(1,3)*b[3])+
      d2a[2]*(C(2,0)*b[0]+C(2,1)*b[1]+C(2,2)*b[2]+C(2,3)*b[3])+
      d2a[3]*(C(3,0)*b[0]+C(3,1)*b[1]+C(3,2)*b[2]+C(3,3)*b[3]));
  
#undef C

}

/* Value, gradient, and Hessian */
inline void
eval_UBspline_2d_z_vgh (UBspline_2d_z * restrict spline, 
			double x, double y, complex_double* restrict val, 
			complex_double* restrict grad, 
			complex_double* restrict hess)
{
  x -= spline->x_grid.start;
  y -= spline->y_grid.start;
  double ux = x*spline->x_grid.delta_inv;
  double uy = y*spline->y_grid.delta_inv;
  double ipartx, iparty, tx, ty;
  tx = modf (ux, &ipartx);
  ty = modf (uy, &iparty);
  int ix = (int) ipartx;
  int iy = (int) iparty;
  
  double tpx[4], tpy[4], a[4], b[4], da[4], db[4], d2a[4], d2b[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  tpy[0] = ty*ty*ty;  tpy[1] = ty*ty;  tpy[2] = ty;  tpy[3] = 1.0;
  complex_double* restrict coefs = spline->coefs;

  a[0]   = (  Ad[ 0]*tpx[0] +   Ad[ 1]*tpx[1] +  Ad[ 2]*tpx[2] + Ad[ 3]*tpx[3]);
  a[1]   = (  Ad[ 4]*tpx[0] +   Ad[ 5]*tpx[1] +  Ad[ 6]*tpx[2] + Ad[ 7]*tpx[3]);
  a[2]   = (  Ad[ 8]*tpx[0] +   Ad[ 9]*tpx[1] +  Ad[10]*tpx[2] + Ad[11]*tpx[3]);
  a[3]   = (  Ad[12]*tpx[0] +   Ad[13]*tpx[1] +  Ad[14]*tpx[2] + Ad[15]*tpx[3]);
  da[0]  = ( dAd[ 1]*tpx[1] +  dAd[ 2]*tpx[2] + dAd[ 3]*tpx[3]);
  da[1]  = ( dAd[ 5]*tpx[1] +  dAd[ 6]*tpx[2] + dAd[ 7]*tpx[3]);
  da[2]  = ( dAd[ 9]*tpx[1] +  dAd[10]*tpx[2] + dAd[11]*tpx[3]);
  da[3]  = ( dAd[13]*tpx[1] +  dAd[14]*tpx[2] + dAd[15]*tpx[3]);
  d2a[0] = (d2Ad[ 2]*tpx[2] + d2Ad[ 3]*tpx[3]);
  d2a[1] = (d2Ad[ 6]*tpx[2] + d2Ad[ 7]*tpx[3]);
  d2a[2] = (d2Ad[10]*tpx[2] + d2Ad[11]*tpx[3]);
  d2a[3] = (d2Ad[14]*tpx[2] + d2Ad[15]*tpx[3]);

  b[0]   = (  Ad[ 0]*tpy[0] +   Ad[ 1]*tpy[1] +  Ad[ 2]*tpy[2] + Ad[ 3]*tpy[3]);
  b[1]   = (  Ad[ 4]*tpy[0] +   Ad[ 5]*tpy[1] +  Ad[ 6]*tpy[2] + Ad[ 7]*tpy[3]);
  b[2]   = (  Ad[ 8]*tpy[0] +   Ad[ 9]*tpy[1] +  Ad[10]*tpy[2] + Ad[11]*tpy[3]);
  b[3]   = (  Ad[12]*tpy[0] +   Ad[13]*tpy[1] +  Ad[14]*tpy[2] + Ad[15]*tpy[3]);
  db[0]  = ( dAd[ 1]*tpy[1] +  dAd[ 2]*tpy[2] + dAd[ 3]*tpy[3]);
  db[1]  = ( dAd[ 5]*tpy[1] +  dAd[ 6]*tpy[2] + dAd[ 7]*tpy[3]);
  db[2]  = ( dAd[ 9]*tpy[1] +  dAd[10]*tpy[2] + dAd[11]*tpy[3]);
  db[3]  = ( dAd[13]*tpy[1] +  dAd[14]*tpy[2] + dAd[15]*tpy[3]);
  d2b[0] = (d2Ad[ 2]*tpy[2] + d2Ad[ 3]*tpy[3]);
  d2b[1] = (d2Ad[ 6]*tpy[2] + d2Ad[ 7]*tpy[3]);
  d2b[2] = (d2Ad[10]*tpy[2] + d2Ad[11]*tpy[3]);
  d2b[3] = (d2Ad[14]*tpy[2] + d2Ad[15]*tpy[3]);
  
  int xs = spline->x_stride;
#define C(i,j) coefs[(ix+(i))*xs+iy+(j)]
  *val =    
    (  a[0]*(C(0,0)*  b[0]+C(0,1)*  b[1]+C(0,2)*  b[2]+C(0,3)*  b[3])+
       a[1]*(C(1,0)*  b[0]+C(1,1)*  b[1]+C(1,2)*  b[2]+C(1,3)*  b[3])+
       a[2]*(C(2,0)*  b[0]+C(2,1)*  b[1]+C(2,2)*  b[2]+C(2,3)*  b[3])+
       a[3]*(C(3,0)*  b[0]+C(3,1)*  b[1]+C(3,2)*  b[2]+C(3,3)*  b[3]));
  grad[0] = spline->x_grid.delta_inv *
    ( da[0]*(C(0,0)*  b[0]+C(0,1)*  b[1]+C(0,2)*  b[2]+C(0,3)*  b[3])+
      da[1]*(C(1,0)*  b[0]+C(1,1)*  b[1]+C(1,2)*  b[2]+C(1,3)*  b[3])+
      da[2]*(C(2,0)*  b[0]+C(2,1)*  b[1]+C(2,2)*  b[2]+C(2,3)*  b[3])+
      da[3]*(C(3,0)*  b[0]+C(3,1)*  b[1]+C(3,2)*  b[2]+C(3,3)*  b[3]));
  grad[1] = spline->y_grid.delta_inv *
    (  a[0]*(C(0,0)* db[0]+C(0,1)* db[1]+C(0,2)* db[2]+C(0,3)* db[3])+
       a[1]*(C(1,0)* db[0]+C(1,1)* db[1]+C(1,2)* db[2]+C(1,3)* db[3])+
       a[2]*(C(2,0)* db[0]+C(2,1)* db[1]+C(2,2)* db[2]+C(2,3)* db[3])+
       a[3]*(C(3,0)* db[0]+C(3,1)* db[1]+C(3,2)* db[2]+C(3,3)* db[3]));
  hess[0] = spline->x_grid.delta_inv * spline->x_grid.delta_inv *
    (d2a[0]*(C(0,0)*  b[0]+C(0,1)*  b[1]+C(0,2)*  b[2]+C(0,3)*  b[3])+
     d2a[1]*(C(1,0)*  b[0]+C(1,1)*  b[1]+C(1,2)*  b[2]+C(1,3)*  b[3])+
     d2a[2]*(C(2,0)*  b[0]+C(2,1)*  b[1]+C(2,2)*  b[2]+C(2,3)*  b[3])+
     d2a[3]*(C(3,0)*  b[0]+C(3,1)*  b[1]+C(3,2)*  b[2]+C(3,3)*  b[3]));
  hess[1] = spline->x_grid.delta_inv * spline->y_grid.delta_inv *
    ( da[0]*(C(0,0)* db[0]+C(0,1)* db[1]+C(0,2)* db[2]+C(0,3)* db[3])+
      da[1]*(C(1,0)* db[0]+C(1,1)* db[1]+C(1,2)* db[2]+C(1,3)* db[3])+
      da[2]*(C(2,0)* db[0]+C(2,1)* db[1]+C(2,2)* db[2]+C(2,3)* db[3])+
      da[3]*(C(3,0)* db[0]+C(3,1)* db[1]+C(3,2)* db[2]+C(3,3)* db[3]));
  hess[3] = spline->y_grid.delta_inv * spline->y_grid.delta_inv *
    (  a[0]*(C(0,0)*d2b[0]+C(0,1)*d2b[1]+C(0,2)*d2b[2]+C(0,3)*d2b[3])+
       a[1]*(C(1,0)*d2b[0]+C(1,1)*d2b[1]+C(1,2)*d2b[2]+C(1,3)*d2b[3])+
       a[2]*(C(2,0)*d2b[0]+C(2,1)*d2b[1]+C(2,2)*d2b[2]+C(2,3)*d2b[3])+
       a[3]*(C(3,0)*d2b[0]+C(3,1)*d2b[1]+C(3,2)*d2b[2]+C(3,3)*d2b[3]));
  hess[2] = hess[1];
  
#undef C

}


/************************************************************/
/* 3D double-precision, complex evaulation functions           */
/************************************************************/

/* Value only */
inline void
eval_UBspline_3d_z (UBspline_3d_z * restrict spline, 
		    double x, double y, double z,
		    complex_double* restrict val)
{
  x -= spline->x_grid.start;
  y -= spline->y_grid.start;
  z -= spline->z_grid.start;
  double ux = x*spline->x_grid.delta_inv;
  double uy = y*spline->y_grid.delta_inv;
  double uz = z*spline->z_grid.delta_inv;
  double ipartx, iparty, ipartz, tx, ty, tz;
  tx = modf (ux, &ipartx);  int ix = (int) ipartx;
  ty = modf (uy, &iparty);  int iy = (int) iparty;
  tz = modf (uz, &ipartz);  int iz = (int) ipartz;
  
  double tpx[4], tpy[4], tpz[4], a[4], b[4], c[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  tpy[0] = ty*ty*ty;  tpy[1] = ty*ty;  tpy[2] = ty;  tpy[3] = 1.0;
  tpz[0] = tz*tz*tz;  tpz[1] = tz*tz;  tpz[2] = tz;  tpz[3] = 1.0;
  complex_double* restrict coefs = spline->coefs;

  a[0] = (Ad[ 0]*tpx[0] + Ad[ 1]*tpx[1] + Ad[ 2]*tpx[2] + Ad[ 3]*tpx[3]);
  a[1] = (Ad[ 4]*tpx[0] + Ad[ 5]*tpx[1] + Ad[ 6]*tpx[2] + Ad[ 7]*tpx[3]);
  a[2] = (Ad[ 8]*tpx[0] + Ad[ 9]*tpx[1] + Ad[10]*tpx[2] + Ad[11]*tpx[3]);
  a[3] = (Ad[12]*tpx[0] + Ad[13]*tpx[1] + Ad[14]*tpx[2] + Ad[15]*tpx[3]);

  b[0] = (Ad[ 0]*tpy[0] + Ad[ 1]*tpy[1] + Ad[ 2]*tpy[2] + Ad[ 3]*tpy[3]);
  b[1] = (Ad[ 4]*tpy[0] + Ad[ 5]*tpy[1] + Ad[ 6]*tpy[2] + Ad[ 7]*tpy[3]);
  b[2] = (Ad[ 8]*tpy[0] + Ad[ 9]*tpy[1] + Ad[10]*tpy[2] + Ad[11]*tpy[3]);
  b[3] = (Ad[12]*tpy[0] + Ad[13]*tpy[1] + Ad[14]*tpy[2] + Ad[15]*tpy[3]);

  c[0] = (Ad[ 0]*tpz[0] + Ad[ 1]*tpz[1] + Ad[ 2]*tpz[2] + Ad[ 3]*tpz[3]);
  c[1] = (Ad[ 4]*tpz[0] + Ad[ 5]*tpz[1] + Ad[ 6]*tpz[2] + Ad[ 7]*tpz[3]);
  c[2] = (Ad[ 8]*tpz[0] + Ad[ 9]*tpz[1] + Ad[10]*tpz[2] + Ad[11]*tpz[3]);
  c[3] = (Ad[12]*tpz[0] + Ad[13]*tpz[1] + Ad[14]*tpz[2] + Ad[15]*tpz[3]);
  
  int xs = spline->x_stride;
  int ys = spline->y_stride;
#define P(i,j,k) coefs[(ix+(i))*xs+(iy+(j))*ys+(iz+(k))]
  *val = (a[0]*(b[0]*(P(0,0,0)*c[0]+P(0,0,1)*c[1]+P(0,0,2)*c[2]+P(0,0,3)*c[3])+
		b[1]*(P(0,1,0)*c[0]+P(0,1,1)*c[1]+P(0,1,2)*c[2]+P(0,1,3)*c[3])+
		b[2]*(P(0,2,0)*c[0]+P(0,2,1)*c[1]+P(0,2,2)*c[2]+P(0,2,3)*c[3])+
		b[3]*(P(0,3,0)*c[0]+P(0,3,1)*c[1]+P(0,3,2)*c[2]+P(0,3,3)*c[3]))+
	  a[1]*(b[0]*(P(1,0,0)*c[0]+P(1,0,1)*c[1]+P(1,0,2)*c[2]+P(1,0,3)*c[3])+
		b[1]*(P(1,1,0)*c[0]+P(1,1,1)*c[1]+P(1,1,2)*c[2]+P(1,1,3)*c[3])+
		b[2]*(P(1,2,0)*c[0]+P(1,2,1)*c[1]+P(1,2,2)*c[2]+P(1,2,3)*c[3])+
		b[3]*(P(1,3,0)*c[0]+P(1,3,1)*c[1]+P(1,3,2)*c[2]+P(1,3,3)*c[3]))+
	  a[2]*(b[0]*(P(2,0,0)*c[0]+P(2,0,1)*c[1]+P(2,0,2)*c[2]+P(2,0,3)*c[3])+
		b[1]*(P(2,1,0)*c[0]+P(2,1,1)*c[1]+P(2,1,2)*c[2]+P(2,1,3)*c[3])+
		b[2]*(P(2,2,0)*c[0]+P(2,2,1)*c[1]+P(2,2,2)*c[2]+P(2,2,3)*c[3])+
		b[3]*(P(2,3,0)*c[0]+P(2,3,1)*c[1]+P(2,3,2)*c[2]+P(2,3,3)*c[3]))+
	  a[3]*(b[0]*(P(3,0,0)*c[0]+P(3,0,1)*c[1]+P(3,0,2)*c[2]+P(3,0,3)*c[3])+
		b[1]*(P(3,1,0)*c[0]+P(3,1,1)*c[1]+P(3,1,2)*c[2]+P(3,1,3)*c[3])+
		b[2]*(P(3,2,0)*c[0]+P(3,2,1)*c[1]+P(3,2,2)*c[2]+P(3,2,3)*c[3])+
		b[3]*(P(3,3,0)*c[0]+P(3,3,1)*c[1]+P(3,3,2)*c[2]+P(3,3,3)*c[3])));
#undef P

}

/* Value and gradient */
inline void
eval_UBspline_3d_z_vg (UBspline_3d_z * restrict spline, 
		       double x, double y, double z,
		       complex_double* restrict val, 
		       complex_double* restrict grad)
{
  x -= spline->x_grid.start;
  y -= spline->y_grid.start;
  z -= spline->z_grid.start;
  double ux = x*spline->x_grid.delta_inv;
  double uy = y*spline->y_grid.delta_inv;
  double uz = z*spline->z_grid.delta_inv;
  double ipartx, iparty, ipartz, tx, ty, tz;
  tx = modf (ux, &ipartx);  int ix = (int) ipartx;  
  ty = modf (uy, &iparty);  int iy = (int) iparty; 
  tz = modf (uz, &ipartz);  int iz = (int) ipartz; 
  
  double tpx[4], tpy[4], tpz[4], a[4], b[4], c[4], da[4], db[4], dc[4];
  complex_double cP[16], bcP[4], dbcP[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  tpy[0] = ty*ty*ty;  tpy[1] = ty*ty;  tpy[2] = ty;  tpy[3] = 1.0;
  tpz[0] = tz*tz*tz;  tpz[1] = tz*tz;  tpz[2] = tz;  tpz[3] = 1.0;
  complex_double* restrict coefs = spline->coefs;

  a[0]   = (  Ad[ 0]*tpx[0] +   Ad[ 1]*tpx[1] +  Ad[ 2]*tpx[2] + Ad[ 3]*tpx[3]);
  a[1]   = (  Ad[ 4]*tpx[0] +   Ad[ 5]*tpx[1] +  Ad[ 6]*tpx[2] + Ad[ 7]*tpx[3]);
  a[2]   = (  Ad[ 8]*tpx[0] +   Ad[ 9]*tpx[1] +  Ad[10]*tpx[2] + Ad[11]*tpx[3]);
  a[3]   = (  Ad[12]*tpx[0] +   Ad[13]*tpx[1] +  Ad[14]*tpx[2] + Ad[15]*tpx[3]);
  da[0]  = ( dAd[ 1]*tpx[1] +  dAd[ 2]*tpx[2] + dAd[ 3]*tpx[3]);
  da[1]  = ( dAd[ 5]*tpx[1] +  dAd[ 6]*tpx[2] + dAd[ 7]*tpx[3]);
  da[2]  = ( dAd[ 9]*tpx[1] +  dAd[10]*tpx[2] + dAd[11]*tpx[3]);
  da[3]  = ( dAd[13]*tpx[1] +  dAd[14]*tpx[2] + dAd[15]*tpx[3]);

  b[0]  = ( Ad[ 0]*tpy[0] + Ad[ 1]*tpy[1] + Ad[ 2]*tpy[2] + Ad[ 3]*tpy[3]);
  b[1]  = ( Ad[ 4]*tpy[0] + Ad[ 5]*tpy[1] + Ad[ 6]*tpy[2] + Ad[ 7]*tpy[3]);
  b[2]  = ( Ad[ 8]*tpy[0] + Ad[ 9]*tpy[1] + Ad[10]*tpy[2] + Ad[11]*tpy[3]);
  b[3]  = ( Ad[12]*tpy[0] + Ad[13]*tpy[1] + Ad[14]*tpy[2] + Ad[15]*tpy[3]);
  db[0] = (dAd[ 1]*tpy[1] + dAd[ 2]*tpy[2] + dAd[ 3]*tpy[3]);
  db[1] = (dAd[ 5]*tpy[1] + dAd[ 6]*tpy[2] + dAd[ 7]*tpy[3]);
  db[2] = (dAd[ 9]*tpy[1] + dAd[10]*tpy[2] + dAd[11]*tpy[3]);
  db[3] = (dAd[13]*tpy[1] + dAd[14]*tpy[2] + dAd[15]*tpy[3]);

  c[0]  = ( Ad[ 0]*tpz[0] + Ad[ 1]*tpz[1] + Ad[ 2]*tpz[2] + Ad[ 3]*tpz[3]);
  c[1]  = ( Ad[ 4]*tpz[0] + Ad[ 5]*tpz[1] + Ad[ 6]*tpz[2] + Ad[ 7]*tpz[3]);
  c[2]  = ( Ad[ 8]*tpz[0] + Ad[ 9]*tpz[1] + Ad[10]*tpz[2] + Ad[11]*tpz[3]);
  c[3]  = ( Ad[12]*tpz[0] + Ad[13]*tpz[1] + Ad[14]*tpz[2] + Ad[15]*tpz[3]);
  dc[0] = (dAd[ 1]*tpz[1] + dAd[ 2]*tpz[2] + dAd[ 3]*tpz[3]);
  dc[1] = (dAd[ 5]*tpz[1] + dAd[ 6]*tpz[2] + dAd[ 7]*tpz[3]);
  dc[2] = (dAd[ 9]*tpz[1] + dAd[10]*tpz[2] + dAd[11]*tpz[3]);
  dc[3] = (dAd[13]*tpz[1] + dAd[14]*tpz[2] + dAd[15]*tpz[3]);
  
  int xs = spline->x_stride;
  int ys = spline->y_stride;
#define P(i,j,k) coefs[(ix+(i))*xs+(iy+(j))*ys+(iz+(k))]
  cP[ 0] = (P(0,0,0)*c[0]+P(0,0,1)*c[1]+P(0,0,2)*c[2]+P(0,0,3)*c[3]);
  cP[ 1] = (P(0,1,0)*c[0]+P(0,1,1)*c[1]+P(0,1,2)*c[2]+P(0,1,3)*c[3]);
  cP[ 2] = (P(0,2,0)*c[0]+P(0,2,1)*c[1]+P(0,2,2)*c[2]+P(0,2,3)*c[3]);
  cP[ 3] = (P(0,3,0)*c[0]+P(0,3,1)*c[1]+P(0,3,2)*c[2]+P(0,3,3)*c[3]);
  cP[ 4] = (P(1,0,0)*c[0]+P(1,0,1)*c[1]+P(1,0,2)*c[2]+P(1,0,3)*c[3]);
  cP[ 5] = (P(1,1,0)*c[0]+P(1,1,1)*c[1]+P(1,1,2)*c[2]+P(1,1,3)*c[3]);
  cP[ 6] = (P(1,2,0)*c[0]+P(1,2,1)*c[1]+P(1,2,2)*c[2]+P(1,2,3)*c[3]);
  cP[ 7] = (P(1,3,0)*c[0]+P(1,3,1)*c[1]+P(1,3,2)*c[2]+P(1,3,3)*c[3]);
  cP[ 8] = (P(2,0,0)*c[0]+P(2,0,1)*c[1]+P(2,0,2)*c[2]+P(2,0,3)*c[3]);
  cP[ 9] = (P(2,1,0)*c[0]+P(2,1,1)*c[1]+P(2,1,2)*c[2]+P(2,1,3)*c[3]);
  cP[10] = (P(2,2,0)*c[0]+P(2,2,1)*c[1]+P(2,2,2)*c[2]+P(2,2,3)*c[3]);
  cP[11] = (P(2,3,0)*c[0]+P(2,3,1)*c[1]+P(2,3,2)*c[2]+P(2,3,3)*c[3]);
  cP[12] = (P(3,0,0)*c[0]+P(3,0,1)*c[1]+P(3,0,2)*c[2]+P(3,0,3)*c[3]);
  cP[13] = (P(3,1,0)*c[0]+P(3,1,1)*c[1]+P(3,1,2)*c[2]+P(3,1,3)*c[3]);
  cP[14] = (P(3,2,0)*c[0]+P(3,2,1)*c[1]+P(3,2,2)*c[2]+P(3,2,3)*c[3]);
  cP[15] = (P(3,3,0)*c[0]+P(3,3,1)*c[1]+P(3,3,2)*c[2]+P(3,3,3)*c[3]);

  bcP[0] = ( b[0]*cP[ 0] + b[1]*cP[ 1] + b[2]*cP[ 2] + b[3]*cP[ 3]);
  bcP[1] = ( b[0]*cP[ 4] + b[1]*cP[ 5] + b[2]*cP[ 6] + b[3]*cP[ 7]);
  bcP[2] = ( b[0]*cP[ 8] + b[1]*cP[ 9] + b[2]*cP[10] + b[3]*cP[11]);
  bcP[3] = ( b[0]*cP[12] + b[1]*cP[13] + b[2]*cP[14] + b[3]*cP[15]);

  dbcP[0] = ( db[0]*cP[ 0] + db[1]*cP[ 1] + db[2]*cP[ 2] + db[3]*cP[ 3]);
  dbcP[1] = ( db[0]*cP[ 4] + db[1]*cP[ 5] + db[2]*cP[ 6] + db[3]*cP[ 7]);
  dbcP[2] = ( db[0]*cP[ 8] + db[1]*cP[ 9] + db[2]*cP[10] + db[3]*cP[11]);
  dbcP[3] = ( db[0]*cP[12] + db[1]*cP[13] + db[2]*cP[14] + db[3]*cP[15]);

  *val    = ( a[0]*bcP[0] +  a[1]*bcP[1] +  a[2]*bcP[2] +  a[3]*bcP[3]);
  grad[0] = spline->x_grid.delta_inv * 
    (da[0]*bcP[0] + da[1]*bcP[1] + da[2]*bcP[2] + da[3]*bcP[3]);
  grad[1] = spline->y_grid.delta_inv * 
    (a[0]*dbcP[0] + a[1]*dbcP[1] + a[2]*dbcP[2] + a[3]*dbcP[3]);
  grad[2] = spline->z_grid.delta_inv * 
    (a[0]*(b[0]*(P(0,0,0)*dc[0]+P(0,0,1)*dc[1]+P(0,0,2)*dc[2]+P(0,0,3)*dc[3])+
	   b[1]*(P(0,1,0)*dc[0]+P(0,1,1)*dc[1]+P(0,1,2)*dc[2]+P(0,1,3)*dc[3])+
	   b[2]*(P(0,2,0)*dc[0]+P(0,2,1)*dc[1]+P(0,2,2)*dc[2]+P(0,2,3)*dc[3])+
	   b[3]*(P(0,3,0)*dc[0]+P(0,3,1)*dc[1]+P(0,3,2)*dc[2]+P(0,3,3)*dc[3]))+
     a[1]*(b[0]*(P(1,0,0)*dc[0]+P(1,0,1)*dc[1]+P(1,0,2)*dc[2]+P(1,0,3)*dc[3])+
	   b[1]*(P(1,1,0)*dc[0]+P(1,1,1)*dc[1]+P(1,1,2)*dc[2]+P(1,1,3)*dc[3])+
	   b[2]*(P(1,2,0)*dc[0]+P(1,2,1)*dc[1]+P(1,2,2)*dc[2]+P(1,2,3)*dc[3])+
	   b[3]*(P(1,3,0)*dc[0]+P(1,3,1)*dc[1]+P(1,3,2)*dc[2]+P(1,3,3)*dc[3]))+
     a[2]*(b[0]*(P(2,0,0)*dc[0]+P(2,0,1)*dc[1]+P(2,0,2)*dc[2]+P(2,0,3)*dc[3])+
	   b[1]*(P(2,1,0)*dc[0]+P(2,1,1)*dc[1]+P(2,1,2)*dc[2]+P(2,1,3)*dc[3])+
	   b[2]*(P(2,2,0)*dc[0]+P(2,2,1)*dc[1]+P(2,2,2)*dc[2]+P(2,2,3)*dc[3])+
	   b[3]*(P(2,3,0)*dc[0]+P(2,3,1)*dc[1]+P(2,3,2)*dc[2]+P(2,3,3)*dc[3]))+
     a[3]*(b[0]*(P(3,0,0)*dc[0]+P(3,0,1)*dc[1]+P(3,0,2)*dc[2]+P(3,0,3)*dc[3])+
	   b[1]*(P(3,1,0)*dc[0]+P(3,1,1)*dc[1]+P(3,1,2)*dc[2]+P(3,1,3)*dc[3])+
	   b[2]*(P(3,2,0)*dc[0]+P(3,2,1)*dc[1]+P(3,2,2)*dc[2]+P(3,2,3)*dc[3])+
	   b[3]*(P(3,3,0)*dc[0]+P(3,3,1)*dc[1]+P(3,3,2)*dc[2]+P(3,3,3)*dc[3])));
#undef P

}



/* Value, gradient, and laplacian */
inline void
eval_UBspline_3d_z_vgl (UBspline_3d_z * restrict spline, 
			double x, double y, double z,
			complex_double* restrict val, 
			complex_double* restrict grad, 
			complex_double* restrict lapl)
{
  x -= spline->x_grid.start;
  y -= spline->y_grid.start;
  z -= spline->z_grid.start;
  double ux = x*spline->x_grid.delta_inv;
  double uy = y*spline->y_grid.delta_inv;
  double uz = z*spline->z_grid.delta_inv;
  double ipartx, iparty, ipartz, tx, ty, tz;
  tx = modf (ux, &ipartx);  int ix = (int) ipartx;  
  ty = modf (uy, &iparty);  int iy = (int) iparty; 
  tz = modf (uz, &ipartz);  int iz = (int) ipartz; 
  
  double tpx[4], tpy[4], tpz[4], a[4], b[4], c[4], da[4], db[4], dc[4], 
    d2a[4], d2b[4], d2c[4];
  complex_double cP[16], dcP[16], bcP[4], dbcP[4], d2bcP[4], bdcP[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  tpy[0] = ty*ty*ty;  tpy[1] = ty*ty;  tpy[2] = ty;  tpy[3] = 1.0;
  tpz[0] = tz*tz*tz;  tpz[1] = tz*tz;  tpz[2] = tz;  tpz[3] = 1.0;
  complex_double* restrict coefs = spline->coefs;

  a[0]   = (  Ad[ 0]*tpx[0] +   Ad[ 1]*tpx[1] +  Ad[ 2]*tpx[2] + Ad[ 3]*tpx[3]);
  a[1]   = (  Ad[ 4]*tpx[0] +   Ad[ 5]*tpx[1] +  Ad[ 6]*tpx[2] + Ad[ 7]*tpx[3]);
  a[2]   = (  Ad[ 8]*tpx[0] +   Ad[ 9]*tpx[1] +  Ad[10]*tpx[2] + Ad[11]*tpx[3]);
  a[3]   = (  Ad[12]*tpx[0] +   Ad[13]*tpx[1] +  Ad[14]*tpx[2] + Ad[15]*tpx[3]);
  da[0]  = ( dAd[ 1]*tpx[1] +  dAd[ 2]*tpx[2] + dAd[ 3]*tpx[3]);
  da[1]  = ( dAd[ 5]*tpx[1] +  dAd[ 6]*tpx[2] + dAd[ 7]*tpx[3]);
  da[2]  = ( dAd[ 9]*tpx[1] +  dAd[10]*tpx[2] + dAd[11]*tpx[3]);
  da[3]  = ( dAd[13]*tpx[1] +  dAd[14]*tpx[2] + dAd[15]*tpx[3]);
  d2a[0] = (d2Ad[ 2]*tpx[2] + d2Ad[ 3]*tpx[3]);
  d2a[1] = (d2Ad[ 6]*tpx[2] + d2Ad[ 7]*tpx[3]);
  d2a[2] = (d2Ad[10]*tpx[2] + d2Ad[11]*tpx[3]);
  d2a[3] = (d2Ad[14]*tpx[2] + d2Ad[15]*tpx[3]);

  b[0]  = ( Ad[ 0]*tpy[0] + Ad[ 1]*tpy[1] + Ad[ 2]*tpy[2] + Ad[ 3]*tpy[3]);
  b[1]  = ( Ad[ 4]*tpy[0] + Ad[ 5]*tpy[1] + Ad[ 6]*tpy[2] + Ad[ 7]*tpy[3]);
  b[2]  = ( Ad[ 8]*tpy[0] + Ad[ 9]*tpy[1] + Ad[10]*tpy[2] + Ad[11]*tpy[3]);
  b[3]  = ( Ad[12]*tpy[0] + Ad[13]*tpy[1] + Ad[14]*tpy[2] + Ad[15]*tpy[3]);
  db[0] = (dAd[ 1]*tpy[1] + dAd[ 2]*tpy[2] + dAd[ 3]*tpy[3]);
  db[1] = (dAd[ 5]*tpy[1] + dAd[ 6]*tpy[2] + dAd[ 7]*tpy[3]);
  db[2] = (dAd[ 9]*tpy[1] + dAd[10]*tpy[2] + dAd[11]*tpy[3]);
  db[3] = (dAd[13]*tpy[1] + dAd[14]*tpy[2] + dAd[15]*tpy[3]);
  d2b[0] = (d2Ad[ 2]*tpy[2] + d2Ad[ 3]*tpy[3]);
  d2b[1] = (d2Ad[ 6]*tpy[2] + d2Ad[ 7]*tpy[3]);
  d2b[2] = (d2Ad[10]*tpy[2] + d2Ad[11]*tpy[3]);
  d2b[3] = (d2Ad[14]*tpy[2] + d2Ad[15]*tpy[3]);

  c[0]  = ( Ad[ 0]*tpz[0] + Ad[ 1]*tpz[1] + Ad[ 2]*tpz[2] + Ad[ 3]*tpz[3]);
  c[1]  = ( Ad[ 4]*tpz[0] + Ad[ 5]*tpz[1] + Ad[ 6]*tpz[2] + Ad[ 7]*tpz[3]);
  c[2]  = ( Ad[ 8]*tpz[0] + Ad[ 9]*tpz[1] + Ad[10]*tpz[2] + Ad[11]*tpz[3]);
  c[3]  = ( Ad[12]*tpz[0] + Ad[13]*tpz[1] + Ad[14]*tpz[2] + Ad[15]*tpz[3]);
  dc[0] = (dAd[ 1]*tpz[1] + dAd[ 2]*tpz[2] + dAd[ 3]*tpz[3]);
  dc[1] = (dAd[ 5]*tpz[1] + dAd[ 6]*tpz[2] + dAd[ 7]*tpz[3]);
  dc[2] = (dAd[ 9]*tpz[1] + dAd[10]*tpz[2] + dAd[11]*tpz[3]);
  dc[3] = (dAd[13]*tpz[1] + dAd[14]*tpz[2] + dAd[15]*tpz[3]);
  d2c[0] = (d2Ad[ 2]*tpz[2] + d2Ad[ 3]*tpz[3]);
  d2c[1] = (d2Ad[ 6]*tpz[2] + d2Ad[ 7]*tpz[3]);
  d2c[2] = (d2Ad[10]*tpz[2] + d2Ad[11]*tpz[3]);
  d2c[3] = (d2Ad[14]*tpz[2] + d2Ad[15]*tpz[3]);
  
  int xs = spline->x_stride;
  int ys = spline->y_stride;
#define P(i,j,k) coefs[(ix+(i))*xs+(iy+(j))*ys+(iz+(k))]
  cP[ 0] = (P(0,0,0)*c[0]+P(0,0,1)*c[1]+P(0,0,2)*c[2]+P(0,0,3)*c[3]);
  cP[ 1] = (P(0,1,0)*c[0]+P(0,1,1)*c[1]+P(0,1,2)*c[2]+P(0,1,3)*c[3]);
  cP[ 2] = (P(0,2,0)*c[0]+P(0,2,1)*c[1]+P(0,2,2)*c[2]+P(0,2,3)*c[3]);
  cP[ 3] = (P(0,3,0)*c[0]+P(0,3,1)*c[1]+P(0,3,2)*c[2]+P(0,3,3)*c[3]);
  cP[ 4] = (P(1,0,0)*c[0]+P(1,0,1)*c[1]+P(1,0,2)*c[2]+P(1,0,3)*c[3]);
  cP[ 5] = (P(1,1,0)*c[0]+P(1,1,1)*c[1]+P(1,1,2)*c[2]+P(1,1,3)*c[3]);
  cP[ 6] = (P(1,2,0)*c[0]+P(1,2,1)*c[1]+P(1,2,2)*c[2]+P(1,2,3)*c[3]);
  cP[ 7] = (P(1,3,0)*c[0]+P(1,3,1)*c[1]+P(1,3,2)*c[2]+P(1,3,3)*c[3]);
  cP[ 8] = (P(2,0,0)*c[0]+P(2,0,1)*c[1]+P(2,0,2)*c[2]+P(2,0,3)*c[3]);
  cP[ 9] = (P(2,1,0)*c[0]+P(2,1,1)*c[1]+P(2,1,2)*c[2]+P(2,1,3)*c[3]);
  cP[10] = (P(2,2,0)*c[0]+P(2,2,1)*c[1]+P(2,2,2)*c[2]+P(2,2,3)*c[3]);
  cP[11] = (P(2,3,0)*c[0]+P(2,3,1)*c[1]+P(2,3,2)*c[2]+P(2,3,3)*c[3]);
  cP[12] = (P(3,0,0)*c[0]+P(3,0,1)*c[1]+P(3,0,2)*c[2]+P(3,0,3)*c[3]);
  cP[13] = (P(3,1,0)*c[0]+P(3,1,1)*c[1]+P(3,1,2)*c[2]+P(3,1,3)*c[3]);
  cP[14] = (P(3,2,0)*c[0]+P(3,2,1)*c[1]+P(3,2,2)*c[2]+P(3,2,3)*c[3]);
  cP[15] = (P(3,3,0)*c[0]+P(3,3,1)*c[1]+P(3,3,2)*c[2]+P(3,3,3)*c[3]);

  dcP[ 0] = (P(0,0,0)*dc[0]+P(0,0,1)*dc[1]+P(0,0,2)*dc[2]+P(0,0,3)*dc[3]);
  dcP[ 1] = (P(0,1,0)*dc[0]+P(0,1,1)*dc[1]+P(0,1,2)*dc[2]+P(0,1,3)*dc[3]);
  dcP[ 2] = (P(0,2,0)*dc[0]+P(0,2,1)*dc[1]+P(0,2,2)*dc[2]+P(0,2,3)*dc[3]);
  dcP[ 3] = (P(0,3,0)*dc[0]+P(0,3,1)*dc[1]+P(0,3,2)*dc[2]+P(0,3,3)*dc[3]);
  dcP[ 4] = (P(1,0,0)*dc[0]+P(1,0,1)*dc[1]+P(1,0,2)*dc[2]+P(1,0,3)*dc[3]);
  dcP[ 5] = (P(1,1,0)*dc[0]+P(1,1,1)*dc[1]+P(1,1,2)*dc[2]+P(1,1,3)*dc[3]);
  dcP[ 6] = (P(1,2,0)*dc[0]+P(1,2,1)*dc[1]+P(1,2,2)*dc[2]+P(1,2,3)*dc[3]);
  dcP[ 7] = (P(1,3,0)*dc[0]+P(1,3,1)*dc[1]+P(1,3,2)*dc[2]+P(1,3,3)*dc[3]);
  dcP[ 8] = (P(2,0,0)*dc[0]+P(2,0,1)*dc[1]+P(2,0,2)*dc[2]+P(2,0,3)*dc[3]);
  dcP[ 9] = (P(2,1,0)*dc[0]+P(2,1,1)*dc[1]+P(2,1,2)*dc[2]+P(2,1,3)*dc[3]);
  dcP[10] = (P(2,2,0)*dc[0]+P(2,2,1)*dc[1]+P(2,2,2)*dc[2]+P(2,2,3)*dc[3]);
  dcP[11] = (P(2,3,0)*dc[0]+P(2,3,1)*dc[1]+P(2,3,2)*dc[2]+P(2,3,3)*dc[3]);
  dcP[12] = (P(3,0,0)*dc[0]+P(3,0,1)*dc[1]+P(3,0,2)*dc[2]+P(3,0,3)*dc[3]);
  dcP[13] = (P(3,1,0)*dc[0]+P(3,1,1)*dc[1]+P(3,1,2)*dc[2]+P(3,1,3)*dc[3]);
  dcP[14] = (P(3,2,0)*dc[0]+P(3,2,1)*dc[1]+P(3,2,2)*dc[2]+P(3,2,3)*dc[3]);
  dcP[15] = (P(3,3,0)*dc[0]+P(3,3,1)*dc[1]+P(3,3,2)*dc[2]+P(3,3,3)*dc[3]);

  bcP[0] = ( b[0]*cP[ 0] + b[1]*cP[ 1] + b[2]*cP[ 2] + b[3]*cP[ 3]);
  bcP[1] = ( b[0]*cP[ 4] + b[1]*cP[ 5] + b[2]*cP[ 6] + b[3]*cP[ 7]);
  bcP[2] = ( b[0]*cP[ 8] + b[1]*cP[ 9] + b[2]*cP[10] + b[3]*cP[11]);
  bcP[3] = ( b[0]*cP[12] + b[1]*cP[13] + b[2]*cP[14] + b[3]*cP[15]);

  dbcP[0] = ( db[0]*cP[ 0] + db[1]*cP[ 1] + db[2]*cP[ 2] + db[3]*cP[ 3]);
  dbcP[1] = ( db[0]*cP[ 4] + db[1]*cP[ 5] + db[2]*cP[ 6] + db[3]*cP[ 7]);
  dbcP[2] = ( db[0]*cP[ 8] + db[1]*cP[ 9] + db[2]*cP[10] + db[3]*cP[11]);
  dbcP[3] = ( db[0]*cP[12] + db[1]*cP[13] + db[2]*cP[14] + db[3]*cP[15]);

  bdcP[0] = ( b[0]*dcP[ 0] + b[1]*dcP[ 1] + b[2]*dcP[ 2] + b[3]*dcP[ 3]);
  bdcP[1] = ( b[0]*dcP[ 4] + b[1]*dcP[ 5] + b[2]*dcP[ 6] + b[3]*dcP[ 7]);
  bdcP[2] = ( b[0]*dcP[ 8] + b[1]*dcP[ 9] + b[2]*dcP[10] + b[3]*dcP[11]);
  bdcP[3] = ( b[0]*dcP[12] + b[1]*dcP[13] + b[2]*dcP[14] + b[3]*dcP[15]);

  d2bcP[0] = ( d2b[0]*cP[ 0] + d2b[1]*cP[ 1] + d2b[2]*cP[ 2] + d2b[3]*cP[ 3]);
  d2bcP[1] = ( d2b[0]*cP[ 4] + d2b[1]*cP[ 5] + d2b[2]*cP[ 6] + d2b[3]*cP[ 7]);
  d2bcP[2] = ( d2b[0]*cP[ 8] + d2b[1]*cP[ 9] + d2b[2]*cP[10] + d2b[3]*cP[11]);
  d2bcP[3] = ( d2b[0]*cP[12] + d2b[1]*cP[13] + d2b[2]*cP[14] + d2b[3]*cP[15]);


  *val    = 
    ( a[0]*bcP[0] +  a[1]*bcP[1] +  a[2]*bcP[2] +  a[3]*bcP[3]);

  grad[0] = spline->x_grid.delta_inv *
    (da[0]*bcP[0] + da[1]*bcP[1] + da[2]*bcP[2] + da[3]*bcP[3]);
  grad[1] = spline->y_grid.delta_inv * 
    (a[0]*dbcP[0] + a[1]*dbcP[1] + a[2]*dbcP[2] + a[3]*dbcP[3]);
  grad[2] = spline->z_grid.delta_inv * 
    (a[0]*bdcP[0] + a[1]*bdcP[1] + a[2]*bdcP[2] + a[3]*bdcP[3]);

  *lapl = 
    spline->x_grid.delta_inv * spline->x_grid.delta_inv * 
    (d2a[0]*bcP[0] + d2a[1]*bcP[1] + d2a[2]*bcP[2] + d2a[3]*bcP[3])
    
    + spline->y_grid.delta_inv * spline->y_grid.delta_inv * 
    (a[0]*d2bcP[0] + a[1]*d2bcP[1] + a[2]*d2bcP[2] + a[3]*d2bcP[3]) +
    
    + spline->z_grid.delta_inv * spline->z_grid.delta_inv * 
    (a[0]*(b[0]*(P(0,0,0)*d2c[0]+P(0,0,1)*d2c[1]+P(0,0,2)*d2c[2]+P(0,0,3)*d2c[3])+    
	   b[1]*(P(0,1,0)*d2c[0]+P(0,1,1)*d2c[1]+P(0,1,2)*d2c[2]+P(0,1,3)*d2c[3])+
	   b[2]*(P(0,2,0)*d2c[0]+P(0,2,1)*d2c[1]+P(0,2,2)*d2c[2]+P(0,2,3)*d2c[3])+
	   b[3]*(P(0,3,0)*d2c[0]+P(0,3,1)*d2c[1]+P(0,3,2)*d2c[2]+P(0,3,3)*d2c[3]))+
     a[1]*(b[0]*(P(1,0,0)*d2c[0]+P(1,0,1)*d2c[1]+P(1,0,2)*d2c[2]+P(1,0,3)*d2c[3])+
	   b[1]*(P(1,1,0)*d2c[0]+P(1,1,1)*d2c[1]+P(1,1,2)*d2c[2]+P(1,1,3)*d2c[3])+
	   b[2]*(P(1,2,0)*d2c[0]+P(1,2,1)*d2c[1]+P(1,2,2)*d2c[2]+P(1,2,3)*d2c[3])+
	   b[3]*(P(1,3,0)*d2c[0]+P(1,3,1)*d2c[1]+P(1,3,2)*d2c[2]+P(1,3,3)*d2c[3]))+
     a[2]*(b[0]*(P(2,0,0)*d2c[0]+P(2,0,1)*d2c[1]+P(2,0,2)*d2c[2]+P(2,0,3)*d2c[3])+
	   b[1]*(P(2,1,0)*d2c[0]+P(2,1,1)*d2c[1]+P(2,1,2)*d2c[2]+P(2,1,3)*d2c[3])+
	   b[2]*(P(2,2,0)*d2c[0]+P(2,2,1)*d2c[1]+P(2,2,2)*d2c[2]+P(2,2,3)*d2c[3])+
	   b[3]*(P(2,3,0)*d2c[0]+P(2,3,1)*d2c[1]+P(2,3,2)*d2c[2]+P(2,3,3)*d2c[3]))+
     a[3]*(b[0]*(P(3,0,0)*d2c[0]+P(3,0,1)*d2c[1]+P(3,0,2)*d2c[2]+P(3,0,3)*d2c[3])+
	   b[1]*(P(3,1,0)*d2c[0]+P(3,1,1)*d2c[1]+P(3,1,2)*d2c[2]+P(3,1,3)*d2c[3])+
	   b[2]*(P(3,2,0)*d2c[0]+P(3,2,1)*d2c[1]+P(3,2,2)*d2c[2]+P(3,2,3)*d2c[3])+
	   b[3]*(P(3,3,0)*d2c[0]+P(3,3,1)*d2c[1]+P(3,3,2)*d2c[2]+P(3,3,3)*d2c[3])));
#undef P

}





/* Value, gradient, and Hessian */
inline void
eval_UBspline_3d_z_vgh (UBspline_3d_z * restrict spline, 
			double x, double y, double z,
			complex_double* restrict val, 
			complex_double* restrict grad, 
			complex_double* restrict hess)
{
  x -= spline->x_grid.start;
  y -= spline->y_grid.start;  
  z -= spline->z_grid.start;
  double ux = x*spline->x_grid.delta_inv;
  double uy = y*spline->y_grid.delta_inv;
  double uz = z*spline->z_grid.delta_inv;
  ux = fmin (ux, (double)(spline->x_grid.num)-1.0e-5);
  uy = fmin (uy, (double)(spline->y_grid.num)-1.0e-5);
  uz = fmin (uz, (double)(spline->z_grid.num)-1.0e-5);
  double ipartx, iparty, ipartz, tx, ty, tz;
  tx = modf (ux, &ipartx);  int ix = (int) ipartx;
  ty = modf (uy, &iparty);  int iy = (int) iparty;
  tz = modf (uz, &ipartz);  int iz = (int) ipartz;

//   if ((ix >= spline->x_grid.num))    x = spline->x_grid.num;
//   if ((ix < 0))                      x = 0;                 
//   if ((iy >= spline->y_grid.num))    y = spline->y_grid.num;
//   if ((iy < 0))                      y = 0;                 
//   if ((iz >= spline->z_grid.num))    z = spline->z_grid.num;
//   if ((iz < 0))                      z = 0;                 

  double tpx[4], tpy[4], tpz[4], a[4], b[4], c[4], da[4], db[4], dc[4], 
    d2a[4], d2b[4], d2c[4];
  complex_double cP[16], dcP[16], d2cP[16], bcP[4], dbcP[4],
    d2bcP[4], dbdcP[4], bd2cP[4], bdcP[4];
  tpx[0] = tx*tx*tx;  tpx[1] = tx*tx;  tpx[2] = tx;  tpx[3] = 1.0;
  tpy[0] = ty*ty*ty;  tpy[1] = ty*ty;  tpy[2] = ty;  tpy[3] = 1.0;
  tpz[0] = tz*tz*tz;  tpz[1] = tz*tz;  tpz[2] = tz;  tpz[3] = 1.0;
  complex_double* restrict coefs = spline->coefs;

  a[0]   = (  Ad[ 0]*tpx[0] +   Ad[ 1]*tpx[1] +  Ad[ 2]*tpx[2] + Ad[ 3]*tpx[3]);
  a[1]   = (  Ad[ 4]*tpx[0] +   Ad[ 5]*tpx[1] +  Ad[ 6]*tpx[2] + Ad[ 7]*tpx[3]);
  a[2]   = (  Ad[ 8]*tpx[0] +   Ad[ 9]*tpx[1] +  Ad[10]*tpx[2] + Ad[11]*tpx[3]);
  a[3]   = (  Ad[12]*tpx[0] +   Ad[13]*tpx[1] +  Ad[14]*tpx[2] + Ad[15]*tpx[3]);
  da[0]  = ( dAd[ 1]*tpx[1] +  dAd[ 2]*tpx[2] + dAd[ 3]*tpx[3]);
  da[1]  = ( dAd[ 5]*tpx[1] +  dAd[ 6]*tpx[2] + dAd[ 7]*tpx[3]);
  da[2]  = ( dAd[ 9]*tpx[1] +  dAd[10]*tpx[2] + dAd[11]*tpx[3]);
  da[3]  = ( dAd[13]*tpx[1] +  dAd[14]*tpx[2] + dAd[15]*tpx[3]);
  d2a[0] = (d2Ad[ 2]*tpx[2] + d2Ad[ 3]*tpx[3]);
  d2a[1] = (d2Ad[ 6]*tpx[2] + d2Ad[ 7]*tpx[3]);
  d2a[2] = (d2Ad[10]*tpx[2] + d2Ad[11]*tpx[3]);
  d2a[3] = (d2Ad[14]*tpx[2] + d2Ad[15]*tpx[3]);

  b[0]  = ( Ad[ 0]*tpy[0] + Ad[ 1]*tpy[1] + Ad[ 2]*tpy[2] + Ad[ 3]*tpy[3]);
  b[1]  = ( Ad[ 4]*tpy[0] + Ad[ 5]*tpy[1] + Ad[ 6]*tpy[2] + Ad[ 7]*tpy[3]);
  b[2]  = ( Ad[ 8]*tpy[0] + Ad[ 9]*tpy[1] + Ad[10]*tpy[2] + Ad[11]*tpy[3]);
  b[3]  = ( Ad[12]*tpy[0] + Ad[13]*tpy[1] + Ad[14]*tpy[2] + Ad[15]*tpy[3]);
  db[0] = (dAd[ 1]*tpy[1] + dAd[ 2]*tpy[2] + dAd[ 3]*tpy[3]);
  db[1] = (dAd[ 5]*tpy[1] + dAd[ 6]*tpy[2] + dAd[ 7]*tpy[3]);
  db[2] = (dAd[ 9]*tpy[1] + dAd[10]*tpy[2] + dAd[11]*tpy[3]);
  db[3] = (dAd[13]*tpy[1] + dAd[14]*tpy[2] + dAd[15]*tpy[3]);
  d2b[0] = (d2Ad[ 2]*tpy[2] + d2Ad[ 3]*tpy[3]);
  d2b[1] = (d2Ad[ 6]*tpy[2] + d2Ad[ 7]*tpy[3]);
  d2b[2] = (d2Ad[10]*tpy[2] + d2Ad[11]*tpy[3]);
  d2b[3] = (d2Ad[14]*tpy[2] + d2Ad[15]*tpy[3]);

  c[0]  = ( Ad[ 0]*tpz[0] + Ad[ 1]*tpz[1] + Ad[ 2]*tpz[2] + Ad[ 3]*tpz[3]);
  c[1]  = ( Ad[ 4]*tpz[0] + Ad[ 5]*tpz[1] + Ad[ 6]*tpz[2] + Ad[ 7]*tpz[3]);
  c[2]  = ( Ad[ 8]*tpz[0] + Ad[ 9]*tpz[1] + Ad[10]*tpz[2] + Ad[11]*tpz[3]);
  c[3]  = ( Ad[12]*tpz[0] + Ad[13]*tpz[1] + Ad[14]*tpz[2] + Ad[15]*tpz[3]);
  dc[0] = (dAd[ 1]*tpz[1] + dAd[ 2]*tpz[2] + dAd[ 3]*tpz[3]);
  dc[1] = (dAd[ 5]*tpz[1] + dAd[ 6]*tpz[2] + dAd[ 7]*tpz[3]);
  dc[2] = (dAd[ 9]*tpz[1] + dAd[10]*tpz[2] + dAd[11]*tpz[3]);
  dc[3] = (dAd[13]*tpz[1] + dAd[14]*tpz[2] + dAd[15]*tpz[3]);
  d2c[0] = (d2Ad[ 2]*tpz[2] + d2Ad[ 3]*tpz[3]);
  d2c[1] = (d2Ad[ 6]*tpz[2] + d2Ad[ 7]*tpz[3]);
  d2c[2] = (d2Ad[10]*tpz[2] + d2Ad[11]*tpz[3]);
  d2c[3] = (d2Ad[14]*tpz[2] + d2Ad[15]*tpz[3]);
  
  int xs = spline->x_stride;
  int ys = spline->y_stride;
  int offmax = (ix+3)*xs + (iy+3)*ys + iz+3;
//   if (offmax > spline->coef_size) {
//      fprintf (stderr, "Outside bounds in spline evalutation.\n"
// 	      "offmax = %d  csize = %d\n", offmax, spline->csize);
//      fprintf (stderr, "ix=%d   iy=%d   iz=%d\n", ix,iy,iz);
//   }
#define P(i,j,k) coefs[(ix+(i))*xs+(iy+(j))*ys+(iz+(k))]
  cP[ 0] = (P(0,0,0)*c[0]+P(0,0,1)*c[1]+P(0,0,2)*c[2]+P(0,0,3)*c[3]);
  cP[ 1] = (P(0,1,0)*c[0]+P(0,1,1)*c[1]+P(0,1,2)*c[2]+P(0,1,3)*c[3]);
  cP[ 2] = (P(0,2,0)*c[0]+P(0,2,1)*c[1]+P(0,2,2)*c[2]+P(0,2,3)*c[3]);
  cP[ 3] = (P(0,3,0)*c[0]+P(0,3,1)*c[1]+P(0,3,2)*c[2]+P(0,3,3)*c[3]);
  cP[ 4] = (P(1,0,0)*c[0]+P(1,0,1)*c[1]+P(1,0,2)*c[2]+P(1,0,3)*c[3]);
  cP[ 5] = (P(1,1,0)*c[0]+P(1,1,1)*c[1]+P(1,1,2)*c[2]+P(1,1,3)*c[3]);
  cP[ 6] = (P(1,2,0)*c[0]+P(1,2,1)*c[1]+P(1,2,2)*c[2]+P(1,2,3)*c[3]);
  cP[ 7] = (P(1,3,0)*c[0]+P(1,3,1)*c[1]+P(1,3,2)*c[2]+P(1,3,3)*c[3]);
  cP[ 8] = (P(2,0,0)*c[0]+P(2,0,1)*c[1]+P(2,0,2)*c[2]+P(2,0,3)*c[3]);
  cP[ 9] = (P(2,1,0)*c[0]+P(2,1,1)*c[1]+P(2,1,2)*c[2]+P(2,1,3)*c[3]);
  cP[10] = (P(2,2,0)*c[0]+P(2,2,1)*c[1]+P(2,2,2)*c[2]+P(2,2,3)*c[3]);
  cP[11] = (P(2,3,0)*c[0]+P(2,3,1)*c[1]+P(2,3,2)*c[2]+P(2,3,3)*c[3]);
  cP[12] = (P(3,0,0)*c[0]+P(3,0,1)*c[1]+P(3,0,2)*c[2]+P(3,0,3)*c[3]);
  cP[13] = (P(3,1,0)*c[0]+P(3,1,1)*c[1]+P(3,1,2)*c[2]+P(3,1,3)*c[3]);
  cP[14] = (P(3,2,0)*c[0]+P(3,2,1)*c[1]+P(3,2,2)*c[2]+P(3,2,3)*c[3]);
  cP[15] = (P(3,3,0)*c[0]+P(3,3,1)*c[1]+P(3,3,2)*c[2]+P(3,3,3)*c[3]);

  dcP[ 0] = (P(0,0,0)*dc[0]+P(0,0,1)*dc[1]+P(0,0,2)*dc[2]+P(0,0,3)*dc[3]);
  dcP[ 1] = (P(0,1,0)*dc[0]+P(0,1,1)*dc[1]+P(0,1,2)*dc[2]+P(0,1,3)*dc[3]);
  dcP[ 2] = (P(0,2,0)*dc[0]+P(0,2,1)*dc[1]+P(0,2,2)*dc[2]+P(0,2,3)*dc[3]);
  dcP[ 3] = (P(0,3,0)*dc[0]+P(0,3,1)*dc[1]+P(0,3,2)*dc[2]+P(0,3,3)*dc[3]);
  dcP[ 4] = (P(1,0,0)*dc[0]+P(1,0,1)*dc[1]+P(1,0,2)*dc[2]+P(1,0,3)*dc[3]);
  dcP[ 5] = (P(1,1,0)*dc[0]+P(1,1,1)*dc[1]+P(1,1,2)*dc[2]+P(1,1,3)*dc[3]);
  dcP[ 6] = (P(1,2,0)*dc[0]+P(1,2,1)*dc[1]+P(1,2,2)*dc[2]+P(1,2,3)*dc[3]);
  dcP[ 7] = (P(1,3,0)*dc[0]+P(1,3,1)*dc[1]+P(1,3,2)*dc[2]+P(1,3,3)*dc[3]);
  dcP[ 8] = (P(2,0,0)*dc[0]+P(2,0,1)*dc[1]+P(2,0,2)*dc[2]+P(2,0,3)*dc[3]);
  dcP[ 9] = (P(2,1,0)*dc[0]+P(2,1,1)*dc[1]+P(2,1,2)*dc[2]+P(2,1,3)*dc[3]);
  dcP[10] = (P(2,2,0)*dc[0]+P(2,2,1)*dc[1]+P(2,2,2)*dc[2]+P(2,2,3)*dc[3]);
  dcP[11] = (P(2,3,0)*dc[0]+P(2,3,1)*dc[1]+P(2,3,2)*dc[2]+P(2,3,3)*dc[3]);
  dcP[12] = (P(3,0,0)*dc[0]+P(3,0,1)*dc[1]+P(3,0,2)*dc[2]+P(3,0,3)*dc[3]);
  dcP[13] = (P(3,1,0)*dc[0]+P(3,1,1)*dc[1]+P(3,1,2)*dc[2]+P(3,1,3)*dc[3]);
  dcP[14] = (P(3,2,0)*dc[0]+P(3,2,1)*dc[1]+P(3,2,2)*dc[2]+P(3,2,3)*dc[3]);
  dcP[15] = (P(3,3,0)*dc[0]+P(3,3,1)*dc[1]+P(3,3,2)*dc[2]+P(3,3,3)*dc[3]);

  d2cP[ 0] = (P(0,0,0)*d2c[0]+P(0,0,1)*d2c[1]+P(0,0,2)*d2c[2]+P(0,0,3)*d2c[3]);
  d2cP[ 1] = (P(0,1,0)*d2c[0]+P(0,1,1)*d2c[1]+P(0,1,2)*d2c[2]+P(0,1,3)*d2c[3]);
  d2cP[ 2] = (P(0,2,0)*d2c[0]+P(0,2,1)*d2c[1]+P(0,2,2)*d2c[2]+P(0,2,3)*d2c[3]);
  d2cP[ 3] = (P(0,3,0)*d2c[0]+P(0,3,1)*d2c[1]+P(0,3,2)*d2c[2]+P(0,3,3)*d2c[3]);
  d2cP[ 4] = (P(1,0,0)*d2c[0]+P(1,0,1)*d2c[1]+P(1,0,2)*d2c[2]+P(1,0,3)*d2c[3]);
  d2cP[ 5] = (P(1,1,0)*d2c[0]+P(1,1,1)*d2c[1]+P(1,1,2)*d2c[2]+P(1,1,3)*d2c[3]);
  d2cP[ 6] = (P(1,2,0)*d2c[0]+P(1,2,1)*d2c[1]+P(1,2,2)*d2c[2]+P(1,2,3)*d2c[3]);
  d2cP[ 7] = (P(1,3,0)*d2c[0]+P(1,3,1)*d2c[1]+P(1,3,2)*d2c[2]+P(1,3,3)*d2c[3]);
  d2cP[ 8] = (P(2,0,0)*d2c[0]+P(2,0,1)*d2c[1]+P(2,0,2)*d2c[2]+P(2,0,3)*d2c[3]);
  d2cP[ 9] = (P(2,1,0)*d2c[0]+P(2,1,1)*d2c[1]+P(2,1,2)*d2c[2]+P(2,1,3)*d2c[3]);
  d2cP[10] = (P(2,2,0)*d2c[0]+P(2,2,1)*d2c[1]+P(2,2,2)*d2c[2]+P(2,2,3)*d2c[3]);
  d2cP[11] = (P(2,3,0)*d2c[0]+P(2,3,1)*d2c[1]+P(2,3,2)*d2c[2]+P(2,3,3)*d2c[3]);
  d2cP[12] = (P(3,0,0)*d2c[0]+P(3,0,1)*d2c[1]+P(3,0,2)*d2c[2]+P(3,0,3)*d2c[3]);
  d2cP[13] = (P(3,1,0)*d2c[0]+P(3,1,1)*d2c[1]+P(3,1,2)*d2c[2]+P(3,1,3)*d2c[3]);
  d2cP[14] = (P(3,2,0)*d2c[0]+P(3,2,1)*d2c[1]+P(3,2,2)*d2c[2]+P(3,2,3)*d2c[3]);
  d2cP[15] = (P(3,3,0)*d2c[0]+P(3,3,1)*d2c[1]+P(3,3,2)*d2c[2]+P(3,3,3)*d2c[3]);

  bcP[0] = ( b[0]*cP[ 0] + b[1]*cP[ 1] + b[2]*cP[ 2] + b[3]*cP[ 3]);
  bcP[1] = ( b[0]*cP[ 4] + b[1]*cP[ 5] + b[2]*cP[ 6] + b[3]*cP[ 7]);
  bcP[2] = ( b[0]*cP[ 8] + b[1]*cP[ 9] + b[2]*cP[10] + b[3]*cP[11]);
  bcP[3] = ( b[0]*cP[12] + b[1]*cP[13] + b[2]*cP[14] + b[3]*cP[15]);

  dbcP[0] = ( db[0]*cP[ 0] + db[1]*cP[ 1] + db[2]*cP[ 2] + db[3]*cP[ 3]);
  dbcP[1] = ( db[0]*cP[ 4] + db[1]*cP[ 5] + db[2]*cP[ 6] + db[3]*cP[ 7]);
  dbcP[2] = ( db[0]*cP[ 8] + db[1]*cP[ 9] + db[2]*cP[10] + db[3]*cP[11]);
  dbcP[3] = ( db[0]*cP[12] + db[1]*cP[13] + db[2]*cP[14] + db[3]*cP[15]);

  bdcP[0] = ( b[0]*dcP[ 0] + b[1]*dcP[ 1] + b[2]*dcP[ 2] + b[3]*dcP[ 3]);
  bdcP[1] = ( b[0]*dcP[ 4] + b[1]*dcP[ 5] + b[2]*dcP[ 6] + b[3]*dcP[ 7]);
  bdcP[2] = ( b[0]*dcP[ 8] + b[1]*dcP[ 9] + b[2]*dcP[10] + b[3]*dcP[11]);
  bdcP[3] = ( b[0]*dcP[12] + b[1]*dcP[13] + b[2]*dcP[14] + b[3]*dcP[15]);

  bd2cP[0] = ( b[0]*d2cP[ 0] + b[1]*d2cP[ 1] + b[2]*d2cP[ 2] + b[3]*d2cP[ 3]);
  bd2cP[1] = ( b[0]*d2cP[ 4] + b[1]*d2cP[ 5] + b[2]*d2cP[ 6] + b[3]*d2cP[ 7]);
  bd2cP[2] = ( b[0]*d2cP[ 8] + b[1]*d2cP[ 9] + b[2]*d2cP[10] + b[3]*d2cP[11]);
  bd2cP[3] = ( b[0]*d2cP[12] + b[1]*d2cP[13] + b[2]*d2cP[14] + b[3]*d2cP[15]);

  d2bcP[0] = ( d2b[0]*cP[ 0] + d2b[1]*cP[ 1] + d2b[2]*cP[ 2] + d2b[3]*cP[ 3]);
  d2bcP[1] = ( d2b[0]*cP[ 4] + d2b[1]*cP[ 5] + d2b[2]*cP[ 6] + d2b[3]*cP[ 7]);
  d2bcP[2] = ( d2b[0]*cP[ 8] + d2b[1]*cP[ 9] + d2b[2]*cP[10] + d2b[3]*cP[11]);
  d2bcP[3] = ( d2b[0]*cP[12] + d2b[1]*cP[13] + d2b[2]*cP[14] + d2b[3]*cP[15]);
  
  dbdcP[0] = ( db[0]*dcP[ 0] + db[1]*dcP[ 1] + db[2]*dcP[ 2] + db[3]*dcP[ 3]);
  dbdcP[1] = ( db[0]*dcP[ 4] + db[1]*dcP[ 5] + db[2]*dcP[ 6] + db[3]*dcP[ 7]);
  dbdcP[2] = ( db[0]*dcP[ 8] + db[1]*dcP[ 9] + db[2]*dcP[10] + db[3]*dcP[11]);
  dbdcP[3] = ( db[0]*dcP[12] + db[1]*dcP[13] + db[2]*dcP[14] + db[3]*dcP[15]);

  *val = a[0]*bcP[0] + a[1]*bcP[1] + a[2]*bcP[2] + a[3]*bcP[3];
  grad[0] = spline->x_grid.delta_inv *
    (da[0] *bcP[0] + da[1]*bcP[1] + da[2]*bcP[2] + da[3]*bcP[3]);
  grad[1] = spline->y_grid.delta_inv *
    (a[0]*dbcP[0] + a[1]*dbcP[1] + a[2]*dbcP[2] + a[3]*dbcP[3]);
  grad[2] = spline->z_grid.delta_inv *
    (a[0]*bdcP[0] + a[1]*bdcP[1] + a[2]*bdcP[2] + a[3]*bdcP[3]);
  // d2x
  hess[0] = spline->x_grid.delta_inv * spline->x_grid.delta_inv *
    (d2a[0]*bcP[0] + d2a[1]*bcP[1] + d2a[2]*bcP[2] + d2a[3]*bcP[3]);
  // dx dy
  hess[1] = spline->x_grid.delta_inv * spline->y_grid.delta_inv *
    (da[0]*dbcP[0] + da[1]*dbcP[1] + da[2]*dbcP[2] + da[3]*dbcP[3]);
  hess[3] = hess[1];
  // dx dz;
  hess[2] = spline->x_grid.delta_inv * spline->z_grid.delta_inv *
    (da[0]*bdcP[0] + da[1]*bdcP[1] + da[2]*bdcP[2] + da[3]*bdcP[3]);
  hess[6] = hess[2];
  // d2y
  hess[4] = spline->y_grid.delta_inv * spline->y_grid.delta_inv *
    (a[0]*d2bcP[0] + a[1]*d2bcP[1] + a[2]*d2bcP[2] + a[3]*d2bcP[3]);
  // dy dz
  hess[5] = spline->y_grid.delta_inv * spline->z_grid.delta_inv *
    (a[0]*dbdcP[0] + a[1]*dbdcP[1] + a[2]*dbdcP[2] + a[3]*dbdcP[3]);
  hess[7] = hess[5];
  // d2z
  hess[8] = spline->z_grid.delta_inv * spline->z_grid.delta_inv *
    (a[0]*bd2cP[0] + a[1]*bd2cP[1] + a[2]*bd2cP[2] + a[3]*bd2cP[3]);
#undef P

}

#endif
