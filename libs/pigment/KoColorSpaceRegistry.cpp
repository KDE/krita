/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004,2010 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#include <QReadWriteLock>
#include <QStringList>
#include <QDir>
#include <QGlobalStatic>

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

Q_GLOBAL_STATIC(KoColorSpaceRegistry, s_instance)


struct Q_DECL_HIDDEN KoColorSpaceRegistry::Private {
    KoGenericRegistry<KoColorSpaceFactory *> colorSpaceFactoryRegistry;
    QList<KoColorSpaceFactory *> localFactories;
    QHash<QString, KoColorProfile * > profileMap;
    QHash<QString, QString> profileAlias;
    QHash<QString, const KoColorSpace * > csMap;
    KoColorConversionSystem *colorConversionSystem;
    KoColorConversionCache* colorConversionCache;
    KoColorSpaceFactory* alphaCSF;
    const KoColorSpace *rgbU8sRGB;
    const KoColorSpace *lab16sLAB;
    const KoColorSpace *alphaCs;
    QReadWriteLock registrylock;
};

KoColorSpaceRegistry* KoColorSpaceRegistry::instance()
{
    if (!s_instance.exists()) {
        s_instance->init();
    }
    return s_instance;
}


void KoColorSpaceRegistry::init()
{
    d->rgbU8sRGB = 0;
    d->lab16sLAB = 0;
    d->alphaCs = 0;

    d->colorConversionSystem = new KoColorConversionSystem;
    d->colorConversionCache = new KoColorConversionCache;

    KoColorSpaceEngineRegistry::instance()->add(new KoSimpleColorSpaceEngine());

    addProfile(new KoDummyColorProfile);

    // Create the built-in colorspaces
    d->localFactories << new KoLabColorSpaceFactory()
            << new KoRgbU8ColorSpaceFactory()
            << new KoRgbU16ColorSpaceFactory();
    Q_FOREACH (KoColorSpaceFactory *factory, d->localFactories) {
        add(factory);
    }

    d->alphaCs = new KoAlphaColorSpace();
    d->alphaCs->d->deletability = OwnedByRegistryRegistryDeletes;

    KoPluginLoader::PluginsConfig config;
    config.whiteList = "ColorSpacePlugins";
    config.blacklist = "ColorSpacePluginsDisabled";
    config.group = "calligra";
    KoPluginLoader::instance()->load("Calligra/ColorSpace", "[X-Pigment-PluginVersion] == 28", config);

    KoPluginLoader::PluginsConfig configExtensions;
    configExtensions.whiteList = "ColorSpaceExtensionsPlugins";
    configExtensions.blacklist = "ColorSpaceExtensionsPluginsDisabled";
    configExtensions.group = "calligra";
    KoPluginLoader::instance()->load("Calligra/ColorSpaceExtension", "[X-Pigment-PluginVersion] == 28", configExtensions);


    dbgPigment << "Loaded the following colorspaces:";
    Q_FOREACH (const KoID& id, listKeys()) {
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
    // Just leak on exit... It's faster.
//    delete d->colorConversionSystem;
//    Q_FOREACH (KoColorProfile* profile, d->profileMap) {
//        delete profile;
//    }
//    d->profileMap.clear();

//    Q_FOREACH (const KoColorSpace * cs, d->csMap) {
//        cs->d->deletability = OwnedByRegistryRegistryDeletes;
//    }
//    d->csMap.clear();

//    // deleting colorspaces calls a function in the cache
//    delete d->colorConversionCache;
//    d->colorConversionCache = 0;

//    // Delete the colorspace factories
//    qDeleteAll(d->localFactories);

//    delete d->alphaCSF;

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
    Q_FOREACH (const KoColorSpace * cs, d->csMap) {
        if (cs->id() == item->id()) {
            toremove.push_back(idsToCacheName(cs->id(), cs->profile()->name()));
            cs->d->deletability = OwnedByRegistryRegistryDeletes;
        }
    }
    d->registrylock.unlock();
    d->registrylock.lockForWrite();
    Q_FOREACH (const QString& id, toremove) {
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

QString KoColorSpaceRegistry::profileAlias(const QString& _name) const
{
    QReadLocker l(&d->registrylock);
    return d->profileAlias.value(_name, _name);
}

const KoColorProfile *  KoColorSpaceRegistry::profileByName(const QString & _name) const
{
    QReadLocker l(&d->registrylock);
    return d->profileMap.value( profileAlias(_name), 0);
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
    Q_FOREACH (KoColorSpaceFactory* csf, d->colorSpaceFactoryRegistry.values()) {
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

bool KoColorSpaceRegistry::isCached(const QString & csID, const QString & profileName) const
{
    QReadLocker l(&d->registrylock);
    return !(d->csMap.find(idsToCacheName(csID, profileName)) == d->csMap.end());
}

QString KoColorSpaceRegistry::idsToCacheName(const QString & csID, const QString & profileName) const
{
    return csID + "<comb>" + profileName;
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

    if (profileName.isEmpty()) {
        return 0;
    }

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

            /**
             * If the requested profile is not available, try fetching the
             * default one
             */
            profileName = csf->defaultProfile();
            p = profileByName(profileName);

            /**
             * If there is no luck, try to fetch the first one
             */
            if (!p) {
                QList<const KoColorProfile *> profiles = profilesFor(csID);
                if (!profiles.isEmpty()) {
                    p = profiles[0];
                    Q_ASSERT(p);
                }
            }
        }

        // We did our best, but still have no profile: and since csf->grabColorSpace
        // needs the profile to find the colorspace, we have to give up.
        if (!p) {
            return 0;
        }
        profileName = p->name();

        const KoColorSpace *cs = csf->grabColorSpace(p);
        if (!cs) {
            dbgPigmentCSRegistry << "Unable to create color space";
            return 0;
        }

        QWriteLocker l(&d->registrylock);
        dbgPigmentCSRegistry << "colorspace count: " << d->csMap.count()
                             << ", adding name: " << idsToCacheName(cs->id(), cs->profile()->name())
                             << "\n\tcsID" << csID
                             << "\n\tprofileName" << profileName
                             << "\n\tcs->id()" << cs->id()
                             << "\n\tcs->profile()->name()" << cs->profile()->name()
                             << "\n\tpName" << pName;
        Q_ASSERT(cs->id() == csID);
        Q_ASSERT(cs->profile()->name() == profileName);
        d->csMap[idsToCacheName(cs->id(), cs->profile()->name())] = cs;
        cs->d->deletability = OwnedByRegistryDoNotDelete;

    }
    QReadLocker l(&d->registrylock);

    if (d->csMap.contains(idsToCacheName(csID, profileName))) {
        const KoColorSpace *cs = d->csMap[idsToCacheName(csID, profileName)];
        Q_ASSERT(cs->profile()->name() == profileName);
        return cs;
    }
    else {
        return 0;
    }
}


const KoColorSpace * KoColorSpaceRegistry::colorSpace(const QString &csID, const KoColorProfile *profile)
{
    if (csID.isEmpty()) {
        return 0;
    }
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
        return colorSpace(csID);
    }
}

const KoColorSpace * KoColorSpaceRegistry::alpha8()
{
    if (!d->alphaCs) {
        d->alphaCs = colorSpace(KoAlphaColorSpace::colorSpaceId());
    }
    Q_ASSERT(d->alphaCs);
    return d->alphaCs;
}

const KoColorSpace * KoColorSpaceRegistry::rgb8(const QString &profileName)
{
    if (profileName.isEmpty()) {
        if (!d->rgbU8sRGB) {
            d->rgbU8sRGB = colorSpace(KoRgbU8ColorSpace::colorSpaceId());
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
            d->rgbU8sRGB = colorSpace(KoRgbU8ColorSpace::colorSpaceId());
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
    Q_FOREACH (KoColorSpaceFactory* factory, factories) {
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
    Q_FOREACH (KoColorSpaceFactory* factory, factories) {
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
    Q_FOREACH (KoColorSpaceFactory* factory, factories) {
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
    Q_FOREACH (const QString& key, d->colorSpaceFactoryRegistry.keys()) {
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

    Q_FOREACH (KoColorSpaceFactory* factory, factories) {
        // Don't test with ycbcr for now, since we don't have a default profile for it.
        if (factory->colorModelId().id().startsWith("Y")) continue;
        if (visibility == AllColorSpaces || factory->userVisible()) {
            if (pSelection == OnlyDefaultProfile) {
                const KoColorSpace *cs = colorSpace(factory->id());
                if (cs) {
                    colorSpaces.append(cs);
                }
                else {
                    warnPigment << "Could not create colorspace for id" << factory->id() << "since there is no working default profile";
                }
            } else {
                QList<const KoColorProfile*> profiles = KoColorSpaceRegistry::instance()->profilesFor(factory->id());
                Q_FOREACH (const KoColorProfile * profile, profiles) {
                    const KoColorSpace *cs = colorSpace(factory->id(), profile);
                    if (cs) {
                        colorSpaces.append(cs);
                    }
                    else {
                        warnPigment << "Could not create colorspace for id" << factory->id() << "and profile" << profile->name();
                    }
                }
            }
        }
    }

    return colorSpaces;
}
