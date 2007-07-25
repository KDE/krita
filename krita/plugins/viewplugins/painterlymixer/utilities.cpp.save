/* This file is part of the KDE project
   Made by Emanuele Tamponi (emanuele@valinor.it)
   Copyright (C) 2007 Emanuele Tamponi
   Copyright (C) 1994 Mark A. Zimmer
   Copyright (C) 1991 Tunde Cockshott

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <cmath>
#include <cstdlib>
#include <iostream>

#include <QList>

#include <gmm/gmm.h>
#include <lcms.h>

#include "kis_paint_device.h"
#include "kis_adsorbency_mask.h"
#include "kis_mixability_mask.h"
#include "kis_pigment_concentration_mask.h"
#include "kis_reflectivity_mask.h"
#include "kis_volume_mask.h"
#include "kis_viscosity_mask.h"
#include "kis_wetness_mask.h"

#include "utilities.h"

using namespace std;

#define THICKNESS 1
#define C_SIZE 10
#define MAX_RATIO 1000000000

void addPainterlyOverlays(KisPaintDevice* dev)
{
    dev->addPainterlyChannel(new KisAdsorbencyMask(dev));
    dev->addPainterlyChannel(new KisMixabilityMask(dev));
    dev->addPainterlyChannel(new KisPigmentConcentrationMask(dev));
    dev->addPainterlyChannel(new KisReflectivityMask(dev));
    dev->addPainterlyChannel(new KisVolumeMask(dev));
    dev->addPainterlyChannel(new KisViscosityMask(dev));
    dev->addPainterlyChannel(new KisWetnessMask(dev));
}

/*
This implementation uses
Zimmer, System and method for digital rendering of images and printed articulation, 1994
*/
void transmittanceToDensity(long T, long *D)
{
    float d;
    if (T == 0)
        d = 3.0;
    else
        d = -log10((float)T/255.0);
    d = d * 1024.0/3.0;
    *D = (long)(d + 0.5);
}

/*
This implementation uses
Zimmer, System and method for digital rendering of images and printed articulation, 1994
*/
void densityToTransmittance(long D, long *T)
{
    float d;
    d = 255.0 * pow(M_E, - (float)D * 3.0/1024.0);
    if (d < 0.0)
        d = 0.0;
    if (d > 255.0)
        d = 255.0;
    *T = (long)(d + 0.5);
}

void rgbToCmy(long red, long green, long blue, long *cyan, long *magenta, long *yellow)
{
    transmittanceToDensity(red, cyan);
    transmittanceToDensity(green, magenta);
    transmittanceToDensity(blue, yellow);
}

void cmyToRgb(long cyan, long magenta, long yellow, long *red, long *green, long *blue)
{
    densityToTransmittance(cyan, red);
    densityToTransmittance(magenta, green);
    densityToTransmittance(yellow, blue);
}

float g_REF_XYZ[3][C_SIZE] = {
	{ 0.00332596, 0.168329, 0.0977301, 0.12178, 0.502248, 0.45424, 0.0717572, 0.00266851, 8.2916e-05, 3.38869e-06},
	{ 0.000347754, 0.0275165, 0.169206, 0.502836, 0.539392, 0.230916, 0.0287143, 0.00103668, 3.2496e-05, 1.3527e-06},
	{ 0.0150249, 0.853996, 0.697775, 0.0424252, 0.000349115, 0, 0, 0, 0, 0}
};

// float g_REF_XYZ[3][C_SIZE] = {
// { 4.11299e-05, 0.0173842, 0.115461, 0.122296, 0.014674, 0.0484702, 0.194445, 0.341635, 0.350195, 0.171503, 0.0401296, 0.00526804, 0.000584564, 6.92151e-05, 9.28218e-06, 1.42837e-06},
// { 4.44151e-06, 0.00179642, 0.0166633, 0.0604853, 0.145993, 0.293315, 0.378223, 0.317161, 0.193719, 0.0744568, 0.015876, 0.00204819, 0.000227354, 2.71097e-05, 3.6747e-06, 5.72847e-07},
// { 0.00018183, 0.0802866, 0.579909, 0.718306, 0.198684, 0.0293027, 0.00290049, 0, 0, 0, 0, 0, 0, 0, 0, 0}
// };

