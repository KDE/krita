/*
 * Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef _OPTIMIZATION_H_
#define _OPTIMIZATION_H_

#include <vector>

#include "gmm/gmm_iter.h"
#include "gmm/gmm_iter_solvers.h"
#include "gmm/gmm_matrix.h"
#include "gmm/gmm_precond_ilut.h"
#include "gmm/gmm_transposed.h"
#include "gmm/gmm_dense_lu.h"

#define OPTIMIZATION_DEBUG_SHOW_ITERATIONS

// Declaration
namespace Optimization
{
namespace Methods
{
/**
 * Use this class as a template parameter for the gaussNewton function if you want to use
 * the Gauss-Newton method which uses only the jacobian.
 */
template< class _TFunction_, typename _TType_ >
struct GaussNewton {
    typedef _TType_ Type;
    typedef _TFunction_ Function;
    static void computeM(const gmm::row_matrix< gmm::rsvector<_TType_> >& jacobian, const _TFunction_* f, std::vector<_TType_>& parameters, std::vector<_TType_>& values, gmm::row_matrix< gmm::rsvector<_TType_> > &M);
};
/**
 * Use this class as a template parameter for the gaussNewton function if you want to use
 * the Newton method which uses the Hessian.
 * You need to define gmm::row_matrix\< gmm::rsvector\<double\> \> hessian(const std::vector\<double\>& x, int n) const in _TFunction if you want to use that method.
 */
template< class _TFunction_, typename _TType_ >
struct Newton {
    typedef _TType_ Type;
    typedef _TFunction_ Function;
    static void computeM(const gmm::row_matrix< gmm::rsvector<_TType_> >& jacobian, const _TFunction_* f, std::vector<_TType_>& parameters, std::vector<_TType_>& values, gmm::row_matrix< gmm::rsvector<_TType_> > &M);
};
}
/**
 * This namespace contains function to call some of the classical optimization algorithm for
 * estimating the parameter p and minimizing sum( f_i(p)^2 ).
 *
 * The class _TFunction_ must at least contains the following functions:
 *  - std::vector\<double\> values(const std::vector\<double\>& x) const
 * which return the list of values of each function f_i
 *  - gmm::row_matrix\< gmm::rsvector\<double\> \> jacobian(const std::vector\<double\>& x) const
 * which return the jacobian
 * - int count() const which return the number of functions f_i
 *
 * And optionally when using the gaussNewton algorithm witht the Newton method:
 *  - gmm::row_matrix\< gmm::rsvector\<double\> \> hessian(const std::vector\<double\>& x, int n) const
 * which return the hessian
 *
 * Example:
 * @code
 *   // Optimization de deux fonctions dépendant de (a,b) :
 *  // a*a+ 2*b - 5
 *  // -a + b + 1
 *  class Function {
 *    public:
 *    std::vector\<double\> values(const std::vector\<double\>& x) const
 *    {
 *      std::vector\<double\> v(2);
 *      v[0] = x[0]*x[0] + 2.0 * x[1] - 5.0;
 *      v[1] = -x[0] + x[1] + 1.0;
 *      return v;
 *    }
 *    gmm::row_matrix\< gmm::rsvector\<double\> \> jacobian(const std::vector\<double\>& x) const
 *    {
 *      gmm::row_matrix\< gmm::wsvector\<double\> \> jt(2, 2);
 *      jt(0,0) = 2.0 * x[0];
 *      jt(0,1) = 2.0;
 *      jt(1,0) = -1.0;
 *      jt(1,1) = 1.0;
 *      gmm::row_matrix\< gmm::rsvector\<double\> \> jr(2,2);
 *      gmm::copy(jt,jr);
 *      return jr;
 *    }
 *    gmm::row_matrix\< gmm::rsvector\<double\> \> hessian(const std::vector\<double\>& x, int n) const
 *    {
 *      gmm::row_matrix\< gmm::wsvector\<double\> \> ht(2, 2);
 *      if(n == 0)
 *      {
 *        ht(0,0) = 2.0;
 *        ht(0,1) = 0.0;
 *        ht(1,0) = 0.0;
 *        ht(1,1) = 0.0;
 *      } else if(n == 1)
 *      {
 *        ht(0,0) = 0.0;
 *        ht(0,1) = 0.0;
 *        ht(1,0) = 0.0;
 *        ht(1,1) = 0.0;
 *      }
 *      gmm::row_matrix\< gmm::rsvector\<double\> \> hr(2,2);
 *      gmm::copy(ht,hr);
 *      return hr;
 *    }
 *    int count() const {
 *      return 2;
 *    }
 *  };
 *
 *    Function f;
 *    std::vector\<double\> v(2);
 *    v[0] = 10.0;
 *    v[1] = 10.0;
 *    std::cout \<\< "Gauss-Newton:" \<\< std::endl;
 *    std::cout \<\< "Remain = " \<\< Optimization::Algorithms::gaussNewton\< Optimization::Methods::GaussNewton\< Function, double\> \>(&f, v, 100, 1e-12) \<\< std::endl;
 *    std::cout \<\< v \<\< f.values(v) \<\< std::endl;
 *    v[0] = 10.0;
 *    v[1] = 10.0;
 *    std::cout \<\< "Newton:" \<\< std::endl;
 *    std::cout \<\< "Remain = " \<\< Optimization::Algorithms::gaussNewton\< Optimization::Methods::Newton\< Function, double\> \>(&f, v, 100, 1e-12) \<\< std::endl;
 *    std::cout \<\< v \<\< f.values(v) \<\< std::endl;
 *    v[0] = 1.0;
 *    v[1] = 1.0;
 *    std::cout \<\< "Gradient descent:" \<\< std::endl;
 *    std::cout \<\< "Remain = " \<\< Optimization::Algorithms::gradientDescent\< Function, double\>(&f, v, 100, 1e-12, 1e-2) \<\< std::endl;
 *    std::cout \<\< v \<\< f.values(v) \<\< std::endl;
 *    std::cout \<\< "Levenberg-Marquardt:" \<\< std::endl;
 *    v[0] = 10.0;
 *    v[0] = 10.0;
 *    std::cout \<\< "Remain = " \<\< Optimization::Algorithms::levenbergMarquardt\< Function, double\>(&f, v, 100, 1e-12, 0.01, 10.0) \<\< std::endl;
 *    std::cout \<\< v \<\< f.values(v) \<\< std::endl;
 * @endcode
 */
