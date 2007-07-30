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

#include <cmath>
// #include <glpk.h>

#include <KDebug>

#include "mathematics.h"

namespace maths {

double coth(double z)
{
	return ( cosh(z) / sinh(z) );
}

double acoth(double z)
{
	return 0.5*log((1 + 1/z) / (1 - 1/z));
}

void mult(const int rows, const int cols, const float **M, const float *A, float *R)
{
	for (int i = 0; i < rows; i++) {
		R[i] = 0;
		for (int j = 0; i < cols; j++) {
			R[i] += M[i][j] * A[j];
		}
	}
}

double sigmoid(double value)
{
	//TODO return a sigmoid in [0, 1] here
	// TESTED ONLY WITH MOUSE!
	if (value == 0.5)
		return value + 0.3;
	else
		return value;
}

void computeKS(const int nrefs, const float *vREF, float *vKS)
{
	double R, Rb, a, b, K, S;
	for (int i = 0, j = 0; j < nrefs; i += 2, j += 1) {
		R = vREF[j];

		if (R > MATH_LIM_SUP)
			R = MATH_LIM_SUP;
		if (R < MATH_LIM_SUB)
			R = MATH_LIM_SUB;

		Rb = R - MATH_SUB_BLACK;

		a = 0.5 * ( R + ( Rb - R + 1) / Rb );
		b = sqrt( a*a - 1 );

		S = (1 / b) * 1 / coth( ( b*b - ( a - R ) * ( a - 1 ) ) / ( b * ( 1 - R ) ) );
		K = S * ( a - 1 );

		vKS[i+0] = K;
		vKS[i+1] = log(S);
	}
}

/*
void computeReflectance(const int nrefs, const float *vKS, float *vREF)
{
	double b, K, S, R;
	for (int i = 0, j = 0; j < nrefs; i += 2, j += 1) {
		K = vKS[i+0];
		S = exp(vKS[i+1]);

		b = sqrt( ( K / S ) * ( K / S + 2 ) );
		R = 1 / ( 1 + ( K / S ) + b * coth( b * S * MATH_THICKNESS ) );

		vREF[j] = R;
	}
}
*/

void computeReflectance(const int nrefs, const float *vKS, float *vREF)
{
	double K, S, q, R;
	for (int i = 0, j = 0; j < nrefs; i += 2, j += 1) {
		K = vKS[i+0];
		S = exp(vKS[i+1]);

		q = K / S;
		R = 1 + q - sqrt( q*q + 2*q );

		vREF[j] = R;
	}
}

/*
void simplex(const int rows, const int cols, const float **M, float *X, const float *B)
{
	glp_prob *lp;
	glp_smcp parm;

	glp_init_smcp(&parm);
	parm.msg_lev = GLP_MSG_OFF;

	lp = glp_create_prob();
	glp_set_prob_name(lp, "XYZ2REF");
	glp_set_obj_dir(lp, GLP_MIN);

	glp_add_rows(lp, rows);

	for (int i = 0; i < rows; i++) {
		glp_set_row_name(lp, i+1, QString("B"+QString::number(i+1)).toAscii().data());
		glp_set_row_bnds(lp, i+1, GLP_FX, B[i], B[i]);
	}

	glp_add_cols(lp, cols);

	for (int i = 0; i < cols; i++) {
		glp_set_col_name(lp, i+1, QString("R"+QString::number(i+1)).toAscii().data());
		glp_set_col_bnds(lp, i+1, GLP_DB, MATH_LIM_SUB, MATH_LIM_SUP);
		glp_set_obj_coef(lp, i+1, 1.0);
	}

	int ind[cols+1];
	for (int i = 0; i < cols; i++)
		ind[i+1] = i;

	for (int i = 0; i < rows; i++) {
// 		double row[cols+1];
// 		for (int j = 0; j < cols; j++) {
// 			row[j+1] = M[i][j];
// 		}
		glp_set_mat_row(lp, i+1, cols, ind, M[i]);
	}

	glp_simplex(lp, &parm);

	for (int i = 0; i < cols; i++)
		X[i] = glp_get_col_prim(lp, i+1);

	glp_delete_prob(lp);
}
*/

float convert2f (unsigned short value)
{
	return ((float)value)/MATH_NORMALIZATION;
}

unsigned short convert2i (float value)
{
	return (quint16)(value*MATH_NORMALIZATION);
}

}
