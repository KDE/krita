/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <cmath>

#include <qglobal.h>

#include "KoColorConversions.h"

/**
 * A number of often-used conversions between color models
 */

void rgb_to_hsv(int R, int G, int B, int *H, int *S, int *V)
{
    unsigned int max = R;
    unsigned int min = R;
    unsigned char maxValue = 0; // r = 0, g = 1, b = 2

    // find maximum and minimum RGB values
    if (static_cast<unsigned int>(G) > max) {
        max = G;
        maxValue = 1;
    }

    if (static_cast<unsigned int>(B) > max) {
        max = B;
        maxValue = 2;
    }

    if (static_cast<unsigned int>(G) < min)
        min = G;

    if (static_cast<unsigned int>(B) < min)
        min = B;

    int delta = max - min;
    *V = max; // value
    *S = max ? (510 * delta + max) / (2 * max) : 0;  // saturation

    // calc hue
    if (*S == 0)
        *H = -1; // undefined hue
    else {
        switch (maxValue) {
        case 0:  // red
            if (G >= B)
                *H = (120 * (G - B) + delta) / (2 * delta);
            else
                *H = (120 * (G - B + delta) + delta) / (2 * delta) + 300;
            break;
        case 1:  // green
            if (B > R)
                *H = 120 + (120 * (B - R) + delta) / (2 * delta);
            else
                *H = 60 + (120 * (B - R + delta) + delta) / (2 * delta);
            break;
        case 2:  // blue
            if (R > G)
                *H = 240 + (120 * (R - G) + delta) / (2 * delta);
            else
                *H = 180 + (120 * (R - G + delta) + delta) / (2 * delta);
            break;
        }
    }
}

void hsv_to_rgb(int H, int S, int V, int *R, int *G, int *B)
{
    *R = *G = *B = V;

    if (S != 0 && H != -1) { // chromatic

        if (H >= 360) {
            // angle > 360
            H %= 360;
        }

        unsigned int f = H % 60;
        H /= 60;
        unsigned int p = static_cast<unsigned int>(2 * V * (255 - S) + 255) / 510;
        unsigned int q, t;

        if (H & 1) {
            q = static_cast<unsigned int>(2 * V * (15300 - S * f) + 15300) / 30600;
            switch (H) {
            case 1:
                *R = static_cast<int>(q);
                *G = static_cast<int>(V);
                *B = static_cast<int>(p);
                break;
            case 3:
                *R = static_cast<int>(p);
                *G = static_cast<int>(q);
                *B = static_cast<int>(V);
                break;
            case 5:
                *R = static_cast<int>(V);
                *G = static_cast<int>(p);
                *B = static_cast<int>(q);
                break;
            }
        } else {
            t = static_cast<unsigned int>(2 * V * (15300 - (S * (60 - f))) + 15300) / 30600;
            switch (H) {
            case 0:
                *R = static_cast<int>(V);
                *G = static_cast<int>(t);
                *B = static_cast<int>(p);
                break;
            case 2:
                *R = static_cast<int>(p);
                *G = static_cast<int>(V);
                *B = static_cast<int>(t);
                break;
            case 4:
                *R = static_cast<int>(t);
                *G = static_cast<int>(p);
                *B = static_cast<int>(V);
                break;
            }
        }
    }
}

#define EPSILON 1e-6
#define UNDEFINED_HUE -1

void RGBToHSV(float r, float g, float b, float *h, float *s, float *v)
{
    float max = qMax(r, qMax(g, b));
    float min = qMin(r, qMin(g, b));

    *v = max;

    if (max > EPSILON) {
        *s = (max - min) / max;
    } else {
        *s = 0;
    }

    if (*s < EPSILON) {
        *h = UNDEFINED_HUE;
    } else {
        float delta = max - min;

        if (r == max) {
            *h = (g - b) / delta;
        } else if (g == max) {
            *h = 2 + (b - r) / delta;
        } else {
            *h = 4 + (r - g) / delta;
        }

        *h *= 60;
        if (*h < 0) {
            *h += 360;
        }
    }
}

void HSVToRGB(float h, float s, float v, float *r, float *g, float *b)
{
    if (s < EPSILON || h == UNDEFINED_HUE) {
        // Achromatic case

        *r = v;
        *g = v;
        *b = v;
    } else {
        float f, p, q, t;
        int i;

        if (h > 360 - EPSILON) {
            h -= 360;
        }

        h /= 60;
        i = static_cast<int>(floor(h));
        f = h - i;
        p = v * (1 - s);
        q = v * (1 - (s * f));
        t = v * (1 - (s * (1 - f)));

        switch (i) {
        case 0:
            *r = v;
            *g = t;
            *b = p;
            break;
        case 1:
            *r = q;
            *g = v;
            *b = p;
            break;
        case 2:
            *r = p;
            *g = v;
            *b = t;
            break;
        case 3:
            *r = p;
            *g = q;
            *b = v;
            break;
        case 4:
            *r = t;
            *g = p;
            *b = v;
            break;
        case 5:
            *r = v;
            *g = p;
            *b = q;
            break;
        }
    }
}

