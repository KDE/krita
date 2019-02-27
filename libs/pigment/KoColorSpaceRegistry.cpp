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

#include "kis_assert.h"
#include "KoColorProfileStorage.h"
#include <KisReadWriteLockPolicy.h>

#include <KoColorModelStandardIds.h>

Q_GLOBAL_STATIC(KoColorSpaceRegistry, s_instance)


struct Q_DECL_HIDDEN KoColorSpaceRegistry::Private {

    // interface for KoColorSpaceFactory
    struct ProfileRegistrationInterface;
    // interface for KoColorConversionSystem
    struct ConversionSystemInterface;


    Private(KoColorSpaceRegistry *_q) : q(_q) {}

    KoColorSpaceRegistry *q;

    KoGenericRegistry<KoColorSpaceFactory *> colorSpaceFactoryRegistry;
    KoColorProfileStorage profileStorage;
    QHash<QString, const KoColorSpace *> csMap;
    QScopedPointer<ConversionSystemInterface> conversionSystemInterface;
    KoColorConversionSystem *colorConversionSystem;
    KoColorConversionCache* colorConversionCache;
    const KoColorSpace *rgbU8sRGB;
    const KoColorSpace *lab16sLAB;
    const KoColorSpace *alphaCs;
    const KoColorSpace *alphaU16Cs;
#ifdef HAVE_OPENEXR
    const KoColorSpace *alphaF16Cs;
#endif
    const KoColorSpace *alphaF32Cs;
    QReadWriteLock registrylock;

    /**
     * The function checks if a colorspace with a certain id and profile name can be found in the cache
     * NOTE: the function doesn't take any lock but it needs to be called inside a d->registryLock
     * locked either in read or write.
     * @param csId The colorspace id
     * @param profileName The colorspace profile name
     * @retval KoColorSpace The matching colorspace
     * @retval 0 Null pointer if not match
     */
    const KoColorSpace* getCachedColorSpaceImpl(const QString & csId, const QString & profileName) const;

    QString idsToCacheName(const QString & csId, const QString & profileName) const;
    QString defaultProfileForCsIdImpl(const QString &csID);
    const KoColorProfile * profileForCsIdWithFallbackImpl(const QString &csID, const QString &profileName);
    QString colorSpaceIdImpl(const QString & colorModelId, const QString & colorDepthId) const;

    const KoColorSpace *lazyCreateColorSpaceImpl(const QString &csID, const KoColorProfile *profile);

    /**
     * Return a colorspace that works with the parameter profile.
     * @param profileName the name of the KoColorProfile to be combined with the colorspace
     * @return the wanted colorspace, or 0 when the cs and profile can not be combined.
     */
    template<class LockPolicy = NormalLockPolicy>
    const KoColorSpace * colorSpace1(const QString &colorSpaceId, const QString &pName = QString());

    /**
     * Return a colorspace that works with the parameter profile.
     * @param colorSpaceId the ID string of the colorspace that you want to have returned
     * @param profile the profile be combined with the colorspace
     * @return the wanted colorspace, or 0 when the cs and profile can not be combined.
     */
    const KoColorSpace * colorSpace1(const QString &colorSpaceId, const KoColorProfile *profile);
};

struct KoColorSpaceRegistry::Private::ConversionSystemInterface : public KoColorConversionSystem::RegistryInterface
{
    ConversionSystemInterface(KoColorSpaceRegistry *parentRegistry)
        : q(parentRegistry)
    {
    }

    const KoColorSpace * colorSpace(const QString & colorModelId, const QString & colorDepthId, const QString &profileName) {
        return q->d->colorSpace1<NoLockPolicy>(q->d->colorSpaceIdImpl(colorModelId, colorDepthId), profileName);
    }

    const KoColorSpaceFactory* colorSpaceFactory(const QString &colorModelId, const QString &colorDepthId) const {
        return q->d->colorSpaceFactoryRegistry.get(q->d->colorSpaceIdImpl(colorModelId, colorDepthId));
    }

    QList<const KoColorProfile *>  profilesFor(const KoColorSpaceFactory * csf) const {
        return q->d->profileStorage.profilesFor(csf);
    }

