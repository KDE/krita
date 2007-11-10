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

#include "KoPluginLoader.h"
#include "KoColorSpace.h"
#include "KoColorConversionSystem.h"
#include "KoBasicHistogramProducers.h"

#include "colorprofiles/KoLcmsColorProfileContainer.h"
#include "colorspaces/KoAlphaColorSpace.h"
#include "colorspaces/KoLabColorSpace.h"
#include "colorspaces/KoRgbU16ColorSpace.h"
#include "colorspaces/KoRgbU8ColorSpace.h"

struct KoColorSpaceRegistry::Private {
    QMap<QString, KoColorProfile * > profileMap;
    QMap<QString, KoColorSpace * > csMap;
    typedef QList<KisPaintDeviceAction *> PaintActionList;
    QMap<QString, PaintActionList> paintDevActionMap;
    KoColorSpace *alphaCs;
    KoColorConversionSystem *colorConversionSystem;
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
    d->colorConversionSystem = new KoColorConversionSystem;
    // prepare a list of the profiles
    KGlobal::mainComponent().dirs()->addResourceType("icc_profiles", 0, "share/color/icc/");

    QStringList profileFilenames;
    profileFilenames += KGlobal::mainComponent().dirs()->findAllResources("icc_profiles", "*.icm",  KStandardDirs::Recursive);
    profileFilenames += KGlobal::mainComponent().dirs()->findAllResources("icc_profiles", "*.ICM",  KStandardDirs::Recursive);
    profileFilenames += KGlobal::mainComponent().dirs()->findAllResources("icc_profiles", "*.ICC",  KStandardDirs::Recursive);
    profileFilenames += KGlobal::mainComponent().dirs()->findAllResources("icc_profiles", "*.icc",  KStandardDirs::Recursive);
    // Set lcms to return NUll/false etc from failing calls, rather than aborting the app.
    cmsErrorAction(LCMS_ERROR_SHOW);

    // Load the profiles
    if (!profileFilenames.empty()) {
        KoColorProfile * profile = 0;
        for ( QStringList::Iterator it = profileFilenames.begin(); it != profileFilenames.end(); ++it ) {
            profile = new KoIccColorProfile(*it);
            Q_CHECK_PTR(profile);

            profile->load();
            if (profile->valid()) {
                kDebug(31000) << "Valid profile : " << profile->name();
                d->profileMap[profile->name()] = profile;
            } else {
                kDebug(31000) << "Invalid profile : " << profile->name();
            }
        }
    }

    KoColorProfile *labProfile = KoLcmsColorProfileContainer::createFromLcmsProfile(cmsCreateLabProfile(NULL));
    addProfile(labProfile);
    add(new KoLabColorSpaceFactory());
    KoHistogramProducerFactoryRegistry::instance()->add(
                new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
                (KoID("LABAHISTO", i18n("L*a*b* Histogram")), lab16()));
    KoColorProfile *rgbProfile = KoLcmsColorProfileContainer::createFromLcmsProfile(cmsCreate_sRGBProfile());
    addProfile(rgbProfile);
    add(new KoRgbU16ColorSpaceFactory());

    add(new KoRgbU8ColorSpaceFactory());
    KoHistogramProducerFactoryRegistry::instance()->add(
                new KoBasicHistogramProducerFactory<KoBasicU8HistogramProducer>
                (KoID("RGB8HISTO", i18n("RGB8 Histogram")), rgb8()) );



    // Create the default profile for grayscale, probably not the best place to but that, but still better than in a grayscale plugin
    // .22 gamma grayscale or something like that. Taken from the lcms tutorial...
    LPGAMMATABLE Gamma = cmsBuildGamma(256, 2.2);
    cmsHPROFILE hProfile = cmsCreateGrayProfile(cmsD50_xyY(), Gamma);
    cmsFreeGamma(Gamma);
    KoColorProfile *defProfile = KoLcmsColorProfileContainer::createFromLcmsProfile(hProfile);
    kDebug(31000) << "Gray " << defProfile->name();
    addProfile(defProfile);

    // Create the built-in colorspaces
    d->alphaCs = new KoAlphaColorSpace(this);

    KoPluginLoader::PluginsConfig config;
    config.whiteList = "ColorSpacePlugins";
    config.blacklist = "ColorSpacePluginsDisabled";
    config.group = "koffice";
    KoPluginLoader::instance()->load("KOffice/ColorSpace","[X-Pigment-MinVersion] <= 0", config);

    KoPluginLoader::PluginsConfig configExtensions;
    configExtensions.whiteList = "ColorSpaceExtensionsPlugins";
    configExtensions.blacklist = "ColorSpaceExtensionsPluginsDisabled";
    configExtensions.group = "koffice";
    KoPluginLoader::instance()->load("KOffice/ColorSpaceExtension","[X-Pigment-MinVersion] <= 0", configExtensions);

}

KoColorSpaceRegistry::KoColorSpaceRegistry() : d(new Private())
{
    d->colorConversionSystem = 0;
}

KoColorSpaceRegistry::~KoColorSpaceRegistry()
{
    delete d->colorConversionSystem;
    delete d;
}

