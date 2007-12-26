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
#include <gsl/gsl_errno.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>

#include "gsl_cqp.h"

gsl_cqpminimizer *
gsl_cqpminimizer_alloc (const gsl_cqpminimizer_type * T, const size_t n, const size_t me, const size_t mi)
{
  int status;

  gsl_cqpminimizer *minimizer = (gsl_cqpminimizer *) malloc (sizeof (gsl_cqpminimizer));

  if (minimizer == 0)
  {
    GSL_ERROR_VAL ("failed to allocate space for cqpminimizer", GSL_ENOMEM, 0);
  }

  minimizer->type = T;
  
  if(n <= 0)
  {
	  free (minimizer);
	  GSL_ERROR_VAL ("the problem dimension n must be greater than 0", GSL_ENOMEM, 0);  
  }
  
  if(me <= 0)
  {
	  free (minimizer);
	  GSL_ERROR_VAL ("the number of equality constrains me must be greater or equal to 1", GSL_ENOMEM, 0);  

  }

  if(mi <= 0)
  {
	  free (minimizer);
	  GSL_ERROR_VAL ("the number of inequality constrains me must be greater or equal to 1", GSL_ENOMEM, 0);  

  }
  
  minimizer->x = gsl_vector_alloc(n);
  if (minimizer->x == 0)
  {
    free (minimizer);
    GSL_ERROR_VAL ("failed to allocate space for x", GSL_ENOMEM, 0);
  }
  
  minimizer->y = gsl_vector_alloc(me);
  if(minimizer->y ==0)
	{
		  gsl_vector_free(minimizer->x);
		  free(minimizer);
		  GSL_ERROR_VAL ("failed to allocate space for the Lagrange multiplier y", GSL_ENOMEM, 0);
	  }
	  
	minimizer->z = gsl_vector_alloc(mi);
	if(minimizer->z ==0)
	{
		gsl_vector_free(minimizer->y);  
		gsl_vector_free(minimizer->x);
		free(minimizer);
		GSL_ERROR_VAL ("failed to allocate space for the Lagrange multiplier z", GSL_ENOMEM, 0);
	}
	  

  minimizer->state = malloc (T->size);

  if (minimizer->state == 0)
  {      
    gsl_vector_free (minimizer->x);
    free (minimizer);
    GSL_ERROR_VAL ("failed to allocate space for minimizer state", GSL_ENOMEM, 0);
  }

  status = (T->alloc) (minimizer->state, n, me, mi);

  if (status != GSL_SUCCESS)
  {
    free (minimizer->state);
    gsl_vector_free (minimizer->x);
    free (minimizer);
    GSL_ERROR_VAL ("failed to initialize minimizer state", GSL_ENOMEM, 0);
  }

  return minimizer;
}


int
gsl_cqpminimizer_set (gsl_cqpminimizer * minimizer, gsl_cqp_data * cqp)
{


  if (cqp->Q->size1 != cqp->Q->size2)
  {
		GSL_ERROR ("matrix Q is not square", GSL_EBADLEN);  	
  }

  if (cqp->q->size != cqp->Q->size1)
  {
  	GSL_ERROR ("dim(q)!= #rows(Q)", GSL_EBADLEN);
  }

  if (cqp->Q->size1 != minimizer->x->size)
  {
  	GSL_ERROR ("#columns(Q)!=dim(x)", GSL_EBADLEN);
  }
  
  if(cqp->A != 0 && cqp->b !=0)
  {
  	if(cqp->A->size2 != minimizer->x->size)
  	{
		GSL_ERROR ("#columns(A) != dim(x)", GSL_EBADLEN);
  	}

  	if(cqp->A->size1 != cqp->b->size)
  	{
		GSL_ERROR ("#rows(A) != dim(b)", GSL_EBADLEN);
  	}
	
	if(cqp->A->size1 != minimizer->y->size)
  	{
		GSL_ERROR ("#rows(A) != me", GSL_EBADLEN);
  	}
  }

  if(cqp->C->size2 != minimizer->x->size)
  {
		GSL_ERROR ("#columns(C) != dim(x)", GSL_EBADLEN);
  }

  if(cqp->C->size1 != cqp->d->size)
  {
		GSL_ERROR ("#rows(C) != dim(d)", GSL_EBADLEN);
  }

  if(cqp->C->size1 != minimizer->z->size)
  {
		GSL_ERROR ("#rows(C) != mi", GSL_EBADLEN);
  }


  minimizer->cqp = cqp;

  return (minimizer->type->set) (minimizer->state, minimizer->cqp, minimizer->x, minimizer->y, minimizer->z, &(minimizer->gap),
  &(minimizer->residuals_norm), &(minimizer->data_norm), &(minimizer->quantity_of_infeasibility), &(minimizer->quantity_of_infeasibility_min));
}

