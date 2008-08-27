/*
 *  This file is part of the KDE project
 *
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#include "kis_random_generator.h"
#include <stdlib.h>
#include <math.h>

struct KisRandomGenerator::Private {
    int seed;
};

KisRandomGenerator::KisRandomGenerator(int seed) : d(new Private)
{
    d->seed = seed;
}

KisRandomGenerator::~KisRandomGenerator()
{
    delete d;
}

int KisRandomGenerator::randomAt(int x, int y)
{
    return (int)(doubleRandomAt(x, y) * RAND_MAX);
}

double KisRandomGenerator::doubleRandomAt(int x, int y)
{
    // To plot it in Octave :
    // t = 1:1:100000;
    //plot (t, sort(0.5*(cos( cos(cos(t.*t.*t.*t))) + 1)));
    // This function has a near-gaussian distribtution
    int n = x + (y + 1) * d->seed;
    return 0.5*(cos(pow(n, 4)) + 1);
}

