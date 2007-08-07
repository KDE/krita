/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef MATHEMATICS_H_
#define MATHEMATICS_H_

namespace maths {

const double MATH_THICKNESS = 1.0;
const double MATH_LIM_SUP = 0.99609375; // 1 - 3.90625*10^-3
const double MATH_LIM_SUB = 3.90625e-3;
const double MATH_SUB_BLACK = 3.90625e-4;
const double MATH_NORMALIZATION = 65535.0;

double coth(double z);

double acoth(double z);

void mult(const int rows, const int cols, double **M, const double *A, double *R);

double sigmoid(double value);

void computeKS(const int nrefs, const double *vREF, float *vKS);

void computeReflectance(const int nrefs, const float *vKS, double *vREF);

void simplex(const int rows, const int cols, double **M, double *X, const double *B);

double convert2f (unsigned short value);

unsigned short convert2i (double value);

}


#endif // MATHEMATICS_H_
