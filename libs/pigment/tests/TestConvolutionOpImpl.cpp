/*
 *  Copyright (C) 2008 Cyrille Berger <cberger@cberger.net>
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

#include "TestConvolutionOpImpl.h"

#include <qtest_kde.h>

#include "../KoColorSpaceAbstract.h"
#include "../KoColorSpaceTraits.h"
#include "../DebugPigment.h"

void TestConvolutionOpImpl::testConvolutionOpImpl()
{
    KoConvolutionOpImpl<KoRgbU16Traits> op;
    quint8** colors = new quint8*[3];
    colors[0] = new quint8[KoRgbU16Traits::pixelSize];
    ((quint16*)colors[0])[0] = 100;
    ((quint16*)colors[0])[1] = 200;
    ((quint16*)colors[0])[2] = 300;
    ((quint16*)colors[0])[3] = 0xFFFF;
    colors[1] = new quint8[KoRgbU16Traits::pixelSize];
    ((quint16*)colors[1])[0] = 50;
    ((quint16*)colors[1])[1] = 150;
    ((quint16*)colors[1])[2] = 0;
    ((quint16*)colors[1])[3] = 0xFFFF;
    colors[2] = new quint8[KoRgbU16Traits::pixelSize];
    ((quint16*)colors[2])[0] = 100;
    ((quint16*)colors[2])[1] = 300;
    ((quint16*)colors[2])[2] = 50;
    ((quint16*)colors[2])[3] = 0xFFFF;
    quint8* dst = new quint8[KoRgbU16Traits::pixelSize];
    quint16* dst16 = (quint16*)dst;
    {
        memcpy( dst16, colors[0], KoRgbU16Traits::pixelSize );
        QVERIFY2( dst16[0] == 100, QString("%1 100").arg(dst16[0]).toLatin1() );
        QVERIFY2( dst16[1] == 200, QString("%1 200").arg(dst16[1]).toLatin1() );
        QVERIFY2( dst16[2] == 300, QString("%1 300").arg(dst16[2]).toLatin1() );
        QVERIFY2( dst16[3] == 0xFFFF, QString("%1 0xFFFF").arg(dst16[3]).toLatin1() );
        memcpy( dst16, colors[1], KoRgbU16Traits::pixelSize );
        QVERIFY2( dst16[0] == 50, QString("%1 50").arg(dst16[0]).toLatin1() );
        QVERIFY2( dst16[1] == 150, QString("%1 150").arg(dst16[1]).toLatin1() );
        QVERIFY2( dst16[2] == 0, QString("%1 0").arg(dst16[2]).toLatin1() );
        QVERIFY2( dst16[3] == 0xFFFF, QString("%1 0xFFFF").arg(dst16[3]).toLatin1() );
        memcpy( dst16, colors[2], KoRgbU16Traits::pixelSize );
        QVERIFY2( dst16[0] == 100, QString("%1 100").arg(dst16[0]).toLatin1() );
        QVERIFY2( dst16[1] == 300, QString("%1 300").arg(dst16[1]).toLatin1() );
        QVERIFY2( dst16[2] == 50, QString("%1 50").arg(dst16[2]).toLatin1() );
        QVERIFY2( dst16[3] == 0xFFFF, QString("%1 0xFFFF").arg(dst16[3]).toLatin1() );
    }

    memset( dst16, 0, KoRgbU16Traits::pixelSize );

    // Tests for Case A)

    {
        qint32 kernelValues[] = { 1, 1, 1};
        op.convolveColors(colors, kernelValues, dst, 1, 0, 3, QBitArray() );
        dbgPigment << dst16[0] << " " << dst16[1] << " " << dst16[2] << " " << QBitArray().isEmpty();
        QVERIFY2( dst16[0] == 250, QString("%1 250").arg(dst16[0]).toLatin1() );
        QVERIFY2( dst16[1] == 650, QString("%1 650").arg(dst16[1]).toLatin1() );
        QVERIFY2( dst16[2] == 350, QString("%1 350").arg(dst16[2]).toLatin1() );
        QVERIFY2( dst16[3] == 0xFFFF, QString("%1 0xFFFF").arg(dst16[3]).toLatin1() );
        op.convolveColors(colors, kernelValues, dst, 3, 0, 3, QBitArray() );
        QVERIFY2( dst16[0] == 83, QString("%1 83").arg(dst16[0]).toLatin1() );
        QVERIFY2( dst16[1] == 216, QString("%1 216").arg(dst16[1]).toLatin1() );
        QVERIFY2( dst16[2] == 116, QString("%1 116").arg(dst16[2]).toLatin1() );
        QVERIFY2( dst16[3] == 0xFFFF, QString("%1 0xFFFF").arg(dst16[3]).toLatin1() );
    }
    {
        qint32 kernelValues[] = { -1, 1, -1};
        op.convolveColors(colors, kernelValues, dst, 1, 0, 3, QBitArray() );
        QVERIFY2( dst16[0] == 0, QString("%1 0").arg(dst16[0]).toLatin1() );
        QVERIFY2( dst16[1] == 0, QString("%1 0").arg(dst16[1]).toLatin1() );
        QVERIFY2( dst16[2] == 0, QString("%1 0").arg(dst16[2]).toLatin1() );
        QVERIFY2( dst16[3] == 0, QString("%1 0").arg(dst16[3]).toLatin1() );
   }
   {
        qint32 kernelValues[] = { 1, 2, 1};
        op.convolveColors(colors, kernelValues, dst, 1, 0, 3, QBitArray() );
        QVERIFY2( dst16[0] == 300, QString("%1 300").arg(dst16[0]).toLatin1() );
        QVERIFY2( dst16[1] == 800, QString("%1 800").arg(dst16[1]).toLatin1() );
        QVERIFY2( dst16[2] == 350, QString("%1 350").arg(dst16[2]).toLatin1() );
        QVERIFY2( dst16[3] == 0xFFFF, QString("%1 0xFFFF").arg(dst16[3]).toLatin1() );
   }
   {
        qint32 kernelValues[] = { 1, -1, 1};
        op.convolveColors(colors, kernelValues, dst, 1, 0, 3, QBitArray() );
        QVERIFY2( dst16[0] == 150, QString("%1 150").arg(dst16[0]).toLatin1() );
        QVERIFY2( dst16[1] == 350, QString("%1 350").arg(dst16[1]).toLatin1() );
        QVERIFY2( dst16[2] == 350, QString("%1 350").arg(dst16[2]).toLatin1() );
        QVERIFY2( dst16[3] == 0xFFFF, QString("%1 0xFFFF").arg(dst16[3]).toLatin1() );
   }
   {
        qint32 kernelValues[] = { 1, -1, 1};
        op.convolveColors(colors, kernelValues, dst, 1, 100, 3, QBitArray() );
        QVERIFY2( dst16[0] == 250, QString("%1 250").arg(dst16[0]).toLatin1() );
        QVERIFY2( dst16[1] == 450, QString("%1 450").arg(dst16[1]).toLatin1() );
        QVERIFY2( dst16[2] == 450, QString("%1 450").arg(dst16[2]).toLatin1() );
        QVERIFY2( dst16[3] == 0xFFFF, QString("%1 0xFFFF").arg(dst16[3]).toLatin1() );
        op.convolveColors(colors, kernelValues, dst, 1, -100, 3, QBitArray() );
        QVERIFY2( dst16[0] == 50, QString("%1 50").arg(dst16[0]).toLatin1() );
        QVERIFY2( dst16[1] == 250, QString("%1 250").arg(dst16[1]).toLatin1() );
        QVERIFY2( dst16[2] == 250, QString("%1 250").arg(dst16[2]).toLatin1() );
        QVERIFY2( dst16[3] == 0xFF9B, QString("%1 0xFF9B").arg(dst16[3]).toLatin1() );
   }
}

void TestConvolutionOpImpl::testOneSemiTransparent()
{
    KoConvolutionOpImpl<KoRgbU16Traits> op;
    quint8** colors = new quint8*[3];
    colors[0] = new quint8[KoRgbU16Traits::pixelSize];
    ((quint16*)colors[0])[0] = 100;
    ((quint16*)colors[0])[1] = 200;
    ((quint16*)colors[0])[2] = 300;

    ((quint16*)colors[0])[3] = 0x00FF;

    colors[1] = new quint8[KoRgbU16Traits::pixelSize];
    ((quint16*)colors[1])[0] = 50;
    ((quint16*)colors[1])[1] = 150;
    ((quint16*)colors[1])[2] = 0;
    ((quint16*)colors[1])[3] = 0xFFFF;
    colors[2] = new quint8[KoRgbU16Traits::pixelSize];
    ((quint16*)colors[2])[0] = 100;
    ((quint16*)colors[2])[1] = 300;
    ((quint16*)colors[2])[2] = 50;
    ((quint16*)colors[2])[3] = 0xFFFF;
    quint8* dst = new quint8[KoRgbU16Traits::pixelSize];
    quint16* dst16 = (quint16*)dst;
    memset( dst16, 0, KoRgbU16Traits::pixelSize );

    {
        // Tests for Case A)

        qint32 kernelValues[] = { 1, 1, 1};
        op.convolveColors(colors, kernelValues, dst, 1, 0, 3, QBitArray() );
        QVERIFY2( dst16[0] == 250, QString("%1 250").arg(dst16[0]).toLatin1() );
        QVERIFY2( dst16[1] == 650, QString("%1 650").arg(dst16[1]).toLatin1() );
        QVERIFY2( dst16[2] == 350, QString("%1 350").arg(dst16[2]).toLatin1() );

        QVERIFY2( dst16[3] == 0xFFFF, QString("%1 0xFFFF").arg(dst16[3]).toLatin1() );

        op.convolveColors(colors, kernelValues, dst, 3, 0, 3, QBitArray() );
        QVERIFY2( dst16[0] == 83, QString("%1 83").arg(dst16[0]).toLatin1() );
        QVERIFY2( dst16[1] == 216, QString("%1 216").arg(dst16[1]).toLatin1() );
        QVERIFY2( dst16[2] == 116, QString("%1 116").arg(dst16[2]).toLatin1() );

        QVERIFY2( dst16[3] == 0xAAFF, QString("%1 0xAAFF").arg(dst16[3]).toLatin1() );
    }
}

void TestConvolutionOpImpl::testOneFullyTransparent()
{
    KoConvolutionOpImpl<KoRgbU16Traits> op;
    quint8** colors = new quint8*[3];
    colors[0] = new quint8[KoRgbU16Traits::pixelSize];
    ((quint16*)colors[0])[0] = 100;
    ((quint16*)colors[0])[1] = 200;
    ((quint16*)colors[0])[2] = 300;

    ((quint16*)colors[0])[3] = 0x0000;

    colors[1] = new quint8[KoRgbU16Traits::pixelSize];
    ((quint16*)colors[1])[0] = 50;
    ((quint16*)colors[1])[1] = 150;
    ((quint16*)colors[1])[2] = 0;
    ((quint16*)colors[1])[3] = 0xFFFF;
    colors[2] = new quint8[KoRgbU16Traits::pixelSize];
    ((quint16*)colors[2])[0] = 100;
    ((quint16*)colors[2])[1] = 300;
    ((quint16*)colors[2])[2] = 50;
    ((quint16*)colors[2])[3] = 0xFFFF;
    quint8* dst = new quint8[KoRgbU16Traits::pixelSize];
    quint16* dst16 = (quint16*)dst;
    memset( dst16, 0, KoRgbU16Traits::pixelSize );

    {
        qint32 kernelValues[] = { 1, 1, 1};

        // Test for Case C)
        op.convolveColors(colors, kernelValues, dst, 1, 0, 3, QBitArray() );
        QVERIFY2( dst16[0] == 225, QString("%1 225").arg(dst16[0]).toLatin1() );
        QVERIFY2( dst16[1] == 675, QString("%1 675").arg(dst16[1]).toLatin1() );
        QVERIFY2( dst16[2] ==  75, QString("%1  75").arg(dst16[2]).toLatin1() );
        QVERIFY2( dst16[3] == 0xFFFF, QString("%1 0xFFFF").arg(dst16[3]).toLatin1() );

        // Test for Case B)
        op.convolveColors(colors, kernelValues, dst, 3, 0, 3, QBitArray() );
        QVERIFY2( dst16[0] == 75, QString("%1 75").arg(dst16[0]).toLatin1() );
        QVERIFY2( dst16[1] == 225, QString("%1 225").arg(dst16[1]).toLatin1() );
        QVERIFY2( dst16[2] == 25, QString("%1 25").arg(dst16[2]).toLatin1() );
        QVERIFY2( dst16[3] == 0xAAAA, QString("%1 0xAAAA").arg(dst16[3]).toLatin1() );

        // Test for Case C)
        op.convolveColors(colors, kernelValues, dst, 15, 0, 3, QBitArray() );
        QVERIFY2( dst16[0] == 15, QString("%1 15").arg(dst16[0]).toLatin1() );
        QVERIFY2( dst16[1] == 45, QString("%1 45").arg(dst16[1]).toLatin1() );
        QVERIFY2( dst16[2] ==  5, QString("%1  5").arg(dst16[2]).toLatin1() );
        QVERIFY2( dst16[3] == 0x2222, QString("%1 0x2222").arg(dst16[3]).toLatin1() );
    }
}


QTEST_KDEMAIN(TestConvolutionOpImpl, NoGUI)
#include "TestConvolutionOpImpl.moc"
