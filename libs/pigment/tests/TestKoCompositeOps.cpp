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
#define HALF_OPACITY (FULL_OPACITY/2)
#define QUARTER_OPACITY (FULL_OPACITY/4)

#define QCOMPAREui(a,b) QCOMPARE(a, (quint16)b)
#include <KoCompositeOpAdd.h>
#include <KoCompositeOpBurn.h>
#include <KoCompositeOpDivide.h>
#include <KoCompositeOpDodge.h>
#include <KoCompositeOpInversedSubtract.h>
#include <KoCompositeOpMultiply.h>
#include <KoCompositeOpOverlay.h>
#include <KoCompositeOpScreen.h>
#include <KoCompositeOpSubtract.h>

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
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 12500);
    QCOMPAREui(p16f1.green, 7999);
    QCOMPAREui(p16f1.blue, 17999);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, half-transparent src, dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 11666);
    QCOMPAREui(p16f1.green, 10333);
    QCOMPAREui(p16f1.blue, 18666);
    QCOMPAREui(p16f1.alpha, 49150);

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = QUARTER_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 13000);
    QCOMPAREui(p16f1.green, 6599);
    QCOMPAREui(p16f1.blue, 17599);
    QCOMPAREui(p16f1.alpha, 40958);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
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
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    alphaDarken.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    alphaDarken.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 12500);
    QCOMPAREui(p16f1.green, 7999);
    QCOMPAREui(p16f1.blue, 17999);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);


    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    alphaDarken.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = QUARTER_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    alphaDarken.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 12500);
    QCOMPAREui(p16f1.green, 7999);
    QCOMPAREui(p16f1.blue, 17999);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
    alphaDarken.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);
}

void TestKoCompositeOps::testCompositeAdd()
{
    KoRgbU16Traits::Pixel p16f;
    KoRgbU16Traits::Pixel p16f1;
    quint8* p16fPtr = reinterpret_cast<quint8*>(&p16f);
    quint8* p16fPtr1 = reinterpret_cast<quint8*>(&p16f1);
    
    KoCompositeOpAdd<KoRgbU16Traits> add(0);
    // Test no mask, full opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    add.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 24999);
    QCOMPAREui(p16f1.green, 15999);
    QCOMPAREui(p16f1.blue, 35999);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    add.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 127);
    QCOMPAREui(p16f1.red, 19980);
    QCOMPAREui(p16f1.green, 8470);
    QCOMPAREui(p16f1.blue, 25960);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    add.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 19980);
    QCOMPAREui(p16f1.green, 8470);
    QCOMPAREui(p16f1.blue, 25960);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    add.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 127);
    QCOMPAREui(p16f1.red, 17480);
    QCOMPAREui(p16f1.green, 4720);
    QCOMPAREui(p16f1.blue, 20960);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent source
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = 0;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    add.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 15000);
    QCOMPAREui(p16f1.green, 1000);
    QCOMPAREui(p16f1.blue, 16000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = 0;
    add.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 15000);
    QCOMPAREui(p16f1.green, 1000);
    QCOMPAREui(p16f1.blue, 16000);
    QCOMPAREui(p16f1.alpha, 0);

    // Test no mask, full opacity, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    add.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 21666);
    QCOMPAREui(p16f1.green, 10999);
    QCOMPAREui(p16f1.blue, 29333);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    add.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 19999);
    QCOMPAREui(p16f1.green, 8499);
    QCOMPAREui(p16f1.blue, 25999);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);


    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    add.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 21666);
    QCOMPAREui(p16f1.green, 10999);
    QCOMPAREui(p16f1.blue, 29333);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = QUARTER_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    add.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 18999);
    QCOMPAREui(p16f1.green, 6999);
    QCOMPAREui(p16f1.blue, 23999);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
    add.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 20714);
    QCOMPAREui(p16f1.green, 9571);
    QCOMPAREui(p16f1.blue, 27428);
    QCOMPAREui(p16f1.alpha, QUARTER_OPACITY);
}

