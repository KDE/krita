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
#include "kis_illuminant_profile_test.h"

#include "kis_illuminant_profile.h"

#include <KGlobal>
#include <KLocale>
#include <KStandardDirs>

#include <QString>
#include <QStringList>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

#include <iostream>
using namespace std;

void gsl_print(const gsl_matrix *M, const char *name)
{
    cout << name << endl;
    for (uint i = 0; i < M->size1; i++) {
        cout << "\t";
        for (uint j = 0; j < M->size2; j++) {
            if (gsl_matrix_get(M, i, j) >= 0)
                cout << " ";
            cout << gsl_matrix_get(M, i, j) << " ";
        }
        cout << endl;
    }
    cout << endl;
}

void gsl_print(const gsl_vector *V, const char *name)
{
    cout << name << endl;
    cout << "\t";
    for (uint i = 0; i < V->size; i++) {
        if (gsl_vector_get(V, i) >= 0)
            cout << " ";
        cout << gsl_vector_get(V, i) << " ";
    }
    cout << endl;
}

void KisIlluminantProfileTest::initTestCase()
{
    KGlobal::mainComponent().dirs()->addResourceType("illuminant_profiles", 0, "share/apps/krita/illuminants");
    list = KGlobal::mainComponent().dirs()->findAllResources("illuminant_profiles", "*.ill",  KStandardDirs::Recursive);
}

void KisIlluminantProfileTest::testLoading()
{
    QString d9low = list.filter("9_low")[0];
    QString d9high = list.filter("9_high")[0];

    KisIlluminantProfile *p1 = new KisIlluminantProfile;
    QVERIFY(p1->valid() == false);
    p1->setFileName(d9low);
    QVERIFY(p1->valid() == false);
    p1->load();
    QVERIFY(p1->valid() == true);
    QVERIFY(p1->wavelengths() == 9);
    QCOMPARE(p1->Kblack(), 4.3);
    QCOMPARE(p1->Sblack(), 0.14);
    gsl_print(p1->T(), "Transformation matrix: ");
    gsl_print(p1->P(), "Positions vector: ");

    KisIlluminantProfile *p2 = dynamic_cast<KisIlluminantProfile*>(p1->clone());
    delete p1;
    QVERIFY(p2->valid() == true);
    QVERIFY(p2->wavelengths() == 9);
    QCOMPARE(p2->Kblack(), 4.3);
    QCOMPARE(p2->Sblack(), 0.14);
    gsl_print(p2->T(), "Transformation matrix: ");
    gsl_print(p2->P(), "Positions vector: ");

    p2->setFileName(d9high);
    QVERIFY(p2->valid() == true);
    p2->load();
    QVERIFY(p2->valid() == true);
    QVERIFY(p2->wavelengths() == 9);
    QCOMPARE(p2->Kblack(), 11.0);
    QCOMPARE(p2->Sblack(), 0.35);
    gsl_print(p2->T(), "Transformation matrix: ");
    gsl_print(p2->P(), "Positions vector: ");

    delete p2;

    p1 = new KisIlluminantProfile(d9low);
    QVERIFY(p1->valid() == true);
    QVERIFY(p1->wavelengths() == 9);
    QCOMPARE(p1->Kblack(), 4.3);
    QCOMPARE(p1->Sblack(), 0.14);
    gsl_print(p1->T(), "Transformation matrix: ");
    gsl_print(p1->P(), "Positions vector: ");

    delete p1;
}

void KisIlluminantProfileTest::testSaving()
{
    QString d659 = list.filter("9_low")[0];

    KisIlluminantProfile *p = new KisIlluminantProfile(d659);
    p->save("D659Save.ill");
    delete p;

    p = new KisIlluminantProfile("D659Save.ill");
    QVERIFY(p->valid() == true);
    QVERIFY(p->wavelengths() == 9);
    QCOMPARE(p->Kblack(), 4.3);
    QCOMPARE(p->Sblack(), 0.14);
    gsl_print(p->T(), "Transformation matrix: ");
    gsl_print(p->P(), "Positions vector: ");

    delete p;
}

QTEST_KDEMAIN(KisIlluminantProfileTest, NoGUI)
#include "kis_illuminant_profile_test.moc"
