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

#ifndef MULTI_BSPLINE_EVAL_Z_H
#define MULTI_BSPLINE_EVAL_Z_H


/************************************************************/
/* 1D double-precision, complex evaulation functions        */
/************************************************************/
void
eval_multi_UBspline_1d_z (multi_UBspline_1d_z *spline,
			  double x,
			  complex_double* restrict vals);

void
eval_multi_UBspline_1d_z_vg (multi_UBspline_1d_z *spline,
			     double x,
			     complex_double* restrict vals,
			     complex_double* restrict grads);

void
eval_multi_UBspline_1d_z_vgl (multi_UBspline_1d_z *spline,
			      double x,
			      complex_double* restrict vals,
			      complex_double* restrict grads,
			      complex_double* restrict lapl);


void
eval_multi_UBspline_1d_z_vgh (multi_UBspline_1d_z *spline,
			      double x,
			      complex_double* restrict vals,
			      complex_double* restrict grads,
			      complex_double* restrict hess);


/************************************************************/
/* 2D double-precision, complex evaulation functions        */
/************************************************************/
void
eval_multi_UBspline_2d_z (multi_UBspline_2d_z *spline,
			  double x, double y,
			  complex_double* restrict vals);

void
eval_multi_UBspline_2d_z_vg (multi_UBspline_2d_z *spline,
			     double x, double y,
			     complex_double* restrict vals,
			     complex_double* restrict grads);

void
eval_multi_UBspline_2d_z_vgl (multi_UBspline_2d_z *spline,
			      double x, double y,
			      complex_double* restrict vals,
			      complex_double* restrict grads,
			      complex_double* restrict lapl);

void
eval_multi_UBspline_2d_z_vgh (multi_UBspline_2d_z *spline,
			      double x, double y,
			      complex_double* restrict vals,
			      complex_double* restrict grads,
			      complex_double* restrict hess);

/************************************************************/
/* 3D double-precision, complex evaulation functions        */
/************************************************************/
void
eval_multi_UBspline_3d_z (multi_UBspline_3d_z *spline,
			  double x, double y, double z,
			  complex_double* restrict vals);

void
eval_multi_UBspline_3d_z_vg (multi_UBspline_3d_z *spline,
			     double x, double y, double z,
			     complex_double* restrict vals,
			     complex_double* restrict grads);

void
eval_multi_UBspline_3d_z_vgl (multi_UBspline_3d_z *spline,
			      double x, double y, double z,
			      complex_double* restrict vals,
			      complex_double* restrict grads,
			      complex_double* restrict lapl);

void
eval_multi_UBspline_3d_z_vgh (multi_UBspline_3d_z *spline,
			      double x, double y, double z,
			      complex_double* restrict vals,
			      complex_double* restrict grads,
			      complex_double* restrict hess);


#endif
