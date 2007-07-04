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
This implementations use
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

void Cell::mixUsingRgb(const Cell &cell)
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
}

/*
This implementation use Tunde Cockshott Wet&Sticky code.
*/
void Cell::mixUsingHls(const Cell &cell)
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

void Cell::mixUsingCmy(const Cell &cell)
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
