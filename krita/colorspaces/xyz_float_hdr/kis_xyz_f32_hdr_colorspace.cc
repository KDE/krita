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

#include <KoColorConversionTransformationFactory.h>
#include <KoScaleColorConversionTransformation.h>
#include "kis_xyz_to_lab_color_conversion_transformation.h"
#include "kis_lab_to_xyz_color_conversion_transformation.h"

KisXyzF32HDRColorSpace::KisXyzF32HDRColorSpace(KoColorSpaceRegistry * parent, KoColorProfile *p)
: KisXyzFloatHDRColorSpace<XyzF32Traits>(colorSpaceId(), i18n("XYZ (32-bit float/channel) for High Dynamic Range imaging"), parent, p)
{
}

QString KisXyzF32HDRColorSpace::colorSpaceId()
{
    return QString("XyzAF32");
}

QList<KoColorConversionTransformationFactory*> KisXyzF32HDRColorSpaceFactory::colorConversionLinks() const
{
    QList<KoColorConversionTransformationFactory*> list;
    // Conversion to XYZ Float 32bit
    list.append(new KoScaleColorConversionTransformationFactory< KoXyzTraits<quint16>, XyzF32Traits >( XYZAColorModelID.id(), Integer16BitsColorDepthID.id(), Float32BitsColorDepthID.id() ) );
    list.append(new KisLabToXyzColorConversionTransformationFactory<XyzF32Traits>(Float32BitsColorDepthID.id()));
    // Conversion from XYZ Float 32bit
    list.append(new KoScaleColorConversionTransformationFactory< XyzF32Traits, KoXyzTraits<quint16> >( XYZAColorModelID.id(), Float32BitsColorDepthID.id(), Integer16BitsColorDepthID.id() ) );
    list.append(new KisXyzToLabColorConversionTransformationFactory<XyzF32Traits>(Float32BitsColorDepthID.id()));
    
    return list;
}