void TestKoCompositeOps::testCompositeBurn()
{
    KoRgbU16Traits::Pixel p16f;
    KoRgbU16Traits::Pixel p16f1;
    quint8* p16fPtr = reinterpret_cast<quint8*>(&p16f);
    quint8* p16fPtr1 = reinterpret_cast<quint8*>(&p16f1);
    
    KoCompositeOpBurn<KoRgbU16Traits> burn(0);
    // Test no mask, full opacity
    p16f.red = 41120; p16f.green = 47545; p16f.blue = 46003; p16f.alpha = FULL_OPACITY;
    p16f1.red = 26000; p16f1.green = 26000; p16f1.blue = 30000; p16f1.alpha = FULL_OPACITY;
    burn.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 2528);
    QCOMPAREui(p16f1.green, 11043);
    QCOMPAREui(p16f1.blue, 14914);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, half opacity
    p16f.red = 41120; p16f.green = 47545; p16f.blue = 46003; p16f.alpha = FULL_OPACITY;
    p16f1.red = 26000; p16f1.green = 26000; p16f1.blue = 30000; p16f1.alpha = FULL_OPACITY;
    burn.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 127);
    QCOMPAREui(p16f1.red, 21104);
    QCOMPAREui(p16f1.green, 25345);
    QCOMPAREui(p16f1.blue, 25265);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 41120; p16f.green = 47545; p16f.blue = 46003; p16f.alpha = FULL_OPACITY;
    p16f1.red = 26000; p16f1.green = 26000; p16f1.blue = 30000; p16f1.alpha = FULL_OPACITY;
    burn.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 21104);
    QCOMPAREui(p16f1.green, 25345);
    QCOMPAREui(p16f1.blue, 25265);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, half opacity
    p16f.red = 41120; p16f.green = 47545; p16f.blue = 46003; p16f.alpha = FULL_OPACITY;
    p16f1.red = 26000; p16f1.green = 26000; p16f1.blue = 30000; p16f1.alpha = FULL_OPACITY;
    burn.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 127);
    QCOMPAREui(p16f1.red, 30356);
    QCOMPAREui(p16f1.green, 32468);
    QCOMPAREui(p16f1.blue, 30421);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent source
    p16f.red = 41120; p16f.green = 47545; p16f.blue = 46003; p16f.alpha = 0;
    p16f1.red = 26000; p16f1.green = 26000; p16f1.blue = 30000; p16f1.alpha = FULL_OPACITY;
    burn.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 26000);
    QCOMPAREui(p16f1.green, 26000);
    QCOMPAREui(p16f1.blue, 30000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent dst
    p16f.red = 41120; p16f.green = 47545; p16f.blue = 46003; p16f.alpha = FULL_OPACITY;
    p16f1.red = 26000; p16f1.green = 26000; p16f1.blue = 30000; p16f1.alpha = 0;
    burn.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 26000);
    QCOMPAREui(p16f1.green, 26000);
    QCOMPAREui(p16f1.blue, 30000);
    QCOMPAREui(p16f1.alpha, 0);

    // Test no mask, full opacity, half-transparent dst
    p16f.red = 41120; p16f.green = 47545; p16f.blue = 46003; p16f.alpha = FULL_OPACITY;
    p16f1.red = 26000; p16f1.green = 26000; p16f1.blue = 30000; p16f1.alpha = HALF_OPACITY;
    burn.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 14864);
    QCOMPAREui(p16f1.green, 20540);
    QCOMPAREui(p16f1.blue, 21788);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, half-transparent src
    p16f.red = 41120; p16f.green = 47545; p16f.blue = 46003; p16f.alpha = HALF_OPACITY;
    p16f1.red = 26000; p16f1.green = 26000; p16f1.blue = 30000; p16f1.alpha = FULL_OPACITY;
    burn.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 21032);
    QCOMPAREui(p16f1.green, 25289);
    QCOMPAREui(p16f1.blue, 25225);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);


    // Test no mask, full opacity, half-transparent src
    p16f.red = 41120; p16f.green = 47545; p16f.blue = 46003; p16f.alpha = HALF_OPACITY;
    p16f1.red = 26000; p16f1.green = 26000; p16f1.blue = 30000; p16f1.alpha = HALF_OPACITY;
    burn.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 14864);
    QCOMPAREui(p16f1.green, 20540);
    QCOMPAREui(p16f1.blue, 21788);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 41120; p16f.green = 47545; p16f.blue = 46003; p16f.alpha = QUARTER_OPACITY;
    p16f1.red = 26000; p16f1.green = 26000; p16f1.blue = 30000; p16f1.alpha = HALF_OPACITY;
    burn.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 24733);
    QCOMPAREui(p16f1.green, 28139);
    QCOMPAREui(p16f1.blue, 27287);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 41120; p16f.green = 47545; p16f.blue = 46003; p16f.alpha = HALF_OPACITY;
    p16f1.red = 26000; p16f1.green = 26000; p16f1.blue = 30000; p16f1.alpha = QUARTER_OPACITY;
    burn.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 18389);
    QCOMPAREui(p16f1.green, 23254);
    QCOMPAREui(p16f1.blue, 23752);
    QCOMPAREui(p16f1.alpha, QUARTER_OPACITY);
}

