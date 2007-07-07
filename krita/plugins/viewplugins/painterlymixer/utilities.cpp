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
void transmittanceToDensity(int T, int *D)
{
    double d;
    if (T == 0)
        d = 3.0;
    else
        d = -log10((double)T/255.0);
    d = d * 1024.0/3.0;
    *D = (int)(d + 0.5);
}

/*
This implementation uses
Zimmer, System and method for digital rendering of images and printed articulation, 1994
*/
void densityToTransmittance(int D, int *T)
{
    double d;
    d = 255.0 * pow(10.0, - (double)D * 3.0/1024.0);
    if (d < 0.0)
        d = 0.0;
    if (d > 255.0)
        d = 255.0;
    *T = (int)(d + 0.5);
}

void rgbToCmy(int red, int green, int blue, int *cyan, int *magenta, int *yellow)
{
    transmittanceToDensity(red, cyan);
    transmittanceToDensity(green, magenta);
    transmittanceToDensity(blue, yellow);
}

void cmyToRgb(int cyan, int magenta, int yellow, int *red, int *green, int *blue)
{
    densityToTransmittance(cyan, red);
    densityToTransmittance(magenta, green);
    densityToTransmittance(yellow, blue);
}

float sigmoid(float value)
{
    //TODO return a sigmoid in [0, 1] here
    // TESTED ONLY WITH MOUSE!
    if (value == 0.5)
        return value + 0.8;
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
    o_f = 255 * o_f;

    wetness = w_f;
    opacity = (quint8)o_f;
    volume = V_f;
}

/*
void Cell::mixColorsUsingRgb(const Cell &cell, float)
{
    float ratio;
    int delta;

    ratio = wetness*volume / cell.wetness*cell.volume;
    delta = red - cell.red;
    red = cell.red + (int)(ratio * delta);

    delta = green - cell.green;
    green = cell.green + (int)(ratio * delta);

    delta = blue - cell.blue;
    blue = cell.blue + (int)(ratio * delta);

    updateHlsCmy();
}*/


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
    red = (int)(r_f*255.0);
    green = (int)(g_f*255.0);
    blue = (int)(b_f*255.0);
}

/*
This implementation use Tunde Cockshott Wet&Sticky code.
*/
void Cell::mixColorsUsingHls(const Cell &cell)
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

void Cell::mixColorsUsingCmy(const Cell &cell)
{
    float ratio;
    int delta;

    ratio = wetness*volume / cell.wetness*cell.volume;
    delta = cyan - cell.cyan;
    cyan = cell.cyan + (int)(ratio * delta);

    delta = magenta - cell.magenta;
    magenta = cell.magenta + (int)(ratio * delta);

    delta = yellow - cell.yellow;
    yellow = cell.yellow + (int)(ratio * delta);

    updateRgbHls();
}

void Cell::debug()
{
    kDebug(41006) << "WETNESS: " << wetness << endl
             << "VOLUME: " << volume << endl
             << "OPACITY: " << (int)opacity << endl
             << "RED: " << red << " GREEN: " << green << " BLUE: " << blue << endl;
}
