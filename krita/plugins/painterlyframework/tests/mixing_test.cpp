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

#include "mixing_test.h"

#include "kis_illuminant_profile.h"
#include "kis_kslc_colorspace.h"
#include "kis_ksqp_colorspace.h"

#include <KGlobal>
#include <KLocale>
#include <KStandardDirs>

#include <QImage>
#include <QPainter>
#include <QString>
#include <QStringList>

void mix(const quint32 N, const float *data1, float c1, const float *data2, float c2, float *datam)
{
    for (uint i = 0; i < 2*N; i++)
        datam[i] = c1*data1[i] + c2*data2[i];
    datam[2*N] = 1;
}

void MixingTest::initTestCase()
{
    KGlobal::mainComponent().dirs()->addResourceType("illuminant_profiles", 0, "share/apps/krita/illuminants");
    list = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", "*.ill",  KStandardDirs::Recursive);

    KoColorSpaceRegistry *f = KoColorSpaceRegistry::instance();
    foreach(QString ill6, list.filter("_6_")) {
        css.append(f->colorSpace(KisKSLCColorSpace<float,6>::ColorSpaceId().id(),new KisIlluminantProfile(ill6)));
        css.append(f->colorSpace(KisKSQPColorSpace<float,6>::ColorSpaceId().id(),new KisIlluminantProfile(ill6)));
    }
    foreach(QString ill9, list.filter("_9_")) {
        css.append(f->colorSpace(KisKSLCColorSpace<float,9>::ColorSpaceId().id(),new KisIlluminantProfile(ill9)));
        css.append(f->colorSpace(KisKSQPColorSpace<float,9>::ColorSpaceId().id(),new KisIlluminantProfile(ill9)));
    }
}

void MixingTest::testMixing1()
{
    foreach(const KoColorSpace *cs, css) {
        qDebug() << "Current Color Space:" << cs->name();
        qDebug() << "with profile:" << cs->profile()->name();
    }
}

void MixingTest::testMixing2()
{

}

QTEST_KDEMAIN(MixingTest, NoGUI)
#include "mixing_test.moc"
