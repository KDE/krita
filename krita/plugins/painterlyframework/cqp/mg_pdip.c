/***************************************************************************
 *   Copyright (C) 2004 by Ewgenij Huebner                                  *
 *   huebner@uni-trier.de                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <gsl/gsl_math.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_linalg.h>

#include "gsl_cqp.h"
#include "initial_point.h"

static void
print_vectors(const gsl_vector*,const gsl_vector*,const gsl_vector*,const gsl_vector*);

static double
compute_gap_infeasible_points(const gsl_cqp_data * cqp, const gsl_vector *x, const gsl_vector *y, const gsl_vector *z);


static int
compute_residuals(const gsl_cqp_data * cqp, const gsl_vector *x, const gsl_vector *y, const gsl_vector *z, const gsl_vector *s,
									gsl_vector	*r);

static int
build_kkt_matrix(const gsl_cqp_data * cqp,  const gsl_vector *z, const gsl_vector *s, gsl_matrix *kkt_matrix);

double
step_length(const gsl_vector *s, const gsl_vector *z, const gsl_vector *delta_s, const gsl_vector *delta_z);

double
gsl_vector_max_norm(const gsl_vector *v);

double
gsl_matrix_max_norm(const gsl_matrix *M);

typedef struct
{

	size_t n;  /* dimension of the problem*/
	size_t me; /* number of the equality constraints*/
	size_t mi; /* number of the inequality constraints */
	gsl_vector *s; /* Slack vector for the constrain: Cx+s = d */

	gsl_matrix *kkt_matrix;
	gsl_vector *r; /* the vector of the right-hand side in the kkt-system: r=(r_Q,r_A,r_C) */

	double tau; /* a constant for the Mehrotra's heuristic */

	/* gondzio's parameters */
	size_t k_max; /*maximal number of corrections */
	double beta_min, beta_max; /* relatibe threhold values for outlier compl. products */
	double delta_alpha; /* the required increas of stepsize */
	double gamma; /* the minimum acceptable increase of stepsize */

	double data_norm; /* norm of the problem data */

    gsl_permutation * p;

    gsl_vector * delta;
    gsl_vector * delta_s;

    gsl_vector * r_zs;

    gsl_vector * delta_gondzio;
    gsl_vector * delta_s_gondzio;

} mg_pdip_state;

static int
mg_pdip_alloc (void *vstate, size_t n, size_t me, size_t mi)
{
	mg_pdip_state * state = (mg_pdip_state *) vstate;

	state->s = gsl_vector_alloc(mi);
	if(state->s == 0)
	{
		GSL_ERROR_VAL ("failed to initialize space for the slack vector s", GSL_ENOMEM, 0);
	}

	state->kkt_matrix = gsl_matrix_calloc(n+me+mi, n+me+mi);
	if(state->kkt_matrix == 0)
	{
		gsl_vector_free(state->s);
		GSL_ERROR_VAL ("failed to initialize space for the KKT-Matrix", GSL_ENOMEM, 0);
	}

	state->r = gsl_vector_alloc(state->kkt_matrix->size1);
	if(state->r == 0)
	{
		gsl_matrix_free(state->kkt_matrix);
		gsl_vector_free(state->s);
		GSL_ERROR_VAL ("failed to initialize space for right-hand side of the KKT-System", GSL_ENOMEM, 0);
	}


    state->p = gsl_permutation_alloc(state->kkt_matrix->size1);
    if(state->p == 0)
    {
        GSL_ERROR_VAL ("failed to initialize space for permutation vector", GSL_ENOMEM, 0);
    }

    state->delta = gsl_vector_alloc(state->kkt_matrix->size2);
    if(state->delta == 0)
    {
        gsl_permutation_free(state->p);
        GSL_ERROR_VAL ("failed to initialize space for predictor step", GSL_ENOMEM, 0);
    }
    state->delta_gondzio = gsl_vector_alloc(state->kkt_matrix->size2);
    if(state->delta_gondzio == 0)
    {
        gsl_vector_free(state->delta);
        gsl_permutation_free(state->p);
        GSL_ERROR_VAL ("failed to initialize space for predictor step", GSL_ENOMEM, 0);
    }

    state->delta_s_gondzio = gsl_vector_alloc(mi);
    if(state->delta_s_gondzio == 0)
    {
        gsl_vector_free(state->delta_gondzio);
        gsl_vector_free(state->delta);
        gsl_permutation_free(state->p);
        GSL_ERROR_VAL ("failed to initialize space for LM s", GSL_ENOMEM, 0);
    }

    state->delta_s = gsl_vector_alloc(mi);
    if(state->delta_s == 0)
    {
        gsl_vector_free(state->delta_s_gondzio);
        gsl_vector_free(state->delta_gondzio);
        gsl_vector_free(state->delta);
        gsl_permutation_free(state->p);
        GSL_ERROR_VAL ("failed to initialize space for LM s", GSL_ENOMEM, 0);
    }

    state->r_zs = gsl_vector_alloc(mi);
    if(state->r_zs == 0)
    {
        gsl_vector_free(state->delta_s);
        gsl_vector_free(state->delta_s_gondzio);
        gsl_vector_free(state->delta_gondzio);
        gsl_vector_free(state->delta);
        gsl_permutation_free(state->p);
        GSL_ERROR_VAL ("failed to initialize space for LM s", GSL_ENOMEM, 0);
    }

	return GSL_SUCCESS;
}

