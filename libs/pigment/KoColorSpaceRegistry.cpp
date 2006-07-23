/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <QStringList>
#include <QDir>

#include <kinstance.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kparts/plugin.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kparts/componentfactory.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <lcms.h>

#include "KoColorSpace.h"
#include "KoColorProfile.h"
#include "KoColorSpaceRegistry.h"
#include "colorspaces/KoAlphaColorSpace.h"
#include "colorspaces/KoLabColorSpace.h"

KoColorSpaceRegistry *KoColorSpaceRegistry::m_singleton = 0;

KoColorSpaceRegistry* KoColorSpaceRegistry::instance()
{
    if(KoColorSpaceRegistry::m_singleton == 0)
    {
        KoColorSpaceRegistry::m_singleton = new KoColorSpaceRegistry();
        KoColorSpaceRegistry::m_singleton->init();
    }
    return KoColorSpaceRegistry::m_singleton;
}


void KoColorSpaceRegistry::init()
{
    // prepare a list of the profiles
    KGlobal::instance()->dirs()->addResourceType("kis_profiles",
                                                     KStandardDirs::kde_default("data") + "krita/profiles/");

    QStringList profileFilenames;
    profileFilenames += KGlobal::instance()->dirs()->findAllResources("kis_profiles", "*.icm");
    profileFilenames += KGlobal::instance()->dirs()->findAllResources("kis_profiles", "*.ICM");
    profileFilenames += KGlobal::instance()->dirs()->findAllResources("kis_profiles", "*.ICC");
    profileFilenames += KGlobal::instance()->dirs()->findAllResources("kis_profiles", "*.icc");

    QDir d("/usr/share/color/icc/", "*.icc;*.ICC;*.icm;*.ICM");

    QStringList filenames = d.entryList();

    for (QStringList::iterator it = filenames.begin(); it != filenames.end(); ++it) {
        profileFilenames += d.absoluteFilePath(*it);
    }

    d.setPath(QDir::homePath() + "/.color/icc/");
    filenames = d.entryList();

    for (QStringList::iterator it = filenames.begin(); it != filenames.end(); ++it) {
        profileFilenames += d.absoluteFilePath(*it);
    }

    // Set lcms to return NUll/false etc from failing calls, rather than aborting the app.
    cmsErrorAction(LCMS_ERROR_SHOW);

    // Load the profiles
    if (!profileFilenames.empty()) {
        KoColorProfile * profile = 0;
        for ( QStringList::Iterator it = profileFilenames.begin(); it != profileFilenames.end(); ++it ) {
            profile = new KoColorProfile(*it);
            Q_CHECK_PTR(profile);

            profile->load();
            if (profile->valid()) {
                m_profileMap[profile->productName()] = profile;
            }
        }
    }

    KoColorProfile *labProfile = new KoColorProfile(cmsCreateLabProfile(NULL));
    addProfile(labProfile);
    add(new KoLabColorSpaceFactory());
/* XXX where to put this
    KisHistogramProducerFactoryRegistry::instance()->add(
                new KisBasicHistogramProducerFactory<KisBasicU16HistogramProducer>
                (KoID("LABAHISTO", i18n("L*a*b* Histogram")), new KoLabColorSpace(this, 0);) );
*/

    // Create the built-in colorspaces
    m_alphaCs = new KoAlphaColorSpace(this, 0);

    // Load all colorspace modules
     KService::List offers = KServiceTypeTrader::self()->query(QString::fromLatin1("KOffice/ColorSpace"),
                                                         QString::fromLatin1("(Type == 'Service') and "
                                                                             "([X-Pigment-Version] == 1)"));

    if (offers.empty()) {
        KMessageBox::sorry(0, i18n("Cannot start: No color spaces available. For now you need to install Krita to get colorspaces"));
        abort();
    }

     KService::List::ConstIterator iter;
    for(iter = offers.begin(); iter != offers.end(); ++iter)
    {
        KService::Ptr service = *iter;
        int errCode = 0;
        KParts::Plugin* plugin =
             KService::createInstance<KParts::Plugin> ( service, this, QStringList(), &errCode);
        if ( plugin )
            kDebug(41006) << "found colorspace " << service->property("Name").toString() << "\n";
        else {
            kDebug(41006) << "found plugin " << service->property("Name").toString() << ", " << errCode << "\n";
            if( errCode == KLibLoader::ErrNoLibrary)
            {
                kWarning(41006) << " Error loading plugin was : ErrNoLibrary " << KLibLoader::self()->lastErrorMessage() << endl;
            }
        }
    }
}

