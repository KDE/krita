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

#include "KoRgb16fTest.h"

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

static inline bool qFuzzyCompare(half p1, half p2)
{
    return (qAbs(p1 - p2) <= 0.001f * qMin(qAbs(p1), qAbs(p2)));
}

namespace QTest
{
    template <>
    Q_TESTLIB_EXPORT bool qCompare<half>(half const &t1, half const &t2, const char *actual, const char *expected,
                        const char *file, int line)
    {
        return qFuzzyCompare(t1, t2)
                ? compare_helper(true, "COMPARE()", file, line)
                : compare_helper(false, "Compared halfs are not the same (fuzzy compare)",
                                toString(float(t1)), toString(float(t2)), actual, expected, file, line);
    }
}
#define QCOMPAREh(v,v2) QCOMPARE(v,half(v2))

void KoRgb16fTest::testConversion()
{
    const KoColorSpace* rgb16f = KoColorSpaceRegistry::instance()->colorSpace("RgbAF16", 0);
    QVERIFY(rgb16f);
    KoRgbTraits<half>::Pixel p16f;
    quint8* p16fPtr = reinterpret_cast<quint8*>(&p16f);
    KoRgbTraits<half>::Pixel p16f1;
    quint8* p16fPtr1 = reinterpret_cast<quint8*>(&p16f1);
    KoRgbTraits<half>::Pixel p16f2;
    quint8* p16fPtr2 = reinterpret_cast<quint8*>(&p16f2);
    KoRgbTraits<half>::Pixel p16f3;
    quint8* p16fPtr3 = reinterpret_cast<quint8*>(&p16f3);
    KoRgbU16Traits::Pixel p16u;
    quint8* p16uPtr = reinterpret_cast<quint8*>(&p16u);

    // Test alpha function
    p16f.alpha = 1.0;
    QCOMPARE(qint32(rgb16f->alpha(p16fPtr)), 255);
    p16f.alpha = 0.5;
    QCOMPARE(qint32(rgb16f->alpha(p16fPtr)), 127);

    // Test setAlpha
    rgb16f->setAlpha(p16fPtr, 255, 1);
    QCOMPAREh(p16f.alpha, 1.0f);
    rgb16f->setAlpha(p16fPtr, 0, 1);
    QCOMPAREh(p16f.alpha, 0.0f);
    rgb16f->setAlpha(p16fPtr, 127, 1);
    QCOMPAREh(p16f.alpha, float(127 / half(255.0)));

    // Test conversion of black from 16f to 16u back to 16f
    p16f.red = 0.0;
    p16f.green = 0.0;
    p16f.blue = 0.0;
    p16f.alpha = 1.0;
    randomizator<quint16>(p16u);
    rgb16f->toRgbA16(p16fPtr, p16uPtr, 1);
    QCOMPARE(p16u.red, quint16(0));
    QCOMPARE(p16u.green, quint16(0));
    QCOMPARE(p16u.blue, quint16(0));
    QCOMPARE(p16u.alpha, quint16(65535));
    rgb16f->fromRgbA16(p16uPtr, p16fPtr, 1);
    QCOMPAREh(p16f.red, 0.0f);
    QCOMPAREh(p16f.green, 0.0f);
    QCOMPAREh(p16f.blue, 0.0f);
    QCOMPAREh(p16f.alpha, 1.0f);

    // Test conversion to QColor
    QColor color;
    rgb16f->toQColor(p16fPtr, &color, 0);
    QCOMPARE(color.red(), 0);
    QCOMPARE(color.green(), 0);
    QCOMPARE(color.blue(), 0);
    QCOMPARE(color.alpha(), 255);
    rgb16f->fromQColor(color, p16fPtr, 0);
    QCOMPAREh(p16f.red, 0.0f);
    QCOMPAREh(p16f.green, 0.0f);
    QCOMPAREh(p16f.blue, 0.0f);
    QCOMPAREh(p16f.alpha, 1.0f);

    // Test conversion of white from 16f to 16u back to 16f
    p16f.red = 1.0;
    p16f.green = 1.0;
    p16f.blue = 1.0;
    p16f.alpha = 1.0;
    randomizator<quint16>(p16u);
    rgb16f->toRgbA16(p16fPtr, p16uPtr, 1);
    QCOMPARE(p16u.red, quint16(47803));
    QCOMPARE(p16u.green, quint16(47803));
    QCOMPARE(p16u.blue, quint16(47803));
    QCOMPARE(p16u.alpha, quint16(65535));
    rgb16f->fromRgbA16(p16uPtr, p16fPtr, 1);
    QCOMPAREh(p16f.red, 1.0f);
    QCOMPAREh(p16f.green, 1.0f);
    QCOMPAREh(p16f.blue, 1.0f);
    QCOMPAREh(p16f.alpha, 1.0f);

    // Test mix op
    quint8* colors[3];
    colors[0] = p16fPtr;
    colors[1] = p16fPtr1;
    colors[2] = p16fPtr2;
    p16f.red = 0.5; p16f.green = 0.1; p16f.blue = 0.6; p16f.alpha = 1.0;
    p16f1.red = 0.3; p16f1.green = 0.5; p16f1.blue = 0.8; p16f1.alpha = 1.0;
    p16f2.red = 0.7; p16f2.green = 0.6; p16f2.blue = 0.7; p16f2.alpha = 1.0;
    p16f3.red = -1.0; p16f3.green = -1.0; p16f3.blue = -1.0; p16f3.alpha = -1.0;
    qint16 weights[3];
    weights[0] = qint16(255 / 3);
    weights[1] = qint16(255 / 3);
    weights[2] = qint16(255 / 3);
    rgb16f->mixColorsOp()->mixColors(colors, weights, 3, p16fPtr3);
    QCOMPAREh(p16f3.red, 0.5f);
    QCOMPAREh(p16f3.green, 0.4f);
    QCOMPAREh(p16f3.blue, 0.7f);
    QCOMPAREh(p16f3.alpha, 1.0f);

    // Test composite op
    const KoCompositeOp* over = rgb16f->compositeOp(COMPOSITE_OVER);
    QVERIFY(over);
    // Test no mask, full opacity
    p16f.red = 0.5; p16f.green = 0.1; p16f.blue = 0.6; p16f.alpha = 1.0;
    p16f1.red = 0.3; p16f1.green = 0.5; p16f1.blue = 0.8; p16f1.alpha = 1.0;
    over->composite(p16fPtr1, rgb16f->pixelSize(), p16fPtr, rgb16f->pixelSize(), 0, 0, 1, 1, 255);
    QCOMPAREh(p16f1.red, 0.5f);
    QCOMPAREh(p16f1.green, 0.1f);
    QCOMPAREh(p16f1.blue, 0.6f);
    QCOMPAREh(p16f1.alpha, 1.0f);

    // Test no mask, half opacity
    p16f.red = 0.5; p16f.green = 0.1; p16f.blue = 0.6; p16f.alpha = 1.0;
    p16f1.red = 0.3; p16f1.green = 0.5; p16f1.blue = 0.8; p16f1.alpha = 1.0;
    over->composite(p16fPtr1, rgb16f->pixelSize(), p16fPtr, rgb16f->pixelSize(), 0, 0, 1, 1, 127);
    QCOMPAREh(p16f1.red, 0.399608f);
    QCOMPAREh(p16f1.green, 0.300784f);
    QCOMPAREh(p16f1.blue, 0.700392f);
    QCOMPAREh(p16f1.alpha, 1.0f);

    // Test mask, full opacity
    quint8 mask; mask = 127;
    p16f.red = 0.5; p16f.green = 0.1; p16f.blue = 0.6; p16f.alpha = 1.0;
    p16f1.red = 0.3; p16f1.green = 0.5; p16f1.blue = 0.8; p16f1.alpha = 1.0;
    over->composite(p16fPtr1, rgb16f->pixelSize(), p16fPtr, rgb16f->pixelSize(), &mask, 1, 1, 1, 255);
    QCOMPAREh(p16f1.red, 0.399608f);
    QCOMPAREh(p16f1.green, 0.300784f);
    QCOMPAREh(p16f1.blue, 0.700392f);
    QCOMPAREh(p16f1.alpha, 1.0f);

    // Test mask, full opacity
    p16f.red = 0.5; p16f.green = 0.1; p16f.blue = 0.6; p16f.alpha = 1.0;
    p16f1.red = 0.3; p16f1.green = 0.5; p16f1.blue = 0.8; p16f1.alpha = 1.0;
    over->composite(p16fPtr1, rgb16f->pixelSize(), p16fPtr, rgb16f->pixelSize(), &mask, 1, 1, 1, 127);
    QCOMPAREh(p16f1.red, 0.349609f);
    QCOMPAREh(p16f1.green, 0.400783f);
    QCOMPAREh(p16f1.blue, 0.750391f);
    QCOMPAREh(p16f1.alpha, 1.0f);

    // Test no mask, full opacity, transparent source
    p16f.red = 0.5; p16f.green = 0.1; p16f.blue = 0.6; p16f.alpha = 0.0;
    p16f1.red = 0.3; p16f1.green = 0.5; p16f1.blue = 0.8; p16f1.alpha = 1.0;
    over->composite(p16fPtr1, rgb16f->pixelSize(), p16fPtr, rgb16f->pixelSize(), 0, 0, 1, 1, 255);
    QCOMPAREh(p16f1.red, 0.3f);
    QCOMPAREh(p16f1.green, 0.5f);
    QCOMPAREh(p16f1.blue, 0.8f);
    QCOMPAREh(p16f1.alpha, 1.0f);

    // Test no mask, full opacity, transparent dst
    p16f.red = 0.5; p16f.green = 0.1; p16f.blue = 0.6; p16f.alpha = 1.0;
    p16f1.red = 0.3; p16f1.green = 0.5; p16f1.blue = 0.8; p16f1.alpha = 0.0;
    over->composite(p16fPtr1, rgb16f->pixelSize(), p16fPtr, rgb16f->pixelSize(), 0, 0, 1, 1, 255);
    QCOMPAREh(p16f1.red, 0.5f);
    QCOMPAREh(p16f1.green, 0.1f);
    QCOMPAREh(p16f1.blue, 0.6f);
    QCOMPAREh(p16f1.alpha, 1.0f);

    // Test no mask, full opacity, half-transparent dst
    p16f.red = 0.5; p16f.green = 0.1; p16f.blue = 0.6; p16f.alpha = 1.0;
    p16f1.red = 0.3; p16f1.green = 0.5; p16f1.blue = 0.8; p16f1.alpha = 0.5;
    over->composite(p16fPtr1, rgb16f->pixelSize(), p16fPtr, rgb16f->pixelSize(), 0, 0, 1, 1, 255);
    QCOMPAREh(p16f1.red, 0.5f);
    QCOMPAREh(p16f1.green, 0.1f);
    QCOMPAREh(p16f1.blue, 0.6f);
    QCOMPAREh(p16f1.alpha, 1.0f);
}

QTEST_KDEMAIN(KoRgb16fTest, NoGUI)
#include "KoRgb16fTest.moc"
