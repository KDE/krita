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

#include <QString>

#include <klocale.h>

#include "KoColorSpaceRegistry.h"
#include "KoColorSpaceConstants.h"
#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"
#include "compositeops/KoCompositeOpMultiply.h"
#include "compositeops/KoCompositeOpDivide.h"
#include "compositeops/KoCompositeOpBurn.h"

#include "mathematics.h"

#include "kis_rgbks_colorspace.h"

KisRGBKSColorSpace::KisRGBKSColorSpace()
	: KoIncompleteColorSpace<RGBKSTraits, KoRGB16Fallback>("rgbkscolorspace", "RGB/KS ColorSpace (Float)", KoColorSpaceRegistry::instance())
{
	for (quint32 i = 0; i < 6; i+=2) {
		addChannel(new KoChannelInfo(i18n("Absorption"),
				   i+0 * sizeof(float),
				   KoChannelInfo::COLOR,
				   KoChannelInfo::FLOAT32,
				   sizeof(float),
				   QColor(0,0,255)));

		addChannel(new KoChannelInfo(i18n("Scattering"),
				   i+1 * sizeof(float),
				   KoChannelInfo::COLOR,
				   KoChannelInfo::FLOAT32,
				   sizeof(float),
				   QColor(255,0,0)));
	}

	addChannel(new KoChannelInfo(i18n("Alpha"),
			   6 * sizeof(float),
			   KoChannelInfo::ALPHA,
			   KoChannelInfo::FLOAT32,
			   sizeof(float)));

	addCompositeOp( new KoCompositeOpOver<RGBKSTraits>( this ) );
	addCompositeOp( new KoCompositeOpErase<RGBKSTraits>( this ) );
	addCompositeOp( new KoCompositeOpMultiply<RGBKSTraits>( this ) );
	addCompositeOp( new KoCompositeOpDivide<RGBKSTraits>( this ) );
	addCompositeOp( new KoCompositeOpBurn<RGBKSTraits>( this ) );
}


void KisRGBKSColorSpace::fromRgbA16(const quint8 * srcU8, quint8 * dstU8, quint32 nPixels) const
{
	kDebug() << "CALLED!" << endl;
	const quint16 *src16 = reinterpret_cast<const quint16 *>(srcU8);
	float *dstf = RGBKSTraits::nativeArray(dstU8);

	double REF[3];

	for (quint32 i = 0; i < nPixels; i++) {
		for (quint32 j = 0; j < 3; j++) REF[j] = maths::convert2f(src16[j]);

		maths::computeKS(3, REF, dstf);

		dstf[6] = maths::convert2f(src16[3]);

		dstf += 7;
		src16 += 4;
	}
}


void KisRGBKSColorSpace::toRgbA16(const quint8 * srcU8, quint8 * dstU8, quint32 nPixels) const
{
	const float *srcf = RGBKSTraits::nativeArray(srcU8);
	quint16 *dst16 = reinterpret_cast<quint16 *>(dstU8);

	double REF[3];

	for (quint32 i = 0; i < nPixels; i++) {
		maths::computeReflectance(3, srcf, REF);

		for (quint32 j = 0; j < 3; j++) dst16[j] = maths::convert2i(REF[j]);

		dst16[3] = maths::convert2i(srcf[6]);

		srcf += 7;
		dst16 += 4;
	}
}
