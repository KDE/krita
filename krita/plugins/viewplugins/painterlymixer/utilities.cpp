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

#include <QList>

#include "kis_paint_device.h"
#include "kis_adsorbency_mask.h"
#include "kis_mixability_mask.h"
#include "kis_pigment_concentration_mask.h"
#include "kis_reflectivity_mask.h"
#include "kis_volume_mask.h"
#include "kis_viscosity_mask.h"
#include "kis_wetness_mask.h"

#include "utilities.h"

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
    double d;
    if (T == 0)
        d = 3.0;
    else
        d = -log10((double)T/255.0);
    d = d * 1024.0/3.0;
    *D = (long)(d + 0.5);
}

/*
This implementation uses
Zimmer, System and method for digital rendering of images and printed articulation, 1994
*/
void densityToTransmittance(long D, long *T)
{
    double d;
    d = 255.0 * pow(M_E, - (double)D * 3.0/1024.0);
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

float m_rgb_xyz[3][3] = { 0.576700,   0.297361,   0.0270328,
                          0.185556,   0.627355,   0.0706879,
                          0.188212,   0.0752847,  0.991248 };

float m_xyz_rgb[3][3] = {  2.04148,   -0.969258,   0.0134455,
                          -0.564977,   1.87599,   -0.118373,
                          -0.344713,   0.0415557,  1.01527 };

void rgbToXyz(float r, float g, float b, float *x, float *y, float *z)
{
    *x = m_rgb_xyz[0][0] * r + m_rgb_xyz[0][1] * g + m_rgb_xyz[0][2] * b;
    *y = m_rgb_xyz[1][0] * r + m_rgb_xyz[1][1] * g + m_rgb_xyz[1][2] * b;
    *z = m_rgb_xyz[2][0] * r + m_rgb_xyz[2][1] * g + m_rgb_xyz[2][2] * b;
}

void xyzToRgb(float x, float y, float z, float *r, float *g, float *b)
{
    *r = m_xyz_rgb[0][0] * x + m_xyz_rgb[0][1] * y + m_xyz_rgb[0][2] * z;
    *g = m_xyz_rgb[1][0] * x + m_xyz_rgb[1][1] * y + m_xyz_rgb[1][2] * z;
    *b = m_xyz_rgb[2][0] * x + m_xyz_rgb[2][1] * y + m_xyz_rgb[2][2] * z;
}

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
}

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
