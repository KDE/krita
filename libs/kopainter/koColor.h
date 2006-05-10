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

#ifndef __ko_color_h__
#define __ko_color_h__

#include <QColor>
#include <koffice_export.h>
class KOPAINTER_EXPORT KoColor
{
public:
  enum cSpace{ csIndexed, csRGB, csHSV, csCMYK, csLab };

  KoColor();
  KoColor(int a, int b, int c,  cSpace m = csRGB);
  KoColor(int c, int m, int y,  int k);
  KoColor(const QString &name);
  KoColor(const QColor &c);

  cSpace native() const {return mNative; }

  int R() const;
  int G() const;
  int B() const;
  int H() const;
  int S() const;
  int V() const;
  int L() const;
  int a() const;
  int b() const;
  int C() const;
  int M() const;
  int Y() const;
  int K() const;

  void rgb(int *R, int *G, int *B) const;
  void hsv(int *H, int *S, int *V) const;
  void lab(int *L, int *a, int *b) const;
  void cmyk(int *C, int *M, int *Y, int *K) const;
  QString name() const;
  QColor color() const;

  void setRGB(int R, int G, int B);
  void setHSV(int H, int S, int V);
  void setLab(int L, int a, int b);
  void setCMYK(int C, int M, int Y, int K);
  void setNamedColor(const QString &name);
  void setColor(const QColor &c);
  
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

  static const KoColor black();
  static const KoColor white();
  static const KoColor gray();
  static const KoColor lightGray();
  static const KoColor darkGray();
  static const KoColor red();
  static const KoColor darkRed();
  static const KoColor green();
  static const KoColor darkGreen();
  static const KoColor blue();
  static const KoColor darkBlue();
  static const KoColor cyan();
  static const KoColor darkCyan();
  static const KoColor magenta();
  static const KoColor darkMagenta();
  static const KoColor yellow();
  static const KoColor darkYellow();

protected:
  int hex2int(QChar c);

  void calcRGB() const;
  void calcHSV() const;
  void calcCMYK() const;
  void calcLAB() const;

  void rgbChanged() const;
  void hsvChanged() const;
  void cmykChanged() const;
  void labChanged() const;

private:
  /*
   * Mutable to make it possible for const objects to transform the native cModel
   * in functions like KoColor::rgb(...) to the requested.
   */
  mutable int mR, mG, mB;        // RGB
  mutable int mC, mM, mY, mK;    // CMYK
  mutable int mH, mS, mV;        // HSV
  mutable int mL, ma, mb;        // LAB

  mutable bool mRGBvalid;
  mutable bool mHSVvalid;
  mutable bool mCMYKvalid;
  mutable bool mLABvalid;

  cSpace mNative; 
};

inline const KoColor KoColor::white()
{
  return KoColor(255, 255, 255, csRGB);
}

inline const KoColor KoColor::black()
{
  return KoColor(0, 0, 0, csRGB);
}

inline const KoColor KoColor::gray()
{
  return KoColor(160, 160, 164, csRGB);
}

inline const KoColor KoColor::lightGray()
{
  return KoColor(192, 192, 192, csRGB);
}

inline const KoColor KoColor::darkGray()
{
  return KoColor(128, 128, 128, csRGB);
}

inline const KoColor KoColor::red()
{
  return KoColor(255, 0, 0, csRGB);
}

inline const KoColor KoColor::darkRed()
{
  return KoColor(128, 0, 0, csRGB);
}

inline const KoColor KoColor::green()
{
  return KoColor(0, 255, 0, csRGB);
}

inline const KoColor KoColor::darkGreen()
{
  return KoColor(0, 128, 0, csRGB);
}

inline const KoColor KoColor::blue()
{
  return KoColor(0, 0, 255, csRGB);
}

inline const KoColor KoColor::darkBlue()
{
  return KoColor(0, 0, 128, csRGB);
}

inline const KoColor KoColor::cyan()
{
  return KoColor(0, 255, 255, csRGB);
}

inline const KoColor KoColor::darkCyan()
{
  return KoColor(0, 128, 128, csRGB);
}

inline const KoColor KoColor::magenta()
{
  return KoColor(255, 0, 255, csRGB);
}

inline const KoColor KoColor::darkMagenta()
{
  return KoColor(128, 0, 128, csRGB);
}

inline const KoColor KoColor::yellow()
{
  return KoColor(255, 255, 0, csRGB);
}

inline const KoColor KoColor::darkYellow()
{
  return KoColor(128, 128, 0, csRGB);
}

#endif