void rgb_to_hls(quint8 red, quint8 green, quint8 blue, float * hue, float * lightness, float * saturation)
{
    float r = red / 255.0;
    float g = green / 255.0;
    float b = blue / 255.0;
    float h = 0;
    float l = 0;
    float s = 0;

    float max, min, delta;

    max = qMax(r, g);
    max = qMax(max, b);

    min = qMin(r, g);
    min = qMin(min, b);

    delta = max - min;

    l = (max + min) / 2;

    if (delta == 0) {
        // This is a gray, no chroma...
        h = 0;
        s = 0;
    } else {
        if (l < 0.5)
            s = delta / (max + min);
        else
            s = delta / (2 - max - min);

        float delta_r, delta_g, delta_b;

        delta_r = ((max - r) / 6) / delta;
        delta_g = ((max - g) / 6) / delta;
        delta_b = ((max - b) / 6) / delta;

        if (r == max)
            h = delta_b - delta_g;
        else if (g == max)
            h = (1.0 / 3) + delta_r - delta_b;
        else if (b == max)
            h = (2.0 / 3) + delta_g - delta_r;

        if (h < 0) h += 1;
        if (h > 1) h += 1;

    }

    *hue = h * 360;
    *saturation = s;
    *lightness = l;
}

float hue_value(float n1, float n2, float hue)
{
    if (hue > 360)
        hue = hue - 360;
    else if (hue < 0)
        hue = hue + 360;
    if (hue < 60)
        return n1 + (((n2 - n1) * hue) / 60);
    else if (hue < 180)
        return n2;
    else if (hue < 240)
        return n1 + (((n2 - n1) *(240 - hue)) / 60);
    else return n1;
}


void hls_to_rgb(float h, float l, float s, quint8 * r, quint8 * g, quint8 * b)
{
    float m1, m2;

    if (l <= 0.5)
        m2 = l * (1 + s);
    else
        m2 = l + s - l * s;

    m1 = 2 * l - m2;

    *r = (quint8)(hue_value(m1, m2, h + 120) * 255 + 0.5);
    *g = (quint8)(hue_value(m1, m2, h) * 255 + 0.5);
    *b = (quint8)(hue_value(m1, m2, h - 120) * 255 + 0.5);

}

void rgb_to_hls(quint8 r, quint8 g, quint8 b, int * h, int * l, int * s)
{
    float hue, saturation, lightness;

    rgb_to_hls(r, g, b, &hue, &lightness, &saturation);
    *h = (int)(hue + 0.5);
    *l = (int)(lightness * 255 + 0.5);
    *s = (int)(saturation * 255 + 0.5);
}

void hls_to_rgb(int h, int l, int s, quint8 * r, quint8 * g, quint8 * b)
{
    float hue = h;
    float lightness = l / 255.0;
    float saturation = s / 255.0;

    hls_to_rgb(hue, lightness, saturation, r, g, b);
}

/*
A Fast HSL-to-RGB Transform
by Ken Fishkin
from "Graphics Gems", Academic Press, 1990
*/

void RGBToHSL(float r, float g, float b, float *h, float *s, float *l)
{
    float v;
    float m;
    float vm;
    float r2, g2, b2;

    v = qMax(r, g);
    v = qMax(v, b);
    m = qMin(r, g);
    m = qMin(m, b);

    if ((*l = (m + v) / 2.0) <= 0.0) {
        *h = UNDEFINED_HUE;
        *s = 0;
        return;
    }
    if ((*s = vm = v - m) > 0.0) {
        *s /= (*l <= 0.5) ? (v + m) :
              (2.0 - v - m) ;
    } else {
        *h = UNDEFINED_HUE;
        return;
    }


    r2 = (v - r) / vm;
    g2 = (v - g) / vm;
    b2 = (v - b) / vm;

    if (r == v)
        *h = (g == m ? 5.0 + b2 : 1.0 - g2);
    else if (g == v)
        *h = (b == m ? 1.0 + r2 : 3.0 - b2);
    else
        *h = (r == m ? 3.0 + g2 : 5.0 - r2);

    *h *= 60;
    if (*h == 360.) {
        *h = 0;
    }
}

void HSLToRGB(float h, float sl, float l, float *r, float *g, float *b)

{
    float v;

    v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
    if (v <= 0) {
        *r = *g = *b = 0.0;
    } else {
        float m;
        float sv;
        int sextant;
        float fract, vsf, mid1, mid2;

        m = l + l - v;
        sv = (v - m) / v;
        h /= 60.0;
        sextant = static_cast<int>(h);
        fract = h - sextant;
        vsf = v * sv * fract;
        mid1 = m + vsf;
        mid2 = v - vsf;
        switch (sextant) {
        case 0: *r = v; *g = mid1; *b = m; break;
        case 1: *r = mid2; *g = v; *b = m; break;
        case 2: *r = m; *g = v; *b = mid1; break;
        case 3: *r = m; *g = mid2; *b = v; break;
        case 4: *r = mid1; *g = m; *b = v; break;
        case 5: *r = v; *g = m; *b = mid2; break;
        }
    }
}

