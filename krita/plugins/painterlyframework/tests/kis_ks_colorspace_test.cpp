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

#include "kis_illuminant_profile.h"
#include "kis_ks_colorspace_test.h"
#include "kis_ksqp_colorspace.h"
#include "kis_kslc_colorspace.h"

#include <KoColorSpaceRegistry.h>

#include <KGlobal>
#include <KLocale>
#include <KStandardDirs>

void KisKSColorSpaceTest::testConstructor()
{
    KGlobal::mainComponent().dirs()->addResourceType("illuminant_profiles", 0, "share/apps/krita/illuminants");
    QString d659 = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", "D65_9_high.ill",  KStandardDirs::Recursive)[0];

    KisIlluminantProfile *p = new KisIlluminantProfile(d659);
    KisKSQPColorSpace<float,9> *cs = new KisKSQPColorSpace<float,9>(p);
    QVERIFY(cs->profileIsCompatible(p) == true);
    delete cs;
}

template<typename type, int n>
void print_vector(quint8 *v, const QString &text)
{
    QString vstr;
    qDebug() << text;
    for (int i = 0; i < n; i++)
        vstr += QString::number(reinterpret_cast<type*>(v)[i]/256) + " ";
    qDebug() << vstr;
}

#define N 9

void KisKSColorSpaceTest::testToFromRgbA16()
{
    QString d65 = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", "D65_9_high.ill",  KStandardDirs::Recursive)[0];
    const KisIlluminantProfile *profile = new KisIlluminantProfile(d65);
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace(KisKSLCColorSpace<float,N>::ColorSpaceId().id(), profile);
    QVERIFY2(cs != 0, "Created colorspace");
    QVERIFY(cs->profileIsCompatible(profile));

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
//     QVERIFY(rgb1[0] == rgb2[0]);
//     QVERIFY(rgb1[1] == rgb2[1]);
//     QVERIFY(rgb1[2] == rgb2[2]);
//     QVERIFY(rgb1[3] == rgb2[3]);

    rgb1 = reinterpret_cast<quint8*>(green);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->toRgbA16(kas1, rgb2, 1);
    print_vector<quint16, 4>(rgb1, "GREEN:");
    print_vector<float, 2*N+1>(kas1, "GREEN IN KS:");
    print_vector<quint16, 4>(rgb2, "GREEN AGAIN:");
//     QVERIFY(rgb1[0] == rgb2[0]);
//     QVERIFY(rgb1[1] == rgb2[1]);
//     QVERIFY(rgb1[2] == rgb2[2]);
//     QVERIFY(rgb1[3] == rgb2[3]);

    rgb1 = reinterpret_cast<quint8*>(red);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->toRgbA16(kas1, rgb2, 1);
    print_vector<quint16, 4>(rgb1, "RED:");
    print_vector<float, 2*N+1>(kas1, "RED IN KS:");
    print_vector<quint16, 4>(rgb2, "RED AGAIN:");
//     QVERIFY(rgb1[0] == rgb2[0]);
//     QVERIFY(rgb1[1] == rgb2[1]);
//     QVERIFY(rgb1[2] == rgb2[2]);
//     QVERIFY(rgb1[3] == rgb2[3]);

    rgb1 = reinterpret_cast<quint8*>(yellow);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->toRgbA16(kas1, rgb2, 1);
    print_vector<quint16, 4>(rgb1, "YELLOW:");
    print_vector<float, 2*N+1>(kas1, "YELLOW IN KS:");
    print_vector<quint16, 4>(rgb2, "YELLOW AGAIN:");
//     QVERIFY(rgb1[0] == rgb2[0]);
//     QVERIFY(rgb1[1] == rgb2[1]);
//     QVERIFY(rgb1[2] == rgb2[2]);
//     QVERIFY(rgb1[3] == rgb2[3]);

    rgb1 = reinterpret_cast<quint8*>(bluegreen);
    cs->fromRgbA16(rgb1, kas1, 2);
    cs->toRgbA16(kas1, rgb2, 2);
    print_vector<quint16, 8>(rgb1, "BLUE AND GREEN:");
    print_vector<float, 2*2*N+1>(kas1, "BLUE AND GREEN IN KS:");
    print_vector<quint16, 8>(rgb2, "BLUE AND GREEN AGAIN:");
//     QVERIFY(rgb1[0] == rgb2[0]);
//     QVERIFY(rgb1[1] == rgb2[1]);
//     QVERIFY(rgb1[2] == rgb2[2]);
//     QVERIFY(rgb1[3] == rgb2[3]);
//     QVERIFY(rgb1[4] == rgb2[4]);
//     QVERIFY(rgb1[5] == rgb2[5]);
//     QVERIFY(rgb1[6] == rgb2[6]);
//     QVERIFY(rgb1[7] == rgb2[7]);

    delete [] kas1;
    delete [] rgb2;
    delete profile;
}

