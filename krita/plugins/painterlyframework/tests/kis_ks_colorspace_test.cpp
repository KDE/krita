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
#include "kis_ksqp_colorspace.h"
#include "kis_kslc_colorspace.h"

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

    KisIlluminantProfile *p6 = new KisIlluminantProfile(d656);
    KisIlluminantProfile *p9 = new KisIlluminantProfile(d659);

    KisKSColorSpace<float,6> *cs6 = new KisKSQPColorSpace<float,6>(p6->clone());
    KisKSColorSpace<float,9> *cs9 = new KisKSQPColorSpace<float,9>(p9->clone());

    QVERIFY(cs6->profileIsCompatible(p9) == false);
    QVERIFY(cs6->profileIsCompatible(p6) == true);

    QVERIFY(cs9->profileIsCompatible(p6) == false);
    QVERIFY(cs9->profileIsCompatible(p9) == true);

    delete cs6;
    delete cs9;

    cs6 = new KisKSLCColorSpace<float,6>(p6->clone());
    cs9 = new KisKSLCColorSpace<float,9>(p9->clone());

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

    KisIlluminantProfile *p6 = new KisIlluminantProfile(d656);
    KisIlluminantProfile *p9 = new KisIlluminantProfile(d659);

    // First, load a colorspace with his default profile
    cs = f->colorSpace(KisKSLCColorSpace<float,6>::ColorSpaceId().id(),0);
    QVERIFY2(cs != 0, "ColorSpace LC6 loaded");
    QVERIFY(cs->profile() != 0);
    cs = f->colorSpace(KisKSLCColorSpace<float,9>::ColorSpaceId().id(),0);
    QVERIFY2(cs != 0, "ColorSpace LC9 loaded");
    QVERIFY(cs->profile() != 0);
    cs = f->colorSpace(KisKSQPColorSpace<float,6>::ColorSpaceId().id(),0);
    QVERIFY2(cs != 0, "ColorSpace QP6 loaded");
    QVERIFY(cs->profile() != 0);
    cs = f->colorSpace(KisKSQPColorSpace<float,9>::ColorSpaceId().id(),0);
    QVERIFY2(cs != 0, "ColorSpace QP9 loaded");
    QVERIFY(cs->profile() != 0);

    // Now with a profile
    cs = f->colorSpace(KisKSLCColorSpace<float,6>::ColorSpaceId().id(), p6);
    QVERIFY2(cs != 0, "ColorSpace LC6 loaded");
    cs = f->colorSpace(KisKSLCColorSpace<float,9>::ColorSpaceId().id(), p9);
    QVERIFY2(cs != 0, "ColorSpace LC9 loaded");
    cs = f->colorSpace(KisKSQPColorSpace<float,6>::ColorSpaceId().id(), p6);
    QVERIFY2(cs != 0, "ColorSpace QP6 loaded");
    cs = f->colorSpace(KisKSQPColorSpace<float,9>::ColorSpaceId().id(), p9);
    QVERIFY2(cs != 0, "ColorSpace QP9 loaded");
}

void KisKSColorSpaceTest::testToFromRgbA16()
{
    KoColorSpaceRegistry *f = KoColorSpaceRegistry::instance();
    QVector<const KoColorSpace *> css;
    css.append(f->colorSpace(KisKSLCColorSpace<float,6>::ColorSpaceId().id(),0));
    css.append(f->colorSpace(KisKSLCColorSpace<float,9>::ColorSpaceId().id(),0));
    css.append(f->colorSpace(KisKSQPColorSpace<float,6>::ColorSpaceId().id(),0));
    css.append(f->colorSpace(KisKSQPColorSpace<float,9>::ColorSpaceId().id(),0));

    quint16 red  [4] = { 0x0000, 0x0000, 0xFFFF, 0xFFFF };
    quint16 green[4] = { 0x0000, 0x0000, 0xFFFF, 0xFFFF };
    quint16 blue [4] = { 0x0000, 0x0000, 0xFFFF, 0xFFFF };
    quint16 random[4] = { rand()%0xFFFF, rand()%0xFFFF, rand()%0xFFFF, rand()%0xFFFF };
    quint16 back[4];
    quint8 *curr;
    foreach(const KoColorSpace *cs, css) {
        const int n = cs->pixelSize()/sizeof(float);
        quint8 *data = new quint8[cs->pixelSize()];

        curr = reinterpret_cast<quint8*>(red);
        print_vector<quint16>(4, curr, "RED ARRAY:");
        cs->fromRgbA16(curr, data, 1);
        print_vector<float>(n, data, "RED PIXEL:");
        curr = reinterpret_cast<quint8*>(back);
        cs->toRgbA16(data, curr, 1);
        print_vector<quint16>(4, curr, "RED ARRAY BACK:");
        QVERIFY(back[0]-red[0] <= 2);
        QVERIFY(back[1]-red[1] <= 2);
        QVERIFY(back[2]-red[2] <= 2);
        QVERIFY(back[3]-red[3] <= 2);

        curr = reinterpret_cast<quint8*>(green);
        print_vector<quint16>(4, curr, "GREEN ARRAY:");
        cs->fromRgbA16(curr, data, 1);
        print_vector<float>(n, data, "GREEN PIXEL:");
        curr = reinterpret_cast<quint8*>(back);
        cs->toRgbA16(data, curr, 1);
        print_vector<quint16>(4, curr, "GREEN ARRAY BACK:");
        QVERIFY(back[0]-green[0] <= 2);
        QVERIFY(back[1]-green[1] <= 2);
        QVERIFY(back[2]-green[2] <= 2);
        QVERIFY(back[3]-green[3] <= 2);

        curr = reinterpret_cast<quint8*>(blue);
        print_vector<quint16>(4, curr, "BLUE ARRAY:");
        cs->fromRgbA16(curr, data, 1);
        print_vector<float>(n, data, "BLUE PIXEL:");
        curr = reinterpret_cast<quint8*>(back);
        cs->toRgbA16(data, curr, 1);
        print_vector<quint16>(4, curr, "BLUE ARRAY BACK:");
        QVERIFY(back[0]-blue[0] <= 2);
        QVERIFY(back[1]-blue[1] <= 2);
        QVERIFY(back[2]-blue[2] <= 2);
        QVERIFY(back[3]-blue[3] <= 2);

        curr = reinterpret_cast<quint8*>(random);
        print_vector<quint16>(4, curr, "RANDOM ARRAY:");
        cs->fromRgbA16(curr, data, 1);
        print_vector<float>(n, data, "RANDOM PIXEL:");
        curr = reinterpret_cast<quint8*>(back);
        cs->toRgbA16(data, curr, 1);
        print_vector<quint16>(4, curr, "RANDOM ARRAY BACK:");
        QVERIFY(back[0]-random[0] <= 2);
        QVERIFY(back[1]-random[1] <= 2);
        QVERIFY(back[2]-random[2] <= 2);
        QVERIFY(back[3]-random[3] <= 2);

        delete data;
    }
}

QTEST_KDEMAIN(KisKSColorSpaceTest, NoGUI)
#include "kis_ks_colorspace_test.moc"
