/*
 *  This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_RANDOM_GENERATOR_H_
#define _KIS_RANDOM_GENERATOR_H_

#include <kritaimage_export.h>

#include <QtGlobal>

/**
 * This is a class that return a pseudo-random number that will be constant for a given
 * pixel coordinate.
 * The rational is that filters that use random number (such as noises, or raindrops)
 * needs to always get the same random value at each run, or else the result will constantly
 * changes when used as an adjustment layer.
 */
class KRITAIMAGE_EXPORT KisRandomGenerator
{
public:
    /**
     * Creates a new instance of a random generator with the given seed.
     */
    KisRandomGenerator(quint64 seed);
    ~KisRandomGenerator();
    /**
     * @return the constant random value corresponding to a given pixel, the value is between 0
     *         and RAND_MAX
     */
    quint64 randomAt(qint64 x, qint64 y);
    /**
     * @return the constant random value corresponding to a given pixel, the value is between 0
     *         and 1.0
     */
    double doubleRandomAt(qint64 x, qint64 y);
private:
    struct Private;
    Private* const d;
};

#endif
