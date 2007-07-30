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
// #include <glpk.h>

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

#include <vector>
#include <iostream>
using namespace std;

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
	KisIlluminantProfile *profile = new KisIlluminantProfile("IlluminantD65.ill");

// 	KoColorSpace *cs = new KisKSColorSpace(profile);

	cmsHPROFILE hsRGB, hXYZ;
	cmsHTRANSFORM XYZ_RGB, RGB_XYZ;

	hsRGB = cmsCreate_sRGBProfile();
	hXYZ  = cmsCreateXYZProfile();

	XYZ_RGB = cmsCreateTransform(hXYZ, TYPE_XYZ_DBL, hsRGB, TYPE_RGB_DBL,
								 INTENT_ABSOLUTE_COLORIMETRIC, cmsFLAGS_NOTPRECALC);
	RGB_XYZ = cmsCreateTransform(hsRGB, TYPE_RGB_DBL, hXYZ, TYPE_XYZ_DBL,
								 INTENT_ABSOLUTE_COLORIMETRIC, cmsFLAGS_NOTPRECALC);
	{
		double RGB[] = { 1, 0, 0 };
		double XYZ[3];

		cmsDoTransform(RGB_XYZ, RGB, XYZ, 1);

		vector<float> vXYZ(3), vREF(SAMPLE_NUMBER), vKS(SAMPLE_NUMBER*2);

		for (int i = 0; i < 3; i++) vXYZ[i] = XYZ[i];

		cout << "REAL RGB: " << RGB[0] << ", " << RGB[1] << ", " << RGB[2] << endl;

		simplex(profile->matrix(), vREF, vXYZ);
		gmm::mult(profile->matrix(), vREF, vXYZ);
		for (int i = 0; i < 3; i++) XYZ[i] = vXYZ[i];
		cmsDoTransform(XYZ_RGB, XYZ, RGB, 1);
		computeKS(vREF, vKS);

		cout << "COMPUTED RGB: " << RGB[0] << ", " << RGB[1] << ", " << RGB[2] << endl;
		cout << "--------------------------------------------" << endl;
	}

	{
		double RGB[] = { 0, 1, 0 };
double XYZ[3];

		cmsDoTransform(RGB_XYZ, RGB, XYZ, 1);

		vector<float> vXYZ(3), vREF(SAMPLE_NUMBER), vKS(SAMPLE_NUMBER*2);

		for (int i = 0; i < 3; i++) vXYZ[i] = XYZ[i];

		cout << "REAL RGB: " << RGB[0] << ", " << RGB[1] << ", " << RGB[2] << endl;

		simplex(profile->matrix(), vREF, vXYZ);
		gmm::mult(profile->matrix(), vREF, vXYZ);
		for (int i = 0; i < 3; i++) XYZ[i] = vXYZ[i];
		cmsDoTransform(XYZ_RGB, XYZ, RGB, 1);
		computeKS(vREF, vKS);

		cout << "COMPUTED RGB: " << RGB[0] << ", " << RGB[1] << ", " << RGB[2] << endl;
		cout << "--------------------------------------------" << endl;
	}

	{
		double RGB[] = { 0, 0, 1 };
double XYZ[3];

		cmsDoTransform(RGB_XYZ, RGB, XYZ, 1);

		vector<float> vXYZ(3), vREF(SAMPLE_NUMBER), vKS(SAMPLE_NUMBER*2);

		for (int i = 0; i < 3; i++) vXYZ[i] = XYZ[i];

		cout << "REAL RGB: " << RGB[0] << ", " << RGB[1] << ", " << RGB[2] << endl;

		simplex(profile->matrix(), vREF, vXYZ);
		gmm::mult(profile->matrix(), vREF, vXYZ);
		for (int i = 0; i < 3; i++) XYZ[i] = vXYZ[i];
		cmsDoTransform(XYZ_RGB, XYZ, RGB, 1);
		computeKS(vREF, vKS);

		cout << "COMPUTED RGB: " << RGB[0] << ", " << RGB[1] << ", " << RGB[2] << endl;
		cout << "--------------------------------------------" << endl;
	}

	{
		vector<float> vREF(SAMPLE_NUMBER), vREFafter(SAMPLE_NUMBER), vKS(2*SAMPLE_NUMBER);
		for (int i = 0; i < SAMPLE_NUMBER; i++)
			vREF[i] = (float)i / (float)SAMPLE_NUMBER;

		vREF[SAMPLE_NUMBER-1] = 0.99999;
		computeKS(vREF, vKS);
		computeReflectance(vKS, vREFafter);

		cout << "-----------------------------------------------" << endl;
		for (int i = 0; i < 2*SAMPLE_NUMBER; i+=2)
			cout << "R: " << vREF[i/2] << ", K: " << vKS[i+0] << ", S: " << vKS[i+1] << ", R again: " << vREFafter[i/2] << endl;
	}

	cmsDeleteTransform(RGB_XYZ);
	cmsDeleteTransform(XYZ_RGB);
	cmsCloseProfile(hsRGB);
	cmsCloseProfile(hXYZ);
}

#include "kis_iterators_pixel.h"