    QList<const KoColorSpaceFactory*> colorSpacesFor(const KoColorProfile* profile) const {
        QList<const KoColorSpaceFactory*> csfs;
        Q_FOREACH (KoColorSpaceFactory* csf, q->d->colorSpaceFactoryRegistry.values()) {
            if (csf->profileIsCompatible(profile)) {
                csfs.push_back(csf);
            }
        }
        return csfs;
    }

private:
    KoColorSpaceRegistry *q;
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
    d->alphaU16Cs = 0;
#ifdef HAVE_OPENEXR
    d->alphaF16Cs = 0;
#endif
    d->alphaF32Cs = 0;

    d->conversionSystemInterface.reset(new Private::ConversionSystemInterface(this));
    d->colorConversionSystem = new KoColorConversionSystem(d->conversionSystemInterface.data());
    d->colorConversionCache = new KoColorConversionCache;

    KoColorSpaceEngineRegistry::instance()->add(new KoSimpleColorSpaceEngine());

    addProfile(new KoDummyColorProfile);

    // Create the built-in colorspaces
    QList<KoColorSpaceFactory *> localFactories;
    localFactories
            << new KoAlphaColorSpaceFactory()
            << new KoAlphaU16ColorSpaceFactory()
           #ifdef HAVE_OPENEXR
            << new KoAlphaF16ColorSpaceFactory()
           #endif
            << new KoAlphaF32ColorSpaceFactory()
            << new KoLabColorSpaceFactory()
            << new KoRgbU8ColorSpaceFactory()
            << new KoRgbU16ColorSpaceFactory();

    Q_FOREACH (KoColorSpaceFactory *factory, localFactories) {
        add(factory);
    }

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

KoColorSpaceRegistry::KoColorSpaceRegistry() : d(new Private(this))
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

    delete d;
}

void KoColorSpaceRegistry::add(KoColorSpaceFactory* item)
{
    QWriteLocker l(&d->registrylock);
    d->colorSpaceFactoryRegistry.add(item);
    d->colorConversionSystem->insertColorSpace(item);
}

void KoColorSpaceRegistry::remove(KoColorSpaceFactory* item)
{
    QWriteLocker l(&d->registrylock);

    QList<QString> toremove;
    Q_FOREACH (const KoColorSpace * cs, d->csMap) {
        if (cs->id() == item->id()) {
            toremove.push_back(d->idsToCacheName(cs->id(), cs->profile()->name()));
            cs->d->deletability = OwnedByRegistryRegistryDeletes;
        }
    }

    Q_FOREACH (const QString& id, toremove) {
        d->csMap.remove(id);
        // TODO: should not it delete the color space when removing it from the map ?
    }
    d->colorSpaceFactoryRegistry.remove(item->id());
}

void KoColorSpaceRegistry::addProfileAlias(const QString& name, const QString& to)
{
    d->profileStorage.addProfileAlias(name, to);
}

QString KoColorSpaceRegistry::profileAlias(const QString& name) const
{
    return d->profileStorage.profileAlias(name);
}

const KoColorProfile*  KoColorSpaceRegistry::profileByName(const QString &name) const
{
    return d->profileStorage.profileByName(name);
}

const KoColorProfile *  KoColorSpaceRegistry::profileByUniqueId(const QByteArray &id) const
{
    return d->profileStorage.profileByUniqueId(id);
}

QList<const KoColorProfile *>  KoColorSpaceRegistry::profilesFor(const QString &csID) const
{
    QReadLocker l(&d->registrylock);
    return d->profileStorage.profilesFor(d->colorSpaceFactoryRegistry.value(csID));
}

const KoColorSpace * KoColorSpaceRegistry::colorSpace(const QString & colorModelId, const QString & colorDepthId, const KoColorProfile *profile)
{
    return d->colorSpace1(colorSpaceId(colorModelId, colorDepthId), profile);
}

const KoColorSpace * KoColorSpaceRegistry::colorSpace(const QString & colorModelId, const QString & colorDepthId, const QString &profileName)
{
    return d->colorSpace1(colorSpaceId(colorModelId, colorDepthId), profileName);
}

