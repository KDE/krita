/**
 * @file tmo_icam.cpp
 * @brief Tone map luminance channel using icam model
 */

#include <pfs.h>

#include "tmo_icam.h"
#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "fft.h"
#include "color_spaces.h"

//--- from defines.h
typedef struct {
    int     xmax, ymax;     /* image dimensions */
} CVTS;
static CVTS      cvts             = {0, 0};


typedef double  COLOR[3];       /* red, green, blue (or X,Y,Z) */
//--- end of defines.h


static int       width, height;

static COLOR   **rgb_image;
static COLOR   **white_point;
static double  **low_pass;
static double  **y_channel;


static double **convolved_image;
static zomplex *convolution_fft;

int    dynrange;
double res = 1024.;

//Parameters of the icam algorithm
static int       indep            = 0;
//static double    gamma_val        = GAMMA;
static double    prescaling       = 1000.0;
static double    variance         = -0.1;
static double    variance2        = -0.1;
static double    D                = 0.1;
static double    percentile       = 0.01;


static zomplex  *filter_fft;
static zomplex  *image_fft;


namespace icam
{


double lum(COLOR **rgb_image, int x, int y)
{
    return 0.2125 * rgb_image[y][x][0] +
           0.7154 * rgb_image[y][x][1] +
           0.0721 * rgb_image[y][x][2];
}
void lin_luminance_range(COLOR **rgb_image, int width, int height,
                         double *lum_min, double *lum_max)
{
    int    x, y;
    double min;
    double max;
    double val;
    //double temp1, temp2;

    min = 1e6;
    max = -1e6;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            val      = lum(rgb_image, x, y);
            if (isnan(val) == 1) {
                ;//empty statement
            } else {
                min      = (val < min) ? val : min;
                max      = (val > max) ? val : max;
            }
        }
    }

    if (min == 1e6)
        min = 0.001;
    if (max == -1e6)
        max = 100000;

    *lum_min   = min;
    *lum_max   = max;
}

void set_res(int resolution)
{
    res = resolution;
}

double *lin_histogram(COLOR **rgb_image, int width, int height,
                      double lum_min, double lum_max)
{
    int     x, y, i;
    double  value;
    double  delta_b;
    double *hist;
    double double_dynrange = (res * ceil(lum_max - lum_min));//Shaine modified

    if (double_dynrange < 1.) {
        fprintf(stderr, "Histogram failed\n"); //Shaine modified
        return NULL;
    }
    if (double_dynrange > 1000.) {
        double_dynrange = 1000.;//Shaine modified
        res      = double_dynrange / ceil(lum_max - lum_min); //Shaine modified
    }
    dynrange = (int)double_dynrange;//Shaine modified
    hist     = (double *)calloc((int)(dynrange + 1), sizeof(double));

    for (x = 0; x < width; x++)        /* Create histogram */
        for (y = 0; y < height; y++) {
            value = lum(rgb_image, x, y);
            if (value > 0.0)
                hist[(int)(res *(value - lum_min))]++;
        }

    delta_b = (lum_max - lum_min) / res;
    for (i = 0; i < dynrange; i++)
        hist[i] /= delta_b * (double)(width * height);

    return hist;
}
void perc_luminance_adjust(double *hist, double *lum_min, double *lum_max,
                           int width, int height, double fraction)
{
    int    i;
    int    length = width * height;
    double sum    = 0.;
    double L1     = *lum_min;
    double L99    = *lum_max;

    for (i = dynrange - 1; i >= 0; i--) {
        sum += (double)hist[i];
        if (sum / length > fraction) {
            L99 = ((double)i / (double)res) + *lum_min;
            break;
        }
    }

    sum = 0.;
    for (i = 0; i < dynrange; i++) {
        sum += (double)hist[i];
        if (sum / length > fraction) {
            L1 = ((double)i / (double)res) + *lum_min;
            break;
        }
    }

    *lum_min = L1;
    *lum_max = L99;
} void color_convert(COLOR **image, double matrix[][3], int width, int height)
{
    int       x, y, i, j;
    double    result[3];

    for (x = 0; x < width; x++)
        for (y = 0; y < height; y++) {
            result[0] = result[1] = result[2] = 0.;

            for (i = 0; i < 3; i++)
                for (j = 0; j < 3; j++)
                    result[i] += matrix[i][j] * image[y][x][j];

            for (i = 0; i < 3; i++)
                image[y][x][i] = result[i];
        }
}
void color_convert_point(double *color, double matrix[][3])
{
    int       i, j;
    double    result[3] = {0., 0., 0.};

    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            result[i] += matrix[i][j] * color[j];

    for (i = 0; i < 3; i++)
        color[i] = result[i];
}