float sigmoid(float value)
{
    //TODO return a sigmoid in [0, 1] here
    // TESTED ONLY WITH MOUSE!
    if (value == 0.5)
        return value + 0.3;
    else
        return value;
}

float activeVolume(float volume, float wetness, float force)
{
    return volume * sigmoid(wetness) * sigmoid(force);
}

void Cell::mixProperties(const Cell &cell, float force)
{
    float V_c, V_s; // Volumes in Canvas and Stroke
    float w_c, w_s; // Wetness in the Canvas and in the Stroke
    float o_c, o_s; // Opacities
    float V_ac, V_as; // Active Volumes in Canvas and Stroke
    float a; // Adsorbency
    float V_f, w_f, o_f; // Finals

    V_c = cell.volume;
    V_s = volume;

    w_c = cell.wetness;
    w_s = wetness;

    o_c = (float)cell.opacity / 255.0;
    o_s = (float)opacity / 255.0;

    a = cell.canvasAdsorbency;

    V_ac = activeVolume(V_c, w_c, force);
    V_as = activeVolume(V_s, w_s, force);

    w_f = (V_ac * w_c + V_as * w_s) / (V_ac + V_as);
    o_f = (V_ac * o_c + V_as * o_s) / (V_ac + V_as);
    V_f = ((1 - a) * V_c) + V_as;

    if (V_f > 255.0)
        V_f = 255.0;

    // Normalize
    o_f = 255.0 * o_f;

    wetness = w_f;
    opacity = (quint8)o_f;
    volume = V_f;
}

float coth_utils(float z)
{
    return ( cosh(z) / sinh(z) );
}

float acoth_utils(float z)
{
    return 0.5*log((1 + 1/z) / (1 - 1/z));
}