void TestKoCompositeOps::testCompositeDivide()
{
    KoRgbU16Traits::Pixel p16f;
    KoRgbU16Traits::Pixel p16f1;
    quint8* p16fPtr = reinterpret_cast<quint8*>(&p16f);
    quint8* p16fPtr1 = reinterpret_cast<quint8*>(&p16f1);
    
    KoCompositeOpDivide<KoRgbU16Traits> divide(0);
    // Test no mask, full opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    divide.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 65534);
    QCOMPAREui(p16f1.green, 4368);
    QCOMPAREui(p16f1.blue, 52425);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    divide.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 127);
    QCOMPAREui(p16f1.red, 40168);
    QCOMPAREui(p16f1.green, 2677);
    QCOMPAREui(p16f1.blue, 34141);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    divide.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 40168);
    QCOMPAREui(p16f1.green, 2677);
    QCOMPAREui(p16f1.blue, 34141);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    divide.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 127);
    QCOMPAREui(p16f1.red, 27534);
    QCOMPAREui(p16f1.green, 1835);
    QCOMPAREui(p16f1.blue, 25034);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent source
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = 0;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    divide.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 15000);
    QCOMPAREui(p16f1.green, 1000);
    QCOMPAREui(p16f1.blue, 16000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = 0;
    divide.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 15000);
    QCOMPAREui(p16f1.green, 1000);
    QCOMPAREui(p16f1.blue, 16000);
    QCOMPAREui(p16f1.alpha, 0);

    // Test no mask, full opacity, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    divide.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 48689);
    QCOMPAREui(p16f1.green, 3245);
    QCOMPAREui(p16f1.blue, 40283);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    divide.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 40266);
    QCOMPAREui(p16f1.green, 2684);
    QCOMPAREui(p16f1.blue, 34212);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);


    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    divide.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 48689);
    QCOMPAREui(p16f1.green, 3245);
    QCOMPAREui(p16f1.blue, 40283);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = QUARTER_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    divide.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 35212);
    QCOMPAREui(p16f1.green, 2347);
    QCOMPAREui(p16f1.blue, 30569);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
    divide.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 43876);
    QCOMPAREui(p16f1.green, 2925);
    QCOMPAREui(p16f1.blue, 36814);
    QCOMPAREui(p16f1.alpha, QUARTER_OPACITY);
}

void TestKoCompositeOps::testCompositeDodge()
{
    KoRgbU16Traits::Pixel p16f;
    KoRgbU16Traits::Pixel p16f1;
    quint8* p16fPtr = reinterpret_cast<quint8*>(&p16f);
    quint8* p16fPtr1 = reinterpret_cast<quint8*>(&p16f1);
    
    KoCompositeOpDodge<KoRgbU16Traits> dodge(0);
    // Test no mask, full opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    dodge.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 17699);
    QCOMPAREui(p16f1.green, 1295);
    QCOMPAREui(p16f1.blue, 23026);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    dodge.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 127);
    QCOMPAREui(p16f1.red, 16344);
    QCOMPAREui(p16f1.green, 1147);
    QCOMPAREui(p16f1.blue, 19499);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    dodge.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 16344);
    QCOMPAREui(p16f1.green, 1147);
    QCOMPAREui(p16f1.blue, 19499);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    dodge.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 127);
    QCOMPAREui(p16f1.red, 15669);
    QCOMPAREui(p16f1.green, 1073);
    QCOMPAREui(p16f1.blue, 17742);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent source
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = 0;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    dodge.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 15000);
    QCOMPAREui(p16f1.green, 1000);
    QCOMPAREui(p16f1.blue, 16000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = 0;
    dodge.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 15000);
    QCOMPAREui(p16f1.green, 1000);
    QCOMPAREui(p16f1.blue, 16000);
    QCOMPAREui(p16f1.alpha, 0);

    // Test no mask, full opacity, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    dodge.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 16799);
    QCOMPAREui(p16f1.green, 1197);
    QCOMPAREui(p16f1.blue, 20684);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    dodge.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 16349);
    QCOMPAREui(p16f1.green, 1147);
    QCOMPAREui(p16f1.blue, 19513);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);


    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    dodge.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 16799);
    QCOMPAREui(p16f1.green, 1197);
    QCOMPAREui(p16f1.blue, 20684);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = QUARTER_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    dodge.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 16079);
    QCOMPAREui(p16f1.green, 1118);
    QCOMPAREui(p16f1.blue, 18810);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
    dodge.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 16542);
    QCOMPAREui(p16f1.green, 1169);
    QCOMPAREui(p16f1.blue, 20015);
    QCOMPAREui(p16f1.alpha, QUARTER_OPACITY);
}

