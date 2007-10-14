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

#include <lcms.h>

#include <QString>

#include <KLocale>

#include "KoColorSpaceRegistry.h"
#include "KoColorProfile.h"

#include "KoColorSpaceConstants.h"
#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"
#include "compositeops/KoCompositeOpMultiply.h"
#include "compositeops/KoCompositeOpDivide.h"
#include "compositeops/KoCompositeOpBurn.h"

#include "kis_illuminant_profile.h"
#include "mathematics.h"

#include "kis_reflectance_colorspace.h"

const double g_D50[3][10] = {
	{
		2.1863050424163649830392765205377969550681882537901401519775390625e-06,
		0.000256734345183300236188650433888369661872275173664093017578125,
		0.0029878531181237104157044637275930654141120612621307373046875,
		0.037851368923523427689747933300168369896709918975830078125,
		0.08336187614390022648791500614606775343418121337890625,
		0.0026448644551786176966345909278288672794587910175323486328125,
		0.0171249603823020977999558311921646236442029476165771484375,
		0.207622137629802916425347802942269481718540191650390625,
		0.18802117873769585987275831939768977463245391845703125,
		0.40744521770460584431106099145836196839809417724609375
	},
	{
		8.766910037424569719587109060954244199592721997760236263275146484375e-07,
		0.000100355300049154342960887331681618661605170927941799163818359375,
		0.0003121588056475180336830910210466072385315783321857452392578125,
		0.00421740002016027411280152392691888962872326374053955078125,
		0.0245275014220916361129543048491541412658989429473876953125,
		0.203765366996115859787863655583350919187068939208984375,
		0.00666753667924625988139286647538028773851692676544189453125,
		0.412259283121414910056046210229396820068359375,
		0.0802014279897479698266948844320722855627536773681640625,
		0.2679480929745226003291236338554881513118743896484375
	},
	{
		1.0000000000000000540140885956810330905283414542659480243086298659334589074477275146251465374054496583672623295800994620910810169e-128,
		1.0000000000000000540140885956810330905283414542659480243086298659334589074477275146251465374054496583672623295800994620910810169e-128,
		0.01349517161197633097202697172178886830806732177734375,
		0.183368326468981657217938163739745505154132843017578125,
		0.4591169294386017174502967463922686874866485595703125,
		0.1558243642264197370561618072315468452870845794677734375,
		1.0000000000000000540140885956810330905283414542659480243086298659334589074477275146251465374054496583672623295800994620910810169e-128,
		0.002274617402117811749973075308162151486612856388092041015625,
		1.0000000000000000540140885956810330905283414542659480243086298659334589074477275146251465374054496583672623295800994620910810169e-128,
		1.0000000000000000540140885956810330905283414542659480243086298659334589074477275146251465374054496583672623295800994620910810169e-128
	}
};

