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

#include "KoCtlChannelTest.h"

#include <qtest_kde.h>
#include "../KoCtlChannel.h"
#include <KoID.h>

void KoCtlChannelTest::test()
{
    quint8 rgbu8[] = { 100, 50, 200 };
    KoCtlChannelImpl<quint8> c8(1, 3 * sizeof(quint8));
    QCOMPARE(c8.channelValueText(rgbu8), QString("50"));
    QCOMPARE(c8.normalisedChannelValueText(rgbu8), QString("0.196078"));
    QCOMPARE(c8.scaleToU8(rgbu8), quint8(50));
    QCOMPARE(c8.scaleToU16(rgbu8), quint16(12850));
    QCOMPARE(c8.scaleToF32(rgbu8), 50.0f / 255);
    quint8 rgbu81[3];
    c8.singleChannelPixel(rgbu81, rgbu8);
    QCOMPARE(rgbu81[1], rgbu8[1]);
    c8.scaleFromF32(rgbu8, 0.5);
    QCOMPARE(rgbu8[0], quint8(100));
    QCOMPARE(rgbu8[1], quint8(127));
    QCOMPARE(rgbu8[2], quint8(200));
    c8.scaleFromU8(rgbu8, 34);
    QCOMPARE(rgbu8[0], quint8(100));
    QCOMPARE(rgbu8[1], quint8(34));
    QCOMPARE(rgbu8[2], quint8(200));

    float rgbf32[] = { 1.0, 0.12, -1.0 };
    quint8* rgbf32_ptr = reinterpret_cast<quint8*>(rgbf32);
    KoCtlChannelImpl<float> c32(4, 3 * sizeof(quint8));
    QCOMPARE(c32.channelValueText(rgbf32_ptr), QString("0.12"));
    QCOMPARE(c32.normalisedChannelValueText(rgbf32_ptr), QString("0.12"));
    QCOMPARE(c32.scaleToU8(rgbf32_ptr), quint8(0.12 * 0xFF));
    QCOMPARE(c32.scaleToU16(rgbf32_ptr), quint16(0.12 * 0xFFFF));
    QCOMPARE(c32.scaleToF32(rgbf32_ptr), 0.12f);
    float rgbf321[3];
    c32.singleChannelPixel((quint8*)rgbf321, rgbf32_ptr);
    QCOMPARE(rgbf321[1], rgbf32[1]);
    c32.scaleFromF32(rgbf32_ptr, 3.0f);
    QCOMPARE(rgbf32[0], 1.0f);
    QCOMPARE(rgbf32[1], 3.0f);
    QCOMPARE(rgbf32[2], -1.0f);
    c32.scaleFromU8(rgbf32_ptr, 100);
    QCOMPARE(rgbf32[0], 1.0f);
    QCOMPARE(rgbf32[1], 100 / 255.0f);
    QCOMPARE(rgbf32[2], -1.0f);
}



QTEST_KDEMAIN(KoCtlChannelTest, NoGUI)
#include "KoCtlChannelTest.moc"
