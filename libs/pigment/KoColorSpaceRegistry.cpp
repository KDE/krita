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

#include <QHash>

#include <QStringList>
#include <QDir>

#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kglobal.h>

#include "KoPluginLoader.h"

#include "DebugPigment.h"
#include "KoBasicHistogramProducers.h"
#include "KoColorSpace.h"
#include "KoColorProfile.h"
#include "KoColorProfileFactory.h"
#include "KoColorConversionCache.h"
#include "KoColorConversionSystem.h"

#include "colorspaces/KoAlphaColorSpace.h"
#include "colorspaces/KoLabColorSpace.h"
#include "colorspaces/KoRgbU16ColorSpace.h"
#include "colorspaces/KoRgbU8ColorSpace.h"
#include "colorspaces/KoSimpleColorSpaceEngine.h"
#include "KoColorSpace_p.h"

struct KoColorSpaceRegistry::Private {

    QHash<QString, KoColorProfile * > profileMap;
    QHash<QString, const KoColorSpace * > csMap;
    QHash<QString, KoColorProfileFactory * > profileFactoryMap;
    const KoColorSpace *alphaCs;
    KoColorConversionSystem *colorConversionSystem;
    KoColorConversionCache* colorConversionCache;
    const KoColorSpace *rgbU8sRGB;
    const KoColorSpace *lab16sLAB;
};

KoColorSpaceRegistry* KoColorSpaceRegistry::instance()
{
    K_GLOBAL_STATIC(KoColorSpaceRegistry, s_instance);
    if (!s_instance.exists()) {
        s_instance->init();
    }
    return s_instance;
}


void KoColorSpaceRegistry::init()
{
    d->rgbU8sRGB = 0;
    d->lab16sLAB = 0;
    d->colorConversionSystem = new KoColorConversionSystem;
    d->colorConversionCache = new KoColorConversionCache;

    KoColorSpaceEngineRegistry::instance()->add(new KoSimpleColorSpaceEngine());

    // Create the built-in colorspaces
    add(new KoLabColorSpaceFactory());
    add(new KoRgbU8ColorSpaceFactory());
    add(new KoRgbU16ColorSpaceFactory());

    d->alphaCs = new KoAlphaColorSpace();
    d->alphaCs->d->deletability = OwnedByRegistryDoNotDelete;

    KoPluginLoader::PluginsConfig config;
    config.whiteList = "ColorSpacePlugins";
    config.blacklist = "ColorSpacePluginsDisabled";
    config.group = "koffice";
    KoPluginLoader::instance()->load("KOffice/ColorSpace", "[X-Pigment-MinVersion] <= 0", config);

    KoPluginLoader::PluginsConfig configExtensions;
    configExtensions.whiteList = "ColorSpaceExtensionsPlugins";
    configExtensions.blacklist = "ColorSpaceExtensionsPluginsDisabled";
    configExtensions.group = "koffice";
    KoPluginLoader::instance()->load("KOffice/ColorSpaceExtension", "[X-Pigment-MinVersion] <= 0", configExtensions);


    dbgPigment << "Loaded the following colorspaces:";
    foreach(const KoID id, listKeys()) {
        dbgPigment << "\t" << id.id() << "," << id.name();
    }
}

KoColorSpaceRegistry::KoColorSpaceRegistry() : d(new Private())
{
    d->colorConversionSystem = 0;
    d->colorConversionCache = 0;
}