const KoColorSpace * KoColorSpaceRegistry::colorSpace(const QString & colorModelId, const QString & colorDepthId)
{
    return d->colorSpace1(colorSpaceId(colorModelId, colorDepthId));
}

bool KoColorSpaceRegistry::profileIsCompatible(const KoColorProfile *profile, const QString &colorSpaceId)
{
    QReadLocker l(&d->registrylock);
    KoColorSpaceFactory *csf = d->colorSpaceFactoryRegistry.value(colorSpaceId);

    return csf ? csf->profileIsCompatible(profile) : false;
}

void KoColorSpaceRegistry::addProfileToMap(KoColorProfile *p)
{
    d->profileStorage.addProfile(p);
}

void KoColorSpaceRegistry::addProfile(KoColorProfile *p)
{
    if (!p->valid()) return;

    QWriteLocker locker(&d->registrylock);
    if (p->valid()) {
        addProfileToMap(p);
        d->colorConversionSystem->insertColorProfile(p);
    }
}

void KoColorSpaceRegistry::addProfile(const KoColorProfile* profile)
{
    addProfile(profile->clone());
}

void KoColorSpaceRegistry::removeProfile(KoColorProfile* profile)
{
    d->profileStorage.removeProfile(profile);
    // FIXME: how about removing it from conversion system?
}

const KoColorSpace* KoColorSpaceRegistry::Private::getCachedColorSpaceImpl(const QString & csID, const QString & profileName) const
{
    auto it = csMap.find(idsToCacheName(csID, profileName));

    if (it != csMap.end()) {
        return it.value();
    }

    return 0;
}

QString KoColorSpaceRegistry::Private::idsToCacheName(const QString & csID, const QString & profileName) const
{
    return csID + "<comb>" + profileName;
}

QString KoColorSpaceRegistry::defaultProfileForColorSpace(const QString &colorSpaceId) const
{
    QReadLocker l(&d->registrylock);
    return d->defaultProfileForCsIdImpl(colorSpaceId);
}

KoColorConversionTransformation *KoColorSpaceRegistry::createColorConverter(const KoColorSpace *srcColorSpace, const KoColorSpace *dstColorSpace, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags) const
{
    QWriteLocker l(&d->registrylock);
    return d->colorConversionSystem->createColorConverter(srcColorSpace, dstColorSpace, renderingIntent, conversionFlags);
}

void KoColorSpaceRegistry::createColorConverters(const KoColorSpace *colorSpace, const QList<QPair<KoID, KoID> > &possibilities, KoColorConversionTransformation *&fromCS, KoColorConversionTransformation *&toCS) const
{
    QWriteLocker l(&d->registrylock);
    d->colorConversionSystem->createColorConverters(colorSpace, possibilities, fromCS, toCS);
}

QString KoColorSpaceRegistry::Private::defaultProfileForCsIdImpl(const QString &csID)
{
    QString defaultProfileName;

    KoColorSpaceFactory *csf = colorSpaceFactoryRegistry.value(csID);
    if (csf) {
        defaultProfileName = csf->defaultProfile();
    } else {
        dbgPigmentCSRegistry << "Unknown color space type : " << csID;
    }

    return defaultProfileName;
}

const KoColorProfile *KoColorSpaceRegistry::Private::profileForCsIdWithFallbackImpl(const QString &csID, const QString &profileName)
{
    const KoColorProfile *profile = 0;

    // last attempt at getting a profile, sometimes the default profile, like adobe cmyk isn't available.
    profile = profileStorage.profileByName(profileName);
    if (!profile) {
        dbgPigmentCSRegistry << "Profile not found :" << profileName;

        // first try: default
        profile = profileStorage.profileByName(defaultProfileForCsIdImpl(csID));

        if (!profile) {
            // second try: first profile in the list
            QList<const KoColorProfile *> profiles = profileStorage.profilesFor(colorSpaceFactoryRegistry.value(csID));
            if (profiles.isEmpty() || !profiles.first()) {
                dbgPigmentCSRegistry << "Couldn't fetch a fallback profile:" << profileName;
                qWarning() << "profileForCsIdWithFallbackImpl couldn't fetch a fallback profile for " << qUtf8Printable(profileName);
                return 0;
            }

            profile = profiles.first();
        }
    }

    return profile;
}