/*
 * FFT functions
 */
void build_image_fft(double **image, zomplex *image_fft)
{
    int    i, x, y;
    double length    = cvts.xmax * cvts.ymax;
    double fft_scale = 1. / sqrt(length);

// fprintf (stderr, "Computing image FFT\n");
// fprintf (stderr, "Computing image FFT--mem-allocated\n");

    for (y = 0; y < cvts.ymax; y++)
        for (x = 0; x < cvts.xmax; x++)
            image_fft[y*cvts.xmax + x].re = image[y][x];

    //fprintf (stderr, "Computing image FFT--luminances copied\n");
    compute_fft(image_fft, cvts.xmax, cvts.ymax);
    //fprintf (stderr, "Computing image FFT-compute_fft over\n");
    for (i = 0; i < length; i++) {
        image_fft[i].re *= fft_scale;
        image_fft[i].im *= fft_scale;
    }
// fprintf (stderr, "Computing image FFT-success\n");
}

void convolve_filter(double **image, int width, int height, zomplex* image_fft, zomplex* filter_fft, zomplex *convolution_fft, double **convolved_image)
{
    int     i, x, y;

    for (i = 0; i < width * height; i++) {
        convolution_fft[i].re = image_fft[i].re * filter_fft[i].re -
                                image_fft[i].im * filter_fft[i].im;
        convolution_fft[i].im = image_fft[i].re * filter_fft[i].im +
                                image_fft[i].im * filter_fft[i].re;

    }
    //compute_inverse_fft (convolution_fft, cvts.xmax, cvts.ymax);
    i = 0;
    for (y = 0; y < cvts.ymax; y++)
        for (x = 0; x < cvts.xmax; x++)
            convolved_image[x][y] = convolution_fft[i++].re;//see if indices are interchanged
    //    image[y][x] = image_fft[i++].re;//see if indices are interchanged
}

