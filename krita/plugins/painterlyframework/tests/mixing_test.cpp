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
#include <cmath>

#include "mixing_test.h"

#include "kis_illuminant_profile.h"
#include "kis_ksqp_colorspace.h"

#include <KGlobal>
#include <KLocale>
#include <KStandardDirs>

#include <QImage>
#include <QPainter>
#include <QPicture>

// Stress test, it doesn't do compares and the like... if you come to an end, then probably everything works ;-)
void MixingTest::testMixing()
{
    // These values are tied for blue - yellow mixing.
    const int T = 16;
    const int step = 2;
    const int base = 256/T;
    const int basewidth = 16;

    const int ncolor1 = (int)pow(T/step+1,2);
    const int ncolor2 = (int)pow(T/step+1,2);

    KGlobal::mainComponent().dirs()->addResourceType("illuminant_profiles", 0, "share/color/illuminants/");
    QString d659 = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", "D65_9.ill",  KStandardDirs::Recursive)[0];
    const KisIlluminantProfile *profile = new KisIlluminantProfile(d659);
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace(KisKSQPColorSpace::colorSpaceId(), profile);
    QVERIFY2(cs != 0, "Created colorspace");
    QVERIFY(cs->profileIsCompatible(profile));

    QColor color1, color2, colorm;
    quint8 *data1 = new quint8[cs->pixelSize()];
    quint8 *data2 = new quint8[cs->pixelSize()];
    quint8 *datam = new quint8[cs->pixelSize()];

    QBrush b;
    QPainter p;
    int currx, curry = 0;
    QImage image(basewidth*3*ncolor2, basewidth*ncolor1, QImage::Format_ARGB32);
    for (int i1 = 0; i1 <= 0; i1+=step) {
        for (int j1 = 0; j1 <= T; j1+=step) {
            for (int k1 = 0; k1 <= T; k1+=step) {
                currx = 0;
                for (int i2 = 0; i2 <= T; i2+=step) {
                    for (int j2 = 0; j2 <= T; j2+=step) {
                        for (int k2 = 0; k2 <= 0; k2+=step) {
                            color1 = QColor(base*i1?base*i1-1:0,base*j1?base*j1-1:0,base*k1?base*k1-1:0);
                            color2 = QColor(base*i2?base*i2-1:0,base*j2?base*j2-1:0,base*k2?base*k2-1:0);
                            cs->fromQColor(color1, data1);
                            cs->fromQColor(color2, data2);
                            for (int i = 0; i < 2*9+1; i++)
                                reinterpret_cast<float*>(datam)[i] =
                                    (reinterpret_cast<float*>(data1)[i] +
                                     reinterpret_cast<float*>(data2)[i]) / 2.0;
                            cs->toQColor(datam, &colorm);

                            p.begin(&image);
                            b = QBrush(color1);
                            p.fillRect(currx+0*basewidth, curry, basewidth,basewidth, b);
                            b = QBrush(color2);
                            p.fillRect(currx+1*basewidth, curry, basewidth,basewidth, b);
                            b = QBrush(colorm);
                            p.fillRect(currx+2*basewidth, curry, basewidth,basewidth, b);
                            p.end();
                            currx += basewidth*3;
                        }
                    }
                }
                curry += basewidth;
            }
        }
    }
    image.save("table.jpg");

    delete [] datam;
    delete [] data2;
    delete [] data1;
    delete cs;
    delete profile;
}

QTEST_KDEMAIN(MixingTest, NoGUI)
#include "mixing_test.moc"
