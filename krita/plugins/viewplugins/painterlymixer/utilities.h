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
void transmittance_to_density(int T, int *D);
void density_to_transmittance(int D, int *T);
void rgb_to_cmy(int red, int green, int blue, int *cyan, int *magenta, int *yellow);
void cmy_to_rgb(int cyan, int magenta, int yellow, int *red, int *green, int *blue);


class Cell {
public:
    Cell()
        {
            cadsorb = 0;
            mixabil = 0;
            pig_con = 0;
            reflect = 0;
            pviscos = 0;
            pvolume = 0;
            wetness = 0;
            set_rgb(0, 0, 0);
        }

    Cell(const Cell &c)
        {
            cadsorb = c.cadsorb;
            mixabil = c.mixabil;
            pig_con = c.pig_con;
            reflect = c.reflect;
            pvolume = c.pvolume;
            wetness = c.wetness;
            set_rgb(c.r, c.g, c.b);
        }

    // Painterly properties
    float cadsorb;
    float mixabil;
    float pig_con;
    float reflect;
    float pviscos;
    float pvolume;
    float wetness;

    // Color
    int r, g, b;
    float h, l, s;
    int c, m, y;

    quint8 opacity;

    void set_rgb(int red, int green, int blue)
        {
            r = red;
            g = green;
            b = blue;
            update_hls_cmy();
        }
    void set_hls(float hue, float lightness, float saturation)
        {
            h = hue;
            l = lightness;
            s = saturation;
            update_rgb_cmy();
        }

    void set_cmy(int cyan, int magenta, int yellow)
        {
            c = cyan;
            m = magenta;
            y = yellow;
            update_rgb_hls();
        }

    void update_hls_cmy()
        {
            rgb_to_hls(r, g, b, &h, &l, &s);
            rgb_to_cmy(r, g, b, &c, &m, &y);
        }

    void update_rgb_cmy()
        {
            quint8 red, green, blue;
            hls_to_rgb(h, l, s, &red, &green, &blue);
            r = (int)red;
            g = (int)green;
            b = (int)blue;
            rgb_to_cmy(r, g, b, &c, &m, &y);
        }

    void update_rgb_hls()
        {
            cmy_to_rgb(c, m, y, &r, &g, &b);
            rgb_to_hls(r, g, b, &h, &l, &s);
        }

    void mix_using_rgb(const Cell &canvas_cell)
        {

            float ratio;
            int delta;

            ratio = wetness*pvolume / canvas_cell.wetness*canvas_cell.pvolume;
            delta = r - canvas_cell.r;
            r = canvas_cell.r + (int)(ratio * delta);

            delta = g - canvas_cell.g;
            g = canvas_cell.g + (int)(ratio * delta);

            delta = b - canvas_cell.b;
            b = canvas_cell.b + (int)(ratio * delta);

            update_hls_cmy();
        }

    void mix_using_hls(const Cell &canvas_cell)
        {

            float ratio, delta;
            ratio = pvolume / canvas_cell.pvolume;
            delta = h - canvas_cell.h;
            if ((int)delta != 0) {
                h = canvas_cell.h + (int)(ratio * delta);
                if (h >= 360)
                    h -= 360;
            }

            delta = l - canvas_cell.l;
            l = canvas_cell.l + ratio * delta;

            delta = s - canvas_cell.s;
            s = canvas_cell.s + ratio * delta;

            update_rgb_cmy();
        }

    void mix_using_cmy(const Cell &canvas_cell)
        {

            float ratio;
            int delta;

            ratio = pvolume / canvas_cell.pvolume;
            delta = c - canvas_cell.c;
            c = canvas_cell.c + (int)(ratio * delta);

            delta = m - canvas_cell.m;
            m = canvas_cell.m + (int)(ratio * delta);

            delta = y - canvas_cell.y;
            y = canvas_cell.y + (int)(ratio * delta);
/*
            c += canvas_cell.c; c /= 2;
            m += canvas_cell.m; m /= 2;
            y += canvas_cell.y; y /= 2;
*/
            update_rgb_hls();
        }
};

#endif // UTILITIES_H_
