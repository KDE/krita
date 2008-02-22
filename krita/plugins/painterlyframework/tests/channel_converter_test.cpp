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

#include "channel_converter_test.h"
#include <qtest_kde.h>

#include <cmath>

#include "channel_converter.h"

void ChannelConverterTest::testKSReflectance()
{
    ChannelConverter<float> c(4.3, 0.14);
    float K, S; double R;

    R = c.Rh()-(1e-8);
    c.reflectanceToKS(R, K, S);
    qDebug() << "Reflectance " << R << "; K = " << K << ", S = " << S;
    QCOMPARE((S/K), (float)(R*2.0/pow(1-R,2)));

    R = c.Rh()+(1e-8);
    c.reflectanceToKS(R, K, S);
    qDebug() << "Reflectance " << R << "; K = " << K << ", S = " << S;
    QCOMPARE((S/K), (float)(R*2.0/pow(1-R,2)));

    for (int i = 0; i <= 100; i++) {
        R = (double)i/100.0;
        c.reflectanceToKS(R, K, S);
        qDebug() << "Reflectance " << R << "; K = " << K << ", S = " << S;
        if ( i <= 50 )
            QCOMPARE((S/K), (float)(2.0*R*pow(1-R,-2)));
        else
            QCOMPARE((K/S), (float)(0.5*pow(1-R, 2)/R));
    }

    // KSToReflectance
    K = 0.0;
    R = c.KSToReflectance(K, S);
    QCOMPARE(R, 1.0);

    S = 0.0;
    R = c.KSToReflectance(K, S);
    QCOMPARE(R, 0.0);

    for (int i = 1; i < 10; i++) {
        K = i * 0.1;
        S = 1.0 - K;
        R = c.KSToReflectance(K, S);
        qDebug() << "K " << K << ", S " << S << "; R = " << R;
        QVERIFY(R > 0.0 && R < 1.0);
        QCOMPARE((S/K), (float)(R*2.0/pow(1-R,2)));
    }
}

QTEST_KDEMAIN(ChannelConverterTest, NoGUI)
#include "channel_converter_test.moc"
