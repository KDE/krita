/*
 Copyright (c) 2005 Erik Reinhard, Greg Ward, Sumanta Pattanaik and
 Paul Debevec.   All rights reserved.

 Copyright (c) 2007 Shaine Joseph

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 1. Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in
       the documentation and/or other materials provided with the
       distribution.

 3. The name "Elsevier" must not be used to endorse or promote products
       derived from this software without prior written permission. For
       written permission, please contact Elsevier.


 THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED.   IN NO EVENT SHALL ELSEVIER OR
 ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 SUCH DAMAGE.

 */

#include "tmo_trilateral.h"
/**
 * @file tmo_trilateral.cpp
 * @brief Tone map luminance channel using trilateral filter model
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <pfs.h>

#include "lum.h"
#include "memory.h"

//--- from defines.h
typedef struct {
    int     xmax, ymax;     /* image dimensions */
} CVTS;
static CVTS      cvts             = {0, 0};

#define FTINY        1e-5


typedef double  COLOR[3];       /* red, green, blue (or X,Y,Z) */
//--- end of defines.h
/////////////////////////////////////////////////////////////////////////////////

//static COLOR   **tmp_image;
//static double  **tmp_float;
static double    sigma_c          = 21.;         /* The only user parameter                */
static double    saturation       = 1.0;
static double    base_range       = 5.0;
static double    base_value       = 0.0;


//////////////////////////////////////////////////////////////////////////////////


void compute_gradients(double **x_grad, double **y_grad, int width, int height)
{
    int x, y;

    fprintf(stderr, "\tComputing gradients\n");

    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++) {
            x_grad[y][x] = luminance[y][x + (x < width - 1)]  - luminance[y][x];
            y_grad[y][x] = luminance[y + (y < height - 1)][x] - luminance[y][x];
        }
}
void bilateral_filter(double **x_grad, double **y_grad,
                      double **x_smooth, double **y_smooth,
                      double sigma_c, double sigma_r, int filter_size)
{
    int    x, y, u, v, ux, vy;
    int    ulow, uhigh;
    int    vlow, vhigh;
    double g1, g2;
    double val_x, val_y;
    double sum_x, sum_y;
    double factor;
    double weight;
    double pos_div   = -2. * sigma_c * sigma_c;
    double range_div = -2. * sigma_r * sigma_r;
    int    half_size = (int)((filter_size - 1.) / 2.);

    fprintf(stderr, "\tBilaterial filtering (%i)\n", half_size);
    //int sj;
    for (y = 0; y < height; y++) {

        for (x = 0; x < width; x++) {

            factor = sum_x = sum_y = 0.0;
            g2     = hypot(x_grad[y][x], y_grad[y][x]);
            ulow   = (x - half_size < 0)       ? 0          : x - half_size;
            vlow   = (y - half_size < 0)       ? 0          : y - half_size;
            uhigh  = (x + half_size >= width)  ? width  - 1 : x + half_size;
            vhigh  = (y + half_size >= height) ? height - 1 : y + half_size;

            for (v = vlow; v <= vhigh; v++) {

                for (u = ulow; u <= uhigh; u++) {

                    ux            = u - x;
                    vy            = v - y;
                    val_x         = x_grad[v][u];
                    val_y         = y_grad[v][u];
                    g1            = hypot(val_x, val_y) - g2;
                    weight        = exp(g1 * g1 / range_div) * exp((ux * ux + vy * vy) / pos_div);
                    sum_x        += val_x * weight;
                    sum_y        += val_y * weight;
                    factor       += weight;
                }
            }

            x_smooth[y][x] = sum_x / factor;
            y_smooth[y][x] = sum_y / factor;
        }
        fprintf(stderr, "\t\tScanline %i (of %i)%c", y, height, (char)13);
    }
    fprintf(stderr, "\n");
}

void find_adaptive_region(double **theta, double ***min_stack,
                          double ***max_stack, double R, int levels)
{
    int    x, y, k;
    double value;

    fprintf(stderr, "\tFinding adaptive region\n");

    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++) {
            value = max_stack[0][y][x];
            for (k = 1; k < levels; k++)
                if ((max_stack[k][y][x] > value + R) ||
                        (min_stack[k][y][x] < value - R))
                    break;
            theta[y][x] = (k == 1) ? 1. : k - 1.;
        }
}

