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

#include "kis_illuminant_profile_test.h"
#include <qtest_kde.h>


#include "kis_illuminant_profile.h"

#include <KGlobal>
#include <KLocale>
#include <KStandardDirs>

#include <QString>
#include <QStringList>

void KisIlluminantProfileTest::initTestCase()
{
    KGlobal::mainComponent().dirs()->addResourceType("illuminant_profiles", 0, "share/apps/krita/illuminants");
    list = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", "*.ill",  KStandardDirs::Recursive);
}

void KisIlluminantProfileTest::testLoading()
{
    QString d1 = list.filter("_121_")[0];
    QString d2 = list.filter("_222_")[0];

    KisIlluminantProfile *p1 = new KisIlluminantProfile;
    QVERIFY(p1->valid() == false);
    p1->setFileName(d1);
    QVERIFY(p1->valid() == false);
    p1->load();
    QVERIFY(p1->valid() == true);
    QVERIFY(p1->wavelengths() == 4);

    KisIlluminantProfile *p2 = dynamic_cast<KisIlluminantProfile*>(p1->clone());
    delete p1;
    QVERIFY(p2->valid() == true);
    QVERIFY(p2->wavelengths() == 4);

    p2->setFileName(d2);
    p2->load();
    QVERIFY(p2->valid() == true);
    QVERIFY(p2->wavelengths() == 6);

    delete p2;

    p1 = new KisIlluminantProfile(d1);
    p1->load();
    QVERIFY(p1->valid() == true);
    QVERIFY(p1->wavelengths() == 4);

    delete p1;
}

void KisIlluminantProfileTest::testSaving()
{
    QString d = list.filter("_222_")[0];

    KisIlluminantProfile *p = new KisIlluminantProfile(d);
    p->load();
    p->save("IlluminantSave.ill");
    delete p;

    p = new KisIlluminantProfile("IlluminantSave.ill");
    p->load();
    QVERIFY(p->valid() == true);
    QVERIFY(p->wavelengths() == 6);

    delete p;
}

QTEST_KDEMAIN(KisIlluminantProfileTest, NoGUI)
#include "kis_illuminant_profile_test.moc"
