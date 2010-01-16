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

#include "mixing_test.h"
#include <qtest_kde.h>
#include "kis_illuminant_profile.h"

#include "kis_ks_colorspace.h"
#include "kis_ksf32_colorspace.h"

#include <KGlobal>
#include <KLocale>
#include <KStandardDirs>

#include <QImage>
#include <QPainter>
#include <QString>
#include <QStringList>

void mix(int N, const float *data1, float c1, const float *data2, float c2, float *datam)
{
    for (int i = 0; i < 2*N; i++)
        datam[i] = c1 * data1[i] + c2 * data2[i];
    datam[2*N] = 1;
}

void MixingTest::initTestCase()
{
    KGlobal::mainComponent().dirs()->addResourceType("illuminant_profiles", 0, "share/apps/krita/illuminants");
    list = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", "*.xll",  KStandardDirs::Recursive);

    KoColorSpaceRegistry *f = KoColorSpaceRegistry::instance();
    KisIlluminantProfile *p;
    foreach(QString ill, list.filter("_111_")) {
        p = new KisIlluminantProfile(ill); p->load();
        css.append(f->colorSpace(KisKSF32ColorSpace<3>::ColorModelId().id(), KisKSF32ColorSpace<3>::ColorDepthId().id(), p));
        delete p;
    }
    foreach(QString ill, list.filter("_222_")) {
        p = new KisIlluminantProfile(ill); p->load();
        css.append(f->colorSpace(KisKSF32ColorSpace<6>::ColorModelId().id(), KisKSF32ColorSpace<6>::ColorDepthId().id(), p));
        delete p;
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
