/* This file is part of the KDE project
  Copyright (c) 1999 Matthias Elter (me@kde.org)
  Copyright (c) 2001-2002 Igor Jansen (rm@kde.org)

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

#include "koColor.h"
#include "kdebug.h"
#include <cmath>

KoOldColor::KoOldColor()
{
  // initialise to black
  mNative = csRGB;
  // RGB
  mR = 0;
  mG = 0;
  mB = 0;
  // HSV
  mH =  mV = 0;
  mS = 100;
  // CMYK
  mC = 0;
  mY = 0;
  mM = 0;
  mK = 0;
  // Lab
  mL = 0;
  ma = 0;
  mB = 0;
  rgbChanged();
}

KoOldColor::KoOldColor(int a, int b, int c, cSpace m)
{
  switch(m)
  {
  case csRGB:
    mR = a;
    mG = b;
    mB = c;
    mNative = csRGB;
    rgbChanged();
    break;
  case csHSV:
    mH = a;
    mS = b;
    mV = c;
    mNative = csHSV;
    hsvChanged();
    break;
  case csLab:
    mL = a;
    ma = b;
    mB = c;
    mNative = csLab;
    labChanged();
    break;
  default:
    mR = 0;
    mG = 0;
    mB = 0;
    mNative = csRGB;
    rgbChanged();
  }
}

KoOldColor::KoOldColor(int c, int m, int y, int k)
{
  mC = c;
  mM = m;
  mY = y;
  mK = k;
  mNative = csCMYK;
  cmykChanged();
}

KoOldColor::KoOldColor(const QColor &c)
{
  mR = c.red();
  mG = c.green();
  mB = c.blue();
  mNative = csRGB;
  rgbChanged();
}

KoOldColor::KoOldColor(const QString &name)
{
  setNamedColor(name);
}

int KoOldColor::R() const
{
  if(!mRGBvalid)
    calcRGB();
  return mR;
}

int KoOldColor::G() const
{
  if(!mRGBvalid)
    calcRGB();
  return mG;
}

int KoOldColor::B() const
{
  if(!mRGBvalid)
    calcRGB();
  return mB;
}

int KoOldColor::H() const
{
  if(!mHSVvalid)
    calcHSV();
  return mH;
}

int KoOldColor::S() const
{
  if(!mHSVvalid)
    calcHSV();
  return mS;
}

int KoOldColor::V() const
{
  if(!mHSVvalid)
    calcHSV();
  return mV;
}

int KoOldColor::C() const
{
  if(!mCMYKvalid)
    calcCMYK();
  return mC;
}

int KoOldColor::M() const
{
  if(!mCMYKvalid)
    calcCMYK();
  return mM;
}

int KoOldColor::Y() const
{
  if(!mCMYKvalid)
    calcCMYK();
  return mY;
}

int KoOldColor::K() const
{
  if(!mCMYKvalid)
    calcCMYK();
  return mK;
}

int KoOldColor::L() const
{
  if(!mLABvalid)
    calcLAB();
  return mL;
}

int KoOldColor::a() const
{
  if(!mLABvalid)
    calcLAB();
  return ma;
}

int KoOldColor::b() const
{
  if(!mLABvalid)
    calcLAB();
  return mB;
}

void KoOldColor::rgb(int *R, int *G, int *B) const
{
  if(!mRGBvalid)
    calcRGB();
  *R = mR;
  *G = mG;
  *B = mB;
}

void KoOldColor::hsv(int *H, int *S, int *V) const
{
  if(!mHSVvalid)
    calcHSV();
  *H = mH;
  *S = mS;
  *V = mV;
}

void KoOldColor::lab(int *L, int *a, int *b) const
{
  if(!mLABvalid)
    calcLAB();
  *L = mL;
  *a = ma;
  *b = mB;
}

void KoOldColor::cmyk(int *C, int *M, int *Y, int *K) const
{
  if(!mCMYKvalid)
    calcCMYK();
  *C = mC;
  *M = mM;
  *Y = mY;
  *K = mK;
}

QString KoOldColor::name() const
{
  QString s;
  switch(mNative)
  {
  case csRGB:
    s.sprintf("#%02x%02x%02x", R(), G(), B());
    break;
  case csHSV:
    s.sprintf("$%02x%02x%02x", H(), S(), V());
    break;
  case csCMYK:
    s.sprintf("@%02x%02x%02x%02x", C(), M(), Y(), K());
    break;
  case csLab:
    s.sprintf("*%02x%02x%02x", L(), a(), b());
    break;
  default:
    s.sprintf("#%02x%02x%02x", R(), G(), B());
  }
  return s;
}

QColor KoOldColor::color() const
{
  if(!mRGBvalid)
    calcRGB();
  return QColor(mR, mG, mB);
}

void KoOldColor::setRGB(int R, int G, int B)
{
  mR = R;
  mG = G;
  mB = B;
  mNative = csRGB;
  rgbChanged();
}

void KoOldColor::setHSV(int H, int S, int V)
{
  mH = H;
  mS = S;
  mV = V;
  mNative = csHSV;
  hsvChanged();
}

void KoOldColor::setLab(int L, int a, int b)
{
  mL = L;
  ma = a;
  mB = b;
  mNative = csLab;
  labChanged();
}

void KoOldColor::setCMYK(int C, int M, int Y, int K)
{
  mC = C;
  mM = M;
  mY = Y;
  mK = K;
  mNative = csCMYK;
  cmykChanged();
}

void KoOldColor::setNamedColor(const QString &name)
{
  switch(name[0].toLatin1())
  {
  case '#':
    mR = (hex2int(name[1]) << 4) + hex2int(name[2]);
    mG = (hex2int(name[3]) << 4) + hex2int(name[4]);
    mB = (hex2int(name[5]) << 4) + hex2int(name[6]);
    mNative = csRGB;
    rgbChanged();
    break;
  case '$':
    mH = (hex2int(name[1]) << 4) + hex2int(name[2]);
    mS = (hex2int(name[3]) << 4) + hex2int(name[4]);
    mV = (hex2int(name[5]) << 4) + hex2int(name[6]);
    mNative = csHSV;
    hsvChanged();
    break;
  case '@':
    mC = (hex2int(name[1]) << 4) + hex2int(name[2]);
    mM = (hex2int(name[3]) << 4) + hex2int(name[4]);
    mY = (hex2int(name[5]) << 4) + hex2int(name[6]);
    mK = (hex2int(name[7]) << 4) + hex2int(name[8]);
    mNative = csCMYK;
    cmykChanged();
    break;
  case '*':
    mL = (hex2int(name[1]) << 4) + hex2int(name[2]);
    ma = (hex2int(name[3]) << 4) + hex2int(name[4]);
    mb = (hex2int(name[5]) << 4) + hex2int(name[6]);
    mNative = csLab;
    labChanged();
    break;
  default:
    mR = 0;
    mG = 0;
    mB = 0;
    mNative = csRGB;
    rgbChanged();
  }
}

void KoOldColor::setColor(const QColor &c)
{
  mR = c.red();
  mG = c.green();
  mB = c.blue();
  mNative = csRGB;
  rgbChanged();
}

void KoOldColor::RGBtoHSV(int R, int G, int B, int *H, int *S, int *V)
{
  unsigned int max = R;
  unsigned int min = R;
  unsigned char maxValue = 0; // r = 0, g = 1, b = 2

  // find maximum and minimum RGB values
  if(static_cast<unsigned int>(G) > max)
  {
    max = G;
    maxValue = 1;
  }
  if(static_cast<unsigned int>(B) > max)
  {
    max = B;
    maxValue = 2;
  }

  if(static_cast<unsigned int>(G) < min)
    min = G;
  if(static_cast<unsigned int>(B) < min )
    min = B;

  int delta = max - min;
  *V = max; // value
  *S = max ? (510 * delta + max) / ( 2 * max) : 0; // saturation

  // calc hue
  if(*S == 0)
    *H = -1; // undefined hue
  else
  {
    switch(maxValue)
    {
    case 0:  // red
      if(G >= B)
        *H = (120 * (G - B) + delta) / (2 * delta);
      else
        *H = (120 * (G - B + delta) + delta) / (2 * delta) + 300;
      break;
    case 1:  // green
      if(B > R)
        *H = 120 + (120 * (B - R) + delta) / (2 * delta);
      else
        *H = 60 + (120 * (B - R + delta) + delta) / (2 * delta);
      break;
    case 2:  // blue
      if(R > G)
        *H = 240 + (120 * (R - G) + delta) / (2 * delta);
      else
        *H = 180 + (120 * (R - G + delta) + delta) / (2 * delta);
      break;
    }
  }
}

void KoOldColor::RGBtoLAB(int R, int G, int B, int *L, int *a, int *b)
{
  // Convert between RGB and CIE-Lab color spaces
  // Uses ITU-R recommendation BT.709 with D65 as reference white.
  // algorithm contributed by "Mark A. Ruzon" <ruzon@CS.Stanford.EDU>

  double X, Y, Z, fX, fY, fZ;

  X = 0.412453 * R + 0.357580 * G + 0.180423 * B;
  Y = 0.212671 * R + 0.715160 * G + 0.072169 * B;
  Z = 0.019334 * R + 0.119193 * G + 0.950227 * B;

  X /= (255 * 0.950456);
  Y /=  255;
  Z /= (255 * 1.088754);

  if(Y > 0.008856)
  {
    fY = pow(Y, 1.0 / 3.0);
    *L = static_cast<int>(116.0 * fY - 16.0 + 0.5);
  }
  else
  {
    fY = 7.787 * Y + 16.0 / 116.0;
    *L = static_cast<int>(903.3 * Y + 0.5);
  }

  if(X > 0.008856)
    fX = pow(X, 1.0 / 3.0);
  else
    fX = 7.787 * X + 16.0 / 116.0;

  if(Z > 0.008856)
    fZ = pow(Z, 1.0 / 3.0);
  else
    fZ = 7.787 * Z + 16.0 / 116.0;

  *a = static_cast<int>(500.0 * (fX - fY) + 0.5);
  *b = static_cast<int>(200.0 * (fY - fZ) + 0.5);
}

void KoOldColor::RGBtoCMYK(int R, int G, int B, int *C, int *M, int *Y, int *K)
{
    // XXX: these algorithms aren't the best. See www.littlecms.com
    // for a suitable library, or the posting by Leo Rosenthol for
    // a better, but slower algorithm at
    // http://lists.kde.org/?l=koffice-devel&m=106698241227054&w=2

    *C = 255 - R;
    *M = 255 - G;
    *Y = 255 - B;

    int min = (*C < *M) ? *C : *M;
    *K = (min < *Y) ? min : *Y;

    *C -= *K;
    *M -= *K;
    *Y -= *K;

}


void KoOldColor::HSVtoRGB(int H, int S, int V, int *R, int *G, int *B)
{
  *R = *G = *B = V;

  if(S != 0 && H != -1) // chromatic
  {
    if(H >= 360) // angle > 360
      H %= 360;

    unsigned int f = H % 60;
    H /= 60;
    unsigned int p = static_cast<unsigned int>(2*V*(255-S)+255)/510;
    unsigned int q, t;

    if(H & 1)
    {
      q = static_cast<unsigned int>(2 * V * (15300 - S * f) + 15300) / 30600;
      switch(H)
      {
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
    }
    else
    {
      t = static_cast<unsigned int>(2 * V * (15300 - (S * (60 - f))) + 15300) / 30600;
      switch(H)
      {
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

void KoOldColor::HSVtoLAB(int H, int S, int V, int *L, int *a, int *b)
{
  int R, G, B;
  HSVtoRGB(H, S, V, &R, &G, &B);
  RGBtoLAB(R, G, B, L, a, b);
}

void KoOldColor::HSVtoCMYK(int H, int S, int V, int *C, int *M, int *Y, int*K)
{
  int R, G, B;
  HSVtoRGB(H, S, V, &R, &G, &B);
  RGBtoCMYK(R, G, B, C, M, Y, K);
}

void KoOldColor::LABtoRGB(int L, int a, int b, int *R, int *G, int *B)
{
  // Convert between RGB and CIE-Lab color spaces
  // Uses ITU-R recommendation BT.709 with D65 as reference white.
  // algorithm contributed by "Mark A. Ruzon" <ruzon@CS.Stanford.EDU>

  double X, Y, Z, fX, fY, fZ;
  int RR, GG, BB;

  fY = pow((L + 16.0) / 116.0, 3.0);
  if(fY < 0.008856)
    fY = L / 903.3;
  Y = fY;

  if(fY > 0.008856)
    fY = pow(fY, 1.0 / 3.0);
  else
    fY = 7.787 * fY + 16.0 / 116.0;

  fX = a / 500.0 + fY;
  if(fX > 0.206893)
    X = pow(fX, 3.0);
  else
    X = (fX - 16.0 / 116.0) / 7.787;

  fZ = fY - b / 200.0;
  if(fZ > 0.206893)
    Z = pow(fZ, 3.0);
  else
    Z = (fZ - 16.0/116.0) / 7.787;

  X *= 0.950456 * 255;
  Y *= 255;
  Z *= 1.088754 * 255;

  RR = static_cast<int>(3.240479 * X - 1.537150 * Y - 0.498535 * Z + 0.5);
  GG = static_cast<int>(-0.969256 * X + 1.875992 * Y + 0.041556 * Z + 0.5);
  BB = static_cast<int>(0.055648 * X - 0.204043 * Y + 1.057311 * Z + 0.5);

  *R = RR < 0 ? 0 : RR > 255 ? 255 : RR;
  *G = GG < 0 ? 0 : GG > 255 ? 255 : GG;
  *B = BB < 0 ? 0 : BB > 255 ? 255 : BB;
}

void KoOldColor::LABtoHSV(int L, int a, int b, int *H, int *S, int *V)
{
  int R, G, B;
  LABtoRGB(L, a, b, &R, &G, &B);
  RGBtoHSV(R, G, B, H, S, V);
}

void KoOldColor::LABtoCMYK(int L, int a, int b, int *C, int *M, int *Y, int*K)
{
  int R, G, B;
  LABtoRGB(L, a, b, &R, &G, &B);
  RGBtoCMYK(R, G, B, C, M, Y, K);
}

void KoOldColor::CMYKtoRGB(int C, int M, int Y, int K, int *R, int *G, int *B)
{
  *R = 255 - (C + K);
  *G = 255 - (M + K);
  *B = 255 - (Y + K);
}

void KoOldColor::CMYKtoHSV(int C, int M, int Y, int K, int *H, int *S, int *V)
{
  int R, G, B;
  CMYKtoRGB(C, M, Y, K, &R, &G, &B);
  RGBtoHSV(R, G, B, H, S, V);
}

void KoOldColor::CMYKtoLAB(int C, int M, int Y, int K, int *L, int *a, int *b)
{
  int R, G, B;
  CMYKtoRGB(C, M, Y, K, &R, &G, &B);
  RGBtoLAB(R, G, B, L, a, b);
}

int KoOldColor::hex2int(QChar c)
{
  if(c.isDigit())
    return c.digitValue();
  else if('A' <= c && c <= 'F')
    return c.toLatin1() - 'A' + 10;
  else if('a' <= c && c <= 'f')
    return c.toLatin1() - 'a' + 10;
  else
    return 0;
}

void KoOldColor::calcRGB() const
{
  switch(mNative)
  {
  case csHSV:
    HSVtoRGB(mH, mS, mV, &mR, &mG, &mB);
    break;
  case csLab:
    LABtoRGB(mL, ma, mB, &mR, &mG, &mB);
    break;
  case csCMYK:
    CMYKtoRGB(mC, mM, mY, mK, &mR, &mG, &mB);
    break;
  default:
    break;
  }
  mRGBvalid = true;
}

void KoOldColor::calcHSV() const
{
  switch(mNative)
  {
  case csRGB:
    RGBtoHSV(mR, mG, mB, &mH, &mS, &mV);
    break;
  case csLab:
    LABtoHSV(mL, ma, mB, &mH, &mS, &mV);
    break;
  case csCMYK:
    CMYKtoHSV(mC, mM, mY, mK, &mH, &mS, &mV);
    break;
  default:
    break;
  }
  mHSVvalid = true;
}

void KoOldColor::calcCMYK() const
{
  switch(mNative)
  {
  case csRGB:
    RGBtoCMYK(mR, mG, mB, &mC, &mM, &mY, &mK);
    break;
  case csLab:
    LABtoCMYK(mL, ma, mB, &mC, &mM, &mY, &mK);
    break;
  case csHSV:
    HSVtoCMYK(mH, mS, mV, &mC, &mM, &mY, &mK);
    break;
  default:
    break;
  }
  mCMYKvalid = true;
}

void KoOldColor::calcLAB() const
{
  switch(mNative)
  {
  case csRGB:
    RGBtoLAB(mR, mG, mB, &mL, &ma, &mB);
    break;
  case csHSV:
    HSVtoLAB(mH, mS, mV, &mL, &ma, &mB);
    break;
  case csCMYK:
    CMYKtoLAB(mC, mM, mY, mK, &mL, &ma, &mB);
    break;
  default:
    break;
  }
  mLABvalid = true;
}

void KoOldColor::rgbChanged() const
{
  mRGBvalid = true;
  mHSVvalid = false;
  mCMYKvalid = false;
  mLABvalid = false;
}

void KoOldColor::hsvChanged() const
{
  mRGBvalid = false;
  mHSVvalid = true;
  mCMYKvalid = false;
  mLABvalid = false;
}

void KoOldColor::cmykChanged() const
{
  mRGBvalid = false;
  mHSVvalid = false;
  mCMYKvalid = true;
  mLABvalid = false;
}

void KoOldColor::labChanged() const
{
  mRGBvalid = false;
  mHSVvalid = false;
  mCMYKvalid = false;
  mLABvalid = true;
}
