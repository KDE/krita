/*
 *  Copyright (c) 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "random_gauss.h"
#include <cmath>

double RandomGauss::nextGaussian(double mean, double sigma)
{
    if (m_next) {
        m_next = false;
        return (m_gauss + mean) * sigma;
    }

    double v1, v2, s;
    do {
        v1 = 2.0 * drand48() - 1.0;
        v2 = 2.0 * drand48() - 1.0;
        s = v1 * v1 + v2 * v2;
    } while (s >= 1.0);

    double norm = sqrt(-2.0 * log(s) / s);
    m_gauss = v2 * norm;
    m_next = true;

    return (mean + (v1 * norm)) * sigma;
}
