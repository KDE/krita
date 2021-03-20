/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2005-2006 C. Boemann <cbo@boemann.dk>
 *  SPDX-FileCopyrightText: 2004, 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "LcmsColorSpace.h"
#include "KoColorConversionTransformationFactory.h"
#include "KoColorModelStandardIds.h"

#include "QDebug"

cmsHPROFILE KoLcmsDefaultTransformations::s_RGBProfile = 0;
QMap< QString, QMap< LcmsColorProfileContainer *, KoLcmsDefaultTransformations * > > KoLcmsDefaultTransformations::s_transformations;

// -- LcmsColorSpaceFactory --
QList<KoColorConversionTransformationFactory *> LcmsColorSpaceFactory::colorConversionLinks() const
{
    return QList<KoColorConversionTransformationFactory *>();
}

KoColorProfile *LcmsColorSpaceFactory::createColorProfile(const QByteArray &rawData) const
{
    return new IccColorProfile(rawData);
}
