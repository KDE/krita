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

#include <cstring>

#include "kis_ks_colorspace_test.h"
#include "kis_ks_colorspace.h"
#include "kis_ks3_colorspace.h"

#include "kis_illuminant_profile.h"

void KisKSColorSpaceTest::testConstructor()
{
    KisIlluminantProfile *p1 = new KisIlluminantProfile("D653Test.ill");
    KisIlluminantProfile *p2 = new KisIlluminantProfile("D659Test.ill");
    KisKS3ColorSpace *cs1 = new KisKS3ColorSpace(p1);
    QVERIFY(cs1->profileIsCompatible(p2) == false);
    delete cs1;
    delete p2;
    delete p1;
}

template<typename type, int n>
void print_vector(quint8 *v, const QString &text)
{
    QString vstr;
    qDebug() << text;
    for (int i = 0; i < n; i++)
        vstr += QString::number(reinterpret_cast<type*>(v)[i]) + " ";
    qDebug() << vstr;
}

#define N 9

void KisKSColorSpaceTest::testToFromRgbA16()
{
    KisIlluminantProfile *p = new KisIlluminantProfile("D65"+QString::number(N)+"Test.ill");
    KisKSColorSpace<N> *cs = new KisKSColorSpace<N>(p);
//     KisKS3ColorSpace *cs = new KisKS3ColorSpace(p);

    quint8 *rgb1;
    quint8 *kas1 = new quint8[2*cs->pixelSize()];
    quint8 *rgb2 = new quint8[2*8];

    quint32 val = 65535;
    quint16 blue[4]   = { val,   0, 0,   val };
    quint16 green[4]  = { 0,   val, 0,   val };
    quint16 red[4]    = { 0,     0, val, val };
    quint16 yellow[4] = { 0,   val, val, val };

    quint16 bluegreen[8] = { val, 0, 0, val, 0, val, 0, val };

    rgb1 = reinterpret_cast<quint8*>(blue);
    print_vector<quint16, 4>(rgb1, "BLUE:");
    cs->fromRgbA16(rgb1, kas1, 1);
    print_vector<float, 2*N+1>(kas1, "BLUE IN KS:");
    cs->toRgbA16(kas1, rgb2, 1);
    print_vector<quint16, 4>(rgb2, "BLUE AGAIN:");
    QVERIFY(rgb1[0] == rgb2[0]);
    QVERIFY(rgb1[1] == rgb2[1]);
    QVERIFY(rgb1[2] == rgb2[2]);
    QVERIFY(rgb1[3] == rgb2[3]);

    rgb1 = reinterpret_cast<quint8*>(green);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->toRgbA16(kas1, rgb2, 1);
    print_vector<quint16, 4>(rgb1, "GREEN:");
    print_vector<float, 2*N+1>(kas1, "GREEN IN KS:");
    print_vector<quint16, 4>(rgb2, "GREEN AGAIN:");
    QVERIFY(rgb1[0] == rgb2[0]);
    QVERIFY(rgb1[1] == rgb2[1]);
    QVERIFY(rgb1[2] == rgb2[2]);
    QVERIFY(rgb1[3] == rgb2[3]);

    rgb1 = reinterpret_cast<quint8*>(red);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->toRgbA16(kas1, rgb2, 1);
    print_vector<quint16, 4>(rgb1, "RED:");
    print_vector<float, 2*N+1>(kas1, "RED IN KS:");
    print_vector<quint16, 4>(rgb2, "RED AGAIN:");
    QVERIFY(rgb1[0] == rgb2[0]);
    QVERIFY(rgb1[1] == rgb2[1]);
    QVERIFY(rgb1[2] == rgb2[2]);
    QVERIFY(rgb1[3] == rgb2[3]);

    rgb1 = reinterpret_cast<quint8*>(yellow);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->toRgbA16(kas1, rgb2, 1);
    print_vector<quint16, 4>(rgb1, "YELLOW:");
    print_vector<float, 2*N+1>(kas1, "YELLOW IN KS:");
    print_vector<quint16, 4>(rgb2, "YELLOW AGAIN:");
    QVERIFY(rgb1[0] == rgb2[0]);
    QVERIFY(rgb1[1] == rgb2[1]);
    QVERIFY(rgb1[2] == rgb2[2]);
    QVERIFY(rgb1[3] == rgb2[3]);

    rgb1 = reinterpret_cast<quint8*>(bluegreen);
    cs->fromRgbA16(rgb1, kas1, 2);
    cs->toRgbA16(kas1, rgb2, 2);
    print_vector<quint16, 8>(rgb1, "BLUE AND GREEN:");
    print_vector<float, 2*2*N+1>(kas1, "BLUE AND GREEN IN KS:");
    print_vector<quint16, 8>(rgb2, "BLUE AND GREEN AGAIN:");
    QVERIFY(rgb1[0] == rgb2[0]);
    QVERIFY(rgb1[1] == rgb2[1]);
    QVERIFY(rgb1[2] == rgb2[2]);
    QVERIFY(rgb1[3] == rgb2[3]);
    QVERIFY(rgb1[4] == rgb2[4]);
    QVERIFY(rgb1[5] == rgb2[5]);
    QVERIFY(rgb1[6] == rgb2[6]);
    QVERIFY(rgb1[7] == rgb2[7]);

    delete [] kas1;
    delete [] rgb2;
    delete cs;
    delete p;
}