void detail_filter(double **base, double **luminance, double **x_smooth,
                   double **y_smooth, double **theta, double sigma_c_theta,
                   double sigma_r_theta)
{
    int    x, y, u, v, ux, vy;
    int    ulow, uhigh;
    int    vlow, vhigh;
    int    half_size;
    double weight, domain_weight, range_weight;
    double factor, value;
    double coeff_a, coeff_b, coeff_c;
    double detail_value;
    double domain_div = -2. * sigma_c_theta * sigma_c_theta;
    double range_div  = -2. * sigma_r_theta * sigma_r_theta;

    fprintf(stderr, "\tDetail filtering\n");

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            factor     = value = 0.0;
            half_size  = 1 << ((int)theta[y][x] - 1);
            coeff_a    = x_smooth[y][x];
            coeff_b    = y_smooth[y][x];
            coeff_c    = luminance[y][x];
            ulow       = (x - half_size < 0)       ? 0          : x - half_size;
            vlow       = (y - half_size < 0)       ? 0          : y - half_size;
            uhigh      = (x + half_size >= width)  ? width  - 1 : x + half_size;
            vhigh      = (y + half_size >= height) ? height - 1 : y + half_size;

            for (v = vlow; v <= vhigh; v++)
                for (u = ulow; u <= uhigh; u++) {
                    ux            = u - x;
                    vy            = v - y;
                    domain_weight = exp((ux * ux + vy * vy) / domain_div);
                    detail_value  = luminance[v][u] - coeff_a * ux - coeff_b * vy - coeff_c;
                    range_weight  = exp(detail_value * detail_value / range_div);
                    weight        = domain_weight * range_weight;
                    value        += detail_value * weight;
                    factor       += weight;
                }
            base[y][x] = coeff_c + (value / factor);
        }
        fprintf(stderr, "\t\tScanline %i (of %i)%c", y, height, (char)13);
    }
    fprintf(stderr, "\n");
}

void compute_detail_layer(double **detail, double **base, double **luminance,
                          int width, int height)
{
    int x, y;

    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            detail[y][x] = luminance[y][x] - base[y][x];
}

void compress_base(double **base, int width, int height)
{
    int x, y;
    double minval = 1e20;
    double maxval = -1e20;
    double factor;

    int minx, miny, maxx, maxy;

    fprintf(stderr, "Compressing base layer\n");

    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++) {
            if (base[y][x] < minval) {
                minx = x;
                miny = y;
            }
            if (base[y][x] > maxval) {
                maxx = x;
                maxy = y;
            }
            minval = (minval < base[y][x]) ? minval : base[y][x];
            maxval = (maxval > base[y][x]) ? maxval : base[y][x];
        }

    factor = base_range / (maxval - minval);

    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            base[y][x] = minval + base_value + (base[y][x] - minval) * factor;
}///////////////////////////////////
void tonemap_image2()
{
////////////////////////////////

    int    levels, filter_size;
    double sigma_r, sigma_c_theta, sigma_r_theta;
    double R, beta;

    sigma_c_theta = sigma_c;
    beta          = 0.15;
    filter_size   = (int) sigma_c;

    if (width > height)
        levels = 1 + (int)(log10(width)  / log10(2.));
    else
        levels = 1 + (int)(log10(height) / log10(2.));
    levels = (levels > 5) ? 5 : levels;

    compute_luminance(rgb_image, luminance, width, height);
    log_space_lum(luminance, width, height);

    allocate_stack(width, height, levels);
    compute_gradients(x_grad, y_grad, width, height);
    sigma_r = build_stack(x_grad, y_grad, min_stack, max_stack, levels, beta);
    sigma_r_theta = R = sigma_r;
    bilateral_filter(x_grad, y_grad, x_smooth, y_smooth, sigma_c, sigma_r, filter_size);
    find_adaptive_region(theta, min_stack, max_stack, R, levels);
    detail_filter(base, luminance, x_smooth, y_smooth, theta, sigma_c_theta, sigma_r_theta);
    compute_detail_layer(detail, base, luminance, width, height);
    compress_base(base, width, height);
    compose_luminance(detail, base, luminance, width, height);
    exponentiate_lum(luminance, width, height);
    colour_processing(luminance, rgb_image, width, height, saturation);
    clamp_image(rgb_image, width, height, 1.0);

/////////////////////////////////

}

/*
 * @brief trilateral filter tone-mapping
 *
 * @param Y input luminance
 * @param L output tonemapped intensities
 * @param contrast
 * @param sigma
 * @param shift
 * @param saturation
 */
void tmo_trilateral(const pfs::Array2D *Y, pfs::Array2D *L,
                    float contrast, float sigma, float shift, float saturation)
{
    int x, y;

    ::base_range = contrast;
    ::sigma_c = sigma;
    ::base_value = shift;
    ::saturation = saturation;


    cvts.xmax = Y->getCols();
    cvts.ymax = Y->getRows();

    height = cvts.ymax;
    width = cvts.xmax;
    allocate_memory(cvts.xmax, cvts.ymax);

    // reading image
    for (y = 0 ; y < cvts.ymax ; y++)
        for (x = 0 ; x < cvts.xmax ; x++)
            rgb_image[y][x][0] = (*Y)(x, y);


    tonemap_image2();
//normalise_image               (rgb_image, width, height, 255.);
    // saving image
    for (y = 0 ; y < cvts.ymax ; y++)
        for (x = 0 ; x < cvts.xmax ; x++)
            (*L)(x, y) = rgb_image[y][x][0];


}
