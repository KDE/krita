/*
 *  Copyright (c) 2008-2009 Cyrille Berger <cberger@cberger.net>
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

#include "KoCtlColorSpaceFactory.h"

#include "KoCtlColorProfile.h"
#include <KoColorSpaceRegistry.h>
#include "KoCtlColorSpaceInfo.h"
#include "KoCtlColorSpace.h"

KoCtlColorSpaceFactory::KoCtlColorSpaceFactory(KoCtlColorSpaceInfo* info) : m_info(info)
{
}

KoCtlColorSpaceFactory::~KoCtlColorSpaceFactory()
{
}

bool KoCtlColorSpaceFactory::profileIsCompatible(const KoColorProfile* profile) const
{
    return KoCtlColorSpace::profileIsCompatible(m_info, profile);
}

QList<KoColorConversionTransformationFactory*> KoCtlColorSpaceFactory::colorConversionLinks() const
{
    QList<const KoColorProfile*> profiles = KoColorSpaceRegistry::instance()->profilesFor(this);
    QList<KoColorConversionTransformationFactory*> list;
    foreach(const KoColorProfile* profile, profiles) {
        const KoCtlColorProfile* ctlprofile = dynamic_cast<const KoCtlColorProfile*>(profile);
        if (ctlprofile) {
            list += ctlprofile->createColorConversionTransformationFactories();
        }
    }
    return list;
}

QString KoCtlColorSpaceFactory::id() const
{
    return m_info->colorSpaceId();
}

QString KoCtlColorSpaceFactory::name() const
{
    return m_info->name();
}

bool KoCtlColorSpaceFactory::userVisible() const
{
    return true;
}

KoID KoCtlColorSpaceFactory::colorModelId() const
{
    return m_info->colorModelId();
}

KoID KoCtlColorSpaceFactory::colorDepthId() const
{
    return m_info->colorDepthId();
}

KoColorSpace *KoCtlColorSpaceFactory::createColorSpace(const KoColorProfile * profile) const
{
    const KoCtlColorProfile* ctlprofile = dynamic_cast<const KoCtlColorProfile*>(profile);
    Q_ASSERT(ctlprofile);
    return new KoCtlColorSpace(m_info, ctlprofile);
}

QString KoCtlColorSpaceFactory::colorSpaceEngine() const
{
    return "";
}

bool KoCtlColorSpaceFactory::isHdr() const
{
    return m_info->isHdr();
}

int KoCtlColorSpaceFactory::referenceDepth() const
{
    return m_info->referenceDepth();
}

QString KoCtlColorSpaceFactory::defaultProfile() const
{
    return m_info->defaultProfile();
}
