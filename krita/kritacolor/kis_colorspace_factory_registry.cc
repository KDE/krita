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

#include <kdebug.h>
#include <kparts/plugin.h>
#include <kservice.h>
#include <ktrader.h>
#include <kparts/componentfactory.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "kis_debug_areas.h"
#include "kis_colorspace.h"
#include "kis_profile.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_alpha_colorspace.h"
#include "kis_lab_colorspace.h"


KisColorSpaceFactoryRegistry::KisColorSpaceFactoryRegistry(QStringList profileFilenames)
{
    // Create the built-in colorspaces

    m_alphaCs = new KisAlphaColorSpace(this, 0);

    // Load the profiles
    if (!profileFilenames.empty()) {
        KisProfile * profile = 0;
        for ( QStringList::Iterator it = profileFilenames.begin(); it != profileFilenames.end(); ++it ) {
            profile = new KisProfile(*it);
            Q_CHECK_PTR(profile);

            profile->load();
            if (profile->valid()) {
                m_profileMap[profile->productName()] = profile;
            }
        }
    }

    KisProfile *labProfile = new KisProfile(cmsCreateLabProfile(NULL));
    addProfile(labProfile);
    add(new KisLabColorSpaceFactory());
/* XXX where to put this
    KisHistogramProducerFactoryRegistry::instance()->add(
                new KisBasicHistogramProducerFactory<KisBasicU16HistogramProducer>
                (KisID("LABAHISTO", i18n("L*a*b* Histogram")), new KisLabColorSpace(this, 0);) );
*/

    // Load all colorspace modules
    KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("Krita/ColorSpace"),
                                                         QString::fromLatin1("(Type == 'Service') and "
                                                                             "([X-Krita-Version] == 2)"));

    if (offers.empty()) {
        KMessageBox::sorry(0, i18n("Cannot start Krita: no colorspaces available."));
        abort();
    }

    KTrader::OfferList::ConstIterator iter;
    for(iter = offers.begin(); iter != offers.end(); ++iter)
    {
        KService::Ptr service = *iter;
        int errCode = 0;
        KParts::Plugin* plugin =
             KParts::ComponentFactory::createInstanceFromService<KParts::Plugin> ( service, this, QStringList(), &errCode);
        if ( plugin )
            kDebug(DBG_AREA_PLUGINS) << "found colorspace " << service->property("Name").toString() << "\n";
        else {
            kDebug(41006) << "found plugin " << service->property("Name").toString() << ", " << errCode << "\n";
            if( errCode == KParts::ComponentFactory::ErrNoLibrary)
            {
                kWarning(41006) << " Error loading plugin was : ErrNoLibrary " << KLibLoader::self()->lastErrorMessage() << endl;
            }
        }
    }
}

KisColorSpaceFactoryRegistry::KisColorSpaceFactoryRegistry()
{
}

KisColorSpaceFactoryRegistry::~KisColorSpaceFactoryRegistry()
{
}

KisProfile *  KisColorSpaceFactoryRegistry::getProfileByName(const QString & name)
{
    if (m_profileMap.find(name) == m_profileMap.end()) {
        return 0;
    }

    return m_profileMap[name];
}

QList<KisProfile *>  KisColorSpaceFactoryRegistry::profilesFor(KisID id)
{
    return profilesFor(get(id));
}

QList<KisProfile *>  KisColorSpaceFactoryRegistry::profilesFor(KisColorSpaceFactory * csf)
{
    QList<KisProfile *>  profiles;

    QMap<QString, KisProfile * >::Iterator it;
    for (it = m_profileMap.begin(); it != m_profileMap.end(); ++it) {
        KisProfile *  profile = it.value();
        if (profile->colorSpaceSignature() == csf->colorSpaceSignature()) {
            profiles.push_back(profile);
        }
    }
    return profiles;
}

void KisColorSpaceFactoryRegistry::addProfile(KisProfile *p)
{
      if (p->valid()) {
          m_profileMap[p->productName()] = p;
      }
}

void KisColorSpaceFactoryRegistry::addPaintDeviceAction(KisColorSpace* cs,
        KisPaintDeviceAction* action) {
    m_paintDevActionMap[cs->id()].append(action);
}

QList<KisPaintDeviceAction *>
KisColorSpaceFactoryRegistry::paintDeviceActionsFor(KisColorSpace* cs) {
    return m_paintDevActionMap[cs->id()];
}

KisColorSpace * KisColorSpaceFactoryRegistry::getColorSpace(const KisID & csID, const QString & pName)
{
    QString profileName = pName;

    if(profileName.isEmpty())
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
        if(!p && profileName != "")
            return 0;
        KisColorSpace *cs = csf->createColorSpace(this, p);
        if(!cs)
            return 0;

        m_csMap[name] = cs;
    }

    if(m_csMap.contains(name))
        return m_csMap[name];
    else
        return 0;
}


KisColorSpace * KisColorSpaceFactoryRegistry::getColorSpace(const KisID & csID, const KisProfile * profile)
{
    if( profile )
    {
        KisColorSpace *cs = getColorSpace( csID, profile->productName());

        if(!cs)
        {
            // The profile was not stored and thus not the combination either
            KisColorSpaceFactory *csf = get(csID);
            if(!csf)
                return 0;

            cs = csf->createColorSpace(this, const_cast<KisProfile *>(profile));
            if(!cs )
                return 0;

            QString name = csID.id() + "<comb>" + profile->productName();
            m_csMap[name] = cs;
        }

        return cs;
    } else {
        return getColorSpace( csID, "");
    }
}

KisColorSpace * KisColorSpaceFactoryRegistry::getAlpha8()
{
   return m_alphaCs;
}

KisColorSpace * KisColorSpaceFactoryRegistry::getRGB8()
{
    return getColorSpace(KisID("RGBA", ""), "");
}

#include "kis_colorspace_factory_registry.moc"
