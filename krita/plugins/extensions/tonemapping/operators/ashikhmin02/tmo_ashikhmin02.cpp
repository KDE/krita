/**
 * @brief Michael Ashikhmin tone mapping operator 2002
 *
 * This file is a part of PFSTMO package.
 * ----------------------------------------------------------------------
 * Copyright (C) 2003,2004 Grzegorz Krawczyk
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ----------------------------------------------------------------------
 *
 * @author Akiko Yoshida, <yoshida@mpi-sb.mpg.de>
 * @author Grzegorz Krawczyk, <krawczyk@mpi-sb.mpg.de>
 *
 * $Id: tmo_ashikhmin02.cpp,v 1.6 2004/11/16 13:40:46 yoshida Exp $
 */

#include "tmo_ashikhmin02.h"
#include <iostream>
#include <math.h>
#include <array2d.h>#include <assert.h>

#include "pyramid.h"

#define SMAX 10
#define LDMAX 500.0
#define EPSILON 0.00001

//-------------------------------------------

float calc_LAL_interpolated(GaussianPyramid *myPyramid, int x, int y, int s)
{
    float ratio = myPyramid->p[s-1].lambda;

    float newX = (float)x * ratio;
    float newY = (float)y * ratio;
    int X_int = (int)newX;
    int Y_int = (int)newY;

    float dx, dy, omdx, omdy;
    dx = newX - (float)X_int;
    omdx = 1.0 - dx;
    dy = newY - (float)Y_int;
    omdy = 1.0 - dy;

    int w = myPyramid->p[s-1].GP->getCols();
    int h = myPyramid->p[s-1].GP->getRows();

    float g;
    if (X_int < w - 1 && Y_int < h - 1) {
        g = omdx * omdy * myPyramid->p[s-1].getPixel(X_int, Y_int)
            +   dx * omdy * myPyramid->p[s-1].getPixel(X_int + 1, Y_int)
            + omdx *   dy * myPyramid->p[s-1].getPixel(X_int, Y_int + 1)
            +   dx *   dy * myPyramid->p[s-1].getPixel(X_int + 1, Y_int + 1);
    } else if (X_int <  w - 1 && Y_int >= h - 1) {
        Y_int = h - 1;
        g = omdx * myPyramid->p[s-1].getPixel(X_int, Y_int) + dx * myPyramid->p[s-1].getPixel(X_int + 1, Y_int);
    } else if (X_int >= w - 1 && Y_int < h - 1) {
        X_int = w - 1;
        g = omdy * myPyramid->p[s-1].getPixel(X_int, Y_int) + dy * myPyramid->p[s-1].getPixel(X_int, Y_int + 1);
    } else
        g = myPyramid->p[s-1].getPixel(w - 1, h - 1);

    return g;
}

float calc_LAL(GaussianPyramid *myPyramid, int x, int y, int s)
{
    float ratio = myPyramid->p[s-1].lambda;

    float newX = (float)x * ratio;
    float newY = (float)y * ratio;
    int X_int = (int)newX;
    int Y_int = (int)newY;

    if (X_int >= myPyramid->p[s-1].GP->getCols()) X_int = myPyramid->p[s-1].GP->getCols() - 1;
    if (Y_int >= myPyramid->p[s-1].GP->getRows()) Y_int = myPyramid->p[s-1].GP->getRows() - 1;

    return myPyramid->p[s-1].getPixel(X_int, Y_int);
}

float LAL(GaussianPyramid *myPyramid, int x, int y, float LOCAL_CONTRAST)
{
    float g, gg;
    for (int s = 1; s <= SMAX; s++) {
        // with interpolation
        g = calc_LAL_interpolated(myPyramid, x, y, s);
        gg = calc_LAL_interpolated(myPyramid, x, y, 2 * s);

        // w/o interpolation
        //    g = calc_LAL(myPyramid, x, y, s);
        //     gg = calc_LAL(myPyramid, x, y, 2*s);

        if (fabs((g - gg) / g) >= LOCAL_CONTRAST)
            return g;
    }
    return g;
}

////////////////////////////////////////////////////////