static int
mg_pdip_set (void *vstate, const gsl_cqp_data *cqp, gsl_vector *x, gsl_vector *y, gsl_vector *z,
		double *gap, double *residuals_norm, double *data_norm, double *infeasibility, double *infeasibility_min)
{
	int status;
	size_t i, j, debug=0;


	mg_pdip_state *state = (mg_pdip_state *) vstate;


	/* Initial points */
	status = pdip_initial_point_feasible_x(cqp->A, cqp->b, x);
	status = pdip_initial_point_feasible_s(cqp->C, cqp->d, x, state->s);
	status = pdip_initial_point_y(cqp->Q, cqp->q, cqp->A, x, y);
	status = pdip_initial_point_z(z);
	status = pdip_initial_point_strict_feasible(z, state->s);


	/* Dualtity gap */
	status = gsl_blas_ddot(z, state->s, gap);
	*gap /= (double) cqp->C->size1;


	status = build_kkt_matrix(cqp, z, state->s, state->kkt_matrix);



	state->tau = 3.0;

	/* for the convergence conditions */
	/* data_norm = ||Q,A,C,q,b,d||_{infinity} */
	*data_norm = gsl_matrix_max_norm(cqp->Q);

	*data_norm = GSL_MAX_DBL(*data_norm, gsl_matrix_max_norm(cqp->A));

	*data_norm = GSL_MAX_DBL(*data_norm, gsl_matrix_max_norm(cqp->C));

	*data_norm = GSL_MAX_DBL(*data_norm, gsl_vector_max_norm(cqp->q));

	*data_norm = GSL_MAX_DBL(*data_norm, gsl_vector_max_norm(cqp->b));

	*data_norm = GSL_MAX_DBL(*data_norm, gsl_vector_max_norm(cqp->d));

	state->data_norm = *data_norm;

	/* ||r||_{infinity}=||r_Q, r_A, r_C||_{infinity} */
	status = compute_residuals(cqp, x, y, z, state->s, state->r);
	*residuals_norm = gsl_vector_max_norm(state->r);



	*infeasibility = (*residuals_norm+compute_gap_infeasible_points(cqp, x, y, z))/(state->data_norm);
	*infeasibility_min = *infeasibility;

	/* Gondzio's paameters */
	state->k_max = 0;
	state->beta_min = 0.1;
	state->beta_max = 10.0;
	state->delta_alpha = 0.1;
	state->gamma = 0.1;

	if(debug)
	{
		printf("\nStart points:\n");
		print_vectors(x, y, z, state->s);
		printf("\nDuality gap: %e\n",*gap);

		printf("\nKKT-matrix:\n");
		for(i=0; i<state->kkt_matrix->size1; i++)
		{
			for(j=0; j<state->kkt_matrix->size2; j++)
				printf("%4.2f ",gsl_matrix_get(state->kkt_matrix,i,j));
			printf("\n");
		}
		printf("\n");
	}

  return GSL_SUCCESS;
}


