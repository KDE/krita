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

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>

#include <qtest_kde.h>
#include "plot_test.h"
#include "kis_illuminant_profile.h"
#include "kis_layer.h"
#include "kis_reflectance_colorspace.h"
#include "kis_ks_colorspace.h"
#include "kis_paint_device.h"
#include "kis_paintop.h"
#include "kis_paintop_registry.h"
#include "kis_resource_provider.h"
#include "kis_types.h"
#include <kdebug.h>

#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

void KisPlotTester::testPlotKS()
{
/*
	KGlobal::mainComponent().dirs()->addResourceType("kis_illuminants",
													 "data", "krita/profiles/");

	KisPaintDeviceSP pdev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
*/
	vector<float> REF(SAMPLE_NUMBER), KS(2*SAMPLE_NUMBER);

	for (int i = 0; i < SAMPLE_NUMBER; i++)
		REF[i] = (float)i/(float)SAMPLE_NUMBER;

	computeKS(REF, KS);

	ofstream out("plot.ps");

	out << "%!\n" << endl;

	const int x = 10;
	const double y = 80;
	const double s = 1.5;

	out << "newpath" << endl;
	out << x << " " << y << " moveto" << endl;
	out << x+470 << " " << y << " lineto" << endl;
	out << x << " " << y << " moveto" << endl;
	out << x << " " << y+600 << " lineto" << endl;
	out << "stroke\n\n" << endl;

	out << "newpath" << endl;
	out << x << " " << y + REF[0]*s << " moveto" << endl;

	for (int i = 1; i < SAMPLE_NUMBER; i++)
		out << x + i << " " << y+REF[i]*s << " lineto" << endl;

	out << "1.0 0.0 0.0 setrgbcolor" << endl;
	out << "stroke\n\n" << endl;

	out << "newpath" << endl;
	out << x << " " << y + KS[0]*s << " moveto" << endl;

	for (int i = 2; i < 2*SAMPLE_NUMBER; i+=2)
		out << x + i/2 << " " << y+(double)KS[i]*s << " lineto" << endl;

	out << "0.0 1.0 0.0 setrgbcolor" << endl;
	out << "stroke\n\n" << endl;

	out << "newpath" << endl;
	out << x << " " << y + exp(KS[1])*s << " moveto" << endl;

	for (int i = 3; i < 2*SAMPLE_NUMBER; i+=2)
		out << x + i/2 << " " << y+(double)exp(KS[i])*s << " lineto" << endl;

	out << "0.0 0.0 1.0 setrgbcolor" << endl;
	out << "stroke\n\n" << endl;

	out << "newpath" << endl;
	out << x << " " << y + KS[0]/exp(KS[1])*s << " moveto" << endl;

	for (int i = 3; i < 2*SAMPLE_NUMBER; i+=2)
		out << x + i/2 << " " << y+(double)(KS[i-1]/exp(KS[i]))*s << " lineto" << endl;

	out << "0.0 1.0 1.0 setrgbcolor" << endl;
	out << "stroke\n\n" << endl;

	out << "\n\nshowpage" << endl;
}


QTEST_KDEMAIN(KisPlotTester, NoGUI)
#include "plot_test.moc"