float C(float lum_val)   // linearly approximated TVI function
{
    if (lum_val <= 1e-20)
        return 0.0;

    if (lum_val < 0.0034)
        return lum_val / 0.0014;

    if (lum_val < 1.0)
        return 2.4483 + log(lum_val / 0.0034) / 0.4027;

    if (lum_val < 7.2444)
        return 16.5630 + (lum_val - 1.0) / 0.4027;

    return 32.0693 + log(lum_val / 7.2444) / 0.0556;
}

inline float TM(float lum_val, float maxLum, float minLum)
{
    float div = C(maxLum) - C(minLum);
    if (div != 0.0)
        return (LDMAX *(C(lum_val) - C(minLum)) / div);
    else
        return (LDMAX *(C(lum_val) - C(minLum)) / EPSILON);
}

////////////////////////////////////////////////////////

void getMaxMin(pfs::Array2D* lum_map, float& maxLum, float& minLum)
{
    maxLum = minLum = 0.0;

    for (int i = 0; i < lum_map->getCols() * lum_map->getRows(); i++) {
        maxLum = ((*lum_map)(i) > maxLum) ? (*lum_map)(i) : maxLum;
        minLum = ((*lum_map)(i) < minLum) ? (*lum_map)(i) : minLum;
    }
}

void Normalize(pfs::Array2D* lum_map, int nrows, int ncols)
{
    float maxLum, minLum;
    getMaxMin(lum_map, maxLum, minLum);
    float range = maxLum - minLum;
    for (int y = 0; y < nrows; y++)
        for (int x = 0; x < ncols; x++)
            (*lum_map)(x, y) = ((*lum_map)(x, y) - minLum) / range;
}

////////////////////////////////////////////////////////

int tmo_ashikhmin02(pfs::Array2D* Y, pfs::Array2D* L, float maxLum, float minLum, float avLum, bool simple_flag, float lc_value, int eq)
{
    assert(Y != NULL);
    assert(L != NULL);

    int nrows = Y->getRows();   // image size
    int ncols = Y->getCols();
    assert(nrows == L->getRows() && ncols == L->getCols());

    int im_size = nrows * ncols;

    //  maxLum /= avLum;       // normalize maximum luminance by average luminance

    // apply ToneMapping function only
    if (simple_flag) {
        for (int y = 0; y < nrows; y++)
            for (int x = 0; x < ncols; x++) {
                (*L)(x, y) = TM((*Y)(x, y), maxLum, minLum);

                //!! FIX:
                // to keep output values in range 0.01 - 1
                //        (*L)(x,y) /= 100.0f;
            }
        Normalize(L, nrows, ncols);

        return 0;
    }

    // applying the full functions....
    GaussianPyramid *myPyramid = new GaussianPyramid(Y, nrows, ncols);

    // LAL calculation
    pfs::Array2D* la = new pfs::Array2DImpl(ncols, nrows);
    for (int y = 0; y < nrows; y++) {
        for (int x = 0; x < ncols; x++) {
            (*la)(x, y) = LAL(myPyramid, x, y, lc_value);
            if ((*la)(x, y) == 0.0)
                (*la)(x, y) == EPSILON;
        }
    }
    delete(myPyramid);

    // TM function
    pfs::Array2D* tm = new pfs::Array2DImpl(ncols, nrows);
    for (int y = 0; y < nrows; y++)
        for (int x = 0; x < ncols; x++)
            (*tm)(x, y) = TM((*la)(x, y), maxLum, minLum);

    // final computation for each pixel
    for (int y = 0; y < nrows; y++)
        for (int x = 0; x < ncols; x++) {
            switch (eq) {
            case 2:
                (*L)(x, y) = (*Y)(x, y) * (*tm)(x, y) / (*la)(x, y);
                break;
            case 4:
                (*L)(x, y) = (*tm)(x, y) + C((*tm)(x, y)) / C((*la)(x, y)) * ((*Y)(x, y) - (*la)(x, y));
                break;
            default:
                exit(0);
            }

            //!! FIX:
            // to keep output values in range 0.01 - 1
            //(*L)(x,y) /= 100.0f;
        }

    Normalize(L, nrows, ncols);

    // cleaning
    delete(la);
    delete(tm);

    return 0;
}