void Cell::mixColorsUsingKS(const Cell &cell, float force)
{
    float V_c, V_s; // Volumes in Canvas and Stroke
    float w_c, w_s; // Wetness in the Canvas and in the Stroke
    float V_ac, V_as; // Active Volumes in Canvas and Stroke

    V_c = cell.volume;
    V_s = volume;

    w_c = cell.wetness;
    w_s = wetness;

    V_ac = activeVolume(V_c, w_c, force);
    V_as = activeVolume(V_s, w_s, force);

    vector<float> c(6), s(6), f(3);
    float a_c, b_c/*, c_c*/;
    float a_s, b_s/*, c_s*/;
    float /*a_f, b_f,*/ c_f;
    float S_c, S_s, S_f;
    float K_c, K_s, K_f;
    float R_f, T_f;

    c[0] = (float)cell.red/255;
    c[1] = (float)cell.green/255;
    c[2] = (float)cell.blue/255;

    s[0] = (float)red/255;
    s[1] = (float)green/255;
    s[2] = (float)blue/255;

    for (int i = 0; i < 3; i++) {
        if (c[i] == 0.0 && s[i] == 0.0) {
            f[i] = 0;
            continue;
        }

        if (c[i] == 1.0) c[i] -= 0.4/255.0;
        if (c[i] == 0.0) c[i] += 0.4/255.0;
        if (s[i] == 1.0) s[i] -= 0.4/255.0;
        if (s[i] == 0.0) s[i] += 0.4/255.0;

        c[i+3] = c[i] - (0.4/255.0)*c[i];
        s[i+3] = s[i] - (0.4/255.0)*s[i];

        a_c = 0.5*( c[i] + ( c[i+3] - c[i] + 1) / c[i+3] );
        a_s = 0.5*( s[i] + ( s[i+3] - s[i] + 1) / s[i+3] );
        b_c = sqrt( pow( a_c, 2 ) - 1 );
        b_s = sqrt( pow( a_s, 2 ) - 1 );

        S_c = ( 1 / b_c ) * acoth_utils( ( pow( b_c, 2 ) - ( a_c - c[i] ) * ( a_c - 1 ) ) / ( b_c * ( 1 - c[i] ) ) );
        S_s = ( 1 / b_s ) * acoth_utils( ( pow( b_s, 2 ) - ( a_s - s[i] ) * ( a_s - 1 ) ) / ( b_s * ( 1 - s[i] ) ) );

        K_c = S_c * ( a_c - 1 );
        K_s = S_s * ( a_s - 1 );

        S_f = ( V_ac * S_c + V_as * S_s ) / ( V_ac + V_as );
        K_f = ( V_ac * K_c + V_as * K_s ) / ( V_ac + V_as );

        c_f = sqrt( ( K_f / S_f ) * ( K_f / S_f + 2 ) );

        R_f = 1 / ( 1 + ( K_f / S_f ) + c_f * coth_utils( c_f * S_f * THICKNESS ) );
        T_f = c_f * R_f * ( 1 / sinh( c_f * S_f * THICKNESS ) );

        f[i] = R_f + T_f;

/* Curtis et al. idea
        c_c = a_c * sinh( b_c * S_c * THICKNESS ) + b_c * cosh( b_c * S_c * THICKNESS );
        c_s = a_s * sinh( b_s * S_s * THICKNESS ) + b_s * cosh( b_s * S_s * THICKNESS );

        a_f = ( V_ac * a_c + V_as * a_s ) / ( V_ac + V_as );
        b_f = ( V_ac * b_c + V_as * b_s ) / ( V_ac + V_as );
        c_f = ( V_ac * c_c + V_as * c_s ) / ( V_ac + V_as );

        S_f = ( V_ac * S_c + V_as * S_s ) / ( V_ac + V_as );

        f[i]  = sinh( b_f * S_f * THICKNESS ) / c_f;
        f[i] += b_f / c_f;
*/

/* Implement mixing as glazing (Curtis et al.)

        R_c = sinh( b_c * S_c ) / c_c;
        T_c = b_c / c_c;

        R_s = sinh( b_s * S_s ) / c_s;
        T_s = b_s / c_s;

        R_f = R_s + ( pow( T_s, 2 ) * R_c ) / ( 1 - R_s * R_c );
        T_f = ( T_s * T_c ) / ( 1 - R_s * R_c );

        kDebug() << "Channel " << i + 1 << " CANVAS R: " << R_c << " T: " << T_c << endl;
        kDebug() << "Channel " << i + 1 << " STROKE R: " << R_s << " T: " << T_s << endl;
        kDebug() << "Channel " << i + 1 << " FINAL  R: " << R_f << " T: " << T_f << endl << "------------------------" << endl;
*/

        if (f[i] < 0) f[i] = 0; if (f[i] > 1) f[i] = 1;
    }

    red = (long) (f[0]*255);
    green = (long) (f[1]*255);
    blue = (long) (f[2]*255);

    updateHlsCmy();
}

void simplex_utilities(const gmm::dense_matrix<float> &M_1, vector<float> &X, const vector<float> &B)
{
/*
	const int rows = 6 + C_SIZE + 1;
	const int cols = C_SIZE*2 + 3 + 2;
	gmm::dense_matrix<float> M(rows, cols);

	gmm::clear(M);

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < C_SIZE; j++) {
			M(i, j) = M_1(i, j);
			M(i+3, j) = M_1(i, j);
		}
		M(i, i+C_SIZE) = 1;
		M(i+3, i+C_SIZE) = -1;
		M(i, cols-1) = M(i+3, cols-1) = B[i];
	}

	for (int i = 6; i < C_SIZE + 6; i++) {
		M(i, i - 6) = M(i, (i+C_SIZE+3) - 6) = 1;
		M(i, cols-1) = 1;
	}

	for (int j = 0; j < C_SIZE; j++)
		M(rows-1, j) = -1;

	M(rows-1, cols-2) = 1;
*/
	const int rows = 3 + C_SIZE + 1;
	const int cols = C_SIZE + C_SIZE + 1 + 1;
	gmm::dense_matrix<float> M(rows, cols);

	gmm::clear(M);

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < C_SIZE; j++) {
			M(i, j) = M_1(i, j);
		}
		M(i, cols-1) = B[i];
	}

// 	for (int i = 3; i < rows - 1; i++)
// 		M(i, i-3) = M(i, (i+C_SIZE)-3) = M(i, cols-1) = 1;

	for (int i = 3; i < rows - 1; i++) {
		M(i, i-3) = M(i, (i+C_SIZE)-3) = 1;
		M(i, cols-1) = 1;
	}

	for (int j = 0; j < C_SIZE; j++)
		M(rows-1, j) = -1;

	M(rows-1, cols-2) = 1;