void TestKoCompositeOps::testCompositeInversedSubtract()
{
    KoRgbU16Traits::Pixel p16f;
    KoRgbU16Traits::Pixel p16f1;
    quint8* p16fPtr = reinterpret_cast<quint8*>(&p16f);
    quint8* p16fPtr1 = reinterpret_cast<quint8*>(&p16f1);
    
    KoCompositeOpInversedSubtract<KoRgbU16Traits> inversedSubtract(0);
    // Test no mask, full opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    inversedSubtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 0);
    QCOMPAREui(p16f1.green, 13999);
    QCOMPAREui(p16f1.blue, 4000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    inversedSubtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 127);
    QCOMPAREui(p16f1.red, 7529);
    QCOMPAREui(p16f1.green, 7474);
    QCOMPAREui(p16f1.blue, 10023);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    inversedSubtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 7529);
    QCOMPAREui(p16f1.green, 7474);
    QCOMPAREui(p16f1.blue, 10023);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    inversedSubtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 127);
    QCOMPAREui(p16f1.red, 11279);
    QCOMPAREui(p16f1.green, 4224);
    QCOMPAREui(p16f1.blue, 13023);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent source
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = 0;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    inversedSubtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 15000);
    QCOMPAREui(p16f1.green, 1000);
    QCOMPAREui(p16f1.blue, 16000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = 0;
    inversedSubtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 15000);
    QCOMPAREui(p16f1.green, 1000);
    QCOMPAREui(p16f1.blue, 16000);
    QCOMPAREui(p16f1.alpha, 0);

    // Test no mask, full opacity, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    inversedSubtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 5000);
    QCOMPAREui(p16f1.green, 9666);
    QCOMPAREui(p16f1.blue, 8000);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    inversedSubtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 7500);
    QCOMPAREui(p16f1.green, 7499);
    QCOMPAREui(p16f1.blue, 10000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);


    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    inversedSubtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 5000);
    QCOMPAREui(p16f1.green, 9666);
    QCOMPAREui(p16f1.blue, 8000);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = QUARTER_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    inversedSubtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 9000);
    QCOMPAREui(p16f1.green, 6199);
    QCOMPAREui(p16f1.blue, 11200);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
    inversedSubtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 6428);
    QCOMPAREui(p16f1.green, 8428);
    QCOMPAREui(p16f1.blue, 9143);
    QCOMPAREui(p16f1.alpha, QUARTER_OPACITY);
}

