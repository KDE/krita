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
void transmittanceToDensity(int T, int *D);
void densityToTransmittance(int D, int *T);
void rgbToCmy(int red, int green, int blue, int *cyan, int *magenta, int *yellow);
void cmyToRgb(int cyan, int magenta, int yellow, int *red, int *green, int *blue);


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
    int red, green, blue;
    float hue, lightness, saturation;
    int cyan, magenta, yellow;

    quint8 opacity;

    void setRgb(int r, int g, int b)
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

    void setCmy(int c, int m, int y)
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
            red = (int)r;
            green = (int)g;
            blue = (int)b;
            rgbToCmy(red, green, blue, &cyan, &magenta, &yellow);
        }

    void updateRgbHls()
        {
            cmyToRgb(cyan, magenta, yellow, &red, &green, &blue);
            rgb_to_hls(red, green, blue, &hue, &lightness, &saturation);
        }

    void mixProperties(const Cell &cell, float force);

    void mixColorsUsingRgb(const Cell &cell, float force);

    void mixColorsUsingHls(const Cell &cell);

    void mixColorsUsingCmy(const Cell &cell);

    void debug();
};

#endif // UTILITIES_H_
