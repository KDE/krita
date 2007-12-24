/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <qtest_kde.h>

#include "channel_converter_test.h"

#include "channel_converter.h"

void ChannelConverterTest::testKSReflectance()
{
    ChannelConverter c(1.0, 10.0);
    float K, S, R;

    // reflectanceToKS
    R = 0.0;
    c.reflectanceToKS(R, K, S);
    QCOMPARE((double)K, 8.0);
    QCOMPARE((double)S, 0.0);

    R = 1.0;
    c.reflectanceToKS(R, K, S);
    QCOMPARE((double)K, 0.0);
    QVERIFY((S - 0.8) < 1e-6);

    for (int i = 1; i < 10; i++) {
        R = (float)i * 0.1;
        c.reflectanceToKS(R, K, S);
        qDebug() << "Reflectance " << R << "; K = " << K << ", S = " << S;
    }

    // KSToReflectance
    K = 0.0;
    c.KSToReflectance(K, S, R);
    QCOMPARE((double)R, 1.0);

    S = 0.0;
    c.KSToReflectance(K, S, R);
    QCOMPARE((double)R, 0.0);

    for (int i = 1; i < 10; i++) {
        K = (float)i * 0.1;
        S = 1.0 - K;
        c.KSToReflectance(K, S, R);
        qDebug() << "K " << K << ", S " << S << "; R = " << R;
        QVERIFY(R > 0.0 && R < 1.0);
    }
}

void ChannelConverterTest::testRGBsRGB()
{
}

QTEST_KDEMAIN(ChannelConverterTest, NoGUI)
#include "channel_converter_test.moc"
