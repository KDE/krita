/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <KoColorSpace.h>

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
#include <KoCompositeOpCopy.h>
#include <KoCompositeOpCopy2.h>

#include <KoColorSpaceRegistry.h>
#include <KoColor.h>

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
    QCOMPAREui(p16f1.red, 12510);
    QCOMPAREui(p16f1.green, 7972);
    QCOMPAREui(p16f1.blue, 17992);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 12510);
    QCOMPAREui(p16f1.green, 7972);
    QCOMPAREui(p16f1.blue, 17992);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 127);
    QCOMPAREui(p16f1.red, 13760);
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
    QCOMPAREui(p16f1.red, 12501);
    QCOMPAREui(p16f1.green, 7999);
    QCOMPAREui(p16f1.blue, 17999);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, half-transparent src, dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 11667);
    QCOMPAREui(p16f1.green, 10333);
    QCOMPAREui(p16f1.blue, 18666);
    QCOMPAREui(p16f1.alpha, 49151);

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = QUARTER_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 13001);
    QCOMPAREui(p16f1.green, 6599);
    QCOMPAREui(p16f1.blue, 17599);
    QCOMPAREui(p16f1.alpha, 40959);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
    over.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 11000);
    QCOMPAREui(p16f1.green, 12200);
    QCOMPAREui(p16f1.blue, 19200);
    QCOMPAREui(p16f1.alpha, 40959);
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
    QCOMPAREui(p16f1.red, 12510);
    QCOMPAREui(p16f1.green, 7972);
    QCOMPAREui(p16f1.blue, 17992);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    alphaDarken.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 12510);
    QCOMPAREui(p16f1.green, 7972);
    QCOMPAREui(p16f1.blue, 17992);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    alphaDarken.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 127);
    QCOMPAREui(p16f1.red, 13760);
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
    QCOMPAREui(p16f1.red, 12501);
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
    QCOMPAREui(p16f1.red, 12501);
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
    QCOMPAREui(p16f1.red, 25000);
    QCOMPAREui(p16f1.green, 16000);
    QCOMPAREui(p16f1.blue, 36000);
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
    QCOMPAREui(p16f1.green, 11000);
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
    QCOMPAREui(p16f1.green, 11000);
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
    QCOMPAREui(p16f1.red, 2527);
    QCOMPAREui(p16f1.green, 11042);
    QCOMPAREui(p16f1.blue, 14913);
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
    QCOMPAREui(p16f1.red, 14863);
    QCOMPAREui(p16f1.green, 20540);
    QCOMPAREui(p16f1.blue, 21787);
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
    QCOMPAREui(p16f1.red, 14863);
    QCOMPAREui(p16f1.green, 20540);
    QCOMPAREui(p16f1.blue, 21787);
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
    QCOMPAREui(p16f1.red, 18388);
    QCOMPAREui(p16f1.green, 23254);
    QCOMPAREui(p16f1.blue, 23751);
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
    QCOMPAREui(p16f1.red, 65535);
    QCOMPAREui(p16f1.green, 4369);
    QCOMPAREui(p16f1.blue, 52426);
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
    QCOMPAREui(p16f1.red, 48690);
    QCOMPAREui(p16f1.green, 3246);
    QCOMPAREui(p16f1.blue, 40284);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    divide.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 40267);
    QCOMPAREui(p16f1.green, 2684);
    QCOMPAREui(p16f1.blue, 34212);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);


    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    divide.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 48690);
    QCOMPAREui(p16f1.green, 3246);
    QCOMPAREui(p16f1.blue, 40284);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = QUARTER_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    divide.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 35213);
    QCOMPAREui(p16f1.green, 2347);
    QCOMPAREui(p16f1.blue, 30569);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
    divide.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 43877);
    QCOMPAREui(p16f1.green, 2925);
    QCOMPAREui(p16f1.blue, 36815);
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
    QCOMPAREui(p16f1.red, 17700);
    QCOMPAREui(p16f1.green, 1296);
    QCOMPAREui(p16f1.blue, 23027);
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
    QCOMPAREui(p16f1.red, 16800);
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
    QCOMPAREui(p16f1.red, 16800);
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
    QCOMPAREui(p16f1.green, 14000);
    QCOMPAREui(p16f1.blue, 4000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    inversedSubtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 127);
    QCOMPAREui(p16f1.red, 7530);
    QCOMPAREui(p16f1.green, 7474);
    QCOMPAREui(p16f1.blue, 10024);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    inversedSubtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 7530);
    QCOMPAREui(p16f1.green, 7474);
    QCOMPAREui(p16f1.blue, 10024);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    inversedSubtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 127);
    QCOMPAREui(p16f1.red, 11280);
    QCOMPAREui(p16f1.green, 4224);
    QCOMPAREui(p16f1.blue, 13024);
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
    QCOMPAREui(p16f1.red, 7501);
    QCOMPAREui(p16f1.green, 7499);
    QCOMPAREui(p16f1.blue, 10001);
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
    QCOMPAREui(p16f1.red, 9001);
    QCOMPAREui(p16f1.green, 6199);
    QCOMPAREui(p16f1.blue, 11201);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
    inversedSubtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 6429);
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
    QCOMPAREui(p16f1.red, 2289);
    QCOMPAREui(p16f1.green, 229);
    QCOMPAREui(p16f1.blue, 4883);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    mulitply.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 127);
    QCOMPAREui(p16f1.red, 8670);
    QCOMPAREui(p16f1.green, 617);
    QCOMPAREui(p16f1.blue, 10464);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    mulitply.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 8670);
    QCOMPAREui(p16f1.green, 617);
    QCOMPAREui(p16f1.blue, 10464);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    mulitply.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 127);
    QCOMPAREui(p16f1.red, 11848);
    QCOMPAREui(p16f1.green, 809);
    QCOMPAREui(p16f1.blue, 13243);
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
    QCOMPAREui(p16f1.red, 6526);
    QCOMPAREui(p16f1.green, 486);
    QCOMPAREui(p16f1.blue, 8589);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    mulitply.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 8645);
    QCOMPAREui(p16f1.green, 615);
    QCOMPAREui(p16f1.blue, 10442);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);


    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    mulitply.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 6526);
    QCOMPAREui(p16f1.green, 486);
    QCOMPAREui(p16f1.blue, 8589);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = QUARTER_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    mulitply.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 9916);
    QCOMPAREui(p16f1.green, 692);
    QCOMPAREui(p16f1.blue, 11554);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
    mulitply.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 7737);
    QCOMPAREui(p16f1.green, 560);
    QCOMPAREui(p16f1.blue, 9648);
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
    QCOMPAREui(p16f1.blue, 11288);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    overlay.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 127);
    QCOMPAREui(p16f1.red, 10998);
    QCOMPAREui(p16f1.green, 735);
    QCOMPAREui(p16f1.blue, 13654);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    overlay.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10998);
    QCOMPAREui(p16f1.green, 735);
    QCOMPAREui(p16f1.blue, 13654);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    overlay.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 127);
    QCOMPAREui(p16f1.red, 13007);
    QCOMPAREui(p16f1.green, 868);
    QCOMPAREui(p16f1.blue, 14832);
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
    QCOMPAREui(p16f1.blue, 12859);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    overlay.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10982);
    QCOMPAREui(p16f1.green, 734);
    QCOMPAREui(p16f1.blue, 13645);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);


    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    overlay.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 9642);
    QCOMPAREui(p16f1.green, 644);
    QCOMPAREui(p16f1.blue, 12859);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = QUARTER_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    overlay.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 11786);
    QCOMPAREui(p16f1.green, 787);
    QCOMPAREui(p16f1.blue, 14116);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
    overlay.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10408);
    QCOMPAREui(p16f1.green, 695);
    QCOMPAREui(p16f1.blue, 13308);
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
    QCOMPAREui(p16f1.blue, 23528);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    screen.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 18840);
    QCOMPAREui(p16f1.green, 8356);
    QCOMPAREui(p16f1.blue, 23528);
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
    QCOMPAREui(p16f1.red, 20140);
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
    QCOMPAREui(p16f1.red, 20140);
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
    QCOMPAREui(p16f1.red, 10020);
    QCOMPAREui(p16f1.green, 502);
    QCOMPAREui(p16f1.blue, 8032);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    subtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10020);
    QCOMPAREui(p16f1.green, 502);
    QCOMPAREui(p16f1.blue, 8032);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    subtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 127);
    QCOMPAREui(p16f1.red, 12520);
    QCOMPAREui(p16f1.green, 752);
    QCOMPAREui(p16f1.blue, 12032);
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
    QCOMPAREui(p16f1.red, 8334);
    QCOMPAREui(p16f1.green, 334);
    QCOMPAREui(p16f1.blue, 5334);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    subtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10001);
    QCOMPAREui(p16f1.green, 501);
    QCOMPAREui(p16f1.blue, 8001);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);


    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    subtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 8334);
    QCOMPAREui(p16f1.green, 334);
    QCOMPAREui(p16f1.blue, 5334);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = QUARTER_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    subtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 11001);
    QCOMPAREui(p16f1.green, 601);
    QCOMPAREui(p16f1.blue, 9601);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
    subtract.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 9286);
    QCOMPAREui(p16f1.green, 429);
    QCOMPAREui(p16f1.blue, 6858);
    QCOMPAREui(p16f1.alpha, QUARTER_OPACITY);
}

