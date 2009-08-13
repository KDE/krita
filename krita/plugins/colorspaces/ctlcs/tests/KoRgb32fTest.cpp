/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoRgb32fTest.h"

#include <qtest_kde.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceTraits.h>

template<typename _T_>
void randomizator( typename KoRgbTraits<_T_>::Pixel& p)
{
  p.red = rand();
  p.green = rand();
  p.blue = rand();
  p.alpha = rand();
}

void KoRgb32fTest::testConversion()
{
  const KoColorSpace* rgb32f = KoColorSpaceRegistry::instance()->colorSpace("RgbAF32", 0);
  QVERIFY(rgb32f);
  KoRgbTraits<float>::Pixel p32f;
  quint8* p32fPtr = reinterpret_cast<quint8*>(&p32f);
  KoRgbU16Traits::Pixel p16u;
  quint8* p16uPtr = reinterpret_cast<quint8*>(&p16u);
  
  // Test alpha function
  p32f.alpha = 1.0;
  QCOMPARE( qint32(rgb32f->alpha(p32fPtr)), 255);
  p32f.alpha = 0.5;
  QCOMPARE( qint32(rgb32f->alpha(p32fPtr)), 125);
  
  // Test conversion of black from 32f to 16u back to 32f
  p32f.red = 0.0;
  p32f.green = 0.0;
  p32f.blue = 0.0;
  p32f.alpha = 1.0;
  randomizator<quint16>(p16u);
  rgb32f->toRgbA16(p32fPtr, p16uPtr, 1);
  QCOMPARE(p16u.red, quint16(0));
  QCOMPARE(p16u.green, quint16(0));
  QCOMPARE(p16u.blue, quint16(0));
  QCOMPARE(p16u.alpha, quint16(65535));
  rgb32f->fromRgbA16(p16uPtr, p32fPtr, 1);
  QCOMPARE(p32f.red, 0.0f);
  QCOMPARE(p32f.green, 0.0f);
  QCOMPARE(p32f.blue, 0.0f);
  QCOMPARE(p32f.alpha, 1.0f);

  // Test conversion to QColor
  QColor color;
  rgb32f->toQColor(p32fPtr, &color, 0);
  QCOMPARE(color.red(), 0);
  QCOMPARE(color.green(), 0);
  QCOMPARE(color.blue(), 0);
  QCOMPARE(color.alpha(), 255);
  rgb32f->fromQColor(color, p32fPtr, 0);
  QCOMPARE(p32f.red, 0.0f);
  QCOMPARE(p32f.green, 0.0f);
  QCOMPARE(p32f.blue, 0.0f);
  QCOMPARE(p32f.alpha, 1.0f);
  
  // Test conversion of white from 32f to 16u back to 32f
  p32f.red = 1.0;
  p32f.green = 1.0;
  p32f.blue = 1.0;
  p32f.alpha = 1.0;
  randomizator<quint16>(p16u);
  rgb32f->toRgbA16(p32fPtr, p16uPtr, 1);
  QCOMPARE(p16u.red, quint16(47803));
  QCOMPARE(p16u.green, quint16(47803));
  QCOMPARE(p16u.blue, quint16(47803));
  QCOMPARE(p16u.alpha, quint16(65535));
  rgb32f->fromRgbA16(p16uPtr, p32fPtr, 1);
  QCOMPARE(p32f.red, 1.0f);
  QCOMPARE(p32f.green, 1.0f);
  QCOMPARE(p32f.blue, 1.0f);
  QCOMPARE(p32f.alpha, 1.0f);
}

QTEST_KDEMAIN(KoRgb32fTest, NoGUI)
#include "KoRgb32fTest.moc"
