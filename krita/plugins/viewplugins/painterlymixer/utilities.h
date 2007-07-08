/* This file is part of the KDE project
   Made by Emanuele Tamponi (emanuele@valinor.it)
   Copyright (C) 2007 Emanuele Tamponi

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

#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <KoColorConversions.h>

void addPainterlyOverlays(KisPaintDevice* dev);
void transmittanceToDensity(long T, long *D);
void densityToTransmittance(long D, long *T);
void rgbToCmy(long red, long green, long blue, long *cyan, long *magenta, long *yellow);
void cmyToRgb(long cyan, long magenta, long yellow, long *red, long *green, long *blue);


class Cell {
public:
    Cell()
        {
            canvasAdsorbency = 0;
            mixability = 0;
            pigmentConcentration = 0;
            reflectivity = 0;
            viscosity = 0;
            volume = 0;
            wetness = 0;
            setRgb(0, 0, 0);
        }

    Cell(const Cell &c)
        {
            canvasAdsorbency = c.canvasAdsorbency;
            mixability = c.mixability;
            pigmentConcentration = c.pigmentConcentration;
            reflectivity = c.reflectivity;
            viscosity = c.viscosity;
            volume = c.volume;
            wetness = c.wetness;
            setRgb(c.red, c.green, c.blue);
        }

    // Painterly properties
    float canvasAdsorbency;
    float mixability;
    float pigmentConcentration;
    float reflectivity;
    float viscosity;
    float volume;
    float wetness;

    // Color
    long red, green, blue;
    float hue, lightness, saturation;
    long cyan, magenta, yellow;

    quint8 opacity;

    void setRgb(long r, long g, long b)
        {
            red = r;
            green = g;
            blue = b;
            updateHlsCmy();
        }
    void setHls(float h, float l, float s)
        {
            hue = h;
            lightness = l;
            saturation = s;
            updateRgbCmy();
        }

    void setCmy(long c, long m, long y)
        {
            cyan = c;
            magenta = m;
            yellow = y;
            updateRgbHls();
        }

    void updateHlsCmy()
        {
            rgb_to_hls(red, green, blue, &hue, &lightness, &saturation);
            rgbToCmy(red, green, blue, &cyan, &magenta, &yellow);
        }

    void updateRgbCmy()
        {
            quint8 r, g, b;
            hls_to_rgb(hue, lightness, saturation, &r, &g, &b);
            red = (long)r;
            green = (long)g;
            blue = (long)b;
            rgbToCmy(red, green, blue, &cyan, &magenta, &yellow);
        }

    void updateRgbHls()
        {
            cmyToRgb(cyan, magenta, yellow, &red, &green, &blue);
            rgb_to_hls(red, green, blue, &hue, &lightness, &saturation);
        }

    void mixProperties(const Cell &cell, float force);

    void mixColorsUsingRgb(const Cell &cell, float force);

    void mixColorsUsingHls(const Cell &cell, float force);

    void mixColorsUsingCmy(const Cell &cell, float force);

    void debug();
};

#endif // UTILITIES_H_
