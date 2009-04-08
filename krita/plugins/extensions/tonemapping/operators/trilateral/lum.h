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
#define FTINY        1e-5


typedef double  COLOR[3];       /* red, green, blue (or X,Y,Z) */
double lum(COLOR **rgb_image, int x, int y)
{
    return 0.2125 * rgb_image[y][x][0] +
           0.7154 * rgb_image[y][x][1] +
           0.0721 * rgb_image[y][x][2];
}
void compute_luminance(COLOR **rgb_image, double **luminance, int width, int height)
{
    int x, y;

    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            luminance[y][x] = lum(rgb_image, x, y);
}

void log_space_lum(double **image, int width, int height)
{
    int x, y;

    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            image[y][x] = log(FTINY + image[y][x]);
}
void compose_luminance(double **detail, double **base, double **luminance,
                       int width, int height)
{
    int x, y;

    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            luminance[y][x] = base[y][x] + detail[y][x];
}
void exponentiate_lum(double **image, int width, int height)
{
    int x, y;

    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            image[y][x] = exp(image[y][x]) - FTINY;
}
void colour_processing(double **luminance, COLOR **rgb_image, int width, int height, double saturation)
{
    int x, y;
    double L;

    fprintf(stderr, "Colour processing\n");

    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++) {
            L = lum(rgb_image, x, y) + FTINY;
            rgb_image[y][x][0] = pow(rgb_image[y][x][0] / L, saturation) * luminance[y][x];
            rgb_image[y][x][1] = pow(rgb_image[y][x][1] / L, saturation) * luminance[y][x];
            rgb_image[y][x][2] = pow(rgb_image[y][x][2] / L, saturation) * luminance[y][x];
        }
}

void clamp_image(COLOR **rgb_image, int width, int height, double maxval)
{
    int x, y, c;

    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            for (c = 0; c < 3; c++) {
                if (rgb_image[y][x][c] < 0.)
                    rgb_image[y][x][c] = 0.;
                if (rgb_image[y][x][c] > maxval)
                    rgb_image[y][x][c] = maxval;
            }
}

void normalise_image(COLOR **rgb_image, int width, int height, double output_range)
{
    int    x, y, c;
    double minval =  1e20;
    double maxval = -1e20;
    double average = 0.;
    double range;
    double value;

    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++) {
            value = lum(rgb_image, x, y);

            if (value < minval)
                minval = value;
            if (value > maxval)
                maxval = value;
            average += value;
        }
    range    = maxval - minval;
    average /= width * height;

    for (x = 0; x < width; x++)
        for (y = 0; y < height; y++)
            for (c = 0; c < 3; c++) {
                rgb_image[y][x][c] = output_range * ((rgb_image[y][x][c] - minval) / range);
                if (rgb_image[y][x][c] < 0.)           rgb_image[y][x][c] = 0.;
                if (rgb_image[y][x][c] > output_range) rgb_image[y][x][c] = output_range;
            }
}
