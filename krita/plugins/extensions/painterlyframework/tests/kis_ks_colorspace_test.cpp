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

#include "kis_ks_colorspace_test.h"
#include <qtest_kde.h>


#include "kis_illuminant_profile.h"

#include "kis_ks_colorspace.h"
#include "kis_ksf32_colorspace.h"

#include <KoColorSpaceRegistry.h>

#include <KGlobal>
#include <KLocale>
#include <KStandardDirs>

#include <QStringList>
#include <QVector>

#include <cmath>
#include <ctime>

template<typename type>
void print_vector(int n, const quint8 *v, const QString &text)
{
    QString vstr;
    qDebug() << text;
    for (int i = 0; i < n; i++)
        vstr += QString::number(reinterpret_cast<const type*>(v)[i]) + " ";
    qDebug() << vstr;
}

void KisKSColorSpaceTest::initTestCase()
{
    srand(time(0));
    KGlobal::mainComponent().dirs()->addResourceType("illuminant_profiles", 0, "share/apps/krita/illuminants");
    list = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", "*.xll",  KStandardDirs::Recursive);
}

void KisKSColorSpaceTest::testConstructor()
{
    QString d651 = list.filter("_121_")[0];
    QString d652 = list.filter("_222_")[0];

    KisIlluminantProfile *p1 = new KisIlluminantProfile(d651);
    KisIlluminantProfile *p2 = new KisIlluminantProfile(d652);
    p1->load();
    p2->load();

    KisKSF32ColorSpace<4> *cs1 = new KisKSF32ColorSpace<4>(p1->clone());
    KisKSF32ColorSpace<6> *cs2 = new KisKSF32ColorSpace<6>(p2->clone());

    QVERIFY(cs1->profileIsCompatible(p1) == true);
    QVERIFY(cs1->profileIsCompatible(p2) == false);

    QVERIFY(cs2->profileIsCompatible(p1) == false);
    QVERIFY(cs2->profileIsCompatible(p2) == true);

    delete cs1;
    delete cs2;

    delete p1;
    delete p2;
}

void KisKSColorSpaceTest::testRegistry()
{
    KoColorSpaceRegistry *f = KoColorSpaceRegistry::instance();
    const KoColorSpace *cs;
    QString d651 = list.filter("_121_")[0];
    QString d652 = list.filter("_222_")[0];

    KisIlluminantProfile *p1 = new KisIlluminantProfile(d651);
    KisIlluminantProfile *p2 = new KisIlluminantProfile(d652);
    p1->load();
    p2->load();
    /*
        // First, load a colorspace with his default profile
        cs = f->colorSpace(KisKSF32ColorSpace<5>::ColorSpaceId().id(),0);
        QVERIFY2(cs != 0, "ColorSpace KS5 loaded");
        QVERIFY(cs->profile() != 0);
        cs = f->colorSpace(KisKSF32ColorSpace<9>::ColorSpaceId().id(),0);
        QVERIFY2(cs != 0, "ColorSpace KS9 loaded");
        QVERIFY(cs->profile() != 0);
    */
    // Now with a profile
    cs = f->colorSpace(KisKSF32ColorSpace<4>::ColorModelId().id(), KisKSF32ColorSpace<4>::ColorDepthId().id(), p1);
    QVERIFY2(cs != 0, "ColorSpace KS4 loaded - with custom profile");
    cs = f->colorSpace(KisKSF32ColorSpace<6>::ColorModelId().id(), KisKSF32ColorSpace<6>::ColorDepthId().id(), p2);
    QVERIFY2(cs != 0, "ColorSpace KS6 loaded - with custom profile");

    delete p1;
    delete p2;
}

