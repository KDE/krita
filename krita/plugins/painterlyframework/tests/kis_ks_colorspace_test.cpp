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
#include "kis_ks_colorspace_test.h"

#include "kis_illuminant_profile.h"
#include "kis_illuminant_profile_qp.h"

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
    srand( time(0) );
    KGlobal::mainComponent().dirs()->addResourceType("illuminant_profiles", 0, "share/apps/krita/illuminants");
    list = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", "*.ill",  KStandardDirs::Recursive);
}

void KisKSColorSpaceTest::testConstructor()
{
    QString d656 = list.filter("6_high")[0];
    QString d659 = list.filter("9_high")[0];

    KisIlluminantProfile *p6 = new KisIlluminantProfileQP(d656);
    KisIlluminantProfile *p9 = new KisIlluminantProfileQP(d659);
    p6->load();
    p9->load();

    KisKSF32ColorSpace<6> *cs6 = new KisKSF32ColorSpace<6>(p6->clone());
    KisKSF32ColorSpace<9> *cs9 = new KisKSF32ColorSpace<9>(p9->clone());

    QVERIFY(cs6->profileIsCompatible(p9) == false);
    QVERIFY(cs6->profileIsCompatible(p6) == true);

    QVERIFY(cs9->profileIsCompatible(p6) == false);
    QVERIFY(cs9->profileIsCompatible(p9) == true);

    delete cs6;
    delete cs9;

    delete p6;
    delete p9;
}

void KisKSColorSpaceTest::testRegistry()
{
    KoColorSpaceRegistry *f = KoColorSpaceRegistry::instance();
    const KoColorSpace *cs;
    QString d656 = list.filter("6_high")[0];
    QString d659 = list.filter("9_high")[0];

    KisIlluminantProfile *p6 = new KisIlluminantProfileQP(d656);
    KisIlluminantProfile *p9 = new KisIlluminantProfileQP(d659);
    p6->load();
    p9->load();

    // First, load a colorspace with his default profile
    cs = f->colorSpace(KisKSF32ColorSpace<6>::ColorSpaceId().id(),0);
    QVERIFY2(cs != 0, "ColorSpace KS6 loaded");
    QVERIFY(cs->profile() != 0);
    cs = f->colorSpace(KisKSF32ColorSpace<9>::ColorSpaceId().id(),0);
    QVERIFY2(cs != 0, "ColorSpace KS9 loaded");
    QVERIFY(cs->profile() != 0);

    // Now with a profile
    cs = f->colorSpace(KisKSF32ColorSpace<6>::ColorSpaceId().id(), p6);
    QVERIFY2(cs != 0, "ColorSpace KS6 loaded - with custom profile");
    cs = f->colorSpace(KisKSF32ColorSpace<9>::ColorSpaceId().id(), p9);
    QVERIFY2(cs != 0, "ColorSpace KS9 loaded - with custom profile");
}