// 	cout << M << endl << "           -------------------------------------------           " << endl;
// 	string s;
// 	cin >> s;

	while ( true ) {
		float min = 0.0;
		float ratio, ratio_min = MAX_RATIO;
		int i_pivot = -1, j_pivot = -1;
		for (int j = 0; j < cols-1; j++) {
			if (M(rows-1, j) < min) {
				for (int i = 0; i < rows-1; i++) {
					if (M(i, j) <= 0)
						continue;
					ratio = M(i, cols-1) / M(i, j);
					if ( ratio <= ratio_min ) {
						ratio_min = ratio;
						min = M(rows-1, j);
						i_pivot = i;
						j_pivot = j;
					}
				}
			}
		}

		if (min == 0.0)
			break;

// 		cout << "PIVOT (" << i_pivot << "," << j_pivot << "): " << M(i_pivot, j_pivot) << endl;

		for (int i = 0; i < rows; i++) {
			if (i == i_pivot || M(i, j_pivot) == 0.0)
				continue;
			float sour = M(i_pivot, j_pivot);
			float dest = M(i, j_pivot);
			for (int j = 0; j < cols; j++) {
				if (j == j_pivot) {
					M(i, j) = 0;
					continue;
				}
				M(i, j) = sour * M(i, j) - dest * M(i_pivot, j);
			}
		}
// 		cout << M << endl;
// 		cin >> s;
	}

	for (int j = 0; j < C_SIZE; j++) {
		int count = 0, i_useful;
		for (int i = 0; i < rows; i++) {
			if (M(i, j) != 0.0) {
				i_useful = i;
				count++;
			}
		}
		if (count != 1)
			X[j] = 0;
		else
			X[j] = M(i_useful, cols-1) / M(i_useful, j);
	}
/*
	const int slack_size = cols - C_SIZE - 1;
	vector<float> slack(slack_size);
	for (int j = C_SIZE; j < C_SIZE + slack_size; j++) {
		int count = 0, i_useful;
		for (int i = 0; i < rows; i++) {
			if (M(i, j) != 0.0) {
				i_useful = i;
				count++;
			}
		}
		if (count != 1)
			slack[j-C_SIZE] = 0;
		else
			slack[j-C_SIZE] = M(i_useful, cols-1) / M(i_useful, j);
	}
*/
// 	cout << "SLACK: " << slack << endl;

// 	cout << "RESULT: " << X << endl;
}

