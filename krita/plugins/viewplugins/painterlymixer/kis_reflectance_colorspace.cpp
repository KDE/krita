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

#include "KoColorProfile.h"
#include "KoColorSpaceRegistry.h"

#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"
#include "compositeops/KoCompositeOpMultiply.h"
#include "compositeops/KoCompositeOpDivide.h"
#include "compositeops/KoCompositeOpBurn.h"

#include "kis_illuminant_profile.h"

#include "kis_reflectance_colorspace.h"

KisReflectanceColorSpace::KisReflectanceColorSpace(KoColorProfile *p)
	: KoIncompleteColorSpace<ReflectanceTraits, KoRGB16Fallback>("reflectancecolorspace", "", KoColorSpaceRegistry::instance())
{
	if (profileIsCompatible(p))
		m_profile = dynamic_cast<KisIlluminantProfile*>(p);
	else {
		kDebug() << "(KisReflectanceColorSpace Constructor) Profile is not compatible" << endl;
		return;
	}

	const quint32 ncols = _to_decide_;

	for (quint32 i = 0; i < ncols; i++) {
		addChannel(new KoChannelInfo(i18n("Reflectance"),
				   i * sizeof(float),
				   KoChannelInfo::COLOR,
				   KoChannelInfo::FLOAT32,
				   sizeof(float),
				   QColor(0,255,0)));
	}

	addChannel(new KoChannelInfo(i18n("Alpha"),
			   2 * ncols * sizeof(float),
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
								 INTENT_ABSOLUTE_COLORIMETRIC, cmsFLAGS_NOTPRECALC);
	BGR_XYZ = cmsCreateTransform(hsRGB, TYPE_BGR_16, hXYZ, TYPE_XYZ_DBL,
								 INTENT_ABSOLUTE_COLORIMETRIC, cmsFLAGS_NOTPRECALC);
}

bool KisReflectanceColorSpace::profileIsCompatible(KoColorProfile* profile) const
{
	if (!dynamic_cast<KisIlluminantProfile*>(profile))
		return false;

	return true;
}

void KisReflectanceColorSpace::fromRgbA16(const quint8 * srcU8, quint8 * dstU8, quint32 nPixels) const
{
	const quint32 ncols = _to_decide_;
	const quint16 *src16 = reinterpret_cast<const quint16 *>(srcU8);
	float *dstf = reinterpret_cast<float *>(dstU8);

	double XYZ[3];

	for (quint32 i = 0; i < nPixels; i++) {
		cmsDoTransform(BGR_XYZ, const_cast<quint16*>(src16), XYZ, 1);

		simplex(3, ncols, m_profile->matrix(), dstf, XYZ);

		dstf[ncols] = convert2f(src16[3]);

		dstf += ncols + 1;
		src16 += 4;
	}
}

void KisReflectanceColorSpace::toRgbA16(const quint8 * srcU8, quint8 * dstU8, quint32 nPixels) const
{
	const quint32 ncols = _to_decide_;
	const float *srcf = reinterpret_cast<const float *>(srcU8);
	quint16 *dst16 = reinterpret_cast<quint16 *>(dstU8);

	double XYZ[3];

	for (quint32 i = 0; i < nPixels; i++) {
		maths::mult(3, ncols, m_profile->matrix(), srcf, XYZ);

		cmsDoTransform(XYZ_BGR, XYZ, dst16, 1);

		dst16[3] = convert2i(srcf[ncols]);

		srcf += ncols + 1;
		dst16 += 4;
	}
}
