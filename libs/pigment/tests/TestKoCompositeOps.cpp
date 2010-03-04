/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "TestKoCompositeOps.h"

#include <qtest_kde.h>

#include "../compositeops/KoCompositeOpAlphaDarken.h"
#include "../compositeops/KoCompositeOpOver.h"

#include <KoColorSpaceTraits.h>

#define FULL_OPACITY KoColorSpaceMathsTraits<quint16>::unitValue

#define QCOMPAREui(a,b) QCOMPARE(a, (quint16)b)

void TestKoCompositeOps::testCompositeOver()
{
    KoRgbU16Traits::Pixel p16f;
    KoRgbU16Traits::Pixel p16f1;
    quint8* p16fPtr = reinterpret_cast<quint8*>(&p16f);
    quint8* p16fPtr1 = reinterpret_cast<quint8*>(&p16f1);
    
    KoCompositeOpOver<KoRgbU16Traits> over(0);
    // Test no mask, full opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 127);
    QCOMPAREui(p16f1.red, 12509);
    QCOMPAREui(p16f1.green, 7972);
    QCOMPAREui(p16f1.blue, 17992);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 12509);
    QCOMPAREui(p16f1.green, 7972);
    QCOMPAREui(p16f1.blue, 17992);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 127);
    QCOMPAREui(p16f1.red, 13759);
    QCOMPAREui(p16f1.green, 4472);
    QCOMPAREui(p16f1.blue, 16992);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent source
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = 0;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 15000);
    QCOMPAREui(p16f1.green, 1000);
    QCOMPAREui(p16f1.blue, 16000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = 0;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY / 2;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY / 2;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 12500);
    QCOMPAREui(p16f1.green, 7999);
    QCOMPAREui(p16f1.blue, 17999);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, half-transparent src, dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY / 2;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY / 2;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 11666);
    QCOMPAREui(p16f1.green, 10333);
    QCOMPAREui(p16f1.blue, 18666);
    QCOMPAREui(p16f1.alpha, 49150);

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY / 4;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY / 2;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 13000);
    QCOMPAREui(p16f1.green, 6599);
    QCOMPAREui(p16f1.blue, 17599);
    QCOMPAREui(p16f1.alpha, 40958);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY / 2;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY / 4;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 11000);
    QCOMPAREui(p16f1.green, 12199);
    QCOMPAREui(p16f1.blue, 19199);
    QCOMPAREui(p16f1.alpha, 40958);
}

void TestKoCompositeOps::testCompositeAlphaDarken()
{
    KoRgbU16Traits::Pixel p16f;
    KoRgbU16Traits::Pixel p16f1;
    quint8* p16fPtr = reinterpret_cast<quint8*>(&p16f);
    quint8* p16fPtr1 = reinterpret_cast<quint8*>(&p16f1);
    
    KoCompositeOpAlphaDarken<KoRgbU16Traits> alphaDarken(0);
    // Test no mask, full opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    alphaDarken.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    alphaDarken.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 127);
    QCOMPAREui(p16f1.red, 12509);
    QCOMPAREui(p16f1.green, 7972);
    QCOMPAREui(p16f1.blue, 17992);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    alphaDarken.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 12509);
    QCOMPAREui(p16f1.green, 7972);
    QCOMPAREui(p16f1.blue, 17992);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    alphaDarken.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 127);
    QCOMPAREui(p16f1.red, 13759);
    QCOMPAREui(p16f1.green, 4472);
    QCOMPAREui(p16f1.blue, 16992);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent source
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = 0;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    alphaDarken.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 15000);
    QCOMPAREui(p16f1.green, 1000);
    QCOMPAREui(p16f1.blue, 16000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = 0;
    alphaDarken.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY / 2;
    alphaDarken.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY / 2;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    alphaDarken.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 12500);
    QCOMPAREui(p16f1.green, 7999);
    QCOMPAREui(p16f1.blue, 17999);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);


    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY / 2;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY / 2;
    alphaDarken.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, (FULL_OPACITY / 2));

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY / 4;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY / 2;
    alphaDarken.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 12500);
    QCOMPAREui(p16f1.green, 7999);
    QCOMPAREui(p16f1.blue, 17999);
    QCOMPAREui(p16f1.alpha, (FULL_OPACITY / 2));

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY / 2;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY / 4;
    alphaDarken.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, (FULL_OPACITY / 2));
}

QTEST_KDEMAIN(TestKoCompositeOps, NoGUI)
#include "TestKoCompositeOps.moc"
