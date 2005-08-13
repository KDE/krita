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
#include "kis_xyz_colorspace.h"
#include "kis_colorspace_registry.h"

KisColorSpaceRegistry *KisColorSpaceRegistry::m_singleton = 0;

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
        m_singleton->add(new KisXyzColorSpace());
        m_singleton->resetProfiles();
    }
    return KisColorSpaceRegistry::m_singleton;
}

KisProfileSP KisColorSpaceRegistry::getProfileByName(const QString & name)
{
    if (m_profileMap.find(name) == m_profileMap.end()) {
        return 0;
    }

    return m_profileMap[name];
}

vKisProfileSP KisColorSpaceRegistry::profilesFor(KisAbstractColorSpace * cs)
{
    vKisProfileSP profiles;
    
    QMap<QString, KisProfileSP>::Iterator it;
    for (it = m_profileMap.begin(); it != m_profileMap.end(); ++it) {
        KisProfileSP profile = it.data();
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