void TestKoCompositeOps::testCompositeCopy2()
{
    KoRgbU16Traits::Pixel p16f;
    KoRgbU16Traits::Pixel p16f1;
    quint8* p16fPtr = reinterpret_cast<quint8*>(&p16f);
    quint8* p16fPtr1 = reinterpret_cast<quint8*>(&p16f1);
    
    KoCompositeOpCopy2<KoRgbU16Traits> copy(0);
    // Test no mask, full opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    copy.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    copy.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 127);
    QCOMPAREui(p16f1.red, 12510);
    QCOMPAREui(p16f1.green, 7972);
    QCOMPAREui(p16f1.blue, 17992);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    copy.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 255);
    QCOMPAREui(p16f1.red, 12510);
    QCOMPAREui(p16f1.green, 7972);
    QCOMPAREui(p16f1.blue, 17992);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test mask, half opacity
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    copy.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 1, 1, 1, 127);
    QCOMPAREui(p16f1.red, 13760);
    QCOMPAREui(p16f1.green, 4472);
    QCOMPAREui(p16f1.blue, 16992);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, transparent source
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = 0;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    copy.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, 0);

    // Test no mask, full opacity, transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = 0;
    copy.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = FULL_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    copy.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, FULL_OPACITY);

    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = FULL_OPACITY;
    copy.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);


    // Test no mask, full opacity, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    copy.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, full opacity, quarter-transparent src, half-transparent dst
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = QUARTER_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = HALF_OPACITY;
    copy.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, QUARTER_OPACITY);

    // Test no mask, full opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
    copy.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 255);
    QCOMPAREui(p16f1.red, 10000);
    QCOMPAREui(p16f1.green, 15000);
    QCOMPAREui(p16f1.blue, 20000);
    QCOMPAREui(p16f1.alpha, HALF_OPACITY);

    // Test no mask, half opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
    copy.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, 0, 0, 1, 1, 127);
    QCOMPAREui(p16f1.red, 12510);
    QCOMPAREui(p16f1.green, 7972);
    QCOMPAREui(p16f1.blue, 17992);
    QCOMPAREui(p16f1.alpha, 24542);

    // Test mask, half opacity, quarter-transparent dst, half-transparent src
    p16f.red = 10000; p16f.green = 15000; p16f.blue = 20000; p16f.alpha = HALF_OPACITY;
    p16f1.red = 15000; p16f1.green = 1000; p16f1.blue = 16000; p16f1.alpha = QUARTER_OPACITY;
    copy.composite(p16fPtr1, KoRgbU16Traits::pixelSize, p16fPtr, KoRgbU16Traits::pixelSize, &mask, 0, 1, 1, 127);
    QCOMPAREui(p16f1.red, 13760);
    QCOMPAREui(p16f1.green, 4472);
    QCOMPAREui(p16f1.blue, 16992);
    QCOMPAREui(p16f1.alpha, 20446);
}

