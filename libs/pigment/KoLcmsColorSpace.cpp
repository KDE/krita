/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2005-2006 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2004,2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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


#include "KoLcmsColorSpace.h"

#include "KoColorConversionLink.h"

cmsHTRANSFORM KoLcmsColorConversionTransformation::createTransform(
        KoLcmsColorProfile *  srcProfile,
        KoLcmsColorProfile *  dstProfile,
        qint32 renderingIntent) const
{
    KConfigGroup cfg = KGlobal::config()->group("");
    bool bpCompensation = cfg.readEntry("useBlackPointCompensation", false);

    int flags = 0;

    if (bpCompensation) {
        flags = cmsFLAGS_BLACKPOINTCOMPENSATION;
    }
    Q_ASSERT(dynamic_cast<const KoLcmsInfo*>(srcColorSpace()));
    Q_ASSERT(dynamic_cast<const KoLcmsInfo*>(dstColorSpace()));
    cmsHTRANSFORM tf = cmsCreateTransform(srcProfile->lcmsProfile(),
            dynamic_cast<const KoLcmsInfo*>(srcColorSpace())->colorSpaceType(),
            dstProfile->lcmsProfile(),
            dynamic_cast<const KoLcmsInfo*>(dstColorSpace())->colorSpaceType(),
            renderingIntent,
            flags);

    return tf;
}

QList<KoColorConversionLink> KoLcmsColorSpaceFactory::colorConversionLinks() const
{
    return QList<KoColorConversionLink>();
}

