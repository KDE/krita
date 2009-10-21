/**
 * @file tmo_reinhard05.cpp
 * @brief Tone map XYZ channels using Reinhard05 model
 *
 * Dynamic Range Reduction Inspired by Photoreceptor Physiology.
 * E. Reinhard and K. Devlin.
 * In IEEE Transactions on Visualization and Computer Graphics, 2005.
 *
 *
 * This file is a part of PFSTMO package.
 * ----------------------------------------------------------------------
 * Copyright (C) 2007 Grzegorz Krawczyk
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
 * @author Grzegorz Krawczyk, <krawczyk@mpi-sb.mpg.de>
 *
 * $Id$
 */

#include "tmo_reinhard05.h"
#include <config.h>

#include <iostream>
#include <math.h>
#include <array2d.h>
#include <assert.h>


void tmo_reinhard05(pfs::Array2D* R, pfs::Array2D* G, pfs::Array2D* B,
                    pfs::Array2D* Y, float br, float ca, float la)
{
    float max_lum = (*Y)(0);
    float min_lum = (*Y)(0);
    float world_lum = 0.0;
    float Cav[] = { 0.0f, 0.0f, 0.0f};
    float Lav = 0.0f;
    int im_width = Y->getCols();
    int im_height = Y->getRows();
    int im_size = im_width * im_height;

    for (int i = 1 ; i < im_size ; i++) {
        float lum = (*Y)(i);
        max_lum = (max_lum > lum) ? max_lum : lum;
        min_lum = (min_lum < lum) ? min_lum : lum;
        world_lum += log(2.3e-5 + lum);
        Cav[0] += (*R)(i);
        Cav[1] += (*G)(i);
        Cav[2] += (*B)(i);
        Lav += lum;
    }
    world_lum /= im_size;
    Cav[0] /= im_size;
    Cav[1] /= im_size;
    Cav[2] /= im_size;
    Lav /= im_size;

    //--- tone map image
    max_lum = log(max_lum);
    min_lum = log(min_lum);

    // image key
    float k = (max_lum - world_lum) / (max_lum - min_lum);
    // image contrast based on key value
    float m = 0.3f + 0.7f * pow(k, 1.4f);
    // image brightness
    float f = exp(-br);

    float max_col = 0.0f;
    float min_col = 1.0f;

    int x, y;
    for (x = 0 ; x < im_width ; x++)
        for (y = 0 ; y < im_height ; y++) {
            float l = (*Y)(x, y);
            float col = 0;
            if (l != 0.0f) {
                for (int c = 0 ; c < 3 ; c++) {
                    switch (c) {
                    case 0: col = (*R)(x, y); break;
                    case 1: col = (*G)(x, y); break;
                    case 2: col = (*B)(x, y); break;
                    };

                    if (col != 0.0f) {
                        // local light adaptation
                        float Il = ca * col + (1 - ca) * l;
                        // global light adaptation
                        float Ig = ca * Cav[c] + (1 - ca) * Lav;
                        // interpolated light adaptation
                        float Ia = la * Il + (1 - la) * Ig;
                        // photoreceptor equation
                        col /= col + pow(f * Ia, m);
                    }

                    max_col = (col > max_col) ? col : max_col;
                    min_col = (col < min_col) ? col : min_col;

                    switch (c) {
                    case 0: (*R)(x, y) = col; break;
                    case 1: (*G)(x, y) = col; break;
                    case 2: (*B)(x, y) = col; break;
                    };
                }
            }
        }

    //--- normalize intensities
    for (x = 0 ; x < im_width ; x++)
        for (y = 0 ; y < im_height ; y++) {
            (*R)(x, y) = ((*R)(x, y) - min_col) / (max_col - min_col);
            (*G)(x, y) = ((*G)(x, y) - min_col) / (max_col - min_col);
            (*B)(x, y) = ((*B)(x, y) - min_col) / (max_col - min_col);
        }
}
