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

#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_math.h>
 
#include "initial_point.h"

int
pdip_initial_point_feasible_x(const gsl_matrix *A, const gsl_vector *b, gsl_vector *x)
{	
	int status, signum;
	
	/* x = A^t*(AA^t)^{-1}*b */
	
	gsl_matrix * AA_t;	
	gsl_vector * tmp;
	gsl_permutation * p;
	
	AA_t = gsl_matrix_calloc(A->size1, A->size1);
	if(AA_t == 0)
	{
	 	GSL_ERROR_VAL ("failed to initialize space for finding initial point", GSL_ENOMEM, 0);	
	}
	
	tmp = gsl_vector_alloc(A->size1);
	if(tmp == 0)
	{
	  gsl_matrix_free(AA_t);
	  GSL_ERROR_VAL ("failed to initialize space for finding initial point", GSL_ENOMEM, 0);
	}
	
	p = gsl_permutation_alloc(A->size1);
	if(p == 0)
	{
	  gsl_vector_free(tmp);
	  gsl_matrix_free(AA_t);
	  GSL_ERROR_VAL ("failed to initialize space for finding initial point", GSL_ENOMEM, 0);
	}
	
	
	/* status = gsl_blas_dsyrk(CblasUpper, CblasNoTrans, 1.0, cqp->A, 0.0, AA_t); */	
	status = gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1.0, A, A, 0.0, AA_t);
	
	
	/* (AA^t)tmp=b */
	status = gsl_linalg_LU_decomp(AA_t, p, &signum);
	status = gsl_linalg_LU_solve(AA_t, p, b, tmp);
	
	
	/*
	status = gsl_blas_dtrsv(CblasUpper, CblasNoTrans, CblasNonUnit, AA_t, tmp);  tmp=(AA^t)^{-1}tmp */	
	
	status = gsl_blas_dgemv(CblasTrans, 1.0, A, tmp, 0.0, x);
	
	gsl_matrix_free(AA_t);
	gsl_vector_free(tmp);
	gsl_permutation_free(p);   
	
	return GSL_SUCCESS;
	
}

int
pdip_initial_point_feasible_s(const gsl_matrix * C, const gsl_vector *d, const gsl_vector *x, gsl_vector *s)
{

  int status;

  /* s=Cx-d */

  status = gsl_blas_dcopy(d, s);

  status = gsl_blas_dgemv(CblasNoTrans, 1.0, C, x, -1.0, s);

  return GSL_SUCCESS;
}


int
pdip_initial_point_y(const gsl_matrix *Q, const gsl_vector *q, const gsl_matrix *A, const gsl_vector *x, gsl_vector *y)
{
  int status, signum;

  /* y=(AA^t)^{-1}(A(Qx+q)) */

  gsl_matrix * AA_t;	
	gsl_vector * tmp;
	gsl_permutation * p;
	
	AA_t = gsl_matrix_calloc(A->size1, A->size1);
	if(AA_t == 0)
	{
	 	GSL_ERROR_VAL ("failed to initialize space for finding initial point", GSL_ENOMEM, 0);	
	}
	
	tmp = gsl_vector_alloc(q->size);
	if(tmp == 0)
	{
	  gsl_matrix_free(AA_t);
	  GSL_ERROR_VAL ("failed to initialize space for finding initial point", GSL_ENOMEM, 0);
	}
	
	p = gsl_permutation_alloc(AA_t->size1);
	if(p == 0)
	{
	  gsl_vector_free(tmp);
	  gsl_matrix_free(AA_t);
	  GSL_ERROR_VAL ("failed to initialize space for finding initial point", GSL_ENOMEM, 0);
	}

  status = gsl_blas_dcopy(q, tmp);
  status = gsl_blas_dsymv(CblasUpper, 1.0, Q, x, 1.0, tmp);
  status = gsl_blas_dgemv(CblasNoTrans, 1.0, A, tmp, 0.0, y);

  /* status = gsl_blas_dsyrk(CblasUpper, CblasNoTrans, 1.0, cqp->A, 0.0, AA_t); */

  gsl_vector_free(tmp);
  tmp = gsl_vector_alloc(y->size);
  if(tmp == 0)
  {
  	gsl_permutation_free(p);
	  gsl_matrix_free(AA_t);
	  GSL_ERROR_VAL ("failed to initialize space for finding initial point", GSL_ENOMEM, 0);	
  }

  status = gsl_blas_dcopy(y, tmp);

  status = gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1.0, A, A, 0.0, AA_t);

  status = gsl_linalg_LU_decomp(AA_t, p, &signum);
	status = gsl_linalg_LU_solve(AA_t, p, tmp, y);


  /* status = gsl_blas_dtrsv(CblasUpper, CblasNoTrans, CblasNonUnit, AA_t, y); */

  gsl_matrix_free(AA_t);
  gsl_vector_free(tmp);
  gsl_permutation_free(p);

  return GSL_SUCCESS;
  
}

int
pdip_initial_point_z(gsl_vector *z)
{
  double delta_z = 0.1;

  gsl_vector_set_all(z, delta_z);

  return GSL_SUCCESS;
}

int
pdip_initial_point_strict_feasible(gsl_vector *x, gsl_vector *s)
{
	double delta_x, delta_s, xs=0.0, sum_x=0.0, sum_s=0.0, tmp, rg=1e-10;
	
	size_t i;
	
	int status;
	
	delta_x = GSL_MAX_DBL(-1.5*gsl_vector_min(x),0.0);
	delta_s = GSL_MAX_DBL(-1.5*gsl_vector_min(s),0.0);
  
	
	if(delta_x < rg && delta_s < rg)
	{
		status = gsl_blas_ddot(x, s, &tmp);
		
		if(tmp < rg) /* the initial point is optimal */
		return GSL_SUCCESS;
		
	}
		
	for(i=0; i<x->size; i++)
	{
		xs += (gsl_vector_get(x,i)+delta_x)*(gsl_vector_get(s,i)+delta_s);
		
		sum_x += gsl_vector_get(x,i);
		sum_s += gsl_vector_get(s,i);	
	}
	
	sum_x += delta_x*(x->size);
	sum_s += delta_s*(s->size);
	
	delta_x += 0.5*xs/sum_s;
	delta_s += 0.5*xs/sum_x;
	
	status = gsl_vector_add_constant(x, delta_x);
	status = gsl_vector_add_constant(s, delta_s);
	
	
	return GSL_SUCCESS;
}