void KisKSColorSpaceTest::testMixing()
{
    QString d65 = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", "D65_9_high.ill",  KStandardDirs::Recursive)[0];
    const KisIlluminantProfile *profile = new KisIlluminantProfile(d65);
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->colorSpace(KisKSLCColorSpace<float,N>::ColorSpaceId().id(), profile);
    QVERIFY2(cs != 0, "Created colorspace");
    QVERIFY(cs->profileIsCompatible(profile));

    quint8 *rgb1, *rgb2;
    quint8 *kas1 = new quint8[cs->pixelSize()];
    quint8 *kas2 = new quint8[cs->pixelSize()];
    quint8 *kasm = new quint8[cs->pixelSize()];
    quint8 *rgbm = new quint8[2*4];

    quint32 val1 = 65535, val2 = 10000;
    quint16 red[4]    = { 0,    val2, val1, val1 };
    quint16 green[4]  = { val2, val1, val2, val1 };
    quint16 blue[4]   = { val1, val2, 0,    val1 };
    quint16 yellow[4] = { 0,    val1, val1, val1 };
    quint16 violet[4] = { val1, 0,    val1, val1 };
    quint16 cyan[4]   = { val1, val1, 0,    val1 };

    rgb1 = reinterpret_cast<quint8*>(red);
    rgb2 = reinterpret_cast<quint8*>(green);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->fromRgbA16(rgb2, kas2, 1);
    for (int i = 0; i < 2*N+1; i++)
        reinterpret_cast<float*>(kasm)[i] = (reinterpret_cast<float*>(kas1)[i] +
                                             reinterpret_cast<float*>(kas2)[i]) / 2.0;
    cs->toRgbA16(kasm, rgbm, 1);
//     print_vector<float,2*N+1>(kasm, "RED + GREEN IN KS:");
    print_vector<quint16,4>(rgbm, "RED + GREEN IN RBG:");

    rgb1 = reinterpret_cast<quint8*>(red);
    rgb2 = reinterpret_cast<quint8*>(blue);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->fromRgbA16(rgb2, kas2, 1);
    for (int i = 0; i < 2*N+1; i++)
        reinterpret_cast<float*>(kasm)[i] = (reinterpret_cast<float*>(kas1)[i] +
                                             reinterpret_cast<float*>(kas2)[i]) / 2.0;
    cs->toRgbA16(kasm, rgbm, 1);
//     print_vector<float,2*N+1>(kasm, "RED + BLUE IN KS:");
    print_vector<quint16,4>(rgbm, "RED + BLUE IN RBG:");

    rgb1 = reinterpret_cast<quint8*>(red);
    rgb2 = reinterpret_cast<quint8*>(yellow);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->fromRgbA16(rgb2, kas2, 1);
    for (int i = 0; i < 2*N+1; i++)
        reinterpret_cast<float*>(kasm)[i] = (reinterpret_cast<float*>(kas1)[i] +
        reinterpret_cast<float*>(kas2)[i]) / 2.0;
    cs->toRgbA16(kasm, rgbm, 1);
//     print_vector<float,2*N+1>(kasm, "RED + YELLOW IN KS:");
    print_vector<quint16,4>(rgbm, "RED + YELLOW IN RBG:");

    rgb1 = reinterpret_cast<quint8*>(red);
    rgb2 = reinterpret_cast<quint8*>(violet);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->fromRgbA16(rgb2, kas2, 1);
    for (int i = 0; i < 2*N+1; i++)
        reinterpret_cast<float*>(kasm)[i] = (reinterpret_cast<float*>(kas1)[i] +
        reinterpret_cast<float*>(kas2)[i]) / 2.0;
    cs->toRgbA16(kasm, rgbm, 1);
//     print_vector<float,2*N+1>(kasm, "RED + VIOLET IN KS:");
    print_vector<quint16,4>(rgbm, "RED + VIOLET IN RBG:");

    rgb1 = reinterpret_cast<quint8*>(red);
    rgb2 = reinterpret_cast<quint8*>(cyan);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->fromRgbA16(rgb2, kas2, 1);
    for (int i = 0; i < 2*N+1; i++)
        reinterpret_cast<float*>(kasm)[i] = (reinterpret_cast<float*>(kas1)[i] +
        reinterpret_cast<float*>(kas2)[i]) / 2.0;
    cs->toRgbA16(kasm, rgbm, 1);
//     print_vector<float,2*N+1>(kasm, "RED + CYAN IN KS:");
    print_vector<quint16,4>(rgbm, "RED + CYAN IN RBG:");

    rgb1 = reinterpret_cast<quint8*>(green);
    rgb2 = reinterpret_cast<quint8*>(blue);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->fromRgbA16(rgb2, kas2, 1);
    for (int i = 0; i < 2*N+1; i++)
        reinterpret_cast<float*>(kasm)[i] = (reinterpret_cast<float*>(kas1)[i] +
                                             reinterpret_cast<float*>(kas2)[i]) / 2.0;
    cs->toRgbA16(kasm, rgbm, 1);
//     print_vector<float,2*N+1>(kasm, "GREEN + BLUE IN KS:");
    print_vector<quint16,4>(rgbm, "GREEN + BLUE IN RBG:");

    rgb1 = reinterpret_cast<quint8*>(green);
    rgb2 = reinterpret_cast<quint8*>(yellow);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->fromRgbA16(rgb2, kas2, 1);
    for (int i = 0; i < 2*N+1; i++)
        reinterpret_cast<float*>(kasm)[i] = (reinterpret_cast<float*>(kas1)[i] +
        reinterpret_cast<float*>(kas2)[i]) / 2.0;
    cs->toRgbA16(kasm, rgbm, 1);
//     print_vector<float,2*N+1>(kasm, "GREEN + YELLOW IN KS:");
    print_vector<quint16,4>(rgbm, "GREEN + YELLOW IN RBG:");

    rgb1 = reinterpret_cast<quint8*>(green);
    rgb2 = reinterpret_cast<quint8*>(violet);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->fromRgbA16(rgb2, kas2, 1);
    for (int i = 0; i < 2*N+1; i++)
        reinterpret_cast<float*>(kasm)[i] = (reinterpret_cast<float*>(kas1)[i] +
        reinterpret_cast<float*>(kas2)[i]) / 2.0;
    cs->toRgbA16(kasm, rgbm, 1);
//     print_vector<float,2*N+1>(kasm, "GREEN + VIOLET IN KS:");
    print_vector<quint16,4>(rgbm, "GREEN + VIOLET IN RBG:");

    rgb1 = reinterpret_cast<quint8*>(green);
    rgb2 = reinterpret_cast<quint8*>(cyan);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->fromRgbA16(rgb2, kas2, 1);
    for (int i = 0; i < 2*N+1; i++)
        reinterpret_cast<float*>(kasm)[i] = (reinterpret_cast<float*>(kas1)[i] +
        reinterpret_cast<float*>(kas2)[i]) / 2.0;
    cs->toRgbA16(kasm, rgbm, 1);
//     print_vector<float,2*N+1>(kasm, "GREEN + CYAN IN KS:");
    print_vector<quint16,4>(rgbm, "GREEN + CYAN IN RBG:");

    rgb1 = reinterpret_cast<quint8*>(blue);
    rgb2 = reinterpret_cast<quint8*>(yellow);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->fromRgbA16(rgb2, kas2, 1);
    for (int i = 0; i < 2*N+1; i++)
        reinterpret_cast<float*>(kasm)[i] = (reinterpret_cast<float*>(kas1)[i] +
        reinterpret_cast<float*>(kas2)[i]) / 2.0;
    cs->toRgbA16(kasm, rgbm, 1);
//     print_vector<float,2*N+1>(kasm, "BLUE + YELLOW IN KS:");
    print_vector<quint16,4>(rgbm, "BLUE + YELLOW IN RBG:");

    rgb1 = reinterpret_cast<quint8*>(blue);
    rgb2 = reinterpret_cast<quint8*>(violet);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->fromRgbA16(rgb2, kas2, 1);
    for (int i = 0; i < 2*N+1; i++)
        reinterpret_cast<float*>(kasm)[i] = (reinterpret_cast<float*>(kas1)[i] +
        reinterpret_cast<float*>(kas2)[i]) / 2.0;
    cs->toRgbA16(kasm, rgbm, 1);
//     print_vector<float,2*N+1>(kasm, "BLUE + VIOLET IN KS:");
    print_vector<quint16,4>(rgbm, "BLUE + VIOLET IN RBG:");

    rgb1 = reinterpret_cast<quint8*>(blue);
    rgb2 = reinterpret_cast<quint8*>(cyan);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->fromRgbA16(rgb2, kas2, 1);
    for (int i = 0; i < 2*N+1; i++)
        reinterpret_cast<float*>(kasm)[i] = (reinterpret_cast<float*>(kas1)[i] +
        reinterpret_cast<float*>(kas2)[i]) / 2.0;
    cs->toRgbA16(kasm, rgbm, 1);
//     print_vector<float,2*N+1>(kasm, "BLUE + CYAN IN KS:");
    print_vector<quint16,4>(rgbm, "BLUE + CYAN IN RBG:");

    rgb1 = reinterpret_cast<quint8*>(yellow);
    rgb2 = reinterpret_cast<quint8*>(violet);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->fromRgbA16(rgb2, kas2, 1);
    for (int i = 0; i < 2*N+1; i++)
        reinterpret_cast<float*>(kasm)[i] = (reinterpret_cast<float*>(kas1)[i] +
        reinterpret_cast<float*>(kas2)[i]) / 2.0;
    cs->toRgbA16(kasm, rgbm, 1);
//     print_vector<float,2*N+1>(kasm, "YELLOW + VIOLET IN KS:");
    print_vector<quint16,4>(rgbm, "YELLOW + VIOLET IN RBG:");

    rgb1 = reinterpret_cast<quint8*>(yellow);
    rgb2 = reinterpret_cast<quint8*>(cyan);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->fromRgbA16(rgb2, kas2, 1);
    for (int i = 0; i < 2*N+1; i++)
        reinterpret_cast<float*>(kasm)[i] = (reinterpret_cast<float*>(kas1)[i] +
        reinterpret_cast<float*>(kas2)[i]) / 2.0;
    cs->toRgbA16(kasm, rgbm, 1);
//     print_vector<float,2*N+1>(kasm, "YELLOW + CYAN IN KS:");
    print_vector<quint16,4>(rgbm, "YELLOW + CYAN IN RBG:");

    rgb1 = reinterpret_cast<quint8*>(violet);
    rgb2 = reinterpret_cast<quint8*>(cyan);
    cs->fromRgbA16(rgb1, kas1, 1);
    cs->fromRgbA16(rgb2, kas2, 1);
    for (int i = 0; i < 2*N+1; i++)
        reinterpret_cast<float*>(kasm)[i] = (reinterpret_cast<float*>(kas1)[i] +
        reinterpret_cast<float*>(kas2)[i]) / 2.0;
    cs->toRgbA16(kasm, rgbm, 1);
//     print_vector<float,2*N+1>(kasm, "VIOLET + CYAN IN KS:");
    print_vector<quint16,4>(rgbm, "VIOLET + CYAN IN RBG:");

    delete [] rgbm;
    delete [] kasm;
    delete [] kas2;
    delete [] kas1;
    delete profile;
}

#undef N

QTEST_KDEMAIN(KisKSColorSpaceTest, NoGUI)
#include "kis_ks_colorspace_test.moc"
