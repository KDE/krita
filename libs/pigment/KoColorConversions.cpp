/*
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2014 Wolthera van Hövell <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KoColorConversions.h"

#include <cmath>

#include <QtGlobal>

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

        if (H & 1) {
            unsigned int q = static_cast<unsigned int>(2 * V * (15300 - S * f) + 15300) / 30600;
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
            unsigned int t = static_cast<unsigned int>(2 * V * (15300 - (S * (60 - f))) + 15300) / 30600;
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
    *h = fmod(*h, 360.0);
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
        h = fmod(h, 360.0);
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

//functions for converting from and back to HSI
void HSIToRGB(const qreal h,const qreal s, const qreal i, qreal *red, qreal *green, qreal *blue)
{//This function takes H, S and I values, which are converted to rgb.
    qreal onethird = 1.0/3.0;
    HSYToRGB(h, s, i, red, green, blue, onethird, onethird, onethird);
}
void RGBToHSI(qreal r,qreal g, qreal b, qreal *h, qreal *s, qreal *i)
{
    qreal onethird = 1.0/3.0;
    RGBToHSY(r, g, b, h, s, i, onethird, onethird, onethird);

}
//functions for converting from and back to hsy'
void HSYToRGB(const qreal h,const qreal s, const qreal y, qreal *red, qreal *green, qreal *blue, qreal R, qreal G, qreal B)
{//This function takes H, S and Y values, which are converted to rgb.
//Those are then used to create a qcolor.
    qreal hue = 0.0;
    qreal sat = 0.0;
    qreal luma = 0.0;
    if (h>1.0 || h<0.0){hue=fmod(h, 1.0);} else {hue=h;}
    if (s<0.0){sat=0.0;} else {sat=s;}
    //if (y>1.0){luma=1.0;}
    if (y<0.0){luma=0.0;}
    else {luma=y;}
    
    qreal segment = 0.166667;//1/6;
    qreal r=0.0;
    qreal g=0.0;
    qreal b=0.0;
//weights for rgb to Y'(Luma), these are the same weights used in color space maths and the desaturate.
//This is not luminance or luminosity, it just quacks like it.
    //qreal R=0.299;
    //qreal G=0.587;
    //qreal B=0.114;
//The intermediary variables for the weighted HSL forumala, based on the HSL in KoColorConversions.
    qreal max_sat, m, fract, luma_a, chroma, x;
    if (hue >= 0.0 && hue < (segment) ) {
            //need to treat this as a weighted hsl thingy.
            //so first things first, at which luma is the maximum saturation for this hue?
            //between R and G+R (yellow)
        max_sat = R + ( G*(hue*6) );	
        if (luma<=max_sat){luma_a = (luma/max_sat)*0.5; chroma=sat*2*luma_a;}
        else {luma_a = ((luma-max_sat)/(1-max_sat)*0.5)+0.5; chroma=sat*(2-2*luma_a);}

        fract = hue*6.0;
        x = (1-fabs(fmod(fract,2)-1))*chroma;
        r = chroma; g=x; b=0;
        m = luma-( (R*r)+(B*b)+(G*g) );
        r += m; g += m; b += m;
    } else if (hue >= (segment) && hue < (2.0*segment) ) {
        max_sat = (G+R) - (R*(hue-segment)*6);

        if (luma<max_sat) {
            luma_a = (luma/max_sat)*0.5; chroma=sat*(2*luma_a);
        } else {
            luma_a = ((luma-max_sat)/(1-max_sat)*0.5)+0.5; chroma=sat*(2-2*luma_a);
        }

        fract = hue*6.0;
        x = (1-fabs(fmod(fract,2)-1) )*chroma;
        r = x; g=chroma; b=0;
        m = luma-( (R*r)+(B*b)+(G*g) );
        r += m; g += m; b += m;
    } else if (hue >= (2.0*segment) && hue < (3.0*segment) ) {
        max_sat = G + (B*(hue-2.0*segment)*6);
        if (luma<max_sat) { 
            luma_a = (luma/max_sat)*0.5; chroma=sat*(2*luma_a);
        } else {
            luma_a = ((luma-max_sat)/(1-max_sat)*0.5)+0.5; chroma=sat*(2-2*luma_a);
        }
        fract = hue*6.0;
        x = (1-fabs(fmod(fract,2)-1) )*chroma;
        r = 0; g=chroma; b=x;
        m = luma-( (R*r)+(B*b)+(G*g) );
        r += m; g += m; b += m;
    } else if (hue >= (3.0*segment) && hue < (4.0*segment) ) {
        max_sat = (G+B) - (G*(hue-3.0*segment)*6);	
        if (luma<max_sat){
            luma_a = (luma/max_sat)*0.5; chroma=sat*(2*luma_a);
        } else {
            luma_a = ((luma-max_sat)/(1-max_sat)*0.5)+0.5; chroma=sat*(2-2*luma_a);
        }

        fract = hue*6.0;
        x = (1-fabs(fmod(fract,2)-1) )*chroma;
        r = 0; g=x; b=chroma;
        m = luma-( (R*r)+(B*b)+(G*g) );
        r += m; g += m; b += m;
    } else if (hue >= (4.0*segment) && hue < (5*segment) ) {
        max_sat = B + (R*((hue-4.0*segment)*6));	
        if (luma<max_sat) {
            luma_a = (luma/max_sat)*0.5; chroma=sat*(2*luma_a);
        } else {
            luma_a = ((luma-max_sat)/(1-max_sat)*0.5)+0.5; chroma=sat*(2-2*luma_a);
        }
        fract = hue*6.0;
        x = (1-fabs(fmod(fract,2)-1) )*chroma;
        r = x; g=0; b=chroma;
        m = luma-( (R*r)+(B*b)+(G*g) );
        r += m; g += m; b += m;
    } else if (hue >= (5.0*segment) && hue <= 1.0) {
        max_sat = (B+R) - (B*(hue-5.0*segment)*6);	
        if (luma<max_sat){
            luma_a = (luma/max_sat)*0.5; chroma=sat*(2*luma_a);
        } else {
            luma_a = ((luma-max_sat)/(1-max_sat)*0.5)+0.5; chroma=sat*(2-2*luma_a);
        }
        fract = hue*6.0;
        x = (1-fabs(fmod(fract,2)-1) )*chroma;
        r = chroma; g=0; b=x;
        m = luma-( (R*r)+(B*b)+(G*g) );
        r += m; g += m; b += m;
    } else {
        r=0.0;
        g=0.0;
        b=0.0;
    }

    //dbgPigment<<"red: "<<r<<", green: "<<g<<", blue: "<<b;
    //if (r>1.0){r=1.0;}
    //if (g>1.0){g=1.0;}
    //if (b>1.0){b=1.0;}
    //don't limit upwards due to floating point.
    if (r<0.0){r=0.0;}
    if (g<0.0){g=0.0;}
    if (b<0.0){b=0.0;}

    *red=r;
    *green=g;
    *blue=b;
}
void RGBToHSY(const qreal r,const qreal g,const qreal b, qreal *h, qreal *s, qreal *y, qreal R, qreal G, qreal B)
{
//This is LUMA btw, not Luminance.
//Using these RGB values, we calculate the H, S and I.
    qreal red; qreal green; qreal blue;
    if (r<0.0){red=0.0;} else {red=r;}
    if (g<0.0){green=0.0;} else {green=g;}
    if (b<0.0){blue=0.0;} else {blue=b;}

    qreal minval = qMin(r, qMin(g, b));
    qreal maxval = qMax(r, qMax(g, b));
    qreal hue = 0.0;
    qreal sat = 0.0;
    qreal luma = 0.0;
    //weights for rgb, these are the same weights used in color space maths and the desaturate.
    //qreal R=0.299;
    //qreal G=0.587;
    //qreal B=0.114;
    luma=(R*red+G*green+B*blue);
    qreal luma_a=luma;//defined later
    qreal chroma = maxval-minval;
    qreal max_sat=0.5;
    if(chroma==0) {
        hue = 0.0;
        sat = 0.0;
    } else {
        //the following finds the hue

        if(maxval==r) {
            //hue = fmod(((g-b)/chroma), 6.0);
            //above doesn't work so let's try this one:
            if (minval==b) {
                hue = (g-b)/chroma;
            } else {
                hue = (g-b)/chroma + 6.0;
            }
        } else if(maxval==g) {
            hue = (b-r)/chroma + 2.0;
        } else if(maxval==b) {
            hue = (r-g)/chroma + 4.0;
        }

        hue /=6.0;//this makes sure that hue is in the 0-1.0 range.
        //Most HSY formula will tell you that Sat=Chroma. However, this HSY' formula tries to be a
        //weighted HSL formula, where instead of 0.5, we search for a Max_Sat value, which is the Y'
        //at which the saturation is maximum.
        //This requires using the hue, and combining the weighting values accordingly.
        qreal segment = 0.166667;
        if (hue>1.0 || hue<0.0) {
            hue=fmod(hue, 1.0);
        }

        if (hue>=0.0 && hue<segment) {
            max_sat = R + G*(hue*6);
        } else if (hue>=segment && hue<(2.0*segment)) {
            max_sat = (G+R) - R*((hue-segment)*6);
        } else if (hue>=(2.0*segment) && hue<(3.0*segment)) {
            max_sat = G + B*((hue-2.0*segment)*6);
        } else if (hue>=(3.0*segment) && hue<(4.0*segment)) {
            max_sat = (B+G) - G*((hue-3.0*segment)*6);
        } else if (hue>=(4.0*segment) && hue<(5.0*segment)) {
            max_sat =  (B) + R*((hue-4.0*segment)*6);
        } else if (hue>=(5.0*segment) && hue<=1.0) {
            max_sat = (R+B) - B*((hue-5.0*segment)*6);
        } else {
            max_sat=0.5;
        }

        if(max_sat>1.0 || max_sat<0.0){ //This should not show up during normal use
            max_sat=(fmod(max_sat,1.0));
        } //If it does, it'll try to correct, but it's not good!

            //This is weighting the luma for the saturation)
        if (luma <= max_sat) {
            luma_a = (luma/max_sat)*0.5;
        } else{
            luma_a = ((luma-max_sat)/(1-max_sat)*0.5)+0.5;
        }
            
        if (chroma > 0.0) {
            sat = (luma <= max_sat) ? (chroma/ (2*luma_a) ) :(chroma/(2.0-(2*luma_a) ) ) ;
        }
    }

    //if (sat>1.0){sat=1.0;}
    //if (luma>1.0){luma=1.0;}
    if (sat<0.0){sat=0.0;}
    if (luma<0.0){luma=0.0;}

    *h=hue;
    *s=sat;
    *y=luma;


}
//Extra: Functions for converting from and back to HCI. Where the HSI function is forced cylindrical, HCI is a 
//double cone. This is for compatibility purposes, and of course, making future programmers who expect a double-cone
// function less sad. These algorithms were taken from wikipedia.

void HCIToRGB(const qreal h, const qreal c, const qreal i, qreal *red, qreal *green, qreal *blue)
{
//This function may not be correct, but it's based on the HCY function on the basis of seeing HCI as similar
//to the weighted HCY, but assuming that the weights are the same(one-third).
    qreal hue=0.0;
    qreal chroma=0.0;
    qreal intensity=0.0;	
    if(i<0.0){intensity = 0.0;} else{intensity = i;}
    if (h>1.0 || h<0.0){hue=fmod(h, 1.0);} else {hue=h;}
    if(c<0.0){chroma = 0.0;} else{chroma = c;}
    const qreal onethird = 1.0/3.0;
    qreal r=0.0;
    qreal g=0.0;
    qreal b=0.0;
	
    int fract = static_cast<int>(hue*6.0);
    qreal x = (1-fabs(fmod(hue*6.0,2)-1) )*chroma;
    switch (fract) {
        case 0:r = chroma; g=x; b=0;break;
        case 1:r = x; g=chroma; b=0;break;
        case 2:r = 0; g=chroma; b=x;break;
        case 3:r = 0; g=x; b=chroma;break;
        case 4:r = x; g=0; b=chroma;break;
        case 5:r = chroma; g=0; b=x;break;
    }
    qreal m = intensity-( onethird*(r+g+b) );
    r += m; g += m; b += m;

    *red=r;
    *green=g;
    *blue=b;
}

void RGBToHCI(const qreal r,const qreal g,const qreal b, qreal *h, qreal *c, qreal *i)
{
    qreal minval = qMin(r, qMin(g, b));
    qreal maxval = qMax(r, qMax(g, b));
    qreal hue = 0.0;
    qreal sat = 0.0;
    qreal intensity = 0.0;
    intensity=(r+g+b)/3.0;
    qreal chroma = maxval-minval;
    if(chroma==0) {
        hue = 0.0;
        sat = 0.0;
    } else {
        //the following finds the hue

        if(maxval==r) {
            if (minval==b) {
                hue = (g-b)/chroma;
            } else {
                hue = (g-b)/chroma + 6.0;
            }
        } else if(maxval==g) {
            hue = (b-r)/chroma + 2.0;
        } else if(maxval==b) {
            hue = (r-g)/chroma + 4.0;
        }
        hue /=6.0;//this makes sure that hue is in the 0-1.0 range.
        sat= 1-(minval/intensity);
    }

    *h=hue;
    *c=sat;
    *i=intensity;

}
void HCYToRGB(const qreal h, const qreal c, const qreal y, qreal *red, qreal *green, qreal *blue, qreal R, qreal G, qreal B)
{
    qreal hue=0.0;
    qreal chroma=c;
    qreal luma=y;
    if (h>1.0 || h<0.0){hue=(fmod((h*2.0), 2.0))/2.0;} else {hue=h;}
    //const qreal R=0.299;
    //const qreal G=0.587;
    //const qreal B=0.114;
    qreal r=0.0;
    qreal g=0.0;
    qreal b=0.0;

    int fract =static_cast<int>(hue*6.0); 
    qreal x = (1-fabs(fmod(hue*6.0,2)-1) )*chroma;
    switch (fract) {
        case 0:r = chroma; g=x; b=0;break;
        case 1:r = x; g=chroma; b=0;break;
        case 2:r = 0; g=chroma; b=x;break;
        case 3:r = 0; g=x; b=chroma;break;
        case 4:r = x; g=0; b=chroma;break;
        case 5:r = chroma; g=0; b=x;break;
    }
    qreal m = luma-( (R*r)+(B*b)+(G*g) );
    r += m; g += m; b += m;

    *red=r;
    *green=g;
    *blue=b;
}

void RGBToHCY(const qreal r,const qreal g,const qreal b, qreal *h, qreal *c, qreal *y, qreal R, qreal G, qreal B)
{
    qreal minval = qMin(r, qMin(g, b));
    qreal maxval = qMax(r, qMax(g, b));
    qreal hue = 0.0;
    qreal chroma = 0.0;
    qreal luma = 0.0;
    //weights for rgb, these are the same weights used in color space maths and the desaturate.
    //qreal R=0.299;
    //qreal G=0.587;
    //qreal B=0.114;
    luma=(R*r+G*g+B*b);
    chroma = maxval-minval;

    if(chroma==0) {
        hue = 0.0;
    }
    else {
        //the following finds the hue

        if(maxval==r) {
            //hue = fmod(((g-b)/chroma), 6.0);
            //above doesn't work so let's try this one:
            if (minval==b) {
                hue = (g-b)/chroma;
            } else {
                hue = (g-b)/chroma + 6.0;
            }
        } else if(maxval==g) {
            hue = (b-r)/chroma + 2.0;
        } else if(maxval==b) {
            hue = (r-g)/chroma + 4.0;
        }
        hue /=6.0;//this makes sure that hue is in the 0-1.0 range.
    }
    if (chroma<0.0){chroma=0.0;}
    if (luma<0.0){luma=0.0;}

    *h=qBound(0.0,hue,1.0);
    *c=chroma;
    *y=luma;

}
void RGBToYUV(const qreal r,const qreal g,const qreal b, qreal *y, qreal *u, qreal *v, qreal R, qreal G, qreal B)
{
    qreal uvmax = 0.5;
    qreal luma = R*r+G*g+B*b;
    qreal chromaBlue = uvmax*( (b - luma) / (1.0-B) );
    qreal chromaRed  = uvmax*( (r - luma) / (1.0-R) );

    *y = luma; //qBound(0.0,luma,1.0);
    *u = chromaBlue+uvmax;//qBound(0.0,chromaBlue+ uvmax,1.0);
    *v = chromaRed+uvmax;//qBound(0.0,chromaRed + uvmax,1.0);
}
void YUVToRGB(const qreal y, const qreal u, const qreal v, qreal *r, qreal *g, qreal *b, qreal R, qreal G, qreal B)
{
    qreal uvmax = 0.5;
    qreal chromaBlue = u-uvmax;//qBound(0.0,u,1.0)- uvmax;//put into -0.5-+0.5 range//
    qreal chromaRed = v-uvmax;//qBound(0.0,v,1.0)- uvmax;

    qreal negB  = 1.0-B;
    qreal negR  = 1.0-R;
    qreal red   = y+(chromaRed  * (negR / uvmax) );
    qreal green = y-(chromaBlue * ((B*negB) / (uvmax*G)) ) - (chromaRed* ((R*negR) / (uvmax*G)));
    qreal blue  = y+(chromaBlue * (negB / uvmax) );

    *r=red;//qBound(0.0,red  ,1.0);
    *g=green;//qBound(0.0,green,1.0);
    *b=blue;//qBound(0.0,blue ,1.0);
}

void LabToLCH(const qreal l, const qreal a, const qreal b, qreal *L, qreal *C, qreal *H)
{
    qreal atemp =  (a - 0.5)*10.0;//the multiplication is only so that we get out of floating-point maths
    qreal btemp =  (b - 0.5)*10.0;
    *L=qBound(0.0,l,1.0);
    *C=sqrt( pow(atemp,2.0) + pow(btemp,2.0) )*0.1;
    qreal hue = (atan2(btemp,atemp))* 180.0 / M_PI;
    
    if (hue<0.0) {
        hue+=360.0;
    } else {
        hue = fmod(hue, 360.0);
    }
    *H=hue/360.0;
}

void LCHToLab(const qreal L, const qreal C, const qreal H, qreal *l, qreal *a, qreal *b)
{
    qreal chroma = qBound(0.0,C,1.0);
    qreal hue = (qBound(0.0,H,1.0)*360.0)* M_PI / 180.0;
    *l=qBound(0.0,L,1.0);
    *a=(chroma * cos(hue) ) + 0.5;
    *b=(chroma * sin(hue) ) + 0.5;
}

void XYZToxyY(const qreal X, const qreal Y, const qreal Z, qreal *x, qreal *y, qreal *yY)
{
    qBound(0.0,X,1.0);
    qBound(0.0,Y,1.0);
    qBound(0.0,Z,1.0);
    *x=X/(X+Y+Z);
    *y=Y/(X+Y+Z);
    *yY=Y;
}  
void xyYToXYZ(const qreal x, const qreal y, const qreal yY, qreal *X, qreal *Y, qreal *Z)
{
    qBound(0.0,x,1.0);
    qBound(0.0,y,1.0);
    qBound(0.0,yY,1.0);
    *X=(x*yY)/y;
    *Z=((1.0-x-y)/yY)/y;
    *Y=yY;
}

void CMYToCMYK(qreal *c, qreal *m, qreal *y, qreal *k)
{
    qreal cyan, magenta, yellow, key = 1.0;
    cyan    = *c;
    magenta = *m;
    yellow  = *y;
    if ( cyan    < key ) {key = cyan;}
    if ( magenta < key ) {key = magenta;}
    if ( yellow  < key ) {key = yellow;}
    
    if ( key == 1 ) { //Black
        cyan    = 0;
        magenta = 0;
        yellow  = 0;
    }
    else {
        cyan    = ( cyan    - key ) / ( 1.0 - key );
        magenta = ( magenta - key ) / ( 1.0 - key );
        yellow  = ( yellow  - key ) / ( 1.0 - key );
    }
    
    *c=qBound(0.0,cyan   ,1.0);
    *m=qBound(0.0,magenta,1.0);
    *y=qBound(0.0,yellow ,1.0);
    *k=qBound(0.0,key    ,1.0);
}

/*code from easyrgb.com*/
void CMYKToCMY(qreal *c, qreal *m, qreal *y, qreal *k)
{
    qreal key     = *k;
    qreal cyan    = *c;
    qreal magenta = *m;
    qreal yellow  = *y;
    
    cyan    = ( cyan    * ( 1.0 - key ) + key );
    magenta = ( magenta * ( 1.0 - key ) + key );
    yellow  = ( yellow  * ( 1.0 - key ) + key );
    
    *c=qBound(0.0,cyan   ,1.0);
    *m=qBound(0.0,magenta,1.0);
    *y=qBound(0.0,yellow ,1.0);
}
