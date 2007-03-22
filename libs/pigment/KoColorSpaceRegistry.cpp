/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include "KoColorSpaceRegistry.h"

#include <QStringList>
#include <QDir>

#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kservice.h>
#include <kservicetypetrader.h>

#include <lcms.h>

#include <KoPluginLoader.h>
#include "KoColorSpace.h"
#include "KoColorProfile.h"
#include "colorspaces/KoAlphaColorSpace.h"
#include "colorspaces/KoLabColorSpace.h"
#include "colorspaces/KoRgbU16ColorSpace.h"

struct KoColorSpaceRegistry::Private {
    QMap<QString, KoColorProfile * > profileMap;
    QMap<QString, KoColorSpace * > csMap;
    typedef QList<KisPaintDeviceAction *> PaintActionList;
    QMap<QString, PaintActionList> paintDevActionMap;
    KoColorSpace *alphaCs;
    static KoColorSpaceRegistry *singleton;
};

KoColorSpaceRegistry *KoColorSpaceRegistry::Private::singleton = 0;

KoColorSpaceRegistry* KoColorSpaceRegistry::instance()
{
    if(KoColorSpaceRegistry::Private::singleton == 0)
    {
        KoColorSpaceRegistry::Private::singleton = new KoColorSpaceRegistry();
        KoColorSpaceRegistry::Private::singleton->init();
    }
    return KoColorSpaceRegistry::Private::singleton;
}


void KoColorSpaceRegistry::init()
{
    // prepare a list of the profiles
    KGlobal::mainComponent().dirs()->addResourceType("kis_profiles",
                                                     KStandardDirs::kde_default("data") + "krita/profiles/");

    QStringList profileFilenames;
    profileFilenames += KGlobal::mainComponent().dirs()->findAllResources("kis_profiles", "*.icm");
    profileFilenames += KGlobal::mainComponent().dirs()->findAllResources("kis_profiles", "*.ICM");
    profileFilenames += KGlobal::mainComponent().dirs()->findAllResources("kis_profiles", "*.ICC");
    profileFilenames += KGlobal::mainComponent().dirs()->findAllResources("kis_profiles", "*.icc");

    QDir dir("/usr/share/color/icc/", "*.icc;*.ICC;*.icm;*.ICM");

    QStringList filenames = dir.entryList();

    for (QStringList::iterator it = filenames.begin(); it != filenames.end(); ++it) {
        profileFilenames += dir.absoluteFilePath(*it);
    }

    dir.setPath(QDir::homePath() + "/.color/icc/");
    filenames = dir.entryList();

    for (QStringList::iterator it = filenames.begin(); it != filenames.end(); ++it) {
        profileFilenames += dir.absoluteFilePath(*it);
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
                d->profileMap[profile->productName()] = profile;
            }
        }
    }

    KoColorProfile *labProfile = new KoColorProfile(cmsCreateLabProfile(NULL));
    addProfile(labProfile);
    add(new KoLabColorSpaceFactory());
    KoColorProfile *rgbProfile = new KoColorProfile(cmsCreate_sRGBProfile());
    addProfile(rgbProfile);
    add(new KoRgbU16ColorSpaceFactory());
/* XXX where to put this
    KisHistogramProducerFactoryRegistry::instance()->add(
                new KisBasicHistogramProducerFactory<KisBasicU16HistogramProducer>
                (KoID("LABAHISTO", i18n("L*a*b* Histogram")), new KoLabColorSpace(this, 0);) );
*/
    // Create the default profile for grayscale, probably not the best place to but that, but still better than in a grayscale plugin
    // .22 gamma grayscale or something like that. Taken from the lcms tutorial...
    LPGAMMATABLE Gamma = cmsBuildGamma(256, 2.2); 
    cmsHPROFILE hProfile = cmsCreateGrayProfile(cmsD50_xyY(), Gamma);
    cmsFreeGamma(Gamma);
    KoColorProfile *defProfile = new KoColorProfile(hProfile);
    addProfile(defProfile);

    // Create the built-in colorspaces
    d->alphaCs = new KoAlphaColorSpace(this);

    // Load all colorspace modules
    KService::List offers = KServiceTypeTrader::self()->query(QString::fromLatin1("KOffice/ColorSpace"),
                                                         QString::fromLatin1("(Type == 'Service') and "
                                                                             "([X-Pigment-Version] == 1)"));

    if (offers.empty()) {
        KMessageBox::sorry(0, i18n("Cannot start: No color spaces available. For now you need to install Krita to get colorspaces"));
        abort();
    }

    KoPluginLoader::instance()->load("KOffice/ColorSpace","[X-Pigment-Version] == 1");
}

