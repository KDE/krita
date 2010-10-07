/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004,2010 Cyrille Berger <cberger@cberger.net>
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
#include "KoGenericRegistry.h"
#include "DebugPigment.h"
#include "KoBasicHistogramProducers.h"
#include "KoColorSpace.h"
#include "KoColorProfile.h"
#include "KoColorConversionCache.h"
#include "KoColorConversionSystem.h"

#include "colorspaces/KoAlphaColorSpace.h"
#include "colorspaces/KoLabColorSpace.h"
#include "colorspaces/KoRgbU16ColorSpace.h"
#include "colorspaces/KoRgbU8ColorSpace.h"
#include "colorspaces/KoSimpleColorSpaceEngine.h"
#include "KoColorSpace_p.h"

struct KoColorSpaceRegistry::Private {
    KoGenericRegistry<KoColorSpaceFactory *> colorSpaceFactoryRegistry;
    QHash<QString, KoColorProfile * > profileMap;
    QHash<QString, QString> profileAlias;
    QHash<QString, const KoColorSpace * > csMap;
    const KoColorSpace *alphaCs;
    KoColorConversionSystem *colorConversionSystem;
    KoColorConversionCache* colorConversionCache;
    const KoColorSpace *rgbU8sRGB;
    const KoColorSpace *lab16sLAB;
    QReadWriteLock registrylock;
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

    addProfile(new KoDummyColorProfile);

    // Create the built-in colorspaces
    add(new KoLabColorSpaceFactory());
    add(new KoRgbU8ColorSpaceFactory());
    add(new KoRgbU16ColorSpaceFactory());

    KoColorSpaceFactory* alphaCSF = new KoAlphaColorSpaceFactory;
    d->colorSpaceFactoryRegistry.add(alphaCSF);
    d->alphaCs = new KoAlphaColorSpace;
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
        releaseColorSpace(const_cast<KoColorSpace*>(cs));
    }
    d->csMap.clear();

    // deleting colorspaces calls a function in the cache
    delete d->colorConversionCache;
    d->colorConversionCache = 0;

    // Delete the colorspace factories
//    qDeleteAll(d->colorSpaceFactoryRegistry.values());
    qDeleteAll(d->colorSpaceFactoryRegistry.doubleEntries());

    // Do not explicitly delete d->rgbU8sRGB and d->lab16sLAB, since they are contained in the d->csMap
    delete d;
}

void KoColorSpaceRegistry::add(KoColorSpaceFactory* item)
{
    {
        QWriteLocker l(&d->registrylock);
        d->colorSpaceFactoryRegistry.add(item);
    }
    d->colorConversionSystem->insertColorSpace(item);
}

void KoColorSpaceRegistry::remove(KoColorSpaceFactory* item)
{
    d->registrylock.lockForRead();
    QList<QString> toremove;
    foreach(const KoColorSpace * cs, d->csMap) {
        if (cs->id() == item->id()) {
            toremove.push_back(idsToCacheName(cs->id(), cs->profile()->name()));
            cs->d->deletability = OwnedByRegistryRegistyDeletes;
            releaseColorSpace(const_cast<KoColorSpace*>(cs));
        }
    }
    d->registrylock.unlock();
    d->registrylock.lockForWrite();
    foreach(const QString& id, toremove) {
        d->csMap.remove(id);
        // TODO: should not it delete the color space when removing it from the map ?
    }
    d->colorSpaceFactoryRegistry.remove(item->id());
    d->registrylock.unlock();
}

void KoColorSpaceRegistry::addProfileAlias(const QString& name, const QString& to)
{
    QWriteLocker l(&d->registrylock);
    d->profileAlias[name] = to;
}

const KoColorProfile *  KoColorSpaceRegistry::profileByName(const QString & _name) const
{
    QReadLocker l(&d->registrylock);
    return d->profileMap.value( d->profileAlias.value(_name, _name) , 0);
}

QList<const KoColorProfile *>  KoColorSpaceRegistry::profilesFor(const QString &id) const
{
    return profilesFor(d->colorSpaceFactoryRegistry.value(id));
}

const KoColorSpace *  KoColorSpaceRegistry::colorSpace(const KoID &csID, const QString & profileName)
{
    return colorSpace(csID.id(), profileName);
}

const KoColorSpace * KoColorSpaceRegistry::colorSpace(const QString & colorModelId, const QString & colorDepthId, const KoColorProfile *profile)
{
    return colorSpace(colorSpaceId(colorModelId, colorDepthId), profile);
}

