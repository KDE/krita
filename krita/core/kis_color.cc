/*
 *  kis_color.cc - part of Krayon
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

#include <math.h>

#include "kis_color.h"
#include <kdebug.h>

KisColor::KisColor()
{
    // initialise to black.  Note that while the default or native
    // color space is rgb, all the others have their equivalents of
    // "black" and can be regarded as different views of the same color
    // the same applies to any other color
    m_native = cs_RGB;

    // rgb
    m_R = m_G = m_B = 0;

    // hsv - value of hue can range from 0 to 359, normally.  This
    // is based on a circular color wheel concept with 360 degrees
    m_H =  m_V = 0;
    m_S = 100;

    // cmyk - the "k" value - what is it?
    m_C = m_Y = m_M = m_K = 0;

    // lab
    m_L = m_a = m_b = 0;
}

void KisColor::rgbChanged() const
{
    m_RGBvalid = true;
    m_HSVvalid = false;
    m_CMYKvalid = false;
    m_LABvalid = false;
}

void KisColor::hsvChanged() const
{
    m_RGBvalid = false;
    m_HSVvalid = true;
    m_CMYKvalid = false;
    m_LABvalid = false;
}

void KisColor::cmykChanged() const
{
    m_RGBvalid = false;
    m_HSVvalid = false;
    m_CMYKvalid = true;
    m_LABvalid = false;
}

void KisColor::labChanged() const
{
    m_RGBvalid = false;
    m_HSVvalid = false;
    m_CMYKvalid = false;
    m_LABvalid = true;
}

KisColor::KisColor(int a, int b, int c, cSpace m)
{
    switch (m)
	{
	    case cs_RGB:
	        m_R = a;
	        m_G = b;
	        m_B = c;
	        m_native = cs_RGB;
	        rgbChanged();
	        break;

	    case cs_HSV:
	        m_H = a;
	        m_S = b;
	        m_V = c;
	        m_native = cs_HSV;
	        hsvChanged();
	        break;

	    case cs_Lab:
	        m_L = a;
	        m_a = b;
	        m_b = c;
	        m_native = cs_Lab;
	        labChanged();
	        break;

	    default:
	        m_R = m_G = m_B = 0;
	        m_native = cs_RGB;
	        rgbChanged();
	        break;
	}
}


KisColor::KisColor(uint color, cSpace m)
{
    int a, b, c;

    a = (int)((color & 0xff0000) >> 16);
    b = (int)((color & 0x00ff00) >>  8);
    c = (int) (color & 0xff);

    switch (m)
	{
	    case cs_RGB:
	        m_R = a;
	        m_G = b;
	        m_B = c;
	        m_native = cs_RGB;
	        rgbChanged();
	        break;

        case cs_HSV:
	        m_H = a;
	        m_S = b;
	        m_V = c;
	        m_native = cs_HSV;
	        hsvChanged();
	        break;

        case cs_Lab:
	        m_L = a;
	        m_a = b;
	        m_b = c;
	        m_native = cs_Lab;
	        labChanged();
	        break;

        default:
	        m_R = m_G = m_B = 0;
	        m_native = cs_RGB;
	        rgbChanged();
	        break;
	}
}


KisColor::KisColor(int c, int m, int y, int k)
{
    m_C = c;
    m_M = m;
    m_Y = y;
    m_K = k;

    m_native = cs_CMYK;
    cmykChanged();
}

KisColor::KisColor(const QColor& c)
{
    m_R = c.red();
    m_G = c.green();
    m_B = c.blue();

    m_native = cs_RGB;
    rgbChanged();
}

void KisColor::setRGB (int R, int G, int B)
{
    m_R = R;
    m_G = G;
    m_B = B;

    m_native = cs_RGB;
    rgbChanged();
}

void KisColor::setHSV (int H, int S, int V)
{
    m_H = H;
    m_S = S;
    m_V = V;

    m_native = cs_HSV;
    hsvChanged();
}

void KisColor::setLAB (int L, int a, int b)
{
    m_L = L;
    m_a = a;
    m_b = b;

    m_native = cs_Lab;
    labChanged();
}

void KisColor::setCMYK (int C, int M, int Y, int K)
{
    m_C = C;
    m_M = M;
    m_Y = Y;
    m_K = K;

    m_native = cs_CMYK;
    cmykChanged();
}

void KisColor::setColor (const QColor& c)
{
    m_R = c.red();
    m_G = c.green();
    m_B = c.blue();

    m_native = cs_RGB;
    rgbChanged();
}

void KisColor::rgb (int *R, int *G, int *B) const
{
    if ( !m_RGBvalid ) calcRGB();

    *R = m_R;
    *G = m_G;
    *B = m_B;
}

void KisColor::hsv (int *H, int *S, int *V) const
{
    if ( !m_HSVvalid ) calcHSV();

    *H = m_H;
    *S = m_S;
    *V = m_V;
}

void KisColor::lab (int *L, int *a, int *b) const
{
    if ( !m_LABvalid ) calcLAB();

    *L = m_L;
    *a = m_a;
    *b = m_b;
}

void KisColor::cmyk (int *C, int *M, int *Y, int *K) const
{
    if ( !m_CMYKvalid )  calcCMYK();

    *C = m_C;
    *M = m_M;
    *Y = m_Y;
    *K = m_K;
}


QColor KisColor::color() const
{
    if ( !m_RGBvalid) calcRGB();

    return QColor(m_R, m_G, m_B);
}


void KisColor::calcRGB() const
{
    if ( m_RGBvalid ) return;

    switch ( m_native )
    {
        case cs_HSV:
            HSVtoRGB(m_H, m_S, m_V, &m_R, &m_G, &m_B);
            break;
        case cs_Lab:
            LABtoRGB(m_L, m_a, m_b, &m_R, &m_G, &m_B);
            break;
        case cs_CMYK:
            CMYKtoRGB(m_C, m_M, m_Y, m_K, &m_R, &m_G, &m_B);
            break;
        default:
            // should never happen!
            m_R = m_G = m_B = 0;
            break;
    }

    m_RGBvalid = true;
}



void KisColor::calcHSV() const
{
    if( m_HSVvalid) return;

    switch ( m_native )
    {
        case cs_RGB:
            RGBtoHSV(m_R, m_G, m_B, &m_H, &m_S, &m_V);
            break;
        case cs_Lab:
            LABtoHSV(m_L, m_a, m_b, &m_H, &m_S, &m_V);
            break;
        case cs_CMYK:
            CMYKtoHSV(m_C, m_M, m_Y, m_K, &m_H, &m_S, &m_V);
            break;
        default:
            // should never happen!
            m_H =  m_S = 0;
            m_V = 100;
            break;
    }

    m_HSVvalid = true;
}


void KisColor::calcLAB() const
{
    if( m_LABvalid ) return;

    switch ( m_native )
    {
        case cs_RGB:
            RGBtoLAB(m_R, m_G, m_B, &m_L, &m_a, &m_b);
            break;
        case cs_HSV:
            HSVtoLAB(m_H, m_S, m_V, &m_L, &m_a, &m_b);
            break;
        case cs_CMYK:
            CMYKtoLAB(m_C, m_M, m_Y, m_K, &m_L, &m_a, &m_b);
            break;
        default:
            // should never happen!
            m_L = 100;
            m_a = m_b = 0;
            break;
    }

    m_LABvalid = true;
}


void KisColor::calcCMYK() const
{
    if( m_CMYKvalid) return;

    switch ( m_native )
    {
        case cs_RGB:
            RGBtoCMYK(m_R, m_G, m_B, &m_C, &m_M, &m_Y, &m_K);
            break;
        case cs_Lab:
            LABtoCMYK(m_L, m_a, m_b, &m_C, &m_M, &m_Y, &m_K);
            break;
        case cs_HSV:
            HSVtoCMYK(m_H, m_S, m_V, &m_C, &m_M, &m_Y, &m_K);
            break;
        default:
            // should never happen!
            m_C = m_M = m_Y = m_K = 0;
            break;
    }

    m_CMYKvalid = true;
}

void KisColor::RGBtoHSV(int R, int G, int B, int *H, int *S, int *V)
{
    kdDebug()<<"KisColor::RGBtoHSV\n";
    unsigned int max = R;
    unsigned int min = R;
    unsigned char maxValue = 0; // r=0, g=1, b=2

    // find maximum and minimum RGB values
    if (static_cast<unsigned int>(G) > max) { max = G; maxValue = 1; }
    if (static_cast<unsigned int>(B) > max) { max = B; maxValue = 2; }

    if (static_cast<unsigned int>(G) < min) min = G;
    if (static_cast<unsigned int>(B) < min ) min = B;

    int delta = max - min;
    *V = max; // value
    *S = max ? (510*delta+max)/(2*max) : 0; // saturation

    // calc hue
    if (*S == 0)
	    *H = -1; // undefined hue
    else
	{
	    switch (maxValue)
		{
	        case 0: // red
		        if (G >= B)
			        *H = (120*(G-B)+delta)/(2*delta);
		        else
		            *H = (120*(G-B+delta)+delta)/(2*delta) + 300;
		        break;

	        case 1:	// green
		        if (B > R)
		            *H = 120 + (120*(B-R)+delta)/(2*delta);
		        else
		            *H = 60 + (120*(B-R+delta)+delta)/(2*delta);
		        break;

	        case 2:	// blue
		        if (R > G)
		            *H = 240 + (120*(R-G)+delta)/(2*delta);
		        else
		            *H = 180 + (120*(R-G+delta)+delta)/(2*delta);
		        break;
		}
    }
}


void KisColor::RGBtoLAB(int R, int G, int B, int *L, int *a, int *b)
{
    kdDebug()<<"KisColor::RGBtoLAB\n";
    // Convert between RGB and CIE-Lab color spaces
    // Uses ITU-R recommendation BT.709 with D65 as reference white.
    // algorithm contributed by "Mark A. Ruzon" <ruzon@CS.Stanford.EDU>

    double X, Y, Z, fX, fY, fZ;

    X = 0.412453*R + 0.357580*G + 0.180423*B;
    Y = 0.212671*R + 0.715160*G + 0.072169*B;
    Z = 0.019334*R + 0.119193*G + 0.950227*B;

    X /= (255 * 0.950456);
    Y /=  255;
    Z /= (255 * 1.088754);

    if (Y > 0.008856)
	{
	    fY = pow(Y, 1.0/3.0);
	    *L = static_cast<int>(116.0*fY - 16.0 + 0.5);
	}
    else
	{
	    fY = 7.787*Y + 16.0/116.0;
	    *L = static_cast<int>(903.3*Y + 0.5);
	}

    if (X > 0.008856)
	    fX = pow(X, 1.0/3.0);
    else
	    fX = 7.787*X + 16.0/116.0;

    if (Z > 0.008856)
	    fZ = pow(Z, 1.0/3.0);
    else
	    fZ = 7.787*Z + 16.0/116.0;

    *a = static_cast<int>(500.0*(fX - fY) + 0.5);
    *b = static_cast<int>(200.0*(fY - fZ) + 0.5);
}


void KisColor::RGBtoCMYK(int R, int G, int B, int *C, int *M, int *Y, int *K)
{
    kdDebug()<<"KisColor::RGBtoCMYK\n";
    int min = (R < G) ? R : G;
    *K = (min < B) ? min : B;

    *C = 255-(R - *K);
    *M = 255-(G - *K);
    *Y = 255-(B - *K);
}

void KisColor::HSVtoRGB(int H, int S, int V, int *R, int *G, int *B)
{
    kdDebug()<<"KisColor::HSVtoRGB\n";
    *R = *G = *B = V;

    if (S != 0 && H != -1) // chromatic
	{
	    if (H >= 360) // angle > 360
		    H %= 360;

	    unsigned int f = H % 60;
	    H /= 60;
	    unsigned int p = static_cast<unsigned int>(2*V*(255-S)+255)/510;
	    unsigned int q, t;

	    if (H&1)
		{
		    q = static_cast<unsigned int>(2*V*(15300-S*f)+15300)/30600;
		    switch(H)
			{
			    case 1:
                    *R=static_cast<int>(q);
                    *G=static_cast<int>(V),
                    *B=static_cast<int>(p);
                    break;
			    case 3:
                    *R=static_cast<int>(p);
                    *G=static_cast<int>(q),
                    *B=static_cast<int>(V);
                    break;
			    case 5:
                    *R=static_cast<int>(V);
                    *G=static_cast<int>(p),
                    *B=static_cast<int>(q);
                    break;
			}
		}
	    else
		{
		    t = static_cast<unsigned int>(2*V*(15300-(S*(60-f)))+15300)/30600;
		    switch(H)
			{
			    case 0:
                    *R=static_cast<int>(V);
                    *G=static_cast<int>(t),
                    *B=static_cast<int>(p);
                    break;

			    case 2:
                    *R=static_cast<int>(p);
                    *G=static_cast<int>(V),
                    *B=static_cast<int>(t);
                    break;

			    case 4:
                    *R=static_cast<int>(t);
                    *G=static_cast<int>(p),
                    *B=static_cast<int>(V);
                    break;
			}
		}
	}
}


void KisColor::HSVtoLAB(int H, int S, int V, int *L, int *a, int *b)
{
    int R, G, B;
    HSVtoRGB(H, S, V, &R, &G, &B);
    RGBtoLAB(R, G, B, L, a, b);
}


void KisColor::HSVtoCMYK(int H, int S, int V, int *C, int *M, int *Y, int*K)
{
    int R, G, B;
    HSVtoRGB(H, S, V, &R, &G, &B);
    RGBtoCMYK(R, G, B, C, M, Y, K);
}

void KisColor::LABtoRGB(int L, int a, int b, int *R, int *G, int *B)
{
    kdDebug()<<"KisColor::LABtoRGB\n";
    // Convert between RGB and CIE-Lab color spaces
    // Uses ITU-R recommendation BT.709 with D65 as reference white.
    // algorithm contributed by "Mark A. Ruzon" <ruzon@CS.Stanford.EDU>

    double X, Y, Z, fX, fY, fZ;
    int RR, GG, BB;

    fY = pow((L + 16.0) / 116.0, 3.0);
    if (fY < 0.008856)
	    fY = L / 903.3;
    Y = fY;

    if (fY > 0.008856)
	    fY = pow(fY, 1.0/3.0);
    else
	    fY = 7.787 * fY + 16.0/116.0;

    fX = a / 500.0 + fY;
    if (fX > 0.206893)
	    X = pow(fX, 3.0);
    else
	    X = (fX - 16.0/116.0) / 7.787;

    fZ = fY - b /200.0;
    if (fZ > 0.206893)
	    Z = pow(fZ, 3.0);
    else
	    Z = (fZ - 16.0/116.0) / 7.787;

    X *= (0.950456 * 255);
    Y *= 255;
    Z *= (1.088754 * 255);

    RR = static_cast<int>(3.240479*X - 1.537150*Y - 0.498535*Z + 0.5);
    GG = static_cast<int>(-0.969256*X + 1.875992*Y + 0.041556*Z + 0.5);
    BB = static_cast<int>(0.055648*X - 0.204043*Y + 1.057311*Z + 0.5);

    *R = RR < 0 ? 0 : RR > 255 ? 255 : RR;
    *G = GG < 0 ? 0 : GG > 255 ? 255 : GG;
    *B = BB < 0 ? 0 : BB > 255 ? 255 : BB;
}

void KisColor::LABtoHSV(int L, int a, int b, int *H, int *S, int *V)
{
    int R, G, B;
    LABtoRGB(L, a, b, &R, &G, &B);
    RGBtoHSV(R, G, B, H, S, V);
}

void KisColor::LABtoCMYK(int L, int a, int b, int *C, int *M, int *Y, int*K)
{
    int R, G, B;
    LABtoRGB(L, a, b, &R, &G, &B);
    RGBtoCMYK(R, G, B, C, M, Y, K);
}

void KisColor::CMYKtoRGB(int C, int M, int Y, int K, int *R, int *G, int *B)
{
    kdDebug()<<"KisColor::CMYKtoRGB\n";
    *R = 255-(C+K);
    *G = 255-(M+K);
    *B = 255-(Y+K);
}

void KisColor::CMYKtoHSV(int C, int M, int Y, int K, int *H, int *S, int *V)
{
    int R, G, B;
    CMYKtoRGB(C, M, Y, K, &R, &G, &B);
    RGBtoHSV(R, G, B, H, S, V);
}

void KisColor::CMYKtoLAB(int C, int M, int Y, int K, int *L, int *a, int *b)
{
    int R, G, B;
    CMYKtoRGB(C, M, Y, K, &R, &G, &B);
    RGBtoLAB(R, G, B, L, a, b);
}

int KisColor::R() const { if( !m_RGBvalid ) calcRGB(); return m_R; }
int KisColor::G() const { if( !m_RGBvalid ) calcRGB(); return m_G; }
int KisColor::B() const { if( !m_RGBvalid ) calcRGB(); return m_B; }

int KisColor::h() const { if( !m_HSVvalid ) calcHSV(); return m_H; }
int KisColor::s() const { if( !m_HSVvalid ) calcHSV(); return m_S; }
int KisColor::v() const { if( !m_HSVvalid ) calcHSV(); return m_V; }

int KisColor::l() const { if( !m_LABvalid ) calcLAB(); return m_L; }
int KisColor::a() const { if( !m_LABvalid ) calcLAB(); return m_a; }
int KisColor::b() const { if( !m_LABvalid ) calcLAB(); return m_b; }

int KisColor::c() const { if( !m_CMYKvalid ) calcCMYK(); return m_C; }
int KisColor::m() const { if( !m_CMYKvalid ) calcCMYK(); return m_M; }
int KisColor::y() const { if( !m_CMYKvalid ) calcCMYK(); return m_Y; }
int KisColor::k() const { if( !m_CMYKvalid ) calcCMYK(); return m_K; }