KoColorSpaceRegistry::KoColorSpaceRegistry() : d(new Private())
{
}

KoColorSpaceRegistry::~KoColorSpaceRegistry()
{
    delete d;
}

KoColorProfile *  KoColorSpaceRegistry::profileByName(const QString & name) const
{
    if (d->profileMap.find(name) == d->profileMap.end()) {
        return 0;
    }

    return d->profileMap[name];
}

QList<KoColorProfile *>  KoColorSpaceRegistry::profilesFor(const QString &id)
{
    return profilesFor(get(id));
}

QList<KoColorProfile *>  KoColorSpaceRegistry::profilesFor(KoColorSpaceFactory * csf)
{
    QList<KoColorProfile *>  profiles;

    QMap<QString, KoColorProfile * >::Iterator it;
    for (it = d->profileMap.begin(); it != d->profileMap.end(); ++it) {
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
          d->profileMap[p->productName()] = p;
      }
}

void KoColorSpaceRegistry::addPaintDeviceAction(KoColorSpace* cs,
        KisPaintDeviceAction* action) {
    d->paintDevActionMap[cs->id()].append(action);
}

QList<KisPaintDeviceAction *>
KoColorSpaceRegistry::paintDeviceActionsFor(KoColorSpace* cs) {
    return d->paintDevActionMap[cs->id()];
}

KoColorSpace * KoColorSpaceRegistry::colorSpace(const QString &csID, const QString &pName)
{
    QString profileName = pName;

    if(profileName.isEmpty())
    {
        KoColorSpaceFactory *csf = get(csID);

        if(!csf)
            return 0;

        profileName = csf->defaultProfile();
    }

    QString name = csID + "<comb>" + profileName;

    if (d->csMap.find(name) == d->csMap.end()) {
        KoColorSpaceFactory *csf = get(csID);
        if(!csf)
        {
            kDebug() << "Unknown color space type : " << csf << endl;
            return 0;
        }

        KoColorProfile *p = profileByName(profileName);
        if(not p and not profileName.isEmpty())
        {
            kDebug() << "Profile not found : " << profileName << endl;
            return 0;
        }
        KoColorSpace *cs = csf->createColorSpace(this, p);
        if(!cs)
            return 0;

        d->csMap[name] = cs;
    }

    if(d->csMap.contains(name))
        return d->csMap[name];
    else
        return 0;
}


KoColorSpace * KoColorSpaceRegistry::colorSpace(const QString &csID, const KoColorProfile *profile)
{
    if( profile )
    {
        KoColorSpace *cs = colorSpace( csID, profile->productName());

        if(!cs)
        {
            // The profile was not stored and thus not the combination either
            KoColorSpaceFactory *csf = get(csID);
            if(!csf)
            {
                kDebug() << "Unknown color space type : " << csf << endl;
                return 0;
            }

            cs = csf->createColorSpace(this, const_cast<KoColorProfile *>(profile));
            if(!cs )
                return 0;

            QString name = csID + "<comb>" + profile->productName();
            d->csMap[name] = cs;
        }

        return cs;
    } else {
        return colorSpace( csID, "");
    }
}

KoColorSpace * KoColorSpaceRegistry::alpha8()
{
   return d->alphaCs;
}

KoColorSpace * KoColorSpaceRegistry::rgb8()
{
    return colorSpace("RGBA", "");
}


KoColorSpace * KoColorSpaceRegistry::lab16()
{
    return colorSpace("LABA", "");
}


KoColorSpace * KoColorSpaceRegistry::rgb16()
{
    return colorSpace("RGBA16", "");
}

#include "KoColorSpaceRegistry.moc"
