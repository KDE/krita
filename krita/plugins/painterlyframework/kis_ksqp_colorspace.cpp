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

#include "kis_ksqp_colorspace.h"

#include "kis_rgb_to_ksqp_color_conversion_transformation.h"
#include "kis_ks_to_rgb_color_conversion_transformation.h"

#include <QList>

QList<KoColorConversionTransformationFactory*> KisKSQPColorSpaceFactory::colorConversionLinks() const
{
    QList<KoColorConversionTransformationFactory*> list;

    // RGB to KS Linear
    list.append(new KisRGBToKSQPColorConversionTransformationFactory);
    // KS Linear to RGB
    list.append(new KisKSToRGBColorConversionTransformationFactory<9>("KS9QP"));

    return list;
}