static int
mg_pdip_iterate (void *vstate, const gsl_cqp_data *cqp, gsl_vector *x, gsl_vector *y, gsl_vector *z,
                  double *gap, double *residuals_norm, double *infeasibility, double *infeasibility_min)
{
	size_t i, j, debug=0;
	int status, signum;
	double sigma, alpha, alpha_gondzio;
	double tmp_v, tmp_vt, tmp_d;

	gsl_vector_view r_block, delta_block;

	mg_pdip_state *state = (mg_pdip_state *) vstate;

	gsl_permutation * p = state->p;
    gsl_vector * delta = state->delta;
    gsl_vector * delta_s = state->delta_s;
    gsl_vector * r_zs = state->r_zs;
    gsl_vector * delta_gondzio = state->delta_gondzio;
    gsl_vector * delta_s_gondzio = state->delta_s_gondzio;

	/* the right-hand side of the KKT-system: r = -(r_Q, r_A, r_C+Z^{-1}*r_zs) */
	/* the vecors of variables: delta = (delta_x, -delta_y, -delta_z) */
	/* delta_s for the slack variable s: delta_s = Z^{-1}(-r_zs - S*delta_z) */

	/********* Predictor Step ******************/
	/* in the predictor step: r_zs = ZSe */
	r_block = gsl_vector_subvector(state->r, cqp->Q->size1+cqp->A->size1, cqp->C->size1);
	status = gsl_blas_daxpy(1.0, state->s, &r_block.vector);
	gsl_blas_dscal(-1.0, state->r);

	/* solve the KKT-system: */
	/* evaluate the LU-decomposition of the KKT-matrix*/
	status = gsl_linalg_LU_decomp(state->kkt_matrix, p, &signum);
	/* find the predictor step */
	status = gsl_linalg_LU_solve(state->kkt_matrix, p, state->r, delta);

	for(i=0; i<delta_s->size; i++)
	{
		/* find delta_s for the slack variable s: delta_s = Z^{-1}(-r_zs - S*delta_z) */
		gsl_vector_set(delta_s, i, gsl_vector_get(state->s,i)*
				(gsl_vector_get(delta,cqp->Q->size2+cqp->A->size1+i)/gsl_vector_get(z,i)-1.0));

	}

	/* find the stepsize of the predictor step */
	delta_block = gsl_vector_subvector(delta, cqp->Q->size1+cqp->A->size1, cqp->C->size1);
	alpha = step_length(state->s, z, delta_s, &delta_block.vector);


	if(debug)
	{
		printf("\n *** Predictor Step ***\n");
		printf("\nthe right-hand side of the KKT-system:\n");
		for(i=0; i<state->r->size; i++)
			printf("r[%d]=%f ",i,gsl_vector_get(state->r,i));
		printf("\n");
		printf("\nsolution (delta_x,-delta_y,-delta_z):\n");
		for(i=0; i<delta->size; i++)
			printf("%6.3f ",gsl_vector_get(delta,i));
		printf("\n");
		printf("\ndelta_s\n");
		for(i=0; i<delta_s->size; i++)
			printf("%6.3f ",gsl_vector_get(delta_s,i));
		printf("\n");
		printf("the stepsize for the predictor step=%f\n",alpha);
	}



	/************  Evaluation of the centering parameter sigma ***************/
	/* sigma = (gap_aff/gap)^tau */
	sigma = 0.0;
	for(i=0; i<z->size; i++)
	{
		sigma += (gsl_vector_get(z,i) - alpha*gsl_vector_get(delta, cqp->Q->size2+cqp->A->size1+i))*
		         (gsl_vector_get(state->s,i) + alpha*gsl_vector_get(delta_s,i));
	}
	sigma /= (cqp->C->size1*(*gap));
	sigma = pow(sigma, state->tau);

	if(debug)
		printf("the centering parameter sigma =%f\n",sigma);


	/************  Corrector Step ******************/

	/* modify the right-hand side of the kkt-system in oder to find the Mehrotra's corrector step */

	r_block = gsl_vector_subvector(state->r, cqp->Q->size1+cqp->A->size1, cqp->C->size1);
	for(i=0; i<cqp->C->size1; i++)
	{
		tmp_d = -sigma*(*gap)-gsl_vector_get(delta, cqp->Q->size2+cqp->A->size1+i)*gsl_vector_get(delta_s, i);
		gsl_vector_set(r_zs, i, gsl_vector_get(z,i)*gsl_vector_get(state->s,i)+tmp_d);
		gsl_vector_set(&r_block.vector, i, gsl_vector_get(&r_block.vector, i)-tmp_d/gsl_vector_get(z,i));
	}

	/* find the corrector step */
	status = gsl_linalg_LU_solve(state->kkt_matrix, p, state->r, delta);


	for(i=0; i<delta_s->size; i++)
	{
		/* delta_s = Z^{-1}*(-r_zs-S*delta_z) */
		gsl_vector_set(delta_s, i, (-gsl_vector_get(r_zs,i)+gsl_vector_get(delta,cqp->Q->size2+cqp->A->size1+i)
				*gsl_vector_get(state->s, i))/gsl_vector_get(z,i));
	}

	/* evaluate the stepsize */
	delta_block = gsl_vector_subvector(delta, cqp->Q->size1+cqp->A->size1, cqp->C->size1);
	alpha = step_length(state->s, z, delta_s, &delta_block.vector);

	if(debug)
	{
		printf("\n *** Corrector Step ***\n");
		printf("the right-hand side:\n");
		for(i=0; i<state->r->size; i++)
			printf("r[%d]=%f ",i,gsl_vector_get(state->r,i));
		printf("\n");
		for(i=0; i<delta->size; i++)
			printf("%6.3f ",gsl_vector_get(delta,i));
		printf("\n");
		printf("\ndelta_s\n");
		for(i=0; i<delta_s->size; i++)
			printf("%6.3f ",gsl_vector_get(delta_s,i));
		printf("\n");
		printf("the stepsize for the corrector step=%f\n",alpha);
	}


	/* Gondzio's centrality correction steps */

	i=0;
	while(i<state->k_max)
	{

		alpha_gondzio = GSL_MIN_DBL(alpha+state->delta_alpha, 1.0);
		r_block = gsl_vector_subvector(state->r, cqp->Q->size1+cqp->A->size1, cqp->C->size1);

		for(j=0; j<z->size; j++)
		{
			tmp_v  = (gsl_vector_get(z,j)-alpha_gondzio*gsl_vector_get(delta,cqp->Q->size1+cqp->A->size1+j))*
					(gsl_vector_get(state->s,j)+alpha_gondzio*gsl_vector_get(delta_s,j));
			tmp_vt = GSL_MIN_DBL(GSL_MAX_DBL(tmp_v,state->beta_min*sigma*(*gap)),state->beta_max*sigma*(*gap));
			gsl_vector_set(r_zs, j, gsl_vector_get(r_zs,j)-(tmp_vt-tmp_v));
			gsl_vector_set(&r_block.vector, j, gsl_vector_get(&r_block.vector,j)+(tmp_vt-tmp_v)/gsl_vector_get(z,j));
		}

		status = gsl_linalg_LU_solve(state->kkt_matrix, p, state->r, delta_gondzio);
		for(j=0; j<delta_s->size; j++)
		{
			gsl_vector_set(delta_s_gondzio, j, (-gsl_vector_get(r_zs,j)+gsl_vector_get(delta_gondzio,cqp->Q->size2+cqp->A->size1+j)
					*gsl_vector_get(state->s, j))/gsl_vector_get(z,j));
		}

		/* evaluate the stepsize */
		delta_block = gsl_vector_subvector(delta_gondzio, cqp->Q->size1+cqp->A->size1, cqp->C->size1);
		alpha_gondzio = step_length(state->s, z, delta_s, &delta_block.vector);

		if(alpha_gondzio >= alpha+state->gamma*state->delta_alpha)
		{
			i++;
			alpha = alpha_gondzio;
			status = gsl_blas_dcopy(delta_gondzio, delta);
			status = gsl_blas_dcopy(delta_s_gondzio, delta_s);
		}
		else
			break;

	}

	/* heuristic for step length */
	alpha = GSL_MIN_DBL(0.995*alpha, 1.0);

	/* Update */
	/* x^k = x^k + alpha*delta_x */
	delta_block = gsl_vector_subvector(delta, 0, cqp->Q->size1);
	status = gsl_blas_daxpy(alpha, &delta_block.vector, x);

	/* y^k = y^k - alpha*(-delta_y) */
	delta_block = gsl_vector_subvector(delta, cqp->Q->size1, cqp->A->size1);
	status = gsl_blas_daxpy(-alpha, &delta_block.vector, y);

	/* z^k = z^k - alpha*(-delta_z) */
	delta_block = gsl_vector_subvector(delta, cqp->Q->size1+cqp->A->size1, cqp->C->size1);
	status = gsl_blas_daxpy(-alpha, &delta_block.vector, z);

	/* s^k = s^k + alpha*(delta_s) */
	status = gsl_blas_daxpy(alpha, delta_s, state->s);


	/* duality gap */
	status = gsl_blas_ddot(z, state->s, gap);
	*gap /= cqp->C->size1;

	/* data for the next iteration */
	status = compute_residuals(cqp, x, y, z, state->s, state->r);
	*residuals_norm = gsl_vector_max_norm(state->r);

	status = build_kkt_matrix(cqp, z, state->s, state->kkt_matrix);

	/* for the infeasibility test */
	*infeasibility = (*residuals_norm+compute_gap_infeasible_points(cqp, x, y, z))/(state->data_norm);
	*infeasibility_min = GSL_MIN_DBL(*infeasibility, *infeasibility_min);


	if(debug)
	{
		printf("current iteration points\n");
		print_vectors(x, y, z, state->s);

		printf("\nduality gap: %e\n",*gap);
	}

  return GSL_SUCCESS;
}

