/*
 *  color.h - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __color_h__
#define __color_h__

#include <qcolor.h>
#include "kis_global.h"

class KisColor
{
 public:

    KisColor();
    
    /* color as 3 ints and a color space def.
    covers RGB, HSV, LAB */
    KisColor(int a, int b, int c,  cSpace m = cs_RGB);
    
    /* color as a uint and a  color space def.
    covers RGB, HSV, LAB . This is for conversion 
    from 32 bit scanline offsets */
    KisColor(uint color, cSpace m = cs_RGB);

    /* color as cyan, magenta, yellow, reflectivity ? 
    for color separation work, mostly */    
    KisColor(int c, int m, int y, int k);

    // construct a color from a QColor
    KisColor(const QColor&);

    void setRGB (int R, int G, int B);
    void setHSV (int H, int S, int V);
    void setLAB (int L, int a, int b);
    void setCMYK (int C, int M, int Y, int K);
    
    void setColor(const QColor&);

    void rgb (int *R, int *G, int *B) const;
    void hsv (int *H, int *S, int *V) const;
    void lab (int *L, int *a, int *b) const;
    void cmyk (int *C, int *M, int *Y, int *K) const;
    
    QColor color() const;

    cSpace native() const { return m_native; }

    int R() const;
    int G() const;
    int B() const;

    int h() const;
    int s() const;
    int v() const;

    int l() const;
    int a() const;
    int b() const;

    int c() const;
    int m() const;
    int y() const;
    int k() const;
  
    static void RGBtoHSV(int R, int G, int B, int *H, int *S, int *V);
    static void RGBtoLAB(int R, int G, int B, int *L, int *a, int *b);
    static void RGBtoCMYK(int R, int G, int B, int *C, int *M, int *Y, int *K);

    static void HSVtoRGB(int H, int S, int V, int *R, int *G, int *B);
    static void HSVtoLAB(int H, int S, int V, int *L, int *a, int *b);
    static void HSVtoCMYK(int H, int S, int V, int *C, int *M, int *Y, int*K);

    static void LABtoRGB(int L, int a, int b, int *R, int *G, int *B);
    static void LABtoHSV(int L, int a, int b, int *H, int *S, int *V);
    static void LABtoCMYK(int L, int a, int b, int *C, int *M, int *Y, int*K);

    static void CMYKtoRGB(int C, int M, int Y, int K, int *R, int *G, int *B);
    static void CMYKtoHSV(int C, int M, int Y, int K, int *H, int *S, int *V);
    static void CMYKtoLAB(int C, int M, int Y, int K, int *L, int *a, int *b);

    static const KisColor black();
    static const KisColor white();
    static const KisColor gray();
    static const KisColor lightGray();
    static const KisColor darkGray();
    static const KisColor red();
    static const KisColor darkRed();
    static const KisColor green();
    static const KisColor darkGreen();
    static const KisColor blue();
    static const KisColor darkBlue();
    static const KisColor cyan();
    static const KisColor darkCyan();
    static const KisColor magenta();
    static const KisColor darkMagenta();
    static const KisColor yellow();
    static const KisColor darkYellow();

 protected:
 
    void calcRGB() const;
    void calcHSV() const;
    void calcCMYK() const;
    void calcLAB() const;

    void rgbChanged() const;
    void hsvChanged() const;
    void cmykChanged() const;
    void labChanged() const;

 private:

    /* Mutable to make it possible for const objects to transform 
    the native cModel in functions like KisColor::rgb(...) to 
    the requested. */
   
    mutable int m_R, m_G, m_B;      // RGB
    mutable int m_C, m_M, m_Y, m_K; // CMYK
    mutable int m_H, m_S, m_V;      // HSV
    mutable int m_L, m_a, m_b;      // LAB

    mutable bool m_RGBvalid;
    mutable bool m_HSVvalid;
    mutable bool m_CMYKvalid;
    mutable bool m_LABvalid;

    cSpace m_native; 
};

inline const KisColor KisColor::white()
{ return KisColor(255,255,255,cs_RGB); }

inline const KisColor KisColor::black()
{ return KisColor(0,0,0,cs_RGB); }

inline const KisColor KisColor::gray()
{ return KisColor(160,160,164,cs_RGB); }

inline const KisColor KisColor::lightGray()
{ return KisColor(192,192,192,cs_RGB); }

inline const KisColor KisColor::darkGray()
{ return KisColor(128,128,128,cs_RGB); }

inline const KisColor KisColor::red()
{ return KisColor(255,0,0,cs_RGB); }

inline const KisColor KisColor::darkRed()
{ return KisColor(128,0,0,cs_RGB); }

inline const KisColor KisColor::green()
{ return KisColor(0,255,0,cs_RGB); }

inline const KisColor KisColor::darkGreen()
{ return KisColor(0,128,0,cs_RGB); }

inline const KisColor KisColor::blue()
{ return KisColor(0,0,255,cs_RGB); }

inline const KisColor KisColor::darkBlue()
{ return KisColor(0,0,128,cs_RGB); }

inline const KisColor KisColor::cyan()
{ return KisColor(0,255,255,cs_RGB); }

inline const KisColor KisColor::darkCyan()
{ return KisColor(0,128,128,cs_RGB); }

inline const KisColor KisColor::magenta()
{ return KisColor(255,0,255,cs_RGB); }

inline const KisColor KisColor::darkMagenta()
{ return KisColor(128,0,128,cs_RGB); }

inline const KisColor KisColor::yellow()
{ return KisColor(255,255,0,cs_RGB); }

inline const KisColor KisColor::darkYellow()
{ return KisColor(128,128,0,cs_RGB); }

#endif