void KisIlluminantTester::testIlluminantProfile()
{/*
	KisIlluminantProfile *profile = new KisIlluminantProfile("IlluminantD65.ill");

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

	float *color1 = new float[2*SAMPLE_NUMBER+1];
	float *color2 = new float[2*SAMPLE_NUMBER+1];

	for (int i = 0; i < 14; i++) {
		for (int j = i+1; j < 15; j++) {
			kDebug() << "----------------------------------------------------" << endl;
			kDebug() << "----------------------------------------------------" << endl;
			cs->fromQColor(colors[i], reinterpret_cast<quint8 *>(color1));
			cs->fromQColor(colors[j], reinterpret_cast<quint8 *>(color2));

			KisRectIteratorPixel it_c1 = pdev->createRectIterator(48*i+0 , 32*j+0 , 16, 16);
			KisRectIteratorPixel it_c2 = pdev->createRectIterator(48*i+0 , 32*j+16, 16, 16);
			KisRectIteratorPixel it_cm = pdev->createRectIterator(48*i+16, 32*j+0 , 32, 32);

			while (!it_c1.isDone()) {
				for (int i = 0; i < 2*SAMPLE_NUMBER+1; i++) {
					reinterpret_cast<float*>(it_c1.rawData())[i] = color1[i];
					reinterpret_cast<float*>(it_c2.rawData())[i] = color2[i];
				}
				++it_c1; ++it_c2;
			}

			while (!it_cm.isDone()) {
				for (int i = 0; i < 2*SAMPLE_NUMBER+1; i++)
					reinterpret_cast<float*>(it_cm.rawData())[i] = 0.5*color1[i] + 0.5*color2[i];
				++it_cm;
			}
// 			{
// 				string s;
// 				cin >> s;
// 			}
		}
	}

	kDebug() << "BOF!" << endl;
	pdev->convertToQImage(0).save("mixing.png");

	delete cs;
	delete profile;
	delete [] color1;
	delete [] color2;*/
}

void KisIlluminantTester::testGLPK()
{/*
	KisIlluminantProfile *profile = new KisIlluminantProfile("IlluminantD65.ill");

	cmsHPROFILE hsRGB, hXYZ;
	cmsHTRANSFORM XYZ_RGB, RGB_XYZ;

	hsRGB = cmsCreate_sRGBProfile();
	hXYZ  = cmsCreateXYZProfile();

	XYZ_RGB = cmsCreateTransform(hXYZ, TYPE_XYZ_DBL, hsRGB, TYPE_RGB_DBL,
								 INTENT_ABSOLUTE_COLORIMETRIC, cmsFLAGS_NOTPRECALC);
	RGB_XYZ = cmsCreateTransform(hsRGB, TYPE_RGB_DBL, hXYZ, TYPE_XYZ_DBL,
								 INTENT_ABSOLUTE_COLORIMETRIC, cmsFLAGS_NOTPRECALC);

	double RGB[] = { 0, 0, 0.5 };
	double XYZ[3];

	cmsDoTransform(RGB_XYZ, RGB, XYZ, 1);

	LPX *lp;

	lp = lpx_create_prob();
	lpx_set_prob_name(lp, "XYZ2REF");
	lpx_set_obj_dir(lp, LPX_MAX);

	lpx_add_rows(lp, 3);

	lpx_set_row_name(lp, 1, "X");
	lpx_set_row_name(lp, 2, "Y");
	lpx_set_row_name(lp, 3, "Z");

	for (int i = 0; i < 3; i++)
		lpx_set_row_bnds(lp, i+1, LPX_FX, XYZ[i], XYZ[i]);

	lpx_add_cols(lp, 10);

	for (int i = 0; i < 10; i++) {
		lpx_set_col_name(lp, i+1, QString("R"+QString::number(i+1)).toAscii().data());
		lpx_set_col_bnds(lp, i+1, LPX_DB, 0.001, 0.999);
		lpx_set_obj_coef(lp, i+1, 1.0);
	}

	int ind[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	for (int i = 0; i < 3; i++) {
		double row[11];
		for (int j = 0; j < 10; j++) {
			if (profile->matrix()(i, j))
				row[j+1] = profile->matrix()(i, j);
			else
				row[j+1] = 1e-11;
		}
		lpx_set_mat_row(lp, i+1, 10, ind, row);
	}
	lpx_simplex(lp);

	vector<float> vXYZ(3), vREF(10);
	for (int i = 0; i < 10; i++)
		vREF[i] = lpx_get_col_prim(lp, i+1);

	for (int i = 0; i < 3; i++) vXYZ[i] = XYZ[i];

	cout << "-------------------------------" << endl;
	cout << "-------------------------------" << endl;
	cout << "REAL XYZ    : " << vXYZ << endl;
	gmm::mult(profile->matrix(), vREF, vXYZ);
	cout << "COMPUTED XYZ: " << vXYZ << endl;
	cout << "-------------------------------" << endl;

	cmsDeleteTransform(RGB_XYZ);
	cmsDeleteTransform(XYZ_RGB);
	cmsCloseProfile(hsRGB);
	cmsCloseProfile(hXYZ);*/
}


QTEST_KDEMAIN(KisIlluminantTester, NoGUI)
#include "illuminant_test.moc"