const KoColorSpace * KoColorSpaceRegistry::colorSpace(const QString & colorModelId, const QString & colorDepthId, const QString &profileName)
{
    return colorSpace(colorSpaceId(colorModelId, colorDepthId), profileName);
}

QList<const KoColorProfile *>  KoColorSpaceRegistry::profilesFor(const KoColorSpaceFactory * csf) const
{
    QReadLocker l(&d->registrylock);
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

QList<const KoColorSpaceFactory*> KoColorSpaceRegistry::colorSpacesFor(const KoColorProfile* _profile) const
{
    QReadLocker l(&d->registrylock);
    QList<const KoColorSpaceFactory*> csfs;
    foreach(KoColorSpaceFactory* csf, d->colorSpaceFactoryRegistry.values()) {
        if (csf->profileIsCompatible(_profile)) {
            csfs.push_back(csf);
        }
    }
    return csfs;
}

QList<const KoColorProfile *>  KoColorSpaceRegistry::profilesFor(const KoID& id) const
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

void KoColorSpaceRegistry::removeProfile(KoColorProfile* profile)
{
    d->profileMap.remove(profile->name());
}

bool KoColorSpaceRegistry::isCached(const QString & csId, const QString & profileName) const
{
    return !(d->csMap.find(idsToCacheName(csId, profileName)) == d->csMap.end());
}

QString KoColorSpaceRegistry::idsToCacheName(const QString & csId, const QString & profileName) const
{
    return csId + "<comb>" + profileName;
}

KoColorSpace* KoColorSpaceRegistry::grabColorSpace(const KoColorSpace* colorSpace)
{
    QReadLocker l(&d->registrylock);
    if(d->colorSpaceFactoryRegistry.contains(colorSpace->id()))
    {
        KoColorSpace* cs = d->colorSpaceFactoryRegistry.value(colorSpace->id())->grabColorSpace(colorSpace->profile());
        return cs;
    }
    warnPigment << "Unknow factory " << colorSpace->id() << " returning the colorspace itself";
    return const_cast<KoColorSpace*>(colorSpace);
}

void KoColorSpaceRegistry::releaseColorSpace(KoColorSpace* colorSpace)
{
    QReadLocker l(&d->registrylock);
    if(d->colorSpaceFactoryRegistry.contains(colorSpace->id()))
    {
        d->colorSpaceFactoryRegistry.value(colorSpace->id())->releaseColorSpace(colorSpace);
    }
}

const KoColorSpaceFactory* KoColorSpaceRegistry::colorSpaceFactory(const QString &colorSpaceId) const
{
    QReadLocker l(&d->registrylock);
    return d->colorSpaceFactoryRegistry.get(colorSpaceId);
}

const KoColorSpace * KoColorSpaceRegistry::colorSpace(const QString &csID, const QString &pName)
{
    QString profileName = pName;

    if (profileName.isEmpty()) {
        QReadLocker l(&d->registrylock);
        KoColorSpaceFactory *csf = d->colorSpaceFactoryRegistry.value(csID);

        if (!csf) {
            dbgPigmentCSRegistry << "Unknown color space type : " << csID;
            return 0;
        }

        profileName = csf->defaultProfile();
    }

    QString name = idsToCacheName(csID, profileName);

    if (!isCached(csID, profileName)) {
        d->registrylock.lockForRead();
        KoColorSpaceFactory *csf = d->colorSpaceFactoryRegistry.value(csID);
        d->registrylock.unlock();
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
                dbgPigmentCSRegistry << "No profile at all available for " << csf << " " << csf->id();
                p = 0;
            } else {
                p = profiles[0];
                Q_ASSERT(p);
            }
        }
        // We did our best, but still have no profile: and since csf->grabColorSpace
        // needs the profile to find the colorspace, we have to give up.
        if (!p) {
            return 0;
        }
        const KoColorSpace *cs = csf->grabColorSpace(p);
        if (!cs) {
            dbgPigmentCSRegistry << "Unable to create color space";
            return 0;
        }

        QWriteLocker l(&d->registrylock);
        d->csMap[name] = cs;
        cs->d->deletability = OwnedByRegistryDoNotDelete;
        dbgPigmentCSRegistry << "colorspace count: " << d->csMap.count() << ", adding name: " << name;
    }
    QReadLocker l(&d->registrylock);

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
            d->registrylock.lockForRead();
            KoColorSpaceFactory *csf = d->colorSpaceFactoryRegistry.value(csID);
            d->registrylock.unlock();
            if (!csf) {
                dbgPigmentCSRegistry << "Unknown color space type :" << csf;
                return 0;
            }
            Q_ASSERT(csf->profileIsCompatible(profile));
            if (!csf->profileIsCompatible(profile ) ) {
                return 0;
            }
            cs = csf->grabColorSpace(profile);
            if (!cs)
                return 0;

            QWriteLocker l(&d->registrylock);
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
    QReadLocker l(&d->registrylock);
    QList<KoID> ids;
    QList<KoColorSpaceFactory*> factories = d->colorSpaceFactoryRegistry.values();
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
    QReadLocker l(&d->registrylock);
    QList<KoID> ids;
    QList<KoColorSpaceFactory*> factories = d->colorSpaceFactoryRegistry.values();
    foreach(KoColorSpaceFactory* factory, factories) {
        if (!ids.contains(KoID(factory->colorDepthId()))
                && factory->colorModelId().id() == colorModelId
                && (option == AllColorSpaces || factory->userVisible())) {
            ids << factory->colorDepthId();
        }
    }
    return ids;
}