void low_pass_filter(double **image, int width, int height, double variance)
{
    int x, y, i;
    double length = sqrt((double)(width * height));
    double x1, y1, s;
    double c = 1. / 4.;
    double a = 1. / variance;


    filter_fft = (zomplex*) calloc(width * height, sizeof(zomplex));
    image_fft = (zomplex*) calloc(width * height, sizeof(zomplex));

    /****************************/

    for (y = 0; y < cvts.ymax; y++) {
        y1 = (y >= cvts.ymax / 2) ? y - cvts.ymax : y;
        s  = erf(a * (y1 - .5)) - erf(a * (y1 + .5));
        for (x = 0; x < cvts.xmax; x++) {
            x1 = (x >= cvts.xmax / 2) ? x - cvts.xmax : x;
            filter_fft[y*cvts.xmax + x].re  = s * (erf(a * (x1 - .5)) - erf(a * (x1 + .5))) * c;
        }
    }
    /************FFT of Gaussian starts****************/
    double fft_scale = 1. / length;
    //compute_fft     (filter_fft, width, height);
    for (i = 0; i < cvts.xmax * cvts.ymax; i++) {
        filter_fft[i].re *= fft_scale;
        filter_fft[i].im *= fft_scale;
    }
    /************FFT of Gaussian ends****************/

    /***********FFT of Image Starts*****************/

    // build_image_fft (image, image_fft);
    int    i_s, x_s, y_s;
    double length_s    = cvts.xmax * cvts.ymax;
    double fft_scale_s = 1. / sqrt(length_s);

// fprintf (stderr, "Computing image FFT\n");
// fprintf (stderr, "Computing image FFT--mem-allocated\n");

    for (y_s = 0; y_s < cvts.ymax; y_s++)
        for (x_s = 0; x_s < cvts.xmax; x_s++)
            image_fft[y_s*cvts.xmax + x_s].re = image[y_s][x_s];

    //fprintf (stderr, "Computing image FFT--luminances copied\n");
    //compute_fft (image_fft, cvts.xmax, cvts.ymax);
    //fprintf (stderr, "Computing image FFT-compute_fft over\n");
    for (i_s = 0; i_s < length_s; i_s++) {
        image_fft[i_s].re *= fft_scale_s;
        image_fft[i_s].im *= fft_scale_s;
    }
    /***********FFT of Image Ends*****************/

    /***************CONVOLUTION STARTS*************/

    convolved_image = (double **) malloc(cvts.xmax * sizeof(double *));

    convolution_fft = (zomplex *) calloc(cvts.xmax * cvts.ymax, sizeof(zomplex));

    //fprintf (stderr, "Memory allocated \n");
    for (x = 0; x < cvts.xmax; x++)
        convolved_image[x] = (double *) malloc(cvts.ymax * sizeof(double));
    // fprintf (stderr, "Before convolve filter \n");

    convolve_filter(image, width, height, image_fft, filter_fft, convolution_fft, convolved_image);

    //fprintf (stderr, "After convolve filter \n");

    //fprintf (stderr, "\n");
    //fprintf (stderr, "before memory free \n");
    //free (convolution_fft);
//fprintf (stderr, "memory freed 1 \n");
    //free (image_fft);
    //fprintf (stderr, "memory freed 2 \n");
    //fprintf (stderr, "After convolve filter: memory freed \n");


    /************CONVOLUTION ENDS****************/

    /************Optional Scaling starts****************/
    //Comment it out if you do not want this part of the code
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            image[y][x] /= length;

        }
    }
    /************Optional Scaling ends****************/

// free               (image_fft);
// free               (filter_fft);
}
void clip(COLOR **rgb_image, int width, int height, double lum_max)
{
    int    x, y, c;
    double value;

    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            for (c = 0; c < 3; c++) {
                value              = rgb_image[y][x][c];
                rgb_image[y][x][c] = (value > lum_max) ? lum_max : value;
            }
}
/*
 * Tonemapping routines
 */