void TestKoCompositeOps::testCompositeMulitply()
{
    KoRgbU16Traits::Pixel p16f;
    KoRgbU16Traits::Pixel p16f1;
    quint8* p16fPtr = reinterpret_cast<quint8*>(&p16f);
    quint8* p16fPtr1 = reinterpret_cast<quint8*>(&p16f1);
    
    KoCompositeOpMultiply<KoRgbU16Traits> mulitply(0);
    // Test no mask, full opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    mulitply.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 2288);
    QCOMPAREui(p16f1.green, 228);
    QCOMPAREui(p16f1.blue, 4882);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    mulitply.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 127);
    QCOMPAREui(p16f1.red, 8669);
    QCOMPAREui(p16f1.green, 615);
    QCOMPAREui(p16f1.blue, 10462);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    mulitply.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 8669);
    QCOMPAREui(p16f1.green, 615);
    QCOMPAREui(p16f1.blue, 10462);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    mulitply.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 127);
    QCOMPAREui(p16f1.red, 11847);
    QCOMPAREui(p16f1.green, 808);
    QCOMPAREui(p16f1.blue, 13242);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent source
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = 0;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    mulitply.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 15000);
    QCOMPAREui(p16f1.green, 1000);
    QCOMPAREui(p16f1.blue, 16000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = 0;
    mulitply.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 15000);
    QCOMPAREui(p16f1.green, 1000);
    QCOMPAREui(p16f1.blue, 16000);
    QCOMPAREui(p16f1.alpha, 0);

    // Test no mask, full opacity, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    mulitply.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 6525);
    QCOMPAREui(p16f1.green, 485);
    QCOMPAREui(p16f1.blue, 8588);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    mulitply.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 8644);
    QCOMPAREui(p16f1.green, 614);
    QCOMPAREui(p16f1.blue, 10441);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);


    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    mulitply.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 6525);
    QCOMPAREui(p16f1.green, 485);
    QCOMPAREui(p16f1.blue, 8588);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = QUARTER_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    mulitply.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 9915);
    QCOMPAREui(p16f1.green, 691);
    QCOMPAREui(p16f1.blue, 11553);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
    mulitply.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 7736);
    QCOMPAREui(p16f1.green, 558);
    QCOMPAREui(p16f1.blue, 9647);
    QCOMPAREui(p16f1.alpha, QUARTER_OPACITY);
}

void TestKoCompositeOps::testCompositeOverlay()
{
    KoRgbU16Traits::Pixel p16f;
    KoRgbU16Traits::Pixel p16f1;
    quint8* p16fPtr = reinterpret_cast<quint8*>(&p16f);
    quint8* p16fPtr1 = reinterpret_cast<quint8*>(&p16f1);
    
    KoCompositeOpOverlay<KoRgbU16Traits> overlay(0);
    // Test no mask, full opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    overlay.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 6963);
    QCOMPAREui(p16f1.green, 466);
    QCOMPAREui(p16f1.blue, 11287);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    overlay.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 127);
    QCOMPAREui(p16f1.red, 10997);
    QCOMPAREui(p16f1.green, 734);
    QCOMPAREui(p16f1.blue, 13652);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    overlay.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10997);
    QCOMPAREui(p16f1.green, 734);
    QCOMPAREui(p16f1.blue, 13652);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    overlay.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 127);
    QCOMPAREui(p16f1.red, 13006);
    QCOMPAREui(p16f1.green, 867);
    QCOMPAREui(p16f1.blue, 14831);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent source
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = 0;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    overlay.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 15000);
    QCOMPAREui(p16f1.green, 1000);
    QCOMPAREui(p16f1.blue, 16000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = 0;
    overlay.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 15000);
    QCOMPAREui(p16f1.green, 1000);
    QCOMPAREui(p16f1.blue, 16000);
    QCOMPAREui(p16f1.alpha, 0);

    // Test no mask, full opacity, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    overlay.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 9642);
    QCOMPAREui(p16f1.green, 644);
    QCOMPAREui(p16f1.blue, 12858);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    overlay.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10981);
    QCOMPAREui(p16f1.green, 733);
    QCOMPAREui(p16f1.blue, 13643);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);


    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    overlay.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 9642);
    QCOMPAREui(p16f1.green, 644);
    QCOMPAREui(p16f1.blue, 12858);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = QUARTER_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    overlay.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 11785);
    QCOMPAREui(p16f1.green, 786);
    QCOMPAREui(p16f1.blue, 14114);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
    overlay.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10407);
    QCOMPAREui(p16f1.green, 694);
    QCOMPAREui(p16f1.blue, 13306);
    QCOMPAREui(p16f1.alpha, QUARTER_OPACITY);
}

