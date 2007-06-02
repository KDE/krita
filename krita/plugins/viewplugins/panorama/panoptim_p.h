/*
 * panoptim_p.h -- Part of Krita
 *
 * Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef _PANOPTIM_P_H_
#define _PANOPTIM_P_H_

#include "optimization.h"
#include "panoptim_functions.h"
// For each image there is four parameters to estimate : translation (2 parameters) + distortion (2 parameters)
template<class _TFunction_, int _TParamCount_>
class PanoptimFunction {
  public:
    PanoptimFunction(const lMatches & m, double xc, double yc, int width, int height) : m_matches(m), m_xc(xc), m_yc(yc), m_norm(4.0 / ( width * width + height * height ) ), m_epsilon(1e-3)
    {
      int indx[_TParamCount_];
      for(int i = 0; i < _TParamCount_; i++)
      {
        indx[i] = i;
      }
      for(lMatches::const_iterator it = m_matches.begin(); it != m_matches.end(); ++it)
      {
        m_functions.push_back(_TFunction_(indx, m_xc, m_yc, m_norm, it->ref->x(), it->ref->y(), it->match->x(), it->match->y()));
      }
    }
  public:
    std::vector<double> values(const std::vector<double>& parameters)
    {
      std::vector<double> v;
      // Compute the values
      for(typename std::vector<_TFunction_>::iterator it = m_functions.begin(); it != m_functions.end(); ++it)
      {
        double f1,f2;
        it->f(parameters, f1, f2);
        v.push_back(f1);
        v.push_back(f2);
//         kDebug() << f1 << " = f1 f2 = " << f2 << " " << it->m_i1 << " " << it->m_j1 << " " << it->m_i2 << " " << it->m_j2 << endl;
      }
      return v;
    }
    gmm::row_matrix< gmm::rsvector<double> > jacobian(const std::vector<double>& parameters)
    {
      gmm::row_matrix< gmm::wsvector<double> > jt(count(), parameters.size());
      
      // Compute the jacobian
      int pos = 0;
      for(typename std::vector<_TFunction_>::iterator it = m_functions.begin(); it != m_functions.end(); ++it)
      {
        it->jac(parameters, jt, pos);
        pos += 2;
      }
      // Copy the result to a read matrix
      gmm::row_matrix< gmm::rsvector<double> > jr(count(),parameters.size());
      gmm::copy(jt,jr);
      return jr;
    }
    inline int count()
    {
      return 2 * m_matches.size();
    }
  private:
    const lMatches& m_matches;
    std::vector<_TFunction_> m_functions;
    double m_xc, m_yc, m_norm;
    const double m_epsilon;
};

#endif
