/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include <qdir.h>

#include "kdebug.h"
#include <kstandarddirs.h>
#include <kinstance.h>

#include "kis_factory.h"
#include "kis_types.h"
#include "kis_colorspace.h"
#include "kis_xyz_colorspace.h"
#include "kis_alpha_colorspace.h"
#include "kis_colorspace_registry.h"
#include "kis_pixel_op.h"

KisColorSpaceRegistry *KisColorSpaceRegistry::m_singleton = 0;
KisColorSpace * KisColorSpaceRegistry::m_rgb = 0;
KisColorSpace * KisColorSpaceRegistry::m_alpha = 0;
KisColorSpace * KisColorSpaceRegistry::m_xyz = 0;

KisColorSpaceRegistry::KisColorSpaceRegistry()
{
    Q_ASSERT(KisColorSpaceRegistry::m_singleton == 0);
    KisColorSpaceRegistry::m_singleton = this;
}

KisColorSpaceRegistry::~KisColorSpaceRegistry()
{
}

KisColorSpaceRegistry* KisColorSpaceRegistry::instance()
{
    if(KisColorSpaceRegistry::m_singleton == 0)
    {
        KisColorSpaceRegistry::m_singleton = new KisColorSpaceRegistry();
        Q_CHECK_PTR(KisColorSpaceRegistry::m_singleton);
        if ( !m_xyz ) {
            m_xyz = new KisXyzColorSpace();
            m_singleton->add(m_xyz);
        }

        if ( !m_alpha ) {
            m_alpha = new KisAlphaColorSpace();
        }

        m_singleton->resetProfiles();
    }
    return KisColorSpaceRegistry::m_singleton;
}


KisColorSpace * KisColorSpaceRegistry::getRGB8()
{
    if ( m_rgb == 0 ) {
        m_rgb = m_singleton->get( "RGBA" );
    }
    return m_rgb;
}

KisColorSpace * KisColorSpaceRegistry::getAlpha8()
{
    if ( m_alpha == 0 ) {
        m_alpha = new KisAlphaColorSpace();
    }
    return m_alpha;
}

KisColorSpace * KisColorSpaceRegistry::getXYZ16()
{
    if ( m_xyz == 0 ) {
        m_xyz = new KisXyzColorSpace();
        m_singleton->add( m_xyz );
    }
    return m_xyz;
}


KisProfile *  KisColorSpaceRegistry::getProfileByName(const QString & name)
{
    if (m_profileMap.find(name) == m_profileMap.end()) {
        return 0;
    }

    return m_profileMap[name];
}

QValueVector<KisProfile *>  KisColorSpaceRegistry::profilesFor(KisColorSpace * cs)
{
    QValueVector<KisProfile *>  profiles;

    QMap<QString, KisProfile * >::Iterator it;
    for (it = m_profileMap.begin(); it != m_profileMap.end(); ++it) {
        KisProfile *  profile = it.data();
        if (profile->colorSpaceSignature() == cs->colorSpaceSignature()) {
            profile->setColorType(cs->colorSpaceType());
            profiles.push_back(profile);
        }
    }
    return profiles;
}

void KisColorSpaceRegistry::resetProfiles()
{
    // XXX: Should find a way to make sure not all profiles are read for all color strategies

    QStringList profileFilenames;
    profileFilenames += KisFactory::global() -> dirs() -> findAllResources("kis_profiles", "*.icm");
    profileFilenames += KisFactory::global() -> dirs() -> findAllResources("kis_profiles", "*.ICM");
    profileFilenames += KisFactory::global() -> dirs() -> findAllResources("kis_profiles", "*.ICC");
    profileFilenames += KisFactory::global() -> dirs() -> findAllResources("kis_profiles", "*.icc");

    QDir d("/usr/share/color/icc/", "*.icc");
    profileFilenames += d.entryList();

    d.setCurrent("/usr/share/color/icc/");
    d.setNameFilter("*.icm");

    profileFilenames += d.entryList();

    if (!profileFilenames.empty()) {
        KisProfile * profile = 0;
        for ( QStringList::Iterator it = profileFilenames.begin(); it != profileFilenames.end(); ++it ) {

            profile = new KisProfile(*it);
            Q_CHECK_PTR(profile);

            profile -> load();
            if (profile -> valid()) {
                m_profileMap[profile->productName()] = profile;
            }
        }
    }

}

void KisColorSpaceRegistry::addFallbackPixelOp(KisPixelOp * pixelop)
{
    if (!pixelop) {
        return;
    }

    if (!pixelop->isValid()) {
        kdDebug() << "Cannot add invalid pixel operation " << pixelop->id().id() << "\n";
        return;
    }

    m_defaultPixelOps[pixelop->id()] = pixelop;


}

KisPixelOp * KisColorSpaceRegistry::getFallbackPixelOp(KisID pixelop)
{
    if (m_defaultPixelOps.find(pixelop) != m_defaultPixelOps.end()) {
        return m_defaultPixelOps[pixelop];
    }

    return 0;
}

