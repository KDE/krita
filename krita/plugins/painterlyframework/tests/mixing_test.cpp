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
#include "kis_kslc_colorspace.h"

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
    const int T1 = 0;
    const int T2 = 16;
    const int T = 16;
    const int step = 2;
    const int base = 256/T;
    const int basewidth = 16;

    const int ncolor1 = (int)pow((T2-T1)/step+1,2);
    const int ncolor2 = (int)pow((T2-T1)/step+1,2);

    KGlobal::mainComponent().dirs()->addResourceType("illuminant_profiles", 0, "share/apps/krita/illuminants");
    QString d65 = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", illuminant,  KStandardDirs::Recursive)[0];
    const KisIlluminantProfile *profile = new KisIlluminantProfile(d65);
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace(KisKSLCColorSpace<float,N>::ColorSpaceId().id(), profile);
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
    for (int i1 = T1; i1 <= T1; i1+=step) {
        for (int j1 = T1; j1 <= T2; j1+=step) {
            for (int k1 = T1; k1 <= T2; k1+=step) {
                currx = 0;
                for (int i2 = T1; i2 <= T2; i2+=step) {
                    for (int j2 = T1; j2 <= T2; j2+=step) {
                        for (int k2 = T1; k2 <= T1; k2+=step) {
                            color1 = QColor(base*i1?base*i1-1:0,base*j1?base*j1-1:0,base*k1?base*k1-1:0);
                            color2 = QColor(base*i2?base*i2-1:0,base*j2?base*j2-1:0,base*k2?base*k2-1:0);
                            cs->fromQColor(color1, data1);
                            cs->fromQColor(color2, data2);
                            for (uint i = 0; i < 2*N+1; i++)
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
    delete profile;
}

template< quint32 N >
void mix(const float *data1, float c1, const float *data2, float c2, float *datam)
{
    for (uint i = 0; i < 2*N; i++)
        datam[i] = c1*data1[i] + c2*data2[i];
    datam[2*N] = 1;
}

void MixingTest::testMixing2()
{
    KGlobal::mainComponent().dirs()->addResourceType("illuminant_profiles", 0, "share/apps/krita/illuminants");
    QString d65 = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", illuminant,  KStandardDirs::Recursive)[0];
    const KisIlluminantProfile *profile = new KisIlluminantProfile(d65);
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace(KisKSLCColorSpace<float,N>::ColorSpaceId().id(), profile);
    QVERIFY2(cs != 0, "Created colorspace");
    QVERIFY(cs->profileIsCompatible(profile));

    QColor red(255,0,0), green(0,255,0), blue(0,0,255), yellow(255,255,0), colorm;
    float *datared = new float[cs->pixelSize()/4];
    float *datagreen = new float[cs->pixelSize()/4];
    float *datablue = new float[cs->pixelSize()/4];
    float *datayellow = new float[cs->pixelSize()/4];
    float *datam = new float[cs->pixelSize()/4];

    cs->fromQColor(red, reinterpret_cast<quint8*>(datared));
    cs->fromQColor(green, reinterpret_cast<quint8*>(datagreen));
    cs->fromQColor(blue, reinterpret_cast<quint8*>(datablue));
    cs->fromQColor(yellow, reinterpret_cast<quint8*>(datayellow));

    QImage image(600, 600, QImage::Format_ARGB32);
    QPainter p(&image);
    for (int x = 0; x <= 599; x++) {
        float cright = ((float)x)/599.0f;
        float cleft = 1.0f - cright;

        mix<N>(datared, cleft, datagreen, cright, datam);
        cs->toQColor(reinterpret_cast<quint8*>(datam), &colorm);
        p.fillRect(x, 0, 1, 100, colorm);

        mix<N>(datared, cleft, datablue, cright, datam);
        cs->toQColor(reinterpret_cast<quint8*>(datam), &colorm);
        p.fillRect(x, 100, 1, 100, QBrush(colorm));

        mix<N>(datared, cleft, datayellow, cright, datam);
        cs->toQColor(reinterpret_cast<quint8*>(datam), &colorm);
        p.fillRect(x, 200, 1, 100, QBrush(colorm));

        mix<N>(datagreen, cleft, datablue, cright, datam);
        cs->toQColor(reinterpret_cast<quint8*>(datam), &colorm);
        p.fillRect(x, 300, 1, 100, QBrush(colorm));

        mix<N>(datagreen, cleft, datayellow, cright, datam);
        cs->toQColor(reinterpret_cast<quint8*>(datam), &colorm);
        p.fillRect(x, 400, 1, 100, QBrush(colorm));

        mix<N>(datablue, cleft, datayellow, cright, datam);
        cs->toQColor(reinterpret_cast<quint8*>(datam), &colorm);
        p.fillRect(x, 500, 1, 100, QBrush(colorm));
    }
    p.end();
    image.save("tableaveraged_"+illuminant+".jpg");

    delete [] datam;
    delete [] datared;
    delete [] datagreen;
    delete [] datablue;
    delete [] datayellow;
    delete profile;
}

QTEST_KDEMAIN(MixingTest, NoGUI)
#include "mixing_test.moc"
