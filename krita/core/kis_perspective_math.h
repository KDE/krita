/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_PERSPECTVE_MATH_H_
#define _KIS_PERSPECTVE_MATH_H_

#include "kis_point.h"

class KisPerspectiveMath {
  private:
        KisPerspectiveMath() { }
  public:
    struct LineEquation {
            // y = a*x + b
      double a, b;
    };
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
