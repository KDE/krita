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
#include <lcms.h>

#include <QColor>
#include <QString>
#include <QStringList>

#include <KGlobal>
#include <KStandardDirs>

#include <qtest_kde.h>
#include "reflectance_test.h"
#include "kis_illuminant_profile.h"
#include "kis_iterators_pixel.h"
#include "kis_layer.h"
#include "kis_reflectance_colorspace.h"
#include "kis_ks_colorspace.h"
#include "kis_paint_device.h"
#include "kis_types.h"
#include <kdebug.h>

#include <vector>
#include <iostream>
using namespace std;

void KisReflectanceTester::testReflectance()
{
	KGlobal::mainComponent().dirs()->addResourceType("kis_illuminants",
                                                     "data", "krita/profiles/");

	KisIlluminantProfile *profile = new KisIlluminantProfile("IlluminantD65.ill");
	KisReflectanceColorSpace *cs = new KisReflectanceColorSpace(profile);
	KisKSColorSpace *cs2 = new KisKSColorSpace(profile);
	KisPaintDeviceSP pdev = new KisPaintDevice(cs);
	KisPaintDeviceSP pdev2 = new KisPaintDevice(cs2);

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

	float *color1;
	float *color2;
	float *colorM;

	KisRectIteratorPixel it = pdev->createRectIterator(0, 0, 3, 1000);

	for (int i = 0; i < 14; i++) {
		for (int j = i+1; j < 15; j++) {
			kDebug() << "----------------------------------------------------" << endl;
			kDebug() << "----------------------------------------------------" << endl;
			color1 = reinterpret_cast<float*>(it.rawData());
			cs->fromQColor(colors[i], it.rawData()); ++it;
			color2 = reinterpret_cast<float*>(it.rawData());
			cs->fromQColor(colors[j], it.rawData()); ++it;
			colorM = reinterpret_cast<float*>(it.rawData());

			for (int i = 0; i < SAMPLE_NUMBER+1; i++)
				colorM[i] = color1[i] + color2[i];

			++it;
		}
	}

	it = pdev->createRectIterator(3, 0, 3, 1000);

	for (int i = 0; i < 14; i++) {
		for (int j = i+1; j < 15; j++) {
			kDebug() << "----------------------------------------------------" << endl;
			kDebug() << "----------------------------------------------------" << endl;
			color1 = reinterpret_cast<float*>(it.rawData());
			cs->fromQColor(colors[i], it.rawData()); ++it;
			color2 = reinterpret_cast<float*>(it.rawData());
			cs->fromQColor(colors[j], it.rawData()); ++it;
			colorM = reinterpret_cast<float*>(it.rawData());

			for (int i = 0; i < SAMPLE_NUMBER+1; i++)
				colorM[i] = 0.5*color1[i] + 0.5*color2[i];

			++it;
		}
	}

	it = pdev2->createRectIterator(0, 0, 3, 1000);

	for (int i = 0; i < 14; i++) {
		for (int j = i+1; j < 15; j++) {
			kDebug() << "----------------------------------------------------" << endl;
			kDebug() << "----------------------------------------------------" << endl;
			color1 = reinterpret_cast<float*>(it.rawData());
			cs2->fromQColor(colors[i], it.rawData()); ++it;
			color2 = reinterpret_cast<float*>(it.rawData());
			cs2->fromQColor(colors[j], it.rawData()); ++it;
			colorM = reinterpret_cast<float*>(it.rawData());

			for (int i = 0; i < 2*SAMPLE_NUMBER+1; i++)
				colorM[i] = 0.5*color1[i] + 0.5*color2[i];

			++it;
		}
	}

	it = pdev2->createRectIterator(3, 0, 3, 1000);

	for (int i = 0; i < 14; i++) {
		for (int j = i+1; j < 15; j++) {
			kDebug() << "----------------------------------------------------" << endl;
			kDebug() << "----------------------------------------------------" << endl;
			color1 = reinterpret_cast<float*>(it.rawData());
			cs2->fromQColor(colors[i], it.rawData()); ++it;
			color2 = reinterpret_cast<float*>(it.rawData());
			cs2->fromQColor(colors[j], it.rawData()); ++it;
			colorM = reinterpret_cast<float*>(it.rawData());

			for (int i = 0; i < 2*SAMPLE_NUMBER; i+=2) {
				colorM[i+0] = color1[i+0] + color2[i+0];
				colorM[i+1] = 0.5*color1[i+1] + 0.5*color2[i+1];
			}
			colorM[2*SAMPLE_NUMBER] = color1[2*SAMPLE_NUMBER] + color2[2*SAMPLE_NUMBER];

			++it;
		}
	}

	pdev->convertToQImage(0).save("mixing_ref.png");
	pdev2->convertToQImage(0).save("mixing_ks.png");

	delete cs;
	delete profile;
}


QTEST_KDEMAIN(KisReflectanceTester, NoGUI)
#include "reflectance_test.moc"
