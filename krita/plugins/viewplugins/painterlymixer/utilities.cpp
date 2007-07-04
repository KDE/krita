/*
 * painterlymixer.h -- Part of Krita
 *
 * Copyright (c) 2007 Boudewijn Rempt (boud@valdyas.org)
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