KoColorSpaceRegistry::~KoColorSpaceRegistry()
{
    delete d->colorConversionSystem;
    foreach(KoColorProfile* profile, d->profileMap) {
        delete profile;
    }
    d->profileMap.clear();

    foreach(const KoColorSpace * cs, d->csMap) {
        cs->d->deletability = OwnedByRegistryRegistyDeletes;
        delete cs;
    }
    d->csMap.clear();

    foreach(const KoColorProfileFactory* factory, d->profileFactoryMap) {
        delete factory;
    }
    d->profileFactoryMap.clear();

    // deleting colorspaces calls a function in the cache
    delete d->colorConversionCache;
    d->colorConversionCache = 0;

    d->alphaCs->d->deletability = OwnedByRegistryRegistyDeletes;
    delete d->alphaCs;
    // Do not explicitly delete d->rgbU8sRGB and d->lab16sLAB, since they are contained in the d->csMap
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

const KoColorProfile *  KoColorSpaceRegistry::profileByName(const QString & name) const
{
    if (d->profileMap.find(name) == d->profileMap.end()) {
        return 0;
    }

    return d->profileMap[name];
}

QList<const KoColorProfile *>  KoColorSpaceRegistry::profilesFor(const QString &id)
{
    return profilesFor(value(id));
}

const KoColorSpace *  KoColorSpaceRegistry::colorSpace(const KoID &csID, const QString & profileName)
{
    return colorSpace(csID.id(), profileName);
}


QList<const KoColorProfile *>  KoColorSpaceRegistry::profilesFor(const KoColorSpaceFactory * csf)
{
    QList<const KoColorProfile *>  profiles;
    if (csf == 0)
        return profiles;

    QHash<QString, KoColorProfile * >::Iterator it;
    for (it = d->profileMap.begin(); it != d->profileMap.end(); ++it) {
        KoColorProfile *  profile = it.value();
        if (csf->profileIsCompatible(profile)) {
            Q_ASSERT(profile);
            //         if (profile->colorSpaceSignature() == csf->colorSpaceSignature()) {
            profiles.push_back(profile);
        }
    }
    return profiles;
}

QList<const KoColorSpaceFactory*> KoColorSpaceRegistry::colorSpacesFor(const KoColorProfile* _profile)
{
    QList<const KoColorSpaceFactory*> csfs;
    foreach(KoColorSpaceFactory* csf, values()) {
        if (csf->profileIsCompatible(_profile)) {
            csfs.push_back(csf);
        }
    }
    return csfs;
}

QList<const KoColorProfile *>  KoColorSpaceRegistry::profilesFor(const KoID& id)
{
    return profilesFor(id.id());
}

void KoColorSpaceRegistry::addProfileToMap(KoColorProfile *p)
{
    Q_ASSERT(p);
    if (p->valid()) {
        d->profileMap[p->name()] = p;
    }
}

void KoColorSpaceRegistry::addProfile(KoColorProfile *p)
{
    Q_ASSERT(p);
    if (p->valid()) {
        d->profileMap[p->name()] = p;
        d->colorConversionSystem->insertColorProfile(p);
    }
}

void KoColorSpaceRegistry::addProfile(const KoColorProfile* profile)
{
    addProfile(profile->clone());
}

bool KoColorSpaceRegistry::isCached(const QString & csId, const QString & profileName) const
{
    return !(d->csMap.find(idsToCacheName(csId, profileName)) == d->csMap.end());
}

QString KoColorSpaceRegistry::idsToCacheName(const QString & csId, const QString & profileName) const
{
    return csId + "<comb>" + profileName;
}

const KoColorSpace * KoColorSpaceRegistry::colorSpace(const QString &csID, const QString &pName)
{
    QString profileName = pName;

    if (profileName.isEmpty()) {
        KoColorSpaceFactory *csf = value(csID);

        if (!csf) {
            dbgPigmentCSRegistry << "Unknown color space type : " << csID;
            return 0;
        }

        profileName = csf->defaultProfile();
    }

    QString name = idsToCacheName(csID, profileName);

    if (!isCached(csID, profileName)) {
        KoColorSpaceFactory *csf = value(csID);
        if (!csf) {
            dbgPigmentCSRegistry << "Unknown color space type :" << csf;
            return 0;
        }

        // last attempt at getting a profile, sometimes the default profile, like adobe cmyk isn't available.
        const KoColorProfile *p = profileByName(profileName);
        if (!p) {
            dbgPigmentCSRegistry << "Profile not found :" << profileName;
            QList<const KoColorProfile *> profiles = profilesFor(csID);
            if (profiles.isEmpty()) {
                dbgPigmentCSRegistry << "No profile at all available for " << csf;
                p = 0;
            }
            else {
                p = profiles[0];
                Q_ASSERT(p);
            }
        }
        const KoColorSpace *cs = csf->createColorSpace(p);
        if (!cs) {
            dbgPigmentCSRegistry << "Unable to create color space";
            return 0;
        }

        d->csMap[name] = cs;
        cs->d->deletability = OwnedByRegistryDoNotDelete;
        dbgPigmentCSRegistry << "colorspace count: " << d->csMap.count() << ", adding name: " << name;
    }

    if (d->csMap.contains(name))
        return d->csMap[name];
    else
        return 0;
}


const KoColorSpace * KoColorSpaceRegistry::colorSpace(const QString &csID, const KoColorProfile *profile)
{
    if (profile) {
        const KoColorSpace *cs = 0;
        if (isCached(csID, profile->name())) {
            cs = colorSpace(csID, profile->name());
        }

        if (!d->profileMap.contains(profile->name())) {
            addProfile(profile);
        }

        if (!cs) {
            // The profile was not stored and thus not the combination either
            KoColorSpaceFactory *csf = value(csID);
            if (!csf) {
                dbgPigmentCSRegistry << "Unknown color space type :" << csf;
                return 0;
            }

            cs = csf->createColorSpace(profile);
            if (!cs)
                return 0;

            QString name = csID + "<comb>" + profile->name();
            d->csMap[name] = cs;
            cs->d->deletability = OwnedByRegistryDoNotDelete;
            dbgPigmentCSRegistry << "colorspace count: " << d->csMap.count() << ", adding name: " << name;
        }

        return cs;
    } else {
        return colorSpace(csID, "");
    }
}

const KoColorSpace * KoColorSpaceRegistry::alpha8()
{
    return d->alphaCs;
}

const KoColorSpace * KoColorSpaceRegistry::rgb8(const QString &profileName)
{
    if (profileName.isEmpty()) {
        if (!d->rgbU8sRGB) {
            d->rgbU8sRGB = colorSpace(KoRgbU8ColorSpace::colorSpaceId(), profileName);
        }
        Q_ASSERT(d->rgbU8sRGB);
        return d->rgbU8sRGB;
    }
    return colorSpace(KoRgbU8ColorSpace::colorSpaceId(), profileName);
}

const KoColorSpace * KoColorSpaceRegistry::rgb8(const KoColorProfile * profile)
{
    if (profile == 0) {
        if (!d->rgbU8sRGB) {
            d->rgbU8sRGB = colorSpace(KoRgbU8ColorSpace::colorSpaceId(), profile);
        }
        Q_ASSERT(d->rgbU8sRGB);
        return d->rgbU8sRGB;
    }
    return colorSpace(KoRgbU8ColorSpace::colorSpaceId(), profile);
}

const KoColorSpace * KoColorSpaceRegistry::rgb16(const QString &profileName)
{
    return colorSpace(KoRgbU16ColorSpace::colorSpaceId(), profileName);
}

const KoColorSpace * KoColorSpaceRegistry::rgb16(const KoColorProfile * profile)
{
    return colorSpace(KoRgbU16ColorSpace::colorSpaceId(), profile);
}

const KoColorSpace * KoColorSpaceRegistry::lab16(const QString &profileName)
{
    if (profileName.isEmpty()) {
        if (!d->lab16sLAB) {
            d->lab16sLAB = colorSpace(KoLabColorSpace::colorSpaceId(), profileName);
        }
        return d->lab16sLAB;
    }
    return colorSpace(KoLabColorSpace::colorSpaceId(), profileName);
}

const KoColorSpace * KoColorSpaceRegistry::lab16(const KoColorProfile * profile)
{
    if (profile == 0) {
        if (!d->lab16sLAB) {
            d->lab16sLAB = colorSpace(KoLabColorSpace::colorSpaceId(), profile);
        }
        Q_ASSERT(d->lab16sLAB);
        return d->lab16sLAB;
    }
    return colorSpace(KoLabColorSpace::colorSpaceId(), profile);
}

QList<KoID> KoColorSpaceRegistry::colorModelsList(ColorSpaceListVisibility option) const
{
    QList<KoID> ids;
    QList<KoColorSpaceFactory*> factories = values();
    foreach(KoColorSpaceFactory* factory, factories) {
        if (!ids.contains(factory->colorModelId())
                && (option == AllColorSpaces || factory->userVisible())) {
            ids << factory->colorModelId();
        }
    }
    return ids;
}
QList<KoID> KoColorSpaceRegistry::colorDepthList(const KoID& colorModelId, ColorSpaceListVisibility option) const
{
    return colorDepthList(colorModelId.id(), option);
}


QList<KoID> KoColorSpaceRegistry::colorDepthList(const QString & colorModelId, ColorSpaceListVisibility option) const
{
    QList<KoID> ids;
    QList<KoColorSpaceFactory*> factories = values();
    foreach(KoColorSpaceFactory* factory, factories) {
        if (!ids.contains(KoID(factory->colorDepthId()))
                && factory->colorModelId().id() == colorModelId
                && (option == AllColorSpaces || factory->userVisible())) {
            ids << factory->colorDepthId();
        }
    }
    return ids;
}

QString KoColorSpaceRegistry::colorSpaceId(const QString & colorModelId, const QString & colorDepthId)
{
    QList<KoColorSpaceFactory*> factories = values();
    foreach(KoColorSpaceFactory* factory, factories) {
        if (factory->colorModelId().id() == colorModelId && factory->colorDepthId().id() == colorDepthId) {
            return factory->id();
        }
    }
    return "";
}


QString KoColorSpaceRegistry::colorSpaceId(const KoID& colorModelId, const KoID& colorDepthId)
{
    return colorSpaceId(colorModelId.id(), colorDepthId.id());
}

const KoColorConversionSystem* KoColorSpaceRegistry::colorConversionSystem() const
{
    return d->colorConversionSystem;
}

KoColorConversionCache* KoColorSpaceRegistry::colorConversionCache() const
{
    return d->colorConversionCache;
}

const KoColorSpace* KoColorSpaceRegistry::permanentColorspace(const KoColorSpace* _colorSpace)
{
    if (_colorSpace->d->deletability != NotOwnedByRegistry) {
        return _colorSpace;
    } else if (*_colorSpace == *d->alphaCs) {
        return d->alphaCs;
    } else {
        const KoColorSpace* cs = colorSpace(_colorSpace->id(), _colorSpace->profile());
        Q_ASSERT(cs);
        Q_ASSERT(*cs == *_colorSpace);
        return cs;
    }
}

QList<KoID> KoColorSpaceRegistry::listKeys() const
{
    QList<KoID> answer;
    foreach (const QString key, keys()) {
        answer.append(KoID(key, get(key)->name()));
    }

    return answer;
}

KoColorProfile* KoColorSpaceRegistry::createProfile(const QString& type, QByteArray rawData)
{
    // FIXME: add a factory structure to create profiles of different types
    if (d->profileFactoryMap.contains(type)) {
        return d->profileFactoryMap[type]->createColorProfile(rawData);
    }
    return 0;
}

void KoColorSpaceRegistry::addColorProfileFactory(const QString& type, KoColorProfileFactory* factory)
{
    d->profileFactoryMap[type] = factory;
}
