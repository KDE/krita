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

#ifndef KIS_REFLECTANCE_COLORSPACE_H_
#define KIS_REFLECTANCE_COLORSPACE_H_

#include "KoColorProfile.h"
#include "KoColorSpaceTraits.h"
#include "KoIncompleteColorSpace.h"

#include "kis_illuminant_profile.h"

using namespace std;

class QString;
class KoColorProfile;
class KoColorSpaceRegistry;

template<typename _channels_type_>
struct KisReflectanceColorSpaceTraits : public KoColorSpaceTrait<_channels_type_, _to_decide_, _to_decide_> {

	struct Cell {
		struct {
			_channels_type_ reflectance;
		} wavelen[_to_decide_];
		_channels_type_ alpha;
	};

};

typedef KisReflectanceColorSpaceTraits<float> ReflectanceTraits;

class KisReflectanceColorSpace : public KoIncompleteColorSpace<ReflectanceTraits, KoRGB16Fallback>
{

	public:

		~KisReflectanceColorSpace()
		{
			cmsDeleteTransform(BGR_XYZ);
			cmsDeleteTransform(XYZ_BGR);
			cmsCloseProfile(hsRGB);
			cmsCloseProfile(hXYZ);
		}

		KisReflectanceColorSpace(KoColorProfile *p);

// 		KisReflectanceColorSpace(const KisReflectanceColorSpace&);
// 		KisReflectanceColorSpace operator=(const KisReflectanceColorSpace&);

	public:

		bool willDegrade(ColorSpaceIndependence) const
		{
			return true;
		}

		KoColorProfile *profile() const { return m_profile; }
		bool profileIsCompatible(KoColorProfile *profile) const;

		void fromRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const;
		void toRgbA16(const quint8 *srcU8, quint8 *dstU8, quint32 nPixels) const;

	private:

		KisIlluminantProfile *m_profile;

		cmsHPROFILE hsRGB, hXYZ;
		cmsHTRANSFORM XYZ_BGR, BGR_XYZ;
};


#endif // KIS_REFLECTANCE_COLORSPACE_H_