void
gsl_cqpminimizer_free (gsl_cqpminimizer * minimizer)
{
  (minimizer->type->free) (minimizer->state);
  gsl_vector_free (minimizer->x);
  gsl_vector_free (minimizer->y);
  gsl_vector_free (minimizer->z);
  free(minimizer);
}

int
gsl_cqpminimizer_iterate (gsl_cqpminimizer * minimizer)
{
  return (minimizer->type->iterate) (minimizer->state, minimizer->cqp, minimizer->x, minimizer->y, minimizer->z, &(minimizer->gap),
  &(minimizer->residuals_norm), &(minimizer->quantity_of_infeasibility), &(minimizer->quantity_of_infeasibility_min));
}

/*
int
gsl_cqpminimizer_restart (gsl_cqpminimizer * minimizer)
{
  return (minimizer->type->restart) (minimizer->state);
}
*/
const char *
gsl_cqpminimizer_name (const gsl_cqpminimizer * minimizer)
{
  return minimizer->type->name;
}


gsl_vector *
gsl_cqpminimizer_x (gsl_cqpminimizer * minimizer)
{
  return minimizer->x;
}

gsl_vector *
gsl_cqpminimizer_lm_eq (gsl_cqpminimizer * minimizer)
{
  return minimizer->y;
}

gsl_vector *
gsl_cqpminimizer_lm_ineq (gsl_cqpminimizer * minimizer)
{
  return minimizer->z;
}

double
gsl_cqpminimizer_f (gsl_cqpminimizer * minimizer)
{
	/* the value of the objective at the point x */
	/* f = 0.5*(x^t)Qx+(q^t)x */
	double f; 
	
	int status;
	
	gsl_vector * tmp = gsl_vector_alloc(minimizer->x->size);
	if(tmp == 0)
	{
		GSL_ERROR_VAL ("failed to initialize workspace", GSL_ENOMEM, 0);
	}
	
	status = gsl_blas_dcopy(minimizer->cqp->q, tmp);
	
	status = gsl_blas_dsymv(CblasUpper, 0.5, minimizer->cqp->Q, minimizer->x, 1.0, tmp);
	
	status = gsl_blas_ddot(minimizer->x, tmp, &f);
	
	gsl_vector_free(tmp);
	
	return f;
}

double
gsl_cqpminimizer_gap (gsl_cqpminimizer * minimizer)
{
	return minimizer->gap;
}

double
gsl_cqpminimizer_residuals_norm (gsl_cqpminimizer *minimizer)
{
	return minimizer->residuals_norm;
}


int
gsl_cqpminimizer_test_convergence(gsl_cqpminimizer * minimizer, double eps_gap, double eps_residuals)
{
	if(minimizer->gap <= eps_gap && minimizer->residuals_norm <= eps_residuals*minimizer->data_norm)
		return GSL_SUCCESS;
	else
		return GSL_CONTINUE;
		
}

int
gsl_cqp_minimizer_test_infeasibility(gsl_cqpminimizer * minimizer, double eps_infeasible)
{

	if(minimizer->quantity_of_infeasibility > eps_infeasible &&
		  minimizer->quantity_of_infeasibility > pow(10.0, 4) * minimizer->quantity_of_infeasibility_min)
		return GSL_SUCCESS;
	else
		return GSL_CONTINUE;  
}

