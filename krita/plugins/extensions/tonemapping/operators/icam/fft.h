#include <math.h>
void SWAP(float &a, float &b)
{
    float temp = a;
    a = b;
    b = temp;
}


typedef struct {
    double re,
    im;
} zomplex;

void fourn(float data[], unsigned long nn[], int ndim, int isign)
{
    /******This is a modified version of the Numerical Recipes in C code ************/

    /******Look at www.nr.com for more information************/
    /*Replaces data by its ndim-dimensional discrete Fourier transform, if isign is input as 1.

    If isign is input as -1, data is replaced by its inverse transform times the product of the lengths of all dimensions.

    nn[1..ndim] is an integer array containing the lengths of each dimension (number of complex values), which MUST all be powers of 2.

    data is a real array of length twice the product of these lengths, in which the data are stored as in a multidimensional complex array.

    The real and imaginary parts of each element are in consecutive locations, and the rightmost index of the array increases most rapidly as one proceeds along data.

    For a two-dimensional array, this is equivalent to storing the array by rows.
    */

    int idim;
    unsigned long i1, i2, i3, i2rev, i3rev, ip1, ip2, ip3, ifp1, ifp2;
    unsigned long ibit, k1, k2, n, nprev, nrem, ntot;
    float tempi, tempr;
    double theta, wi, wpi, wpr, wr, wtemp; //Double precision for trigonometric recurrences.
    for (ntot = 1, idim = 1; idim <= ndim; idim++) { //Compute total number of complex values.
        ntot *= nn[idim];
    }
    nprev = 1;

    for (idim = ndim; idim >= 1; idim--) { //Main loop over the dimensions.
        n = nn[idim];
        nrem = ntot / (n * nprev);
        ip1 = nprev << 1;
        ip2 = ip1 * n;
        ip3 = ip2 * nrem;
        i2rev = 1;
        for (i2 = 1; i2 <= ip2; i2 += ip1) { //This is the bit-reversal section of the routine.
            if (i2 < i2rev) {
                for (i1 = i2; i1 <= i2 + ip1 - 2; i1 += 2) {
                    for (i3 = i1; i3 <= ip3; i3 += ip2) {
                        i3rev = i2rev + i3 - i2;
                        SWAP(data[i3], data[i3rev]);
                        SWAP(data[i3+1], data[i3rev+1]);
                    }
                }
            }//if loop ends

            ibit = ip2 >> 1;
            while (ibit >= ip1 && i2rev > ibit) {
                i2rev -= ibit;
                ibit >>= 1;
            }
            i2rev += ibit;
        }
        ifp1 = ip1; //Here begins the Danielson-Lanczos section of the routine.
        while (ifp1 < ip2) {
            ifp2 = ifp1 << 1;
            theta = isign * 6.28318530717959 / (ifp2 / ip1); //Initialize for the trig. recurrence.
            wtemp = sin(0.5 * theta);
            wpr = -2.0 * wtemp * wtemp;
            wpi = sin(theta);
            wr = 1.0;
            wi = 0.0;
            for (i3 = 1; i3 <= ifp1; i3 += ip1) {
                for (i1 = i3; i1 <= i3 + ip1 - 2; i1 += 2) {
                    for (i2 = i1; i2 <= ip3; i2 += ifp2) {
                        k1 = i2; //Danielson-Lanczos formula:
                        k2 = k1 + ifp1;

                        if (data[k2] > 1000.0 || data[k2] < -1000.0)
                            data[k2] = 1. / data[k2];
                        if (data[k2+1] > 1000.0 || data[k2+1] < -1000.0)
                            data[k2+1] = 1. / data[k2+1];

                        if (data[k1] > 1000.0 || data[k1] < -1000.0)
                            data[k1] = 1. / data[k1];
                        if (data[k1+1] > 1000.0 || data[k1+1] < -1000.0)
                            data[k1+1] = 1. / data[k1+1];

                        data[k2] = 0.0;
                        data[k2+1] = 0.0;
                        data[k1] = 0.0;
                        data[k1+1] = 0.0;
                        tempr = (float)wr * data[k2] - (float)wi * data[k2+1];
                        tempi = (float)wr * data[k2+1] + (float)wi * data[k2];
                        data[k2] = data[k1] - tempr;
                        data[k2+1] = data[k1+1] - tempi;
                        data[k1] += tempr;
                        data[k1+1] += tempi;
                    }
                }
                wr = (wtemp = wr) * wpr - wi * wpi + wr; //Trigonometric recurrence.
                wi = wi * wpr + wtemp * wpi + wi;
            }
            ifp1 = ifp2;
        }//while loop ends
        nprev *= n;
    }//for loop ends

}//function ends

void compute_fft(zomplex *array, int width, int height)
{
    int countre = 0;
    int countim = 0;

    float *data = (float *) malloc((2 * width * height + 1) * sizeof(float));
    for (int p = 0; p < ((width)*(height)); p++) {
        data[(2*p)+1] = (*(array + p)).re;
        data[(2*p)+2] = (*(array + p)).im;
        if (
            (data[(2*p)+1] > 1000000.0) || (data[(2*p)+1] < -1000000.0)
        )
            countre++;
        if (
            (data[(2*p)+2] > 1000000.0) || (data[(2*p)+2] < -1000000.0)
        )
            countim++;

    }
    fprintf(stderr, "\n");
    fprintf(stderr, "\t Before FFT: countre = %d \t \t countim = %d\n", countre, countim);
    unsigned long nn[3];
    nn[1] = width;
    nn[2] = height;
    fourn(data, nn, 2, -1);
    countre = 0;
    countim = 0;
    for (int p = 0; p < ((width)*(height)); p++) {
        (*(array + p)).re = data[(2*p)+1];
        (*(array + p)).im = data[(2*p)+2];
        if (
            (data[(2*p)+1] > 1000000.0) || (data[(2*p)+1] < -1000000.0)
        )
            countre++;
        if (
            (data[(2*p)+2] > 1000000.0) || (data[(2*p)+2] < -1000000.0)
        )
            countim++;
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "\t After FFT: countre = %d \t \t countim = %d\n", countre, countim);
    free(data);

}

void compute_inverse_fft(zomplex *array, int width, int height)
{
    float *data = (float *) malloc((2 * width * height + 1) * sizeof(float));
    for (int p = 0; p < ((width)*(height)); p++) {
        data[(2*p)+1] = (*(array + p)).re;
        data[(2*p)+2] = (*(array + p)).im;
    }

    unsigned long nn[3];
    nn[1] = width;
    nn[2] = height;

    fourn(data, nn, 2, 1);

    for (int p = 0; p < ((width)*(height)); p++) {
        (*(array + p)).re = data[(2*p)+1];
        (*(array + p)).im = data[(2*p)+2];
    }
    free(data);

}

