/*
 *  Copyright (c) 2009 Lukas Tvrdy <lukast.dev@gmail.com>
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

#ifndef RANDOM_GAUSS_H_
#define RANDOM_GAUSS_H_

#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)
#define srand48 srand
#endif

class RandomGauss
{

public:
    RandomGauss() {
        srand48(0);
        m_next = false;
    }

    RandomGauss(int seed) {
        srand48(seed);
        m_next = false;
    }
private:
    bool m_next;
    double m_gauss;

public:
    /**
     * Generates a random Gaussian value with the mean and sigma
     */
    double nextGaussian(double mean = 0.0, double sigma = 1.0);
};

#endif

