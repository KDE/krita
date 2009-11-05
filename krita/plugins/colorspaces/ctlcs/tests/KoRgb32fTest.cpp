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
#include <KoCompositeOp.h>

template<typename _T_>
void randomizator(typename KoRgbTraits<_T_>::Pixel& p)
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
    KoRgbTraits<float>::Pixel p32f1;
    quint8* p32fPtr1 = reinterpret_cast<quint8*>(&p32f1);
    KoRgbTraits<float>::Pixel p32f2;
    quint8* p32fPtr2 = reinterpret_cast<quint8*>(&p32f2);
    KoRgbTraits<float>::Pixel p32f3;
    quint8* p32fPtr3 = reinterpret_cast<quint8*>(&p32f3);
    KoRgbU16Traits::Pixel p16u;
    quint8* p16uPtr = reinterpret_cast<quint8*>(&p16u);

    // Test alpha function
    p32f.alpha = 1.0;
    QCOMPARE(qint32(rgb32f->alpha(p32fPtr)), 255);
    p32f.alpha = 0.5;
    QCOMPARE(qint32(rgb32f->alpha(p32fPtr)), 127);

    // Test setAlpha
    rgb32f->setAlpha(p32fPtr, 255, 1);
    QCOMPARE(p32f.alpha, 1.0f);
    rgb32f->setAlpha(p32fPtr, 0, 1);
    QCOMPARE(p32f.alpha, 0.0f);
    rgb32f->setAlpha(p32fPtr, 127, 1);
    QCOMPARE(p32f.alpha, float(127 / 255.0));

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

    // Test mix op
    quint8* colors[3];
    colors[0] = p32fPtr;
    colors[1] = p32fPtr1;
    colors[2] = p32fPtr2;
    p32f.red = 0.5; p32f.green = 0.1; p32f.blue = 0.6; p32f.alpha = 1.0;
    p32f1.red = 0.3; p32f1.green = 0.5; p32f1.blue = 0.8; p32f1.alpha = 1.0;
    p32f2.red = 0.7; p32f2.green = 0.6; p32f2.blue = 0.7; p32f2.alpha = 1.0;
    p32f3.red = -1.0; p32f3.green = -1.0; p32f3.blue = -1.0; p32f3.alpha = -1.0;
    qint16 weights[3];
    weights[0] = qint16(255 / 3);
    weights[1] = qint16(255 / 3);
    weights[2] = qint16(255 / 3);
    rgb32f->mixColorsOp()->mixColors(colors, weights, 3, p32fPtr3);
    QCOMPARE(p32f3.red, 0.5f);
    QCOMPARE(p32f3.green, 0.4f);
    QCOMPARE(p32f3.blue, 0.7f);
    QCOMPARE(p32f3.alpha, 1.0f);

    // Test composite op
    const KoCompositeOp* over = rgb32f->compositeOp(COMPOSITE_OVER);
    QVERIFY(over);
    // Test no mask, full opacity
    p32f.red = 0.5; p32f.green = 0.1; p32f.blue = 0.6; p32f.alpha = 1.0;
    p32f1.red = 0.3; p32f1.green = 0.5; p32f1.blue = 0.8; p32f1.alpha = 1.0;
    over->composite(p32fPtr1, rgb32f->pixelSize(), p32fPtr, rgb32f->pixelSize(), 0, 0, 1, 1, 255);
    QCOMPARE(p32f1.red, 0.5f);
    QCOMPARE(p32f1.green, 0.1f);
    QCOMPARE(p32f1.blue, 0.6f);
    QCOMPARE(p32f1.alpha, 1.0f);

    // Test no mask, half opacity
    p32f.red = 0.5; p32f.green = 0.1; p32f.blue = 0.6; p32f.alpha = 1.0;
    p32f1.red = 0.3; p32f1.green = 0.5; p32f1.blue = 0.8; p32f1.alpha = 1.0;
    over->composite(p32fPtr1, rgb32f->pixelSize(), p32fPtr, rgb32f->pixelSize(), 0, 0, 1, 1, 127);
    QCOMPARE(p32f1.red, 0.399608f);
    QCOMPARE(p32f1.green, 0.300784f);
    QCOMPARE(p32f1.blue, 0.700392f);
    QCOMPARE(p32f1.alpha, 1.0f);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p32f.red = 0.5; p32f.green = 0.1; p32f.blue = 0.6; p32f.alpha = 1.0;
    p32f1.red = 0.3; p32f1.green = 0.5; p32f1.blue = 0.8; p32f1.alpha = 1.0;
    over->composite(p32fPtr1, rgb32f->pixelSize(), p32fPtr, rgb32f->pixelSize(), &mask, 1, 1, 1, 255);
    QCOMPARE(p32f1.red, 0.399608f);
    QCOMPARE(p32f1.green, 0.300784f);
    QCOMPARE(p32f1.blue, 0.700392f);
    QCOMPARE(p32f1.alpha, 1.0f);

    // Test mask, full opacity
    p32f.red = 0.5; p32f.green = 0.1; p32f.blue = 0.6; p32f.alpha = 1.0;
    p32f1.red = 0.3; p32f1.green = 0.5; p32f1.blue = 0.8; p32f1.alpha = 1.0;
    over->composite(p32fPtr1, rgb32f->pixelSize(), p32fPtr, rgb32f->pixelSize(), &mask, 1, 1, 1, 127);
    QCOMPARE(p32f1.red, 0.349609f);
    QCOMPARE(p32f1.green, 0.400783f);
    QCOMPARE(p32f1.blue, 0.750391f);
    QCOMPARE(p32f1.alpha, 1.0f);

    // Test no mask, full opacity, transparent source
    p32f.red = 0.5; p32f.green = 0.1; p32f.blue = 0.6; p32f.alpha = 0.0;
    p32f1.red = 0.3; p32f1.green = 0.5; p32f1.blue = 0.8; p32f1.alpha = 1.0;
    over->composite(p32fPtr1, rgb32f->pixelSize(), p32fPtr, rgb32f->pixelSize(), 0, 0, 1, 1, 255);
    QCOMPARE(p32f1.red, 0.3f);
    QCOMPARE(p32f1.green, 0.5f);
    QCOMPARE(p32f1.blue, 0.8f);
    QCOMPARE(p32f1.alpha, 1.0f);

    // Test no mask, full opacity, transparent dst
    p32f.red = 0.5; p32f.green = 0.1; p32f.blue = 0.6; p32f.alpha = 1.0;
    p32f1.red = 0.3; p32f1.green = 0.5; p32f1.blue = 0.8; p32f1.alpha = 0.0;
    over->composite(p32fPtr1, rgb32f->pixelSize(), p32fPtr, rgb32f->pixelSize(), 0, 0, 1, 1, 255);
    QCOMPARE(p32f1.red, 0.5f);
    QCOMPARE(p32f1.green, 0.1f);
    QCOMPARE(p32f1.blue, 0.6f);
    QCOMPARE(p32f1.alpha, 1.0f);

    // Test no mask, full opacity, half-transparent dst
    p32f.red = 0.5; p32f.green = 0.1; p32f.blue = 0.6; p32f.alpha = 1.0;
    p32f1.red = 0.3; p32f1.green = 0.5; p32f1.blue = 0.8; p32f1.alpha = 0.5;
    over->composite(p32fPtr1, rgb32f->pixelSize(), p32fPtr, rgb32f->pixelSize(), 0, 0, 1, 1, 255);
    QCOMPARE(p32f1.red, 0.5f);
    QCOMPARE(p32f1.green, 0.1f);
    QCOMPARE(p32f1.blue, 0.6f);
    QCOMPARE(p32f1.alpha, 1.0f);
}

QTEST_KDEMAIN(KoRgb32fTest, NoGUI)
#include "KoRgb32fTest.moc"