static void
mg_pdip_free (void *vstate)
{
	mg_pdip_state *state = (mg_pdip_state *) vstate;

	gsl_vector_free(state->s);
	gsl_matrix_free(state->kkt_matrix);
	gsl_vector_free(state->r);

    gsl_permutation_free(state->p);
    gsl_vector_free(state->delta);
    gsl_vector_free(state->delta_s_gondzio);
    gsl_vector_free(state->delta_gondzio);
    gsl_vector_free(state->delta_s);
    gsl_vector_free(state->r_zs);
}

/*
static int
mg_pdip_restart (void *vstate)
{
  mg_pdip_state *state = (mg_pdip_state *) vstate;

  return GSL_SUCCESS;
}
*/



static int
compute_residuals(const gsl_cqp_data * cqp, const gsl_vector *x, const gsl_vector *y, const gsl_vector *z, const gsl_vector *s,
									gsl_vector *r)
{

	int status;
	gsl_vector_view r_block;

	/*gsl_cqp_geconstraints * constraints = (gsl_cqp_geconstraints *) cqp->constraints;*/

	/* r_Q=Qx+q-A^ty-C^tz */
	r_block = gsl_vector_subvector(r, 0, cqp->Q->size1);
	status = gsl_blas_dcopy(cqp->q, &r_block.vector);
	status = gsl_blas_dsymv(CblasUpper, 1.0, cqp->Q, x, 1.0, &r_block.vector);
	/*status = gsl_blas_dgemv(CblasNoTrans, 1.0, cqp->Q, x, 1.0, r_Q);*/
	status = gsl_blas_dgemv(CblasTrans, -1.0, cqp->A, y, 1.0, &r_block.vector);
	status = gsl_blas_dgemv(CblasTrans, -1.0, cqp->C, z, 1.0, &r_block.vector);

	/* r_A=Ax-b */
	r_block = gsl_vector_subvector(r, cqp->Q->size1, cqp->A->size1);
	status = gsl_blas_dcopy(cqp->b, &r_block.vector);
	status = gsl_blas_dgemv(CblasNoTrans, 1.0, cqp->A, x, -1.0, &r_block.vector);

	/* r_C=Cx-s-d */
	r_block = gsl_vector_subvector(r, cqp->Q->size1+cqp->A->size1, cqp->C->size1);
	status = gsl_blas_dcopy(s, &r_block.vector);
	status = gsl_blas_daxpy(1.0, cqp->d, &r_block.vector);
	status = gsl_blas_dgemv(CblasNoTrans, 1.0, cqp->C, x, -1.0, &r_block.vector);

  return GSL_SUCCESS;
}