void Cell::mixColorsUsingKSXyz(const Cell &cell, float force)
{
    float V_c, V_s; // Volumes in Canvas and Stroke
    float w_c, w_s; // Wetness in the Canvas and in the Stroke
    float V_ac, V_as; // Active Volumes in Canvas and Stroke

    V_c = cell.volume;
    V_s = volume;

    w_c = cell.wetness;
    w_s = wetness;

    V_ac = activeVolume(V_c, w_c, force);
    V_as = activeVolume(V_s, w_s, force);

	gmm::dense_matrix<float> REF_XYZ(3, C_SIZE);

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < C_SIZE; j++)
			REF_XYZ(i, j) = g_REF_XYZ[i][j];

	float RGB[3], XYZ[3];
	vector<float> vREF1(C_SIZE), vREF2(C_SIZE), vXYZ(3);

	cmsHPROFILE hsRGB = cmsCreate_sRGBProfile();
    cmsHPROFILE hXYZ  = cmsCreateXYZProfile();

    cmsHTRANSFORM RGB_XYZ = cmsCreateTransform(hsRGB, TYPE_RGB_DBL, hXYZ, TYPE_XYZ_DBL,
                                               INTENT_ABSOLUTE_COLORIMETRIC, cmsFLAGS_NOTPRECALC);
    cmsHTRANSFORM XYZ_RGB = cmsCreateTransform(hXYZ, TYPE_XYZ_DBL, hsRGB, TYPE_RGB_DBL,
                                               INTENT_ABSOLUTE_COLORIMETRIC, cmsFLAGS_NOTPRECALC);

	RGB[0] = (float)cell.red/255;
	RGB[1] = (float)cell.green/255;
	RGB[2] = (float)cell.blue/255;
    cmsDoTransform(RGB_XYZ, RGB, XYZ, 1);

	for (int i = 0; i < 3; i++)
		vXYZ[i] = XYZ[i];

	simplex_utilities(REF_XYZ, vREF1, vXYZ);

	RGB[0] = (float)red/255;
	RGB[1] = (float)green/255;
	RGB[2] = (float)blue/255;
    cmsDoTransform(RGB_XYZ, RGB, XYZ, 1);

	for (int i = 0; i < 3; i++)
		vXYZ[i] = XYZ[i];

	simplex_utilities(REF_XYZ, vREF2, vXYZ);

	for (int i = 0; i < C_SIZE; i++) {

		if (vREF1[i] > 0.99999) vREF1[i] = 1.0 - (1.0/10000.0);
		if (vREF2[i] > 0.99999) vREF2[i] = 1.0 - (1.0/10000.0);
		if (vREF1[i] < 0.00001) vREF1[i] = (1.0/10000.0);
		if (vREF2[i] < 0.00001) vREF2[i] = (1.0/10000.0);
		float c_b = vREF1[i] - (1.0/100.0)*vREF1[i];
        float s_b = vREF2[i] - (1.0/100.0)*vREF2[i];

        float a_c = 0.5*( vREF1[i] + ( c_b - vREF1[i] + 1) / c_b );
        float a_s = 0.5*( vREF2[i] + ( s_b - vREF2[i] + 1) / s_b );
        float b_c = sqrt( pow( a_c, 2 ) - 1 );
        float b_s = sqrt( pow( a_s, 2 ) - 1 );

        float S_c = ( 1 / b_c ) * acoth_utils( ( pow( b_c, 2 ) - ( a_c - vREF1[i] ) * ( a_c - 1 ) ) / ( b_c * ( 1 - vREF1[i] ) ) );
        float S_s = ( 1 / b_s ) * acoth_utils( ( pow( b_s, 2 ) - ( a_s - vREF2[i] ) * ( a_s - 1 ) ) / ( b_s * ( 1 - vREF2[i] ) ) );

        float K_c = S_c * ( a_c - 1 );
        float K_s = S_s * ( a_s - 1 );

        float S_f = ( V_ac * S_c + V_as * S_s ) / ( V_ac + V_as );
        float K_f = ( V_ac * K_c + V_as * K_s ) / ( V_ac + V_as );
// 		float S_f = 0.5 * S_c + 0.5 * S_s;
// 		float K_f = 0.5 * K_c + 0.5 * K_s;

        float c_f = sqrt( ( K_f / S_f ) * ( K_f / S_f + 2 ) );

        float R_f = 1 / ( 1 + ( K_f / S_f ) + c_f * coth_utils( c_f * S_f * THICKNESS ) );
//         float T_f = c_f * R_f * ( 1 / sinh( c_f * S_f * THICKNESS ) );

		vREF1[i] = R_f; // + T_f;

// 		vREF1[i] = ( V_ac * vREF1[i] + V_as * vREF2[i] ) / ( V_ac + V_as );
	}

	gmm::mult(REF_XYZ, vREF1, vXYZ);
	for (int i = 0; i < 3; i++)
		XYZ[i] = vXYZ[i];
	cmsDoTransform(XYZ_RGB, XYZ, RGB, 1);

    red = (long) (RGB[0]*255);
    green = (long) (RGB[1]*255);
    blue = (long) (RGB[2]*255);

    updateHlsCmy();

	cmsDeleteTransform(RGB_XYZ);
	cmsDeleteTransform(XYZ_RGB);
	cmsCloseProfile(hsRGB);
	cmsCloseProfile(hXYZ);
}
/*
void Cell::mixColorsUsingXyz(const Cell &cell, float force)
{
    float V_c, V_s; // Volumes in Canvas and Stroke
    float w_c, w_s; // Wetness in the Canvas and in the Stroke
    float V_ac, V_as; // Active Volumes in Canvas and Stroke

    V_c = cell.volume;
    V_s = volume;

    w_c = cell.wetness;
    w_s = wetness;

    V_ac = activeVolume(V_c, w_c, force);
    V_as = activeVolume(V_s, w_s, force);

    float r_c, g_c, b_c;
    float r_s, g_s, b_s;
    float r_f, g_f, b_f;

    float X_c, x_c, Y_c, y_c, Z_c;
    float X_s, x_s, Y_s, y_s, Z_s;
    float X_f, x_f, Y_f, y_f, Z_f;

    r_c = (float)cell.red;
    g_c = (float)cell.green;
    b_c = (float)cell.blue;

    r_s = (float)red;
    g_s = (float)green;
    b_s = (float)blue;

    rgbToXyz(r_c, g_c, b_c, &X_c, &Y_c, &Z_c);
    rgbToXyz(r_s, g_s, b_s, &X_s, &Y_s, &Z_s);

    x_c = X_c / (X_c + Y_c + Z_c);
    y_c = Y_c / (X_c + Y_c + Z_c);

    x_s = X_s / (X_s + Y_s + Z_s);
    y_s = Y_s / (X_s + Y_s + Z_s);

    // Luminance is the sum of luminances
    Y_f = (V_ac * Y_c + V_as * Y_s) / (V_ac + V_as);

    x_f = (V_ac * x_c + V_as * x_s) / (V_ac + V_as);
    y_f = (V_ac * y_c + V_as * y_s) / (V_ac + V_as);

    X_f = (Y_f / y_f) * x_f;
    Z_f = (Y_f / y_f) * (1 - x_f - y_f);

    xyzToRgb(X_f, Y_f, Z_f, &r_f, &g_f, &b_f);

    red = (long) ((long)(r_f) < 256 ? r_f : 255);
    green = (long) ((long)(g_f) < 256 ? g_f : 255);
    blue = (long) ((long)(b_f) < 256 ? b_f : 255);

    updateHlsCmy();
}
*/
void Cell::mixColorsUsingRgb_2(const Cell &cell, float force)
{
    float V_c, V_s; // Volumes in Canvas and Stroke
    float w_c, w_s; // Wetness in the Canvas and in the Stroke
    float V_ac, V_as; // Active Volumes in Canvas and Stroke

    float r_c, r_s;
    float g_c, g_s;
    float b_c, b_s;
    float r_f, g_f, b_f;

    V_c = cell.volume;
    V_s = volume;

    w_c = cell.wetness;
    w_s = wetness;

    V_ac = activeVolume(V_c, w_c, force);
    V_as = activeVolume(V_s, w_s, force);

    r_c = (float)cell.red / 255.0;
    g_c = (float)cell.green / 255.0;
    b_c = (float)cell.blue / 255.0;

    r_s = (float)red / 255.0;
    g_s = (float)green / 255.0;
    b_s = (float)blue / 255.0;

#define GREEN_TO_RED 0.5
#define RED_TO_GREEN 0.2
#define GREEN_TO_BLUE 0.5
#define BLUE_TO_GREEN 0.15

    float ratio = V_as / V_ac;
    float delta;
    // Green is near enough to red so that a great amount of green changes the amount of red
    r_f = (V_ac * r_c + V_as * r_s) / (V_ac + V_as);
    delta = (g_c - g_s) * GREEN_TO_RED;
    r_f += ratio*delta;

    g_f = (V_ac * g_c + V_as * g_s) / (V_ac + V_as);
    delta = (r_c - r_s) * RED_TO_GREEN + (b_c - b_s) * BLUE_TO_GREEN;
    g_f += ratio*delta;

    b_f = (V_ac * b_c + V_as * b_s) / (V_ac + V_as);
    delta = (g_c - g_s) * GREEN_TO_BLUE;
    b_f += ratio*delta;

    if (r_f < 0) r_f = 0; if (r_f > 1) r_f = 1;
    if (g_f < 0) g_f = 0; if (g_f > 1) g_f = 1;
    if (b_f < 0) b_f = 0; if (b_f > 1) b_f = 1;

//     Normalize and set
    red = (long)(r_f*255);
    green = (long)(g_f*255);
    blue = (long)(b_f*255);

//     kDebug() << "RED: " << red << " GREEN: " << green << " BLUE: " << blue << endl;

    updateHlsCmy();
}