const KoColorSpace *KoColorSpaceRegistry::Private::lazyCreateColorSpaceImpl(const QString &csID, const KoColorProfile *profile)
{
    const KoColorSpace *cs = 0;

    /*
     * We need to check again here, a thread requesting the same colorspace could've added it
     * already, in between the read unlock and write lock.
     * TODO: We also potentially changed profileName content, which means we maybe are going to
     * create a colorspace that's actually in the space registry cache, but currently this might
     * not be an issue because the colorspace should be cached also by the factory, so it won't
     * create a new instance. That being said, having two caches with the same stuff doesn't make
     * much sense.
     */
    cs = getCachedColorSpaceImpl(csID, profile->name());
    if (!cs) {
        KoColorSpaceFactory *csf = colorSpaceFactoryRegistry.value(csID);
        cs = csf->grabColorSpace(profile);
        if (!cs) {
            dbgPigmentCSRegistry << "Unable to create color space";
            qWarning() << "lazyCreateColorSpaceImpl was unable to create a color space for " << csID;
            return 0;
        }

        dbgPigmentCSRegistry << "colorspace count: " << csMap.count()
                             << ", adding name: " << idsToCacheName(cs->id(), cs->profile()->name())
                             << "\n\tcsID" << csID
                             << "\n\tcs->id()" << cs->id()
                             << "\n\tcs->profile()->name()" << cs->profile()->name()
                             << "\n\tprofile->name()" << profile->name();
        Q_ASSERT(cs->id() == csID);
        Q_ASSERT(cs->profile()->name() == profile->name());
        csMap[idsToCacheName(cs->id(), cs->profile()->name())] = cs;
        cs->d->deletability = OwnedByRegistryDoNotDelete;
    }

    return cs;
}

template<class LockPolicy>
const KoColorSpace * KoColorSpaceRegistry::Private::colorSpace1(const QString &csID, const QString &pName)
{
    QString profileName = pName;

    const KoColorSpace *cs = 0;

    {
        typename LockPolicy::ReadLocker l(&registrylock);

        if (profileName.isEmpty()) {
            profileName = defaultProfileForCsIdImpl(csID);
            if (profileName.isEmpty()) return 0;
        }

        // quick attempt to fetch a cached color space
        cs = getCachedColorSpaceImpl(csID, profileName);
    }

    if (!cs) {
        // slow attempt to create a color space
        typename LockPolicy::WriteLocker l(&registrylock);

        const KoColorProfile *profile =
            profileForCsIdWithFallbackImpl(csID, profileName);

        // until kis_asert.h is not available in 3.1, use this combo
        Q_ASSERT(profile);
        if (!profile) return 0;

        cs = lazyCreateColorSpaceImpl(csID, profile);
    }
    else {
        KIS_SAFE_ASSERT_RECOVER_NOOP(cs->id() == csID);
        KIS_SAFE_ASSERT_RECOVER_NOOP(cs->profile()->name() == profileName);
    }

    return cs;
}


const KoColorSpace * KoColorSpaceRegistry::Private::colorSpace1(const QString &csID, const KoColorProfile *profile)
{
    if (csID.isEmpty()) {
        return 0;
    } else if (!profile) {
        return colorSpace1(csID);
    }

    const KoColorSpace *cs = 0;

    {
        QReadLocker l(&registrylock);
        cs = getCachedColorSpaceImpl(csID, profile->name());
    }

    // the profile should have already been added to the registry by createColorProfile() method
    KIS_SAFE_ASSERT_RECOVER(profileStorage.containsProfile(profile)) {
        // warning! locking happens inside addProfile!
        q->addProfile(profile);
    }

    if (!cs) {
        // The profile was not stored and thus not the combination either
        QWriteLocker l(&registrylock);
        KoColorSpaceFactory *csf = colorSpaceFactoryRegistry.value(csID);

        if (!csf) {
            dbgPigmentCSRegistry << "Unknown color space type :" << csf;
            return 0;
        }

        if (!csf->profileIsCompatible(profile)) {
            dbgPigmentCSRegistry << "Profile is not compatible:" << csf << profile->name();
            return 0;
        }

        cs = lazyCreateColorSpaceImpl(csID, profile);
    }

    return cs;
}