static int
build_kkt_matrix(const gsl_cqp_data * cqp,  const gsl_vector *z, const gsl_vector *s, gsl_matrix * kkt_matrix)
{
	size_t i;

	int status;

	gsl_matrix_view kkt_block;


	/*KKT - Matrix

              |Q A^t    C^t  |
  kkt_matrix =|A  0      0   |
              |C  0  -Z^{-1}S|

  */
	/* 1. Block */
	kkt_block = gsl_matrix_submatrix(kkt_matrix, 0, 0, cqp->Q->size1, cqp->Q->size2);
	status = gsl_matrix_memcpy(&kkt_block.matrix, cqp->Q);

	/* 2. Block */
	kkt_block = gsl_matrix_submatrix(kkt_matrix, 0, cqp->Q->size2, cqp->A->size2, cqp->A->size1);
	status = gsl_matrix_transpose_memcpy(&kkt_block.matrix, cqp->A);

	/* 3. Block */
	kkt_block = gsl_matrix_submatrix(kkt_matrix, 0, cqp->Q->size2+cqp->A->size1, cqp->C->size2, cqp->C->size1);
	status = gsl_matrix_transpose_memcpy(&kkt_block.matrix, cqp->C);

	/* 4. Block */
	kkt_block = gsl_matrix_submatrix(kkt_matrix, cqp->Q->size1, 0, cqp->A->size1, cqp->A->size2);
	status = gsl_matrix_memcpy(&kkt_block.matrix, cqp->A);

	/* 5. Block */
	kkt_block = gsl_matrix_submatrix(kkt_matrix, cqp->Q->size1+cqp->A->size1, 0, cqp->C->size1, cqp->C->size2);
	status = gsl_matrix_memcpy(&kkt_block.matrix, cqp->C);

	/* Null Block */
	kkt_block = gsl_matrix_submatrix(kkt_matrix, cqp->Q->size1, cqp->Q->size2, cqp->A->size1+cqp->C->size1, cqp->A->size1+cqp->C->size1);
	gsl_matrix_set_zero(&kkt_block.matrix);

	/* 6. Block */
	for(i=cqp->Q->size1+cqp->A->size1; i<kkt_matrix->size1; i++)
	{
		gsl_matrix_set(kkt_matrix, i,i, -gsl_vector_get(s, i-(cqp->Q->size1+cqp->A->size1))/
						gsl_vector_get(z, i-(cqp->Q->size1+cqp->A->size1)));
	}

	return GSL_SUCCESS;

}