namespace Algorithms
{
/**
 * Perform an optimization following Gauss Newton algorithm
 * @param f functions
 * @param parameters initial guess of the parameters value
 * @param iter maximal number of iterations
 * @param epsilon if the remain is below epsilon the function return
 * @param gamma the coeficient apply to the derivative
 * @return the remain
 */
template< class _TFunction_, typename _TType_  >
_TType_ gradientDescent(_TFunction_* f, std::vector<_TType_>& parameters, int iter, _TType_ epsilon, _TType_ gamma);
/**
 * Perform an optimization following Gauss Newton algorithm
 * @param f functions
 * @param parameters initial guess of the parameters value
 * @param iter maximal number of iterations
 * @param epsilon if the remain is below epsilon the function return
 * @return the remain
 */
template< class _TMethod_ >
typename _TMethod_::Type gaussNewton(typename _TMethod_::Function* f, std::vector<typename _TMethod_::Type>& parameters, int iter, typename _TMethod_::Type epsilon);
/**
 * Perform an optimization following Levenberg Marquardt algorithm
 * @param f functions
 * @param parameters initial guess of the parameters value
 * @param iter maximal number of iterations
 * @param epsilon if the remain is below epsilon the function return
 * @param lambda0 the initial value of the damping parameter
 * @param nu adjustment coefficient of the damping parameter
 * @return the remain
 */
template< class _TFunction_, typename _TType_  >
_TType_ levenbergMarquardt(_TFunction_* f, std::vector<_TType_>& parameters, int iter, _TType_ epsilon, _TType_ lambda0, _TType_ nu);
}
}
// Implementation
namespace Optimization
{
namespace Details   // Private functions used by the algorithms
{
// Compute the remain
template<typename _TType_>
double computeRemain(const std::vector<_TType_>& values)
{
    _TType_ remain = 0.0;
    for (uint j = 0; j < values.size(); j++) {
        _TType_ v = values[j];
        remain += v * v;
    }
    return sqrt(remain / values.size());
}
// Solve (common to levenbergMarquardt and gaussNewton
template<typename _TType_>
std::vector<double> solve(const gmm::row_matrix< gmm::rsvector<_TType_> >& M, const gmm::row_matrix< gmm::rsvector<_TType_> >& jacobian, const std::vector<_TType_>& values, int p_count)
{
    gmm::dense_matrix<_TType_> MD(M.ncols(), M.nrows());
    gmm::copy(M, MD);
    // Solve the linear system
//       gmm::ilut_precond< gmm::row_matrix< gmm::rsvector<_TType_> > > P(M, 10, 1e-4);
//       gmm::iteration iter(1E-8);  // defines an iteration object, with a max residu of 1E-8
    std::vector<_TType_> X(p_count), B(p_count); // Unknown and left hand side.
    gmm::mult(gmm::transposed(jacobian), values, B);
    gmm::scale(B, -1.0);
    gmm::lu_solve(MD, X, B);
//       std::cout << " M = " << M << endl;
//       std::cout << " Values= " << values << endl;
//       std::cout << " B = " << B << endl;
//       iter.set_maxiter(250);
//       gmm::gmres(M, X, B, P, 50, iter);  // execute the GMRES algorithm
//       gmm::qmr(M, X, B, P, iter);  // execute the GMRES algorithm
    /*      std::cout << " B = " << B << endl;
          std::cout << " X = " << X << endl;*/
    std::vector<_TType_> tmp(p_count);
    gmm::mult(M, X, tmp);
    gmm::scale(tmp, -1.0);
    gmm::add(tmp, B);
//       std::cout << " M * X - B = " << X << endl;
    // Update
    return X;
}
}
namespace Methods
{
template< class _TFunction_, typename _TType_ >
void GaussNewton< _TFunction_, _TType_ >::computeM(const gmm::row_matrix< gmm::rsvector<_TType_> >& jacobian, const _TFunction_* f, std::vector<_TType_>& parameters, std::vector<_TType_>& values, gmm::row_matrix< gmm::rsvector<_TType_> > &M)
{
    gmm::mult(gmm::transposed(jacobian), jacobian, M);
}
template< class _TFunction_, typename _TType_ >
void Newton< _TFunction_, _TType_ >::computeM(const gmm::row_matrix< gmm::rsvector<_TType_> >& jacobian, const _TFunction_* f, std::vector<_TType_>& parameters, std::vector<_TType_>& values, gmm::row_matrix< gmm::rsvector<_TType_> > &M)
{
    gmm::mult(gmm::transposed(jacobian), jacobian, M);
    // Compute the sum hessian
    for (int i = 0; i < f->count(); i++) {
        gmm::row_matrix< gmm::rsvector< _TType_ > > hessian = f->hessian(parameters, i);
        gmm::scale(hessian, values[0]);
        gmm::add(hessian, M);
    }
}
}
namespace Algorithms
{
// Gradient descent
template< class _TFunction_, typename _TType_  >
_TType_ gradientDescent(_TFunction_* f, std::vector<_TType_>& parameters, int iter, _TType_ epsilon, _TType_ gamma)
{
    int p_count = parameters.size();
    int f_count = f->count();
    for (int i = 0; i < iter; i++) {
        // Compute the values
        std::vector<_TType_> values = f->values(parameters);
        // Compute the remaining
        _TType_ remain = Details::computeRemain(values);
#ifdef OPTIMIZATION_DEBUG_SHOW_ITERATIONS
        std::cout << "Iteration " << i << ", error : " << remain << std::endl;
#endif
        if (remain < epsilon) {
            return remain;
        }
        // Compute the jacobian
        gmm::row_matrix< gmm::rsvector<_TType_> > jacobian = f->jacobian(parameters);
        //       std::cout << "jacobian = " << jacobian << " values " << values << std::endl;
        //       std::cout << "parameters = " << parameters << std::endl;
        // update
        for (int j = 0; j < p_count; j++) {
            double update = 0.0;
            for (int i = 0; i < f_count; i++) {
                update += values[i] * jacobian(i, j);
            }
            //         std::cout << " update = " << update << " j = " << j << endl;
            parameters[j] -=  update * gamma;
        }
#ifdef OPTIMIZATION_DEBUG_SHOW_ITERATIONS
        std::cout << "parameters = " << parameters << std::endl;
#endif
    }
    return Details::computeRemain(f->values(parameters));
}
// Gauss-Newton
template< class _TMethod_ >
typename _TMethod_::Type gaussNewton(typename _TMethod_::Function* f, std::vector<typename _TMethod_::Type>& parameters, int iter, typename _TMethod_::Type epsilon)
{
    int p_count = parameters.size();
    int f_count = f->count();
    for (int i = 0; i < iter; i++) {
        // Compute the values
        std::vector<typename _TMethod_::Type> values = f->values(parameters);
        // Compute the remaining
        typename _TMethod_::Type remain = Details::computeRemain(values);
#ifdef OPTIMIZATION_DEBUG_SHOW_ITERATIONS
        std::cout << "Iteration " << i << ", error : " << remain << std::endl;
#endif
        if (remain < epsilon) {
            return remain;
        }
        // Compute the jacobian
        gmm::row_matrix< gmm::rsvector<typename _TMethod_::Type> > jacobian = f->jacobian(parameters);
        //       std::cout << "Jacobian = " << jacobian << endl;
        //       std::cout << "Transposed Jacobian = " << gmm::transposed(jacobian) << endl;
        gmm::row_matrix< gmm::rsvector<typename _TMethod_::Type> > M(jacobian.ncols(), jacobian.ncols());
        _TMethod_::computeM(jacobian, f, parameters, values, M);
        std::vector<typename _TMethod_::Type> X = Details::solve(M, jacobian, values, p_count);
        gmm::add(X, parameters);
#ifdef OPTIMIZATION_DEBUG_SHOW_ITERATIONS
        std::cout << " Parameters " << parameters << endl;
#endif
    }
    return Details::computeRemain(f->values(parameters));
}
template< class _TFunction_, typename _TType_  >
_TType_ levenbergMarquardt(_TFunction_* f, std::vector<_TType_>& parameters, int iter, _TType_ epsilon, _TType_ lambda0, _TType_ nu)
{
    int p_count = parameters.size();
//       int f_count = f->count();
    _TType_ lambda = lambda0 / nu;
    _TType_ invnu = 1.0 / nu;
    if (nu < invnu) {
        double t = nu;
        nu = invnu;
        invnu = t;
    }
    // Compute the values
    std::vector<_TType_> values = f->values(parameters);
    // Compute the remaining
    _TType_ previousremain = Details::computeRemain(f->values(parameters));
    // Optimization
    for (int i = 0; i < iter; i++) {
        if (previousremain < epsilon) {
            return previousremain;
        }
        // Compute the jacobian
        gmm::row_matrix< gmm::rsvector<_TType_> > jacobian = f->jacobian(parameters);
        //       std::cout << "Jacobian = " << jacobian << endl;
        //       std::cout << "Transposed Jacobian = " << gmm::transposed(jacobian) << endl;
        gmm::row_matrix< gmm::rsvector<_TType_> > M(jacobian.ncols(), jacobian.ncols());
        gmm::mult(gmm::transposed(jacobian), jacobian, M);

        // Compute lambda * identity
        gmm::row_matrix< gmm::rsvector<_TType_> > M2(jacobian.ncols(), jacobian.ncols());
        for (uint k = 0; k < M2.ncols(); k++) M2(k, k) = 1.0;
        gmm::scale(M2, -lambda);
        // subtract it to M
        gmm::add(M, M2);
//         std::cout << M2 << std::endl;
        /*        std::cout << std::endl << "[[";
                for(uint m = 0; m < M2.nrows(); m++)
                {
                  for(uint n = 0; n < M2.ncols(); n++)
                  {
                    std::cout << M2(n,m) << " ";
                  }
                  std::cout << "];[";
                }
                std::cout << std::endl;
                std::cout << std::endl;*/
        std::vector<_TType_> X = Details::solve(M2, jacobian, values, p_count);
        // Compute the update
        std::vector<double> newparameters = parameters;
        gmm::add(X, newparameters);
        std::vector<_TType_> newvalues = f->values(newparameters);
        double newremain = Details::computeRemain(newvalues);
        // Check if the update is an improvement
#ifdef OPTIMIZATION_DEBUG_SHOW_ITERATIONS
        std::cout << "Iteration " << i << ", error : " << previousremain << ", new error " << newremain << ", lambda : " << lambda << std::endl;
#endif
        if (newremain < previousremain) {
            parameters = newparameters;
            values = newvalues;
            previousremain = newremain;
            lambda *= invnu;
//           lambda = lambda0;
        } else {
            lambda *= nu;
        }
        if (lambda == 0.0 || lambda >= 1.0e200) {
            lambda = lambda0;
//           return previousremain;
        }
#ifdef OPTIMIZATION_DEBUG_SHOW_ITERATIONS
        std::cout << " Parameters " << parameters << endl;
#endif
    }
    return Details::computeRemain(f->values(parameters));
}
}
}

#endif
