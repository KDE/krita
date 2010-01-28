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


#include "LcmsColorSpace.h"
#include "KoColorConversionTransformationFactory.h"
#include "KoColorModelStandardIds.h"

#include "DebugPigment.h"

cmsHPROFILE KoLcmsDefaultTransformations::s_RGBProfile = 0;
QMap< QString, QMap< LcmsColorProfileContainer*, KoLcmsDefaultTransformations* > > KoLcmsDefaultTransformations::s_transformations;


// -- LcmsColorSpaceFactory --
QList<KoColorConversionTransformationFactory*> LcmsColorSpaceFactory::colorConversionLinks() const
{
    return QList<KoColorConversionTransformationFactory*>();
}

KoColorProfile* LcmsColorSpaceFactory::createColorProfile(const QByteArray& rawData) const
{
    return new IccColorProfile(rawData);
}