KisReflectanceColorSpace::KisReflectanceColorSpace(KoColorProfile *p)
	: KoIncompleteColorSpace<ReflectanceTraits, KoRGB16Fallback>("reflectancecolorspace", "", KoColorSpaceRegistry::instance())
{
	if (profileIsCompatible(p))
		m_profile = dynamic_cast<KisIlluminantProfile*>(p);

	const quint32 ncols = WLS_NUMBER;

	for (quint32 i = 0; i < ncols; i+=2) {
		addChannel(new KoChannelInfo(i18n("Reflectance"),
				   i+0 * sizeof(float),
				   KoChannelInfo::COLOR,
				   KoChannelInfo::FLOAT32,
				   sizeof(float),
				   QColor(0,0,255)));
	}

	addChannel(new KoChannelInfo(i18n("Alpha"),
			   ncols * sizeof(float),
			   KoChannelInfo::ALPHA,
			   KoChannelInfo::FLOAT32,
			   sizeof(float)));

	addCompositeOp( new KoCompositeOpOver<ReflectanceTraits>( this ) );
	addCompositeOp( new KoCompositeOpErase<ReflectanceTraits>( this ) );
	addCompositeOp( new KoCompositeOpMultiply<ReflectanceTraits>( this ) );
	addCompositeOp( new KoCompositeOpDivide<ReflectanceTraits>( this ) );
	addCompositeOp( new KoCompositeOpBurn<ReflectanceTraits>( this ) );

	hsRGB = cmsCreate_sRGBProfile();
	hXYZ  = cmsCreateXYZProfile();

	XYZ_BGR = cmsCreateTransform(hXYZ, TYPE_XYZ_DBL, hsRGB, TYPE_BGR_16,
								 INTENT_PERCEPTUAL, cmsFLAGS_NOTPRECALC);
	BGR_XYZ = cmsCreateTransform(hsRGB, TYPE_BGR_16, hXYZ, TYPE_XYZ_DBL,
								 INTENT_PERCEPTUAL, cmsFLAGS_NOTPRECALC);
/*
	m_D50 = new double*[3];
	for (int i = 0; i < 3; i++) {
		m_D50[i] = new double[WLS_NUMBER];
		for (int j = 0; j < WLS_NUMBER; j++)
			m_D50[i][j] = g_D50[i][j];
	}
*/
}

KisReflectanceColorSpace::~KisReflectanceColorSpace()
{
	cmsDeleteTransform(BGR_XYZ);
	cmsDeleteTransform(XYZ_BGR);
	cmsCloseProfile(hsRGB);
	cmsCloseProfile(hXYZ);
/*
	for (int i = 0; i < 3; i++)
		delete [] m_D50[i];
	delete [] m_D50;
*/
}

bool KisReflectanceColorSpace::profileIsCompatible(KoColorProfile* profile) const
{
	if (!dynamic_cast<KisIlluminantProfile*>(profile))
		return false;

	return true;
}

void KisReflectanceColorSpace::fromRgbA16(const quint8 * srcU8, quint8 * dstU8, quint32 nPixels) const
{
    //kDebug() << "fromRgbA16 CALLED!" << endl;
	const quint32 ncols = WLS_NUMBER;
	const quint16 *src16 = reinterpret_cast<const quint16 *>(srcU8);
	float *dstf = reinterpret_cast<float *>(dstU8);

	double XYZ50[3], XYZCUR[3], REF[ncols];

	for (quint32 i = 0; i < nPixels; i++) {
		cmsDoTransform(BGR_XYZ, const_cast<quint16*>(src16), XYZ50, 1);

		maths::mult(3, 3, m_profile->fromD50(), XYZ50, XYZCUR);
		maths::simplex(3, ncols, m_profile->matrix(), REF, XYZCUR);

		for (uint i = 0; i < ncols; i++)
			dstf[i] = REF[i];

		dstf[ncols] = maths::convert2f(src16[3]);

		dstf += ncols + 1;
		src16 += 4;
	}
}

void KisReflectanceColorSpace::toRgbA16(const quint8 * srcU8, quint8 * dstU8, quint32 nPixels) const
{
	const quint32 ncols = WLS_NUMBER;
	const float *srcf = reinterpret_cast<const float *>(srcU8);
	quint16 *dst16 = reinterpret_cast<quint16 *>(dstU8);

	double XYZ50[3], XYZCUR[3], REF[ncols];

	for (quint32 i = 0; i < nPixels; i++) {
		for (uint i = 0; i < ncols; i++)
			REF[i] = srcf[i];

		maths::mult(3, ncols, m_profile->matrix(), REF, XYZCUR);
		maths::mult(3, 3, m_profile->toD50(), XYZCUR, XYZ50);

		cmsDoTransform(XYZ_BGR, XYZ50, dst16, 1);

		dst16[3] = maths::convert2i(srcf[ncols]);

		srcf += ncols + 1;
		dst16 += 4;
	}
}