void tonemap_image()
{

    int     x, y, c;
    double *hist;
    double  value, factor;
    double  lum_min;
    double  lum_max;
    double  Imax = 0.;
    double  Wmax = 0.;
    double  white[3] = {100.0, 100.0, 100.0};
    double  D65[3]   = {95.05, 100.0, 108.88};

    if (variance < 0.) {
        variance = cvts.xmax / 4.;
    }
    if (variance2 < 0) {
        variance2 = cvts.xmax / 3.;
    }

    for (y = 0; y < cvts.ymax; y++) {
        for (x = 0; x < cvts.xmax; x++) {
            for (c = 0; c < 3; c++) {
                rgb_image[y][x][c] *= prescaling;
            }
        }
    }
//fprintf (stderr, "\t Pre-Scaling done\n");
    color_convert(rgb_image, MXYZ, cvts.xmax, cvts.ymax);
    //fprintf (stderr, "\t Color Convert MXYZ done\n");

    for (y = 0; y < cvts.ymax; y++) {
        for (x = 0; x < cvts.xmax; x++) {
            y_channel[y][x] = rgb_image[y][x][1];
        }
    }

    if (indep) {
        for (c = 0; c < 3; c++) {
            for (y = 0; y < cvts.ymax; y++) {
                for (x = 0; x < cvts.xmax; x++) {
                    low_pass[y][x] = rgb_image[y][x][c];
                }
            }
            fprintf(stderr, "\t before indep=true low-pass-filter\n");
            low_pass_filter(low_pass, cvts.xmax, cvts.ymax, variance);
            fprintf(stderr, "\t after indep=true low-pass-filter\n");
            for (y = 0; y < cvts.ymax; y++) {
                for (x = 0; x < cvts.xmax; x++) {
                    white_point[y][x][c] = (low_pass[y][x] < 0.) ? 0. : low_pass[y][x];
                }
            }
        }
    } else {
        for (y = 0; y < cvts.ymax; y++) {
            for (x = 0; x < cvts.xmax; x++) {
                low_pass[y][x] = rgb_image[y][x][1];
            }
        }
        fprintf(stderr, "\t before indep=false low-pass-filter\n");
        low_pass_filter(low_pass, cvts.xmax, cvts.ymax, variance);
        fprintf(stderr, "\t after indep=false low-pass-filter\n");

        for (y = 0; y < cvts.ymax; y++) {
            for (x = 0; x < cvts.xmax; x++) {
                value = (low_pass[y][x] < 1e-6) ? 1e-6 : low_pass[y][x];
                for (c = 0; c < 3; c++) {
                    white_point[y][x][c] = value;
                }
            }
        }
    }


    color_convert(rgb_image, Mcat, cvts.xmax, cvts.ymax);
    //fprintf (stderr, "\t Color Convert rgb_image Mcat done\n");
    color_convert(white_point, Mcat, cvts.xmax, cvts.ymax);
    color_convert_point(white, Mcat);
    color_convert_point(D65, Mcat);
//fprintf (stderr, "\t Color Convert Point D65 Mcat done\n");
    for (y = 0; y < cvts.ymax; y++) {
        for (x = 0; x < cvts.xmax; x++) {
            for (c = 0; c < 3; c++) {
                Wmax = (white_point[y][x][c] > Wmax) ? white_point[y][x][c] : Wmax;
                Imax = (rgb_image[y][x][c]   > Imax) ? rgb_image[y][x][c]   : Imax;
            }
        }
    }

    for (y = 0; y < cvts.ymax; y++) {
        for (x = 0; x < cvts.xmax; x++) {
            for (c = 0; c < 3; c++) {
                white_point[y][x][c] *= Imax / Wmax;
                rgb_image[y][x][c]   *= 1. - D + (D * D65[c] / white_point[y][x][c]);
            }
        }
    }

    color_convert(rgb_image, Mcatinv, cvts.xmax, cvts.ymax);


    low_pass_filter(y_channel, cvts.xmax, cvts.ymax, variance2);


    for (y = 0; y < cvts.ymax; y++) {
        for (x = 0; x < cvts.xmax; x++) {
            value           = (y_channel[y][x] < 0.) ? 0. : 5. * y_channel[y][x];
            factor          = pow(1. / (value + 1.), 4.);
            y_channel[y][x] = (1. / 1.7) * ((0.2 * value * factor) + (0.1 * pow(1. - factor, 2.) * pow(value, 1. / 3.)));
            for (c = 0; c < 3; c++) {
                rgb_image[y][x][c] /= 100.;
            }
        }
    }

    color_convert(rgb_image, LMS, cvts.xmax, cvts.ymax);

    for (y = 0; y < cvts.ymax; y++) {
        for (x = 0; x < cvts.xmax; x++) {
            for (c = 0; c < 3; c++) {
                rgb_image[y][x][c] = pow(fabs(rgb_image[y][x][c]), 0.43 * y_channel[y][x]);
                rgb_image[y][x][c] = pow(fabs(rgb_image[y][x][c]), 1. / 0.43);
            }
        }
    }

    color_convert(rgb_image, LMSinv, cvts.xmax, cvts.ymax);
    color_convert(rgb_image, Mcatb, cvts.xmax, cvts.ymax);

    white[0] = white[1] = white[2] = 100.;
    D65[0]   = 95.05;
    D65[1]   = 100.0;
    D65[2]   = 108.88;

    color_convert_point(white, Mcatb);
    color_convert_point(D65, Mcatb);

    for (y = 0; y < cvts.ymax; y++) {
        for (x = 0; x < cvts.xmax; x++) {
            for (c = 0; c < 3; c++) {
                rgb_image[y][x][c] *= white[c]  / D65[c];
            }
        }
    }

    color_convert(rgb_image, Mcatbinv, cvts.xmax, cvts.ymax);
    color_convert(rgb_image, MXYZinv, cvts.xmax, cvts.ymax);

    lin_luminance_range(rgb_image, cvts.xmax, cvts.ymax, &lum_min, &lum_max);
    set_res((int)1e6);
    hist = lin_histogram(rgb_image, cvts.xmax, cvts.ymax, lum_min, lum_max);
    if (!hist) return;
    perc_luminance_adjust(hist, &lum_min, &lum_max, cvts.xmax, cvts.ymax, percentile);


    //free                (hist);

    clip(rgb_image, cvts.xmax, cvts.ymax, lum_max);
    //apply_gamma         (rgb_image, cvts.xmax, cvts.ymax, gamma_val);

}

