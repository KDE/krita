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

#include "mathematics.h"
#include <cmath>
#include <cstdio>

namespace maths {

double coth(double z)
{
    return 1.0 / tanh(z);
}

double acoth(double z)
{
    return 0.5*log((1.0 + 1.0/z) / (1.0 - 1.0/z));
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

}
