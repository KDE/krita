/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "kis_xyz_f32_hdr_colorspace.h"

#include "colorprofiles/KoCtlColorProfile.h"

#include <KoColorConversionTransformationFactory.h>
#include <KoCtlColorConversionTransformation.h>

KisXyzF32HDRColorSpace::KisXyzF32HDRColorSpace( const KoCtlColorProfile *p)
: KisXyzFloatHDRColorSpace<XyzF32Traits>(colorSpaceId(), i18n("XYZ (32-bit float/channel) for High Dynamic Range imaging"),  p)
{
}

QString KisXyzF32HDRColorSpace::colorSpaceId()
{
    return QString("XyzAF32");
}

KoColorSpace* KisXyzF32HDRColorSpace::clone() const
{
    return new KisXyzF32HDRColorSpace( static_cast<const KoCtlColorProfile*>(profile()) );
}

KoColorSpace* KisXyzF32HDRColorSpaceFactory::createColorSpace( const KoColorProfile * p) const {
    return new KisXyzF32HDRColorSpace( dynamic_cast<const KoCtlColorProfile*>(p) );
}

bool KisXyzF32HDRColorSpaceFactory::profileIsCompatible(const KoColorProfile* profile) const
{

    const KoCtlColorProfile* ctlp = dynamic_cast<const KoCtlColorProfile*>(profile);
    if(ctlp && ctlp->colorModel() == "XYZA" && ctlp->colorDepth() == "F32")
    {
        return true;
    }
    return false;
}
