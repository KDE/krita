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