QString KoColorSpaceRegistry::colorSpaceId(const QString & colorModelId, const QString & colorDepthId) const
{
    QReadLocker l(&d->registrylock);
    QList<KoColorSpaceFactory*> factories = d->colorSpaceFactoryRegistry.values();
    foreach(KoColorSpaceFactory* factory, factories) {
        if (factory->colorModelId().id() == colorModelId && factory->colorDepthId().id() == colorDepthId) {
            return factory->id();
        }
    }
    return "";
}

QString KoColorSpaceRegistry::colorSpaceId(const KoID& colorModelId, const KoID& colorDepthId) const
{
    return colorSpaceId(colorModelId.id(), colorDepthId.id());
}

KoID KoColorSpaceRegistry::colorSpaceColorModelId(const QString & _colorSpaceId) const
{
    QReadLocker l(&d->registrylock);
    KoColorSpaceFactory* factory = d->colorSpaceFactoryRegistry.get(_colorSpaceId);
    if (factory) {
        return factory->colorModelId();
    } else {
        return KoID();
    }
}

KoID KoColorSpaceRegistry::colorSpaceColorDepthId(const QString & _colorSpaceId) const
{
    QReadLocker l(&d->registrylock);
    KoColorSpaceFactory* factory = d->colorSpaceFactoryRegistry.get(_colorSpaceId);
    if (factory) {
        return factory->colorDepthId();
    } else {
        return KoID();
    }
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
    QReadLocker l(&d->registrylock);
    QList<KoID> answer;
    foreach(const QString key, d->colorSpaceFactoryRegistry.keys()) {
        answer.append(KoID(key, d->colorSpaceFactoryRegistry.get(key)->name()));
    }

    return answer;
}

const KoColorProfile* KoColorSpaceRegistry::createColorProfile(const QString& colorModelId, const QString& colorDepthId, const QByteArray& rawData)
{
    QReadLocker l(&d->registrylock);
    KoColorSpaceFactory* factory_ = d->colorSpaceFactoryRegistry.get(colorSpaceId(colorModelId, colorDepthId));
    return factory_->colorProfile(rawData);
}

QList<const KoColorSpace*> KoColorSpaceRegistry::allColorSpaces(ColorSpaceListVisibility visibility, ColorSpaceListProfilesSelection pSelection)
{
    QList<const KoColorSpace*> colorSpaces;

    d->registrylock.lockForRead();
    QList<KoColorSpaceFactory*> factories = d->colorSpaceFactoryRegistry.values();
    d->registrylock.unlock();

    foreach(KoColorSpaceFactory* factory, factories) {
        if (visibility == AllColorSpaces || factory->userVisible()) {
            if (pSelection == OnlyDefaultProfile) {
                colorSpaces.append(colorSpace(factory->id(), 0));
            } else {
                QList<const KoColorProfile*> profiles = KoColorSpaceRegistry::instance()->profilesFor(factory->id());
                foreach(const KoColorProfile * profile, profiles) {
                    colorSpaces.append(colorSpace(factory->id(), profile));
                }
            }
        }
    }

    return colorSpaces;
}
