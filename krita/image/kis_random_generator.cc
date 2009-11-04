/*
 *  This file is part of the KDE project
 *
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2009 Matthew Woehlke <mw_triad@users.sourceforge.net>
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
#define __STDC_LIMIT_MACROS
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "rand_salt.h"

inline quint64 swapbitsinbytes(quint64 v)
{
    const quint64 h_mask_1 = 0xaaaaaaaaaaaaaaaaLL;
    const quint64 l_mask_1 = 0x5555555555555555LL;

    const quint64 h_mask_2 = 0xccccccccccccccccLL;
    const quint64 l_mask_2 = 0x3333333333333333LL;

    const quint64 h_mask_4 = 0xf0f0f0f0f0f0f0f0LL;
    const quint64 l_mask_4 = 0x0f0f0f0f0f0f0f0fLL;

    v = ((v & h_mask_1) >> 1) | ((v & l_mask_1) << 1);
    v = ((v & h_mask_2) >> 2) | ((v & l_mask_2) << 2);
    return ((v & h_mask_4) >> 4) | ((v & l_mask_4) << 4);
}

quint64 permuteWhole(quint64 n, quint64 a, quint64 b)
{
    return ((n * a) + b);
}

inline quint64 makePart(quint64 v, int idx, int small, int big)
{
    quint64 a = salt[small][big];
    quint64 b = 8 * idx;
    quint64 c = a << b;
    return v | c  ;
}

// This mask and coef generated doing some "random" computation and concatenation of numbers from random.org
#define mask 0x2D88F3F11F491819LL
#define coef 0x37DB094

quint64 myRandom1(quint64 n1, quint64 n2)
{
    quint64 v = 0;
    for (int i = 0; i < 8; ++ i) {
        int a = (n1 >> (8 * i)) ;
        int b = (n2 >> (8 * i)) ;
        v = makePart(v, i, a & 0xFF, b & 0xFF);
    }
    return v;
}

struct KisRandomGenerator::Private {
    quint64 rawSeed;
    quint64 maskedSeed;
};

KisRandomGenerator::KisRandomGenerator(quint64 seed) : d(new Private)
{
    d->rawSeed = seed;
    seed ^= swapbitsinbytes(seed);
    d->maskedSeed = seed ^ mask;
}

KisRandomGenerator::~KisRandomGenerator()
{
    delete d;
}

quint64 KisRandomGenerator::randomAt(qint64 x, qint64 y)
{
    const quint64 kxn = 427140578808118991LL;
    const quint64 kyn = 166552399647317237LL;
    const quint64 kxs = 48058817213113801LL;
    const quint64 kys = 9206429469018994469LL;
    quint64 n1 = (quint64(x + 5) * kxn) * d->rawSeed;
    quint64 n2 = (quint64(y + 7) * kyn) + d->rawSeed;
    n1 = permuteWhole(n1, 8759824322359LL, 13);
    n1 = (n1 >> 32) ^(n1 << 32);
    n2 = permuteWhole(n2, 200560490131LL, 2707);
    n2 = (n2 >> 32) ^(n2 << 32);
    n1 ^= x ^(y * 1040097393733LL) ^ kxs;
    n2 ^= y ^(x * 1040097393733LL) ^ kys;
    return myRandom1(n1, n2);
}

double KisRandomGenerator::doubleRandomAt(qint64 x, qint64 y)
{
    return randomAt(x, y) / (double)UINT64_MAX;
}

