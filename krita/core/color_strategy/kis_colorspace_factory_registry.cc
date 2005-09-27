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
#include "kis_profile.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_xyz_colorspace.h"
#include "kis_alpha_colorspace.h"

KisColorSpaceFactoryRegistry *KisColorSpaceFactoryRegistry::m_singleton = 0;

KisColorSpaceFactoryRegistry::KisColorSpaceFactoryRegistry()
{
    Q_ASSERT(KisColorSpaceFactoryRegistry::m_singleton == 0);
    KisColorSpaceFactoryRegistry::m_singleton = this;
}

KisColorSpaceFactoryRegistry::~KisColorSpaceFactoryRegistry()
{
}

KisColorSpaceFactoryRegistry* KisColorSpaceFactoryRegistry::instance()
{
    if(KisColorSpaceFactoryRegistry::m_singleton == 0)
    {
        KisColorSpaceFactoryRegistry::m_singleton = new KisColorSpaceFactoryRegistry();
        Q_CHECK_PTR(KisColorSpaceFactoryRegistry::m_singleton);
        m_singleton->resetProfiles();

        KisColorSpaceFactoryRegistry::m_singleton->m_xyzCs = new KisXyzColorSpace(0);
//        m_singleton->m_csMap[cs->id().id()] = cs; //XXX should this be user visible
        KisColorSpaceFactoryRegistry::m_singleton->m_alphaCs = new KisAlphaColorSpace(0);
    }
    return KisColorSpaceFactoryRegistry::m_singleton;
}

KisProfile *  KisColorSpaceFactoryRegistry::getProfileByName(const QString & name)
{
    if (m_profileMap.find(name) == m_profileMap.end()) {
        return 0;
    }

    return m_profileMap[name];
}

QValueVector<KisProfile *>  KisColorSpaceFactoryRegistry::profilesFor(KisID id)
{
    return profilesFor(get(id));
}

QValueVector<KisProfile *>  KisColorSpaceFactoryRegistry::profilesFor(KisColorSpaceFactory * csf)
{
    QValueVector<KisProfile *>  profiles;

    QMap<QString, KisProfile * >::Iterator it;
    for (it = m_profileMap.begin(); it != m_profileMap.end(); ++it) {
        KisProfile *  profile = it.data();
        if (profile->colorSpaceSignature() == csf->colorSpaceSignature()) {
            profiles.push_back(profile);
        }
    }
    return profiles;
}

void KisColorSpaceFactoryRegistry::resetProfiles()
{
    // XXX: Should find a way to make sure not all profiles are read for all color strategies

    QStringList profileFilenames;
    profileFilenames += KisFactory::instance() -> dirs() -> findAllResources("kis_profiles", "*.icm");
    profileFilenames += KisFactory::instance() -> dirs() -> findAllResources("kis_profiles", "*.ICM");
    profileFilenames += KisFactory::instance() -> dirs() -> findAllResources("kis_profiles", "*.ICC");
    profileFilenames += KisFactory::instance() -> dirs() -> findAllResources("kis_profiles", "*.icc");

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

KisColorSpace *  KisColorSpaceFactoryRegistry::getColorSpace(const KisID & csID,
const QString & pName)
{
    QString profileName = pName;

    if(profileName == "")
    {
        KisColorSpaceFactory *csf = get(csID);

        if(!csf)
            return 0;

        profileName = csf->defaultProfile();
    }

    QString name = csID.id() + "<comb>" + profileName;

    if (m_csMap.find(name) == m_csMap.end()) {
        KisColorSpaceFactory *csf = get(csID);
        if(!csf)
            return 0;

        KisProfile *p = getProfileByName(profileName);

        KisColorSpace *cs = csf -> createColorSpace(p);
        if(!cs)
            return 0;

        m_csMap[name] = cs;
    }

    return m_csMap[name];
}

KisColorSpace * KisColorSpaceFactoryRegistry::getXYZ16()
{
   return KisColorSpaceFactoryRegistry::instance()->m_xyzCs;
}

KisColorSpace * KisColorSpaceFactoryRegistry::getAlpha8()
{
   return KisColorSpaceFactoryRegistry::instance()->m_alphaCs;
}