void Cell::mixColorsUsingRgbAdditive(const Cell &cell, float force)
{
    float V_c, V_s; // Volumes in Canvas and Stroke
    float w_c, w_s; // Wetness in the Canvas and in the Stroke
    float V_ac, V_as; // Active Volumes in Canvas and Stroke

    float r_c, r_s;
    float g_c, g_s;
    float b_c, b_s;
    float r_f, g_f, b_f;

    V_c = cell.volume;
    V_s = volume;

    w_c = cell.wetness;
    w_s = wetness;

    V_ac = activeVolume(V_c, w_c, force);
    V_as = activeVolume(V_s, w_s, force);

    r_c = (float)cell.red / 255.0;
    g_c = (float)cell.green / 255.0;
    b_c = (float)cell.blue / 255.0;

    r_s = (float)red / 255.0;
    g_s = (float)green / 255.0;
    b_s = (float)blue / 255.0;

    r_f = r_c + r_s;
    r_f = g_c + g_s;
    r_f = b_c + b_s;

    if (r_f < 0) r_f = 0; if (r_f > 1) r_f = 1;
    if (g_f < 0) g_f = 0; if (g_f > 1) g_f = 1;
    if (b_f < 0) b_f = 0; if (b_f > 1) b_f = 1;

//     Normalize and set
    red = (long)(r_f*255);
    green = (long)(g_f*255);
    blue = (long)(b_f*255);

//     kDebug() << "RED: " << red << " GREEN: " << green << " BLUE: " << blue << endl;

    updateHlsCmy();
}

