/*
 *  This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2008, 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2009 Matthew Woehlke <mw_triad@users.sourceforge.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_random_generator.h"

/* Mac OS X doesn't define a number of UINT* macros without this before stdlib.h */
#define __STDC_LIMIT_MACROS

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "rand_salt.h"

inline quint64 permuteWhole(quint64 n, quint64 a, quint64 b)
{
    return ((n * a) + b);
}

inline quint64 part(quint64 n1, quint64 n2, int p)
{
    int b = p * 8;
    int i = (n1 >> b) & 0xFF;
    int j = (n2 >> b) & 0xFF;
    return quint64(salt[i][j]) << b;
}

struct Q_DECL_HIDDEN KisRandomGenerator::Private {
    quint64 seed;
};

KisRandomGenerator::KisRandomGenerator(quint64 seed) : d(new Private)
{
    d->seed = seed;
}

KisRandomGenerator::~KisRandomGenerator()
{
    delete d;
}

quint64 KisRandomGenerator::randomAt(qint64 x, qint64 y)
{
    const quint64 kxa = 427140578808118991LL;
    const quint64 kya = 166552399647317237LL;
    const quint64 kxb = 48058817213113801LL;
    const quint64 kyb = 9206429469018994469LL;

    // Generate salts
    quint64 n1 = (quint64(x + 5) * kxa) * d->seed;
    quint64 n2 = (quint64(y + 7) * kya) + (d->seed * 1040097393733LL);
    n1 = permuteWhole(n1, 8759824322359LL, 13);
    n2 = permuteWhole(n2, 200560490131LL, 2707);
    n1 = (n1 >> 32) ^ (n1 << 32);
    n2 = (n2 >> 32) ^ (n2 << 32);
    n1 ^= x ^ (quint64(y ^ d->seed) * kyb);
    n2 ^= y ^ (quint64(x + 13)   * kxb);

    // Combine salts
    quint64 v = 0;
    for (int p = 0; p < 8; ++p)
        v |= part(n1, n2, p);
    return v;
}

double KisRandomGenerator::doubleRandomAt(qint64 x, qint64 y)
{
    return randomAt(x, y) / (double)UINT64_MAX;
}