void TestKoCompositeOps::testCompositeScreen()
{
    KoRgbU16Traits::Pixel p16f;
    KoRgbU16Traits::Pixel p16f1;
    quint8* p16fPtr = reinterpret_cast<quint8*>(&p16f);
    quint8* p16fPtr1 = reinterpret_cast<quint8*>(&p16f1);
    
    KoCompositeOpScreen<KoRgbU16Traits> screen(0);
    // Test no mask, full opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    screen.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 22711);
    QCOMPAREui(p16f1.green, 15771);
    QCOMPAREui(p16f1.blue, 31117);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    screen.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 127);
    QCOMPAREui(p16f1.red, 18840);
    QCOMPAREui(p16f1.green, 8356);
    QCOMPAREui(p16f1.blue, 23529);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    screen.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 18840);
    QCOMPAREui(p16f1.green, 8356);
    QCOMPAREui(p16f1.blue, 23529);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    screen.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 127);
    QCOMPAREui(p16f1.red, 16912);
    QCOMPAREui(p16f1.green, 4663);
    QCOMPAREui(p16f1.blue, 19749);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent source
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = 0;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    screen.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 15000);
    QCOMPAREui(p16f1.green, 1000);
    QCOMPAREui(p16f1.blue, 16000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = 0;
    screen.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 15000);
    QCOMPAREui(p16f1.green, 1000);
    QCOMPAREui(p16f1.blue, 16000);
    QCOMPAREui(p16f1.alpha, 0);

    // Test no mask, full opacity, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    screen.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 20141);
    QCOMPAREui(p16f1.green, 10847);
    QCOMPAREui(p16f1.blue, 26078);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    screen.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 18855);
    QCOMPAREui(p16f1.green, 8385);
    QCOMPAREui(p16f1.blue, 23558);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);


    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    screen.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 20141);
    QCOMPAREui(p16f1.green, 10847);
    QCOMPAREui(p16f1.blue, 26078);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = QUARTER_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    screen.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 18084);
    QCOMPAREui(p16f1.green, 6908);
    QCOMPAREui(p16f1.blue, 22046);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
    screen.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 19406);
    QCOMPAREui(p16f1.green, 9440);
    QCOMPAREui(p16f1.blue, 24638);
    QCOMPAREui(p16f1.alpha, QUARTER_OPACITY);
}

void TestKoCompositeOps::testCompositeSubtract()
{
    KoRgbU16Traits::Pixel p16f;
    KoRgbU16Traits::Pixel p16f1;
    quint8* p16fPtr = reinterpret_cast<quint8*>(&p16f);
    quint8* p16fPtr1 = reinterpret_cast<quint8*>(&p16f1);
    
    KoCompositeOpSubtract<KoRgbU16Traits> subtract(0);
    // Test no mask, full opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    subtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 5000);
    QCOMPAREui(p16f1.green, 0);
    QCOMPAREui(p16f1.blue, 0);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    subtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 127);
    QCOMPAREui(p16f1.red, 10019);
    QCOMPAREui(p16f1.green, 501);
    QCOMPAREui(p16f1.blue, 8031);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    subtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10019);
    QCOMPAREui(p16f1.green, 501);
    QCOMPAREui(p16f1.blue, 8031);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    subtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 127);
    QCOMPAREui(p16f1.red, 12519);
    QCOMPAREui(p16f1.green, 751);
    QCOMPAREui(p16f1.blue, 12031);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent source
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = 0;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    subtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 15000);
    QCOMPAREui(p16f1.green, 1000);
    QCOMPAREui(p16f1.blue, 16000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = 0;
    subtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 15000);
    QCOMPAREui(p16f1.green, 1000);
    QCOMPAREui(p16f1.blue, 16000);
    QCOMPAREui(p16f1.alpha, 0);

    // Test no mask, full opacity, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    subtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 8333);
    QCOMPAREui(p16f1.green, 333);
    QCOMPAREui(p16f1.blue, 5333);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    subtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 500);
    QCOMPAREui(p16f1.blue, 8000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);


    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    subtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 8333);
    QCOMPAREui(p16f1.green, 333);
    QCOMPAREui(p16f1.blue, 5333);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = QUARTER_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    subtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 11000);
    QCOMPAREui(p16f1.green, 600);
    QCOMPAREui(p16f1.blue, 9600);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
    subtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 9285);
    QCOMPAREui(p16f1.green, 428);
    QCOMPAREui(p16f1.blue, 6857);
    QCOMPAREui(p16f1.alpha, QUARTER_OPACITY);
}

QTEST_KDEMAIN(TestKoCompositeOps, NoGUI)
#include "TestKoCompositeOps.moc"