void Cell::mixColorsUsingRgb(const Cell &cell, float force)
{
    float V_c, V_s; // Volumes in Canvas and Stroke
    float w_c, w_s; // Wetness in the Canvas and in the Stroke
    float V_ac, V_as; // Active Volumes in Canvas and Stroke

    float r_c, r_s;
    float g_c, g_s;
    float b_c, b_s;
    float r_f, g_f, b_f;

    V_c = cell.volume;
    V_s = volume;

    w_c = cell.wetness;
    w_s = wetness;

    V_ac = activeVolume(V_c, w_c, force);
    V_as = activeVolume(V_s, w_s, force);

    r_c = (float)cell.red / 255.0;
    g_c = (float)cell.green / 255.0;
    b_c = (float)cell.blue / 255.0;

    r_s = (float)red / 255.0;
    g_s = (float)green / 255.0;
    b_s = (float)blue / 255.0;

    r_f = (V_ac * r_c + V_as * r_s) / (V_ac + V_as);
    g_f = (V_ac * g_c + V_as * g_s) / (V_ac + V_as);
    b_f = (V_ac * b_c + V_as * b_s) / (V_ac + V_as);

//     Normalize and set
    red = (long)(r_f*255.0);
    green = (long)(g_f*255.0);
    blue = (long)(b_f*255.0);

    updateHlsCmy();
}

/*
This implementation use Tunde Cockshott Wet&Sticky code.
*/
void Cell::mixColorsUsingHls(const Cell &cell, float)
{
    float ratio, delta;
    ratio = volume / cell.volume;
    delta = hue - cell.hue;
    if ((int)delta != 0) {
        hue = cell.hue + (int)(ratio * delta);
        if (hue >= 360)
            hue -= 360;
    }

    delta = lightness - cell.lightness;
    lightness = cell.lightness + ratio * delta;

    delta = saturation - cell.saturation;
    saturation = cell.saturation + ratio * delta;

    updateRgbCmy();
}

void Cell::mixColorsUsingCmy(const Cell &cell, float)
{
    float ratio;
    int delta;

    ratio = (wetness*volume) / (cell.wetness*cell.volume);
    delta = cyan - cell.cyan;
    cyan = cell.cyan + (long)(ratio * delta);

    delta = magenta - cell.magenta;
    magenta = cell.magenta + (long)(ratio * delta);

    delta = yellow - cell.yellow;
    yellow = cell.yellow + (long)(ratio * delta);

    updateRgbHls();
}

void Cell::debug()
{
    kDebug(41006) << "WETNESS: " << wetness << endl
             << "VOLUME: " << volume << endl
             << "OPACITY: " << (int)opacity << endl
             << "RED: " << red << " GREEN: " << green << " BLUE: " << blue << endl;
}
