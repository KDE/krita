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

#include "kis_rgb_f16_hdr_colorspace.h"

#include "colorprofiles/KoCtlColorProfile.h"

#include <KoColorConversionTransformationFactory.h>
#include <KoCtlColorConversionTransformation.h>

KisRgbF16HDRColorSpace::KisRgbF16HDRColorSpace(const KoCtlColorProfile *p)
        : KisRgbFloatHDRColorSpace<RgbF16Traits>(colorSpaceId(), i18n("RGB (16-bit float/channel) for High Dynamic Range imaging"),  p)
{
}

QString KisRgbF16HDRColorSpace::colorSpaceId()
{
    return QString("RgbAF16");
}

KoColorSpace* KisRgbF16HDRColorSpace::clone() const
{
    return new KisRgbF16HDRColorSpace(static_cast<const KoCtlColorProfile*>(profile()));
}

KoColorSpace* KisRgbF16HDRColorSpaceFactory::createColorSpace(const KoColorProfile * p) const
{
    return new KisRgbF16HDRColorSpace(dynamic_cast<const KoCtlColorProfile*>(p));
}

bool KisRgbF16HDRColorSpaceFactory::profileIsCompatible(const KoColorProfile* profile) const
{

    const KoCtlColorProfile* ctlp = dynamic_cast<const KoCtlColorProfile*>(profile);
    if (ctlp && ctlp->colorModel() == "RGBA") {
        return true;
    }
    return false;
}
