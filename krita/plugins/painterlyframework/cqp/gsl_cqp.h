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

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

typedef struct
{
	/* objective function: 0.5*(x^t)Qx+(q^t)x */
	gsl_matrix * Q; 
	gsl_vector * q;
	
	/* constraints: Ax=b; Cx>=d */
	gsl_matrix * A;
	gsl_vector * b;
	gsl_matrix * C;
	gsl_vector * d;
}
gsl_cqp_data;

typedef struct
{
	const char *name;
	size_t size;
	int (*alloc) (void *state, size_t n, size_t me, size_t mi);
	int (*set) (void *state, const gsl_cqp_data *cqp, gsl_vector *x, gsl_vector *y, gsl_vector *z,
              double *gap, double *residuals_norm, double *data_norm, double *inf_barrier, double *inf_barrier_min);
	int (*iterate) (void *state, const gsl_cqp_data * cqp, gsl_vector *x, gsl_vector *y, gsl_vector *z,
                  double *gap, double *residuals_norm, double *inf_barrier, double *inf_barrier_min);
	
	/*  int (*restart) (void *state); */
	void (*free) (void *state);
}
gsl_cqpminimizer_type;

typedef struct
{
	const gsl_cqpminimizer_type * type;
	
	gsl_cqp_data * cqp;
	gsl_vector * x;
	/* Lagrange-multipliers */ 
	gsl_vector * y; /*corresponding to Ax=b */
	gsl_vector * z; /*corresponding to CX>=d */
	
	double gap;
	double residuals_norm;
	double data_norm;
	double quantity_of_infeasibility;
	double quantity_of_infeasibility_min;
	
	void *state;
}
gsl_cqpminimizer;

gsl_cqpminimizer *
gsl_cqpminimizer_alloc(const gsl_cqpminimizer_type *T, size_t n, size_t me, size_t mi);
/*
int
gsl_cqpminimizer_set (gsl_cqpminimizer * minimizer, gsl_cqp_problem * cqp);
*/
int
gsl_cqpminimizer_set (gsl_cqpminimizer * minimizer, gsl_cqp_data * cqp);

void
gsl_cqpminimizer_free(gsl_cqpminimizer *minimizer);

const char *
gsl_cqpminimizer_name (const gsl_cqpminimizer * minimizer);

int
gsl_cqpminimizer_iterate(gsl_cqpminimizer *minimizer);

/*
int
gsl_cqpminimizer_restart(gsl_cqpminimizer *minimizer);
*/

gsl_vector *
gsl_cqpminimizer_x (gsl_cqpminimizer * minimizer);

gsl_vector *
gsl_cqpminimizer_lm_eq (gsl_cqpminimizer * minimizer);

gsl_vector *
gsl_cqpminimizer_lm_ineq (gsl_cqpminimizer * minimizer);

double
gsl_cqpminimizer_f (gsl_cqpminimizer * minimizer);

double
gsl_cqpminimizer_gap (gsl_cqpminimizer *minimizer);

double
gsl_cqpminimizer_residuals_norm (gsl_cqpminimizer *minimizer);

int
gsl_cqpminimizer_test_convergence(gsl_cqpminimizer * minimizer, double eps_gap, double eps_residual);

int
gsl_cqp_minimizer_test_infeasibility(gsl_cqpminimizer * minimizer, double eps_infeasible);

double
gsl_cqpminimizer_minimum (gsl_cqpminimizer * minimizer);

GSL_VAR const gsl_cqpminimizer_type *gsl_cqpminimizer_mg_pdip;
/*GSL_VAR const gsl_cqpminimizer_type *gsl_cqpminimizer_pdip_mpc_eqc;*/
