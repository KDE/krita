/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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

#ifndef _KIS_PERSPECTVE_MATH_H_
#define _KIS_PERSPECTVE_MATH_H_

#include "kis_point.h"

class QRect;

class KisPerspectiveMath {
    private:
        KisPerspectiveMath() { }
        static double* computeMatrixTransfo( const KisPoint& topLeft1, const KisPoint& topRight1, const KisPoint& bottomLeft1, const KisPoint& bottomRight1 , const KisPoint& topLeft2, const KisPoint& topRight2, const KisPoint& bottomLeft2, const KisPoint& bottomRight2);
    public:
      static double* computeMatrixTransfoToPerspective(const KisPoint& topLeft, const KisPoint& topRight, const KisPoint& bottomLeft, const KisPoint& bottomRight, const QRect& r);
      static double* computeMatrixTransfoFromPerspective(const QRect& r, const KisPoint& topLeft, const KisPoint& topRight, const KisPoint& bottomLeft, const KisPoint& bottomRight);
      struct LineEquation {
            // y = a*x + b
      double a, b;
    };
    /// TODO: get ride of this in 2.0
    inline static KisPoint matProd(const double (&m)[3][3], const KisPoint& p)
    {
        double s = ( p.x() * m[2][0] + p.y() * m[2][1] + 1.0);
        s = (s == 0.) ? 1. : 1./s;
        return KisPoint( (p.x() * m[0][0] + p.y() * m[0][1] + m[0][2] ) * s,
                         (p.x() * m[1][0] + p.y() * m[1][1] + m[1][2] ) * s );
    }
    static inline LineEquation computeLineEquation(const KisPoint* p1, const KisPoint* p2)
    {
      LineEquation eq;
      double x1 = p1->x(); double x2 = p2->x();
      if( fabs(x1 - x2) < 0.000001 )
      {
        x1 += 0.0001; // Introduce a small perturbation
      }
      eq.a = (p2->y() - p1->y()) / (double)( x2 - x1 );
      eq.b = -eq.a * x1 + p1->y();
      return eq;
    }
    static inline KisPoint computeIntersection(const LineEquation& d1, const LineEquation& d2)
    {
      double a1 = d1.a; double a2 = d2.a;
      if( fabs(a1 - a2) < 0.000001 )
      {
        a1 += 0.0001; // Introduce a small perturbation
      }
      double x = (d1.b - d2.b) / (a2 - a1);
      return KisPoint(x, a2 * x + d2.b);
    }
};

#endif