KoColorSpaceRegistry::KoColorSpaceRegistry()
{
}

KoColorSpaceRegistry::~KoColorSpaceRegistry()
{
}

KoColorProfile *  KoColorSpaceRegistry::getProfileByName(const QString & name)
{
    if (m_profileMap.find(name) == m_profileMap.end()) {
        return 0;
    }

    return m_profileMap[name];
}

QList<KoColorProfile *>  KoColorSpaceRegistry::profilesFor(KoID id)
{
    return profilesFor(get(id));
}

QList<KoColorProfile *>  KoColorSpaceRegistry::profilesFor(KoColorSpaceFactory * csf)
{
    QList<KoColorProfile *>  profiles;

    QMap<QString, KoColorProfile * >::Iterator it;
    for (it = m_profileMap.begin(); it != m_profileMap.end(); ++it) {
        KoColorProfile *  profile = it.value();
        if (profile->colorSpaceSignature() == csf->colorSpaceSignature()) {
            profiles.push_back(profile);
        }
    }
    return profiles;
}

void KoColorSpaceRegistry::addProfile(KoColorProfile *p)
{
      if (p->valid()) {
          m_profileMap[p->productName()] = p;
      }
}

void KoColorSpaceRegistry::addPaintDeviceAction(KoColorSpace* cs,
        KisPaintDeviceAction* action) {
    m_paintDevActionMap[cs->id()].append(action);
}

QList<KisPaintDeviceAction *>
KoColorSpaceRegistry::paintDeviceActionsFor(KoColorSpace* cs) {
    return m_paintDevActionMap[cs->id()];
}

KoColorSpace * KoColorSpaceRegistry::getColorSpace(const KoID & csID, const QString & pName)
{
    QString profileName = pName;

    if(profileName.isEmpty())
    {
        KoColorSpaceFactory *csf = get(csID);

        if(!csf)
            return 0;

        profileName = csf->defaultProfile();
    }

    QString name = csID.id() + "<comb>" + profileName;

    if (m_csMap.find(name) == m_csMap.end()) {
        KoColorSpaceFactory *csf = get(csID);
        if(!csf)
            return 0;

        KoColorProfile *p = getProfileByName(profileName);
        if(!p && profileName != "")
            return 0;
        KoColorSpace *cs = csf->createColorSpace(this, p);
        if(!cs)
            return 0;

        m_csMap[name] = cs;
    }

    if(m_csMap.contains(name))
        return m_csMap[name];
    else
        return 0;
}


KoColorSpace * KoColorSpaceRegistry::getColorSpace(const KoID & csID, const KoColorProfile * profile)
{
    if( profile )
    {
        KoColorSpace *cs = getColorSpace( csID, profile->productName());

        if(!cs)
        {
            // The profile was not stored and thus not the combination either
            KoColorSpaceFactory *csf = get(csID);
            if(!csf)
                return 0;

            cs = csf->createColorSpace(this, const_cast<KoColorProfile *>(profile));
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

KoColorSpace * KoColorSpaceRegistry::getAlpha8()
{
   return m_alphaCs;
}

KoColorSpace * KoColorSpaceRegistry::getRGB8()
{
    return getColorSpace(KoID("RGBA", ""), "");
}

#include "KoColorSpaceRegistry.moc"
