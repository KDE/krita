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
#include <cstdio>

extern "C" {
    #include <glpk.h>
}

#include "mathematics.h"

namespace maths {

const double MATH_THICKNESS = 1.0;
const double MATH_LIM_SUP = 1.0 - 3.90625e-3;
const double MATH_LIM_SUB = 0.0 + 3.90625e-3;
const double MATH_NORMALIZATION = 65535.0;

// double MATH_SUB_BLACK(double) { return 3.90625e-5; }
double MATH_SUB_BLACK(double R) { return 0.001*R; }

void correctReflectance(double &R)
{
    if (R > MATH_LIM_SUP)
        R = MATH_LIM_SUP;
    if (R < MATH_LIM_SUB)
        R = MATH_LIM_SUB;
}

double coth(double z)
{
    return 1.0 / tanh(z);
}

double acoth(double z)
{
    return 0.5*log((1.0 + 1.0/z) / (1.0 - 1.0/z));
}

void mult(const int rows, const int cols, double **M, const double *A, double *R)
{
    for (int i = 0; i < rows; i++) {
        R[i] = 0;
        for (int j = 0; j < cols; j++) {
            R[i] += M[i][j] * A[j];
        }
    }
}

double sigmoid(double v)
{
    return smoothstep(-0.05, 1.0, v);
}

double smoothstep(double a, double b, double v)
{
    const double b1 = log(1.0/0.00247262 - 1.0);
    const double b2 = log(1.0/0.99752737 - 1.0);

    if (a > b) {
        double tmp = a;
        a = b;
        b = tmp;
    }

    if (v < a)
        return a;
    if (v > b)
        return b;

    if (a == b)
        return v;

    double del = -a + b;
    double del_k = b1 - b2;
    double del_c = -a*b2 + b*b1;

    double k = del_k/del;
    double c = del_c/del;

    double z = 1.0 / ( 1.0 + exp( -k*v + c ) );

    return z;
}

double clamp(double min, double max, double v)
{
    if (min > max) {
        double tmp = min;
        min = max;
        max = tmp;
    }

    if (v < min)
        return min;
    if (v > max)
        return max;
    return v;
}

int sign(double v)
{
    if (v >= 0)
        return 1;
    else
        return -1;
}

void computeKS(const int nrefs, const double *vREF, float *vKS)
{
    double R, Rb, a, b, K, S;
    for (int i = 0, j = 0; j < nrefs; i += 2, j += 1) {
        R = vREF[j];

        correctReflectance(R);

        Rb = R - MATH_SUB_BLACK(R);

        a = 0.5 * ( R + ( Rb - R + 1.0) / Rb );
        b = sqrt( a*a - 1.0 );

        S = (1.0 / b) * acoth( ( b*b - ( a - R ) * ( a - 1.0 ) ) / ( b * ( 1.0 - R ) ) );
        K = S * ( a - 1.0 );

        vKS[i+0] = K;
        vKS[i+1] = log(S);
    }
}

// Do not use this anymore
void computeReflectance(const int nrefs, const float *vKS, double *vREF)
{
    double a, b, K, S, R;
    for (int i = 0, j = 0; j < nrefs; i += 2, j += 1) {
        K = vKS[i+0];
        S = exp(vKS[i+1]);

        a = (S + K) / S;
        b = sqrt( a*a - 1 );
        R = 1.0 / ( a + b * coth( b * S * MATH_THICKNESS ) );

//         correctReflectance(R);

        vREF[j] = R;
    }
}

/*
void computeReflectance(const int nrefs, const float *vKS, double *vREF)
{
    double K, S, q, R;
    for (int i = 0, j = 0; j < nrefs; i += 2, j += 1) {
        K = vKS[i+0];
        S = vKS[i+1];

        q = K / S;
        R = 1.0 + q - sqrt( q*q + 2.0*q );

        correctReflectance(R);

        vREF[j] = R;
    }
}
*/

void simplex(const int rows, const int cols, double **M, double *X, const double *B)
{
    char name[4];
    glp_prob *lp;
    glp_smcp parm;

    glp_init_smcp(&parm);
    parm.msg_lev = GLP_MSG_OFF;
    parm.meth = GLP_DUALP;

    lp = glp_create_prob();
    glp_set_prob_name(lp, "XYZ2REF");
    glp_set_obj_dir(lp, GLP_MAX);

    glp_add_rows(lp, rows);

    for (int i = 0; i < rows; i++) {
        sprintf(name, "B%d", i+1);
        glp_set_row_name(lp, i+1, name);
        glp_set_row_bnds(lp, i+1, GLP_FX, B[i], B[i]);
    }

    glp_add_cols(lp, cols);

    for (int i = 0; i < cols; i++) {
        sprintf(name, "R%d", i+1);
        glp_set_col_name(lp, i+1, name);
        glp_set_col_bnds(lp, i+1, GLP_DB, MATH_LIM_SUB, MATH_LIM_SUP);
        glp_set_obj_coef(lp, i+1, 1.0);
    }

    int ind[cols+1];
    for (int i = 1; i <= cols; i++)
        ind[i] = i;

    for (int i = 0; i < rows; i++) {
        double row[cols+1];
        for (int j = 0; j < cols; j++) {
            row[j+1] = M[i][j];
        }
        glp_set_mat_row(lp, i+1, cols, ind, row);
    }

    lpx_scale_prob(lp);
    glp_simplex(lp, &parm);

    for (int i = 0; i < cols; i++)
        X[i] = glp_get_col_prim(lp, i+1);

    glp_delete_prob(lp);
}


double convert2f (unsigned short value)
{
    return ((double)value)/MATH_NORMALIZATION;
}

unsigned short convert2i (double value)
{
    return (unsigned short)(value*MATH_NORMALIZATION);
}

}