double
step_length(const gsl_vector *s, const gsl_vector *z, const gsl_vector *delta_s, const gsl_vector *delta_z)
{
	double alpha = 1.0;
	size_t i;

	for(i=0; i<delta_s->size; i++)
	{
		if(gsl_vector_get(delta_z,i) > 0.0)
			alpha = GSL_MIN_DBL(alpha, gsl_vector_get(z,i)/gsl_vector_get(delta_z,i));

		if(gsl_vector_get(delta_s,i) < 0.0)
			alpha = GSL_MIN_DBL(alpha, -gsl_vector_get(s,i)/gsl_vector_get(delta_s,i));

	}

	return alpha;
}


static double
compute_gap_infeasible_points(const gsl_cqp_data *cqp, const gsl_vector *x, const gsl_vector *y, const gsl_vector *z)
{


	double g, tmp_d;
	int status;

	gsl_vector * tmp_v = gsl_vector_alloc(cqp->q->size);

	status = gsl_blas_dcopy(cqp->q, tmp_v);
	status = gsl_blas_dsymv(CblasUpper, 1.0, cqp->Q, x, 1.0, tmp_v);
	status = gsl_blas_ddot(x, tmp_v, &g);

	status = gsl_blas_ddot(cqp->b, y, &tmp_d);
	g -= tmp_d;

	status = gsl_blas_ddot(cqp->d, z, &tmp_d);
	g -= tmp_d;

	gsl_vector_free(tmp_v);

	return g;
}