/*
 * Miscellaneous functions
 */void allocate_memory(int width, int height)
{
    int y;

    rgb_image   = (COLOR **)  malloc(height * sizeof(COLOR *));
    white_point = (COLOR **)  malloc(height * sizeof(COLOR *));
    low_pass    = (double **) malloc(height * sizeof(double *));
    y_channel   = (double **) malloc(height * sizeof(double *));
    for (y = 0; y < height; y++) {
        rgb_image[y]   = (COLOR *)  malloc(width * sizeof(COLOR));
        white_point[y] = (COLOR *)  malloc(width * sizeof(COLOR));
        low_pass[y]    = (double *) malloc(width * sizeof(double));
        y_channel[y]   = (double *) malloc(width * sizeof(double));
    }
}
void deallocate_memory(int width, int height)
{
    int y;


    for (y = 0; y < height; y++) {
        free(rgb_image[y]);
        free(white_point[y]);
        free(low_pass[y]);
        free(y_channel[y]);
    }

    free(rgb_image);
    free(white_point);
    free(low_pass);
    free(y_channel);
}

void print_parameter_settings()
{
    //fprintf (stderr, "\nTonemapping: %s  --> %s\n", infilename, outfilename);
    fprintf(stderr, "\tImage size       = %i %i\n", cvts.xmax, cvts.ymax);
}

/*
 * @brief icam tone-mapping
 *
 * @param Y input luminance
 * @param L output tonemapped intensities
 * @param variance
 * @param variance2
 * @param D
 * @param prescaling
 * @param percentile
 * @param indep
 */
void tmo_icam(const pfs::Array2D *Y, pfs::Array2D *L,
              float variance, float variance2, float D, float prescaling, float percentile, bool indep)
{
    int x, y;

    ::variance = variance;
    ::variance2 = variance2;
    ::D = D;
    ::prescaling = prescaling;
    ::percentile = percentile;
    //::indep = (indep) ? 1 : 0;
    ::indep = 1;

    cvts.xmax = Y->getCols();
    cvts.ymax = Y->getRows();

    allocate_memory(cvts.xmax, cvts.ymax);

    // reading image
    for (y = 0 ; y < cvts.ymax ; y++)
        for (x = 0 ; x < cvts.xmax ; x++)
            rgb_image[y][x][0] = (*Y)(x, y);


    tonemap_image();

    // saving image
    for (y = 0 ; y < cvts.ymax ; y++)
        for (x = 0 ; x < cvts.xmax ; x++)
            (*L)(x, y) = rgb_image[y][x][0];

    print_parameter_settings();

    deallocate_memory(cvts.xmax, cvts.ymax);
    //clean_pyramid();
}

}