const KoColorSpace * KoColorSpaceRegistry::alpha8()
{
    if (!d->alphaCs) {
        d->alphaCs = d->colorSpace1(KoAlphaColorSpace::colorSpaceId());
    }
    Q_ASSERT(d->alphaCs);
    return d->alphaCs;
}

const KoColorSpace * KoColorSpaceRegistry::alpha16()
{
    if (!d->alphaU16Cs) {
        d->alphaU16Cs = d->colorSpace1(KoAlphaU16ColorSpace::colorSpaceId());
    }
    Q_ASSERT(d->alphaU16Cs);
    return d->alphaU16Cs;
}

#ifdef HAVE_OPENEXR
const KoColorSpace * KoColorSpaceRegistry::alpha16f()
{
    if (!d->alphaF16Cs) {
        d->alphaF16Cs = d->colorSpace1(KoAlphaF16ColorSpace::colorSpaceId());
    }
    Q_ASSERT(d->alphaF16Cs);
    return d->alphaF16Cs;
}
#endif

const KoColorSpace * KoColorSpaceRegistry::alpha32f()
{
    if (!d->alphaF32Cs) {
        d->alphaF32Cs = d->colorSpace1(KoAlphaF32ColorSpace::colorSpaceId());
    }
    Q_ASSERT(d->alphaF32Cs);
    return d->alphaF32Cs;
}


const KoColorSpace * KoColorSpaceRegistry::rgb8(const QString &profileName)
{
    if (profileName.isEmpty()) {
        if (!d->rgbU8sRGB) {
            d->rgbU8sRGB = d->colorSpace1(KoRgbU8ColorSpace::colorSpaceId());
        }
        Q_ASSERT(d->rgbU8sRGB);
        return d->rgbU8sRGB;
    }
    return d->colorSpace1(KoRgbU8ColorSpace::colorSpaceId(), profileName);
}

const KoColorSpace * KoColorSpaceRegistry::rgb8(const KoColorProfile * profile)
{
    if (profile == 0) {
        if (!d->rgbU8sRGB) {
            d->rgbU8sRGB = d->colorSpace1(KoRgbU8ColorSpace::colorSpaceId());
        }
        Q_ASSERT(d->rgbU8sRGB);
        return d->rgbU8sRGB;
    }
    return d->colorSpace1(KoRgbU8ColorSpace::colorSpaceId(), profile);
}

const KoColorSpace * KoColorSpaceRegistry::rgb16(const QString &profileName)
{
    return d->colorSpace1(KoRgbU16ColorSpace::colorSpaceId(), profileName);
}

const KoColorSpace * KoColorSpaceRegistry::rgb16(const KoColorProfile * profile)
{
    return d->colorSpace1(KoRgbU16ColorSpace::colorSpaceId(), profile);
}

const KoColorSpace * KoColorSpaceRegistry::lab16(const QString &profileName)
{
    if (profileName.isEmpty()) {
        if (!d->lab16sLAB) {
            d->lab16sLAB = d->colorSpace1(KoLabColorSpace::colorSpaceId());
        }
        return d->lab16sLAB;
    }
    return d->colorSpace1(KoLabColorSpace::colorSpaceId(), profileName);
}

const KoColorSpace * KoColorSpaceRegistry::lab16(const KoColorProfile * profile)
{
    if (profile == 0) {
        if (!d->lab16sLAB) {
            d->lab16sLAB = d->colorSpace1(KoLabColorSpace::colorSpaceId());
        }
        Q_ASSERT(d->lab16sLAB);
        return d->lab16sLAB;
    }
    return d->colorSpace1(KoLabColorSpace::colorSpaceId(), profile);
}

const KoColorProfile *KoColorSpaceRegistry::p2020G10Profile() const
{
    return profileByName("Rec2020-elle-V4-g10.icc");
}

const KoColorProfile *KoColorSpaceRegistry::p2020PQProfile() const
{
    return profileByName("High Dynamic Range UHDTV Wide Color Gamut Display (Rec. 2020) - SMPTE ST 2084 PQ EOTF");
}