void TestKoCompositeOps::testCompositeCopy()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    const KoCompositeOp * copy = cs->compositeOp(COMPOSITE_COPY);
   
    KoColor black(Qt::black, cs);
    KoColor white(Qt::white, cs);
    KoColor opaque(QColor(0,0,0,0), cs);
    
    int w = 512;
    int h = 512;
    
    int pixelCount = w * h;
    quint32 pixelSize = cs->pixelSize();
    // dst
    quint8 * layer = new quint8[pixelCount * pixelSize];
    quint8 * iter = layer;
    for (int i = 0; i < pixelCount; i++){
        memcpy(iter, white.data() , pixelSize);
        iter += pixelSize;
    }
    
    // full white image
    //cs->convertToQImage(layer, w, h, 0,KoColorConversionTransformation::IntentPerceptual).save("0dst.png");
    
    // src
    quint8 * dab = new quint8[pixelCount * pixelSize];
    iter = dab;
    for (int i = 0; i < pixelCount; i++){
        memcpy(iter, black.data() , pixelSize);
        iter += pixelSize;
    }
    
    // full black image
    //cs->convertToQImage(dab, w, h, 0,KoColorConversionTransformation::IntentPerceptual).save("1src.png");


    // selection
    quint32 selectionPixelSize = KoColorSpaceRegistry::instance()->alpha8()->pixelSize();
    quint8 * selection = new quint8[pixelCount * selectionPixelSize];
    iter = selection;
    for (int height = 0; height < h; height++){
        for (int width = 0; width < w; width++){
            if ((height > 128) && (height < 256) && (width > 128) && (width < 256)){
                *iter = 255;
            }else{
                *iter = 0;
            }
            iter += selectionPixelSize;
        }
    }
    
    // white rectangle at 128,128  
    //KoColorSpaceRegistry::instance()->alpha8()->convertToQImage(selection, w, h, 0, KoColorConversionTransformation::IntentPerceptual).save("1mask.png");

    copy->composite(layer,w * pixelSize, 
                    dab, w * pixelSize, 
                    0,0, 
                    h, w,
                    255,
                    QBitArray());
                    
    
    // full black image
    //cs->convertToQImage(layer, w, h, 0,KoColorConversionTransformation::IntentPerceptual).save("2result.png");
    
    copy->composite(layer,w * pixelSize,
                    opaque.data(), 0,
                    0,0,
                    h,w,
                    255,
                    QBitArray()
                    );
    
    // full opaque image
    //cs->convertToQImage(layer, w, h, 0,KoColorConversionTransformation::IntentPerceptual).save("3result.png");
    
    copy->composite(layer,w * pixelSize,
                    dab, w * pixelSize,
                    selection, w * selectionPixelSize,
                    h,w,
                    255,
                    QBitArray()
                    );
    
    // black rectangle on opaque background
    QImage result = cs->convertToQImage(layer, w, h, 0,KoColorConversionTransformation::IntentPerceptual);
    QImage expectedResult(QString(FILES_DATA_DIR) + QDir::separator() + "CopyWithSelectionExpectedResult.png");
    
    bool testOk = (result == expectedResult);
    QVERIFY2(testOk, "Images are not equal");
    if (!testOk){
        qDebug() << "Saving the result";
        result.save("CopyWithSelection.png");
    }
    
    copy->composite(layer, w * pixelSize,
                    white.data(), 0,
                    selection, w * selectionPixelSize,
                    h,w,
                    255,
                    QBitArray());
                    
                    
    result = cs->convertToQImage(layer, w, h, 0,KoColorConversionTransformation::IntentPerceptual);
    expectedResult = QImage(QString(FILES_DATA_DIR) + QDir::separator() + "CopySingleWithSelectionExpectedResult.png");
    
    
    testOk = (result == expectedResult);
    QVERIFY2(testOk, "Images with single pixel and selection are not equal");
    if (!testOk){
        qDebug() << "Saving the result";
        result.save("CopySingleWithSelection.png");
    }
    
    
    
}


QTEST_KDEMAIN(TestKoCompositeOps, NoGUI)
#include "TestKoCompositeOps.moc"