void KisKSColorSpaceTest::testToFromRgbA16()
{
    KoColorSpaceRegistry *f = KoColorSpaceRegistry::instance();
    QVector<const KoColorSpace *> css;
    css.append(f->colorSpace(KisKSF32ColorSpace<6>::ColorSpaceId().id(),0));
    css.append(f->colorSpace(KisKSF32ColorSpace<9>::ColorSpaceId().id(),0));

    quint16 red  [4] = { 0x0000, 0x0000, 0xFFFF, 0xFFFF };
    quint16 green[4] = { 0x0000, 0xFFFF, 0x0000, 0xFFFF };
    quint16 blue [4] = { 0xFFFF, 0x0000, 0x0000, 0xFFFF };
    quint16 white[4] = { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };
    quint16 black[4] = { 0     , 0     , 0     , 0      };
    quint16 random[4] = { rand()%0xFFFF, rand()%0xFFFF, rand()%0xFFFF, rand()%0xFFFF };
    quint8 *curr;

    quint16 back[4];
    quint8 *back8 = reinterpret_cast<quint8*>(back);

    #define VERBOSE
    foreach(const KoColorSpace *cs, css) {
        qDebug() << "Current CS:" << cs->name();
        quint8 *data = new quint8[cs->pixelSize()];
        #ifdef VERBOSE
        const int n = cs->pixelSize()/sizeof(float);
        #endif

        curr = reinterpret_cast<quint8*>(red);
        cs->fromRgbA16(curr, data, 1);
        cs->toRgbA16(data, back8, 1);
        QVERIFY(abs(back[0]-red[0]) <= 2);
        QVERIFY(abs(back[1]-red[1]) <= 2);
        QVERIFY(abs(back[2]-red[2]) <= 2);
        QVERIFY(abs(back[3]-red[3]) <= 2);
        #ifdef VERBOSE
        print_vector<quint16>(4, curr, "RED ARRAY:");
        print_vector<float>(n, data, "RED PIXEL:");
        print_vector<quint16>(4, back8, "RED ARRAY BACK:");
        #endif

        curr = reinterpret_cast<quint8*>(green);
        cs->fromRgbA16(curr, data, 1);
        cs->toRgbA16(data, back8, 1);
        QVERIFY(abs(back[0]-green[0]) <= 2);
        QVERIFY(abs(back[1]-green[1]) <= 2);
        QVERIFY(abs(back[2]-green[2]) <= 2);
        QVERIFY(abs(back[3]-green[3]) <= 2);
        #ifdef VERBOSE
        print_vector<quint16>(4, curr, "GREEN ARRAY:");
        print_vector<float>(n, data, "GREEN PIXEL:");
        print_vector<quint16>(4, back8, "GREEN ARRAY BACK:");
        #endif

        curr = reinterpret_cast<quint8*>(blue);
        cs->fromRgbA16(curr, data, 1);
        cs->toRgbA16(data, back8, 1);
        QVERIFY(abs(back[0]-blue[0]) <= 2);
        QVERIFY(abs(back[1]-blue[1]) <= 2);
        QVERIFY(abs(back[2]-blue[2]) <= 2);
        QVERIFY(abs(back[3]-blue[3]) <= 2);
        #ifdef VERBOSE
        print_vector<quint16>(4, curr, "BLUE ARRAY:");
        print_vector<float>(n, data, "BLUE PIXEL:");
        print_vector<quint16>(4, back8, "BLUE ARRAY BACK:");
        #endif

        curr = reinterpret_cast<quint8*>(white);
        cs->fromRgbA16(curr, data, 1);
        cs->toRgbA16(data, back8, 1);
        QVERIFY(abs(back[0]-white[0]) <= 2);
        QVERIFY(abs(back[1]-white[1]) <= 2);
        QVERIFY(abs(back[2]-white[2]) <= 2);
        QVERIFY(abs(back[3]-white[3]) <= 2);
        #ifdef VERBOSE
        print_vector<quint16>(4, curr, "WHITE ARRAY:");
        print_vector<float>(n, data, "WHITE PIXEL:");
        print_vector<quint16>(4, back8, "WHITE ARRAY BACK:");
        #endif

        curr = reinterpret_cast<quint8*>(black);
        cs->fromRgbA16(curr, data, 1);
        cs->toRgbA16(data, back8, 1);
        QVERIFY(abs(back[0]-black[0]) <= 2);
        QVERIFY(abs(back[1]-black[1]) <= 2);
        QVERIFY(abs(back[2]-black[2]) <= 2);
        QVERIFY(abs(back[3]-black[3]) <= 2);
        #ifdef VERBOSE
        print_vector<quint16>(4, curr, "BLACK ARRAY:");
        print_vector<float>(n, data, "BLACK PIXEL:");
        print_vector<quint16>(4, back8, "BLACK ARRAY BACK:");
        #endif

        curr = reinterpret_cast<quint8*>(random);
        cs->fromRgbA16(curr, data, 1);
        cs->toRgbA16(data, back8, 1);
        QVERIFY(abs(back[0]-random[0]) <= 2);
        QVERIFY(abs(back[1]-random[1]) <= 2);
        QVERIFY(abs(back[2]-random[2]) <= 2);
        QVERIFY(abs(back[3]-random[3]) <= 2);
        #ifdef VERBOSE
        print_vector<quint16>(4, curr, "RANDOM ARRAY:");
        print_vector<float>(n, data, "RANDOM PIXEL:");
        print_vector<quint16>(4, back8, "RANDOM ARRAY BACK:");
        #endif

        delete data;
    }
}

QTEST_KDEMAIN(KisKSColorSpaceTest, GUI)
#include "kis_ks_colorspace_test.moc"
