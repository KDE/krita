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

#include <gmm/gmm.h>

#include <QColor>
#include <QString>
#include <QStringList>

#include <KGlobal>
#include <KStandardDirs>

#include <qtest_kde.h>
#include "illuminant_test.h"
#include "kis_illuminant_profile.h"
#include "kis_layer.h"
#include "kis_ks_colorspace.h"
#include "kis_paint_device.h"
#include "kis_types.h"
#include <kdebug.h>

#include <iostream>
using namespace std;

const QString PATH = "/home/emanuele/kde/src/4/koffice/krita/plugins/viewplugins/painterlymixer/";

void KisIlluminantTester::testConstructor()
{
	KGlobal::mainComponent().dirs()->addResourceType("kis_illuminants",
													 "data", "krita/profiles/");

	QStringList m_illuminants = KGlobal::mainComponent().dirs()->findAllResources("kis_illuminants", "*.ill");

	KisIlluminantProfile *profile = new KisIlluminantProfile(m_illuminants[0]);
	Q_ASSERT( profile );

	cout << profile->matrix() << endl;

	KisKSColorSpace *cs = new KisKSColorSpace(profile);
	Q_ASSERT( cs );

}

void KisIlluminantTester::testReflectanceColorSpace()
{
	KisIlluminantProfile *profile = new KisIlluminantProfile(PATH+"/IlluminantD65.ill");

	KoColorSpace *cs = new KisKSColorSpace(profile);

// 	cout << (int)cs->pixelSize() << endl;

	QVERIFY (cs->pixelSize() == 21*sizeof(float));

	QColor c(255,0,0);

	float *REF = new float[21];

	cs->fromQColor(c, reinterpret_cast<quint8 *>(REF));

	for (int i = 0; i < 21; i++) cout << REF[i] << ", ";
	cout << endl;

	cs->toQColor(reinterpret_cast<quint8 *>(REF), &c);

	kDebug() << c << endl;

	KoColor kc(c, cs);

	kDebug() << "KO: " << kc.toQColor() << endl;
	cs->fromQColor(c, reinterpret_cast<quint8 *>(REF));
	for (int i = 0; i < 21; i++) cout << REF[i] << ", ";
	cout << endl;

	delete [] REF;
}

#include "kis_iterators_pixel.h"

void KisIlluminantTester::testIlluminantProfile()
{
	KisIlluminantProfile *profile = new KisIlluminantProfile(PATH+"/IlluminantD65.ill");

	KoColorSpace *cs = new KisKSColorSpace(profile);

	KisPaintDeviceSP pdev = new KisPaintDevice(cs);

	KisRectIteratorPixel it = pdev->createRectIterator(0,0,64,64);

	QColor red(255,0,0);
	QColor yellow(255,255,0);
	float *redKS = new float[21];
	float *yellowKS = new float[21];

	cs->fromQColor(red, reinterpret_cast<quint8 *>(redKS));
	cs->fromQColor(yellow, reinterpret_cast<quint8 *>(yellowKS));


	while (!it.isDone()) {
// 		cs->fromQColor(c, it.rawData());
// 		cs->toQColor(it.rawData(), &c);
		for (int i = 0; i < 21; i++)
			reinterpret_cast<float*>(it.rawData())[i] = 0.5*redKS[i]+0.5*yellowKS[i];
		++it;
	}

	kDebug() << "BOF!" << endl;
	pdev->convertToQImage(0).save("prova.png");

	delete cs; delete profile;
}

QTEST_KDEMAIN(KisIlluminantTester, NoGUI)
#include "illuminant_test.moc"
