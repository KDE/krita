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

	QColor c(255,255,255);

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
	const QString PATH = "/home/emanuele/kde/src/4/koffice/krita/plugins/viewplugins/painterlymixer/";
	KisIlluminantProfile *profile = new KisIlluminantProfile(PATH+"/IlluminantD65.ill");

	KoColorSpace *cs = new KisKSColorSpace(profile);

	KisPaintDeviceSP pdev = new KisPaintDevice(cs);

	QColor colors[] = {
		QColor(255,0,0),
		QColor(0,255,0),
		QColor(0,0,255),
		QColor(255,255,0),
		QColor(255,0,255),
		QColor(0,255,255),
		QColor(255,255,255),
		QColor(0,0,0),
		QColor(127,0,0),
		QColor(0,127,0),
		QColor(0,0,127),
		QColor(127,127,0),
		QColor(127,0,127),
		QColor(0,127,127),
		QColor(127,127,127)
	};

	float *color1 = new float[21];
	float *color2 = new float[21];

	for (int i = 0; i < 14; i++) {
		for (int j = i+1; j < 15; j++) {
			cs->fromQColor(colors[i], reinterpret_cast<quint8 *>(color1));
			cs->fromQColor(colors[j], reinterpret_cast<quint8 *>(color2));

			KisRectIteratorPixel it_c1 = pdev->createRectIterator(48*i+0 , 32*j+0 , 16, 16);
			KisRectIteratorPixel it_c2 = pdev->createRectIterator(48*i+0 , 32*j+16, 16, 16);
			KisRectIteratorPixel it_cm = pdev->createRectIterator(48*i+16, 32*j+0 , 32, 32);

			while (!it_c1.isDone()) {
				for (int i = 0; i < 21; i++) {
					reinterpret_cast<float*>(it_c1.rawData())[i] = color1[i];
					reinterpret_cast<float*>(it_c2.rawData())[i] = color2[i];
				}
				++it_c1; ++it_c2;
			}

			while (!it_cm.isDone()) {
				for (int i = 0; i < 21; i++)
					reinterpret_cast<float*>(it_cm.rawData())[i] = 0.5*color1[i] + 0.5*color2[i];
				++it_cm;
			}
		}
	}

	kDebug() << "BOF!" << endl;
	pdev->convertToQImage(0).save("mixing.png");

	delete cs;
	delete profile;
	delete [] color1;
	delete [] color2;
}

QTEST_KDEMAIN(KisIlluminantTester, NoGUI)
#include "illuminant_test.moc"
