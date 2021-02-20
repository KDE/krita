/*
 * SPDX-FileCopyrightText: 1982 Donald H. House <x@unknown.com>
 * SPDX-FileCopyrightText: 1989 Robert Allen <x@unknown.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _GAUSS_H_
#define _GAUSS_H_

class gauss
{
public:
    /**
     * This function takes as parameters real valued mean and
     * standard-deviation, and an integer valued seed. It returns a
     * real number which may be interpreted as a sample of a normally
     * distributed (Gaussian) random variable with the specified mean
     * and standard deviation. After the first call to gauss, the seed
     * parameter is ignored.
     *
     * The computational technique used is to pass a uniformly
     * distributed random number through the inverse of the Normal
     * Distribution function.
     */
    static double gaussian(double, double, int);
};

#endif