double
gsl_matrix_max_norm(const gsl_matrix *M)
{
	size_t i,j;
	double max_norm = 0.0;

	for(i=0; i<M->size1; i++)
		for(j=0; j<M->size2; j++)
			max_norm = GSL_MAX_DBL(max_norm, fabs(gsl_matrix_get(M, i, j)));

	return max_norm;

}

double
gsl_vector_max_norm(const gsl_vector *v)
{
	size_t i;
	double max_norm = fabs(gsl_vector_get(v,0));

	for(i=1; i<v->size; i++)
		max_norm = GSL_MAX_DBL(max_norm, fabs(gsl_vector_get(v,i)));

	return max_norm;
}


static void
print_vectors(const gsl_vector * x, const gsl_vector * y, const gsl_vector * z, const gsl_vector * s)
{
	size_t i;

	printf("\nx[1 x %d]:\n",x->size);
	for(i=0; i<x->size; i++)
		printf("%f  ",gsl_vector_get(x,i));
	printf("\n");

	printf("\ny[1 x %d]: (LM zu Ax=b)\n",y->size);
	for(i=0; i<y->size; i++)
		printf("%f  ",gsl_vector_get(y,i));
	printf("\n");

	printf("\nz[1 x %d]: (LM zu Cx>=d)\n",z->size);
	for(i=0; i<z->size; i++)
		printf("%f  ",gsl_vector_get(z,i));
	printf("\n");

	printf("\ns[1 x %d]: (Slack zu Cx>=d)\n",s->size);
	for(i=0; i<s->size; i++)
		printf("%f  ",gsl_vector_get(s,i));
	printf("\n");

}



static const gsl_cqpminimizer_type mg_pdip_type = {
	"mg_pdip", /* name of the method: Mehrotra-Gondzio primal-dual interior point method*/
	sizeof (mg_pdip_state),
	&mg_pdip_alloc,
	&mg_pdip_set,
	&mg_pdip_iterate,
	/*  &mg_pdip_restart, */
	&mg_pdip_free
};

const gsl_cqpminimizer_type
	* gsl_cqpminimizer_mg_pdip = &mg_pdip_type;