void KoColorSpaceRegistry::add(KoColorSpaceFactory* item)
{
    KoGenericRegistry<KoColorSpaceFactory *>::add(item);
    d->colorConversionSystem->insertColorSpace(item);
}

void KoColorSpaceRegistry::add(const QString &id, KoColorSpaceFactory* item)
{
    KoGenericRegistry<KoColorSpaceFactory *>::add(id, item);
    d->colorConversionSystem->insertColorSpace(item);
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
    return profilesFor(value(id));
}

QList<KoColorProfile *>  KoColorSpaceRegistry::profilesFor(KoColorSpaceFactory * csf)
{
    QList<KoColorProfile *>  profiles;

    QMap<QString, KoColorProfile * >::Iterator it;
    for (it = d->profileMap.begin(); it != d->profileMap.end(); ++it) {
        KoColorProfile *  profile = it.value();
        if(csf->profileIsCompatible(profile))
        {
//         if (profile->colorSpaceSignature() == csf->colorSpaceSignature()) {
            profiles.push_back(profile);
        }
    }
    return profiles;
}

void KoColorSpaceRegistry::addProfile(KoColorProfile *p)
{
      if (p->valid()) {
          d->profileMap[p->name()] = p;
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
        KoColorSpaceFactory *csf = value(csID);

        if(!csf)
            return 0;

        profileName = csf->defaultProfile();
    }

    QString name = csID + "<comb>" + profileName;

    if (d->csMap.find(name) == d->csMap.end()) {
        KoColorSpaceFactory *csf = value(csID);
        if(!csf)
        {
            kDebug(31000) <<"Unknown color space type :" << csf;
            return 0;
        }

        KoColorProfile *p = profileByName(profileName);
        if(!p && !profileName.isEmpty())
        {
            kDebug(31000) <<"Profile not found :" << profileName;
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
        KoColorSpace *cs = colorSpace( csID, profile->name());

        if(!cs)
        {
            // The profile was not stored and thus not the combination either
            KoColorSpaceFactory *csf = value(csID);
            if(!csf)
            {
                kDebug(31000) <<"Unknown color space type :" << csf;
                return 0;
            }

            cs = csf->createColorSpace(this, const_cast<KoColorProfile *>(profile));
            if(!cs )
                return 0;

            QString name = csID + "<comb>" + profile->name();
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

KoColorSpace * KoColorSpaceRegistry::rgb8(const QString &profileName)
{
    return colorSpace(KoRgbU8ColorSpace::colorSpaceId(), profileName);
}

KoColorSpace * KoColorSpaceRegistry::rgb8(KoColorProfile * profile)
{
    return colorSpace(KoRgbU8ColorSpace::colorSpaceId(), profile);
}

KoColorSpace * KoColorSpaceRegistry::rgb16(const QString &profileName)
{
    return colorSpace(KoRgbU16ColorSpace::colorSpaceId(), profileName);
}

KoColorSpace * KoColorSpaceRegistry::rgb16(KoColorProfile * profile)
{
    return colorSpace(KoRgbU16ColorSpace::colorSpaceId(), profile);
}

KoColorSpace * KoColorSpaceRegistry::lab16(const QString &profileName)
{
    return colorSpace(KoLabColorSpace::colorSpaceId(), profileName);
}

KoColorSpace * KoColorSpaceRegistry::lab16(KoColorProfile * profile)
{
    return colorSpace(KoLabColorSpace::colorSpaceId(), profile);
}

QList<KoID> KoColorSpaceRegistry::colorModelsList(ColorSpaceListVisibility option ) const
{
    QList<KoID> ids;
    QList<KoColorSpaceFactory*> factories = values();
    foreach(KoColorSpaceFactory* factory, factories)
    {
        if(not ids.contains(factory->colorModelId())
           and ( option == AllColorSpaces or factory->userVisible() ) )
        {
            ids << factory->colorModelId();
        }
    }
    return ids;
}

QList<KoID> KoColorSpaceRegistry::colorDepthList(const KoID& colorModelId, ColorSpaceListVisibility option ) const
{
    QList<KoID> ids;
    QList<KoColorSpaceFactory*> factories = values();
    foreach(KoColorSpaceFactory* factory, factories)
    {
        if(not ids.contains(factory->colorDepthId())
           and factory->colorModelId() == colorModelId
           and ( option == AllColorSpaces or factory->userVisible() ))
        {
            ids << factory->colorDepthId();
        }
    }
    return ids;
}

QString KoColorSpaceRegistry::colorSpaceId(QString colorModelId, QString colorDepthId)
{
    QList<KoColorSpaceFactory*> factories = values();
    foreach(KoColorSpaceFactory* factory, factories)
    {
        if(factory->colorModelId().id() == colorModelId and factory->colorDepthId().id() == colorDepthId )
        {
            return factory->id();
        }
    }
    return "";
}


QString KoColorSpaceRegistry::colorSpaceId(const KoID& colorModelId, const KoID& colorDepthId)
{
    return colorSpaceId( colorModelId.id(), colorDepthId.id());
}

const KoColorConversionSystem* KoColorSpaceRegistry::colorConversionSystem() const
{
    return d->colorConversionSystem;
}

#include "KoColorSpaceRegistry.moc"