void KisKSColorSpaceTest::testMixing()
{
    KisIlluminantProfile *p = new KisIlluminantProfile("D65"+QString::number(N)+"Test.ill");
    KisKSColorSpace<N> *cs = new KisKSColorSpace<N>(p);
//     KisKS3ColorSpace *cs = new KisKS3ColorSpace(p);

    quint8 *rgb1, *rgb2;
    quint8 *kas1 = new quint8[cs->pixelSize()];
    quint8 *kas2 = new quint8[cs->pixelSize()];
    quint8 *kasm = new quint8[cs->pixelSize()];
    quint8 *rgbm = new quint8[2*4];

    quint32 val = 65535;
    quint16 blue[4]   = { val,   0, 0,   val };
    quint16 green[4]  = { 0,   val, 0,   val };
    quint16 red[4]    = { 0,     0, val, val };
    quint16 yellow[4] = { 0,   val, val, val };

    rgb1 = reinterpret_cast<quint8*>(blue);
    rgb2 = reinterpret_cast<quint8*>(yellow);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->fromRgbA16(rgb2, kas2, 1);
    for (int i = 0; i < 2*N+1; i++)
        reinterpret_cast<float*>(kasm)[i] = (reinterpret_cast<float*>(kas1)[i] +
                                             reinterpret_cast<float*>(kas2)[i]) / 2.0;
    cs->toRgbA16(kasm, rgbm, 1);
    print_vector<float,2*N+1>(kasm, "BLUE + YELLOW IN KS:");
    print_vector<quint16,4>(rgbm, "BLUE + YELLOW BACK IN RBG:");

    rgb1 = reinterpret_cast<quint8*>(blue);
    rgb2 = reinterpret_cast<quint8*>(green);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->fromRgbA16(rgb2, kas2, 1);
    for (int i = 0; i < 2*N+1; i++)
        reinterpret_cast<float*>(kasm)[i] = (reinterpret_cast<float*>(kas1)[i] +
                                             reinterpret_cast<float*>(kas2)[i]) / 2.0;
    cs->toRgbA16(kasm, rgbm, 1);
    print_vector<float,2*N+1>(kasm, "BLUE + GREEN IN KS:");
    print_vector<quint16,4>(rgbm, "BLUE + GREEN BACK IN RBG:");

    rgb1 = reinterpret_cast<quint8*>(red);
    rgb2 = reinterpret_cast<quint8*>(blue);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->fromRgbA16(rgb2, kas2, 1);
    for (int i = 0; i < 2*N+1; i++)
        reinterpret_cast<float*>(kasm)[i] = (reinterpret_cast<float*>(kas1)[i] +
                                             reinterpret_cast<float*>(kas2)[i]) / 2.0;
    cs->toRgbA16(kasm, rgbm, 1);
    print_vector<float,2*N+1>(kasm, "RED + BLUE IN KS:");
    print_vector<quint16,4>(rgbm, "RED + BLUE BACK IN RBG:");

    delete [] rgbm;
    delete [] kasm;
    delete [] kas2;
    delete [] kas1;
    delete cs;
    delete p;
}

QTEST_KDEMAIN(KisKSColorSpaceTest, NoGUI)
#include "kis_ks_colorspace_test.moc"