void KisKSColorSpaceTest::testToFromRgbA16()
{
    KoColorSpaceRegistry *f = KoColorSpaceRegistry::instance();

    QString d651 = list.filter("_111_")[0];
    QString d652 = list.filter("_222_")[0];

    KisIlluminantProfile *p1 = new KisIlluminantProfile(d651);
    KisIlluminantProfile *p2 = new KisIlluminantProfile(d652);
    p1->load();
    p2->load();

    QVector<const KoColorSpace *> css;
    css.append(f->colorSpace(KisKSF32ColorSpace<3>::ColorModelId().id(), KisKSF32ColorSpace<3>::ColorDepthId().id(), p1));
    css.append(f->colorSpace(KisKSF32ColorSpace<6>::ColorModelId().id(), KisKSF32ColorSpace<3>::ColorDepthId().id(), p2));

    quint16 red  [4] = { 0x0000, 0x0000, 0xFFFF, 0xFFFF };
    quint16 green[4] = { 0x0000, 0xFFFF, 0x0000, 0xFFFF };
    quint16 blue [4] = { 0xFFFF, 0x0000, 0x0000, 0xFFFF };
    quint16 white[4] = { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };
    quint16 black[4] = { 0     , 0     , 0     , 0xFFFF };
    quint16 random[4] = { rand() % 0xFFFF, rand() % 0xFFFF, rand() % 0xFFFF, rand() % 0xFFFF };
    quint8 *curr;

    quint16 back[4];
    quint8 *back8 = reinterpret_cast<quint8*>(back);

#define VERBOSE
    foreach(const KoColorSpace *cs, css) {
        qDebug() << "Current CS:" << cs->name() << cs->profile()->fileName();
        quint8 *data = new quint8[cs->pixelSize()];
#ifdef VERBOSE
        const int n = cs->pixelSize() / sizeof(float);
#endif

        curr = reinterpret_cast<quint8*>(red);
        cs->fromRgbA16(curr, data, 1);
        cs->toRgbA16(data, back8, 1);
#ifdef VERBOSE
        print_vector<quint16>(4, curr, "RED ARRAY:");
        print_vector<float>(n, data, "RED PIXEL:");
        print_vector<quint16>(4, back8, "RED ARRAY BACK:");
#endif
        QVERIFY(abs(back[0] - red[0]) <= 2);
        QVERIFY(abs(back[1] - red[1]) <= 2);
        QVERIFY(abs(back[2] - red[2]) <= 2);
        QVERIFY(abs(back[3] - red[3]) <= 2);

        curr = reinterpret_cast<quint8*>(green);
        cs->fromRgbA16(curr, data, 1);
        cs->toRgbA16(data, back8, 1);
#ifdef VERBOSE
        print_vector<quint16>(4, curr, "GREEN ARRAY:");
        print_vector<float>(n, data, "GREEN PIXEL:");
        print_vector<quint16>(4, back8, "GREEN ARRAY BACK:");
#endif
        QVERIFY(abs(back[0] - green[0]) <= 2);
        QVERIFY(abs(back[1] - green[1]) <= 2);
        QVERIFY(abs(back[2] - green[2]) <= 2);
        QVERIFY(abs(back[3] - green[3]) <= 2);

        curr = reinterpret_cast<quint8*>(blue);
        cs->fromRgbA16(curr, data, 1);
        cs->toRgbA16(data, back8, 1);
#ifdef VERBOSE
        print_vector<quint16>(4, curr, "BLUE ARRAY:");
        print_vector<float>(n, data, "BLUE PIXEL:");
        print_vector<quint16>(4, back8, "BLUE ARRAY BACK:");
#endif
        QVERIFY(abs(back[0] - blue[0]) <= 2);
        QVERIFY(abs(back[1] - blue[1]) <= 2);
        QVERIFY(abs(back[2] - blue[2]) <= 2);
        QVERIFY(abs(back[3] - blue[3]) <= 2);

        curr = reinterpret_cast<quint8*>(white);
        cs->fromRgbA16(curr, data, 1);
        cs->toRgbA16(data, back8, 1);
#ifdef VERBOSE
        print_vector<quint16>(4, curr, "WHITE ARRAY:");
        print_vector<float>(n, data, "WHITE PIXEL:");
        print_vector<quint16>(4, back8, "WHITE ARRAY BACK:");
#endif
        QVERIFY(abs(back[0] - white[0]) <= 2);
        QVERIFY(abs(back[1] - white[1]) <= 2);
        QVERIFY(abs(back[2] - white[2]) <= 2);
        QVERIFY(abs(back[3] - white[3]) <= 2);

        curr = reinterpret_cast<quint8*>(black);
        cs->fromRgbA16(curr, data, 1);
        cs->toRgbA16(data, back8, 1);
#ifdef VERBOSE
        print_vector<quint16>(4, curr, "BLACK ARRAY:");
        print_vector<float>(n, data, "BLACK PIXEL:");
        print_vector<quint16>(4, back8, "BLACK ARRAY BACK:");
#endif
        QVERIFY(abs(back[0] - black[0]) <= 2);
        QVERIFY(abs(back[1] - black[1]) <= 2);
        QVERIFY(abs(back[2] - black[2]) <= 2);
        QVERIFY(abs(back[3] - black[3]) <= 2);

        curr = reinterpret_cast<quint8*>(random);
        cs->fromRgbA16(curr, data, 1);
        cs->toRgbA16(data, back8, 1);
#ifdef VERBOSE
        print_vector<quint16>(4, curr, "RANDOM ARRAY:");
        print_vector<float>(n, data, "RANDOM PIXEL:");
        print_vector<quint16>(4, back8, "RANDOM ARRAY BACK:");
#endif
        QVERIFY(abs(back[0] - random[0]) <= 2);
        QVERIFY(abs(back[1] - random[1]) <= 2);
        QVERIFY(abs(back[2] - random[2]) <= 2);
        QVERIFY(abs(back[3] - random[3]) <= 2);

        delete [] data;
    }

    delete p1;
    delete p2;
}

QTEST_KDEMAIN(KisKSColorSpaceTest, GUI)
#include "kis_ks_colorspace_test.moc"