const KoColorProfile *KoColorSpaceRegistry::p709G10Profile() const
{
    return profileByName("sRGB-elle-V2-g10.icc");
}

const KoColorProfile *KoColorSpaceRegistry::p709SRGBProfile() const
{
    return profileByName("sRGB-elle-V2-srgbtrc.icc");
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
    QList<KoID> r;

    if (ids.contains(Integer8BitsColorDepthID)) r << Integer8BitsColorDepthID;
    if (ids.contains(Integer16BitsColorDepthID)) r << Integer16BitsColorDepthID;
    if (ids.contains(Float16BitsColorDepthID)) r << Float16BitsColorDepthID;
    if (ids.contains(Float32BitsColorDepthID)) r << Float32BitsColorDepthID;
    if (ids.contains(Float64BitsColorDepthID)) r << Float64BitsColorDepthID;

    return r;
}

QString KoColorSpaceRegistry::Private::colorSpaceIdImpl(const QString & colorModelId, const QString & colorDepthId) const
{
    QList<KoColorSpaceFactory*> factories = colorSpaceFactoryRegistry.values();
    Q_FOREACH (KoColorSpaceFactory* factory, factories) {
        if (factory->colorModelId().id() == colorModelId && factory->colorDepthId().id() == colorDepthId) {
            return factory->id();
        }
    }
    return "";
}

QString KoColorSpaceRegistry::colorSpaceId(const QString & colorModelId, const QString & colorDepthId) const
{
    QReadLocker l(&d->registrylock);
    return d->colorSpaceIdImpl(colorModelId, colorDepthId);
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
        const KoColorSpace* cs = d->colorSpace1(_colorSpace->id(), _colorSpace->profile());
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

struct KoColorSpaceRegistry::Private::ProfileRegistrationInterface : public KoColorSpaceFactory::ProfileRegistrationInterface
{
    ProfileRegistrationInterface(KoColorSpaceRegistry::Private *_d) : d(_d) {}

    const KoColorProfile* profileByName(const QString &profileName) const override {
        return d->profileStorage.profileByName(profileName);
    }

    void registerNewProfile(KoColorProfile *profile) override {
        d->profileStorage.addProfile(profile);
        d->colorConversionSystem->insertColorProfile(profile);
    }

    KoColorSpaceRegistry::Private *d;
};

const KoColorProfile* KoColorSpaceRegistry::createColorProfile(const QString& colorModelId, const QString& colorDepthId, const QByteArray& rawData)
{
    QWriteLocker l(&d->registrylock);
    KoColorSpaceFactory* factory_ = d->colorSpaceFactoryRegistry.get(d->colorSpaceIdImpl(colorModelId, colorDepthId));

    Private::ProfileRegistrationInterface interface(d);
    return factory_->colorProfile(rawData, &interface);
}

QList<const KoColorSpace*> KoColorSpaceRegistry::allColorSpaces(ColorSpaceListVisibility visibility, ColorSpaceListProfilesSelection pSelection)
{
    QList<const KoColorSpace*> colorSpaces;

    // TODO: thread-unsafe code: the factories might change right after the lock in released
    // HINT: used in a unittest only!

    d->registrylock.lockForRead();
    QList<KoColorSpaceFactory*> factories = d->colorSpaceFactoryRegistry.values();
    d->registrylock.unlock();

    Q_FOREACH (KoColorSpaceFactory* factory, factories) {
        // Don't test with ycbcr for now, since we don't have a default profile for it.
        if (factory->colorModelId().id().startsWith("Y")) continue;
        if (visibility == AllColorSpaces || factory->userVisible()) {
            if (pSelection == OnlyDefaultProfile) {
                const KoColorSpace *cs = d->colorSpace1(factory->id());
                if (cs) {
                    colorSpaces.append(cs);
                }
                else {
                    warnPigment << "Could not create colorspace for id" << factory->id() << "since there is no working default profile";
                }
            } else {
                QList<const KoColorProfile*> profiles = KoColorSpaceRegistry::instance()->profilesFor(factory->id());
                Q_FOREACH (const KoColorProfile * profile, profiles) {
                    const KoColorSpace *cs = d->colorSpace1(factory->id(), profile);
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
