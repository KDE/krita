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

#include <math.h>



typedef double  COLOR[3];       /* red, green, blue (or X,Y,Z) */
static COLOR   **rgb_image;

static int       width, height;
static double ***min_stack;
static double ***max_stack;
static double  **x_grad;
static double  **y_grad;
static double  **x_smooth;
static double  **y_smooth;
static double  **theta;
static double  **luminance;
static double  **detail;
static double  **base;

void allocate_memory(int width, int height)
{
    int y;

    rgb_image = (COLOR **) malloc(height * sizeof(COLOR *));
    luminance = (double **) malloc(height * sizeof(double *));
    x_grad    = (double **) malloc(height * sizeof(double *));
    y_grad    = (double **) malloc(height * sizeof(double *));
    x_smooth  = (double **) malloc(height * sizeof(double *));
    y_smooth  = (double **) malloc(height * sizeof(double *));
    theta     = (double **) malloc(height * sizeof(double *));
    detail    = (double **) malloc(height * sizeof(double *));
    base      = (double **) malloc(height * sizeof(double *));
    for (y = 0; y < height; y++) {
        rgb_image[y] = (COLOR *) malloc(width * sizeof(COLOR));
        luminance[y] = (double *) malloc(width * sizeof(double));
        x_grad[y]    = (double *) malloc(width * sizeof(double));
        y_grad[y]    = (double *) malloc(width * sizeof(double));
        x_smooth[y]  = (double *) malloc(width * sizeof(double));
        y_smooth[y]  = (double *) malloc(width * sizeof(double));
        theta[y]     = (double *) malloc(width * sizeof(double));
        detail[y]    = (double *) malloc(width * sizeof(double));
        base[y]      = (double *) malloc(width * sizeof(double));
    }
}
void allocate_stack(int width, int height, int levels)
{
    int k, y;

    min_stack = (double ***) malloc(levels * sizeof(double **));
    max_stack = (double ***) malloc(levels * sizeof(double **));

    for (k = 0; k < levels; k++) {
        min_stack[k] = (double **) malloc(height * sizeof(double *));
        max_stack[k] = (double **) malloc(height * sizeof(double *));
        for (y = 0; y < height; y++) {
            min_stack[k][y] = (double *) malloc(width * sizeof(double));
            max_stack[k][y] = (double *) malloc(width * sizeof(double));
        }
    }
}

double build_stack(double **x_grad, double **y_grad,
                   double ***min_stack, double ***max_stack,
                   int levels, double beta)
{
    int    x, y, u, v, k;
    double gx, gy, value;
    double min, max;
    double min_grad =  1e20;
    double max_grad = -1e20;

    fprintf(stderr, "\tComputing stack\n");

    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++) {
            gx                 = x_grad[y][x];
            gy                 = y_grad[y][x];
            value              = hypot(gx, gy);
            max_grad           = (max_grad < value) ? value : max_grad;
            min_grad           = (min_grad > value) ? value : min_grad;
            min_stack[0][y][x] = value;
            max_stack[0][y][x] = value;
        }

    for (k = 1; k < levels; k++)
        for (y = 0; y < height; y++)
            for (x = 0; x < width; x++) {
                min = min_stack[k - 1][y][x];
                max = max_stack[k - 1][y][x];

                for (v = y - 1; v <= y + 1; v++)
                    for (u = x - 1; u <= x + 1; u++)
                        if (u >= 0 && u < width &&
                                v >= 0 && v < height) {
                            value = min_stack[k - 1][v][u];
                            min   = (min > value) ? value : min;
                            value = max_stack[k - 1][v][u];
                            max   = (max < value) ? value : max;
                        }

                min_stack[k][y][x] = min;
                max_stack[k][y][x] = max;
            }

    return beta *(max_grad - min_grad);
}
