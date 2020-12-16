/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KoColorSpaceFactory.h"

#include "DebugPigment.h"

#include <QMutexLocker>

#include "KoColorProfile.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"

#include "kis_assert.h"

struct Q_DECL_HIDDEN KoColorSpaceFactory::Private {
    QList<KoColorProfile*> colorprofiles;
    QList<KoColorSpace*> colorspaces;
    QHash<QString, KoColorSpace* > availableColorspaces;
    QMutex mutex;
#ifndef NDEBUG
    QHash<KoColorSpace*, QString> stackInformation;
#endif
};

KoColorSpaceFactory::KoColorSpaceFactory() : d(new Private)
{
}

KoColorSpaceFactory::~KoColorSpaceFactory()
{
#ifndef NDEBUG
    // Check that all color spaces have been released
    int count = 0;
    count += d->availableColorspaces.size();

    for(QHash<KoColorSpace*, QString>::const_iterator it = d->stackInformation.constBegin();
        it != d->stackInformation.constEnd(); ++it)
    {
        errorPigment << "*******************************************";
        errorPigment << it.key()->id() << " still in used, and grabbed in: ";
        errorPigment << it.value();
    }
    if( count != d->colorspaces.size())
    {
        errorPigment << (d->colorspaces.size() - count) << " colorspaces are still used";
    }
    Q_ASSERT(count == d->colorspaces.size());
#endif
    Q_FOREACH (KoColorSpace* cs, d->colorspaces) {
        delete cs;
    }
    Q_FOREACH (KoColorProfile* profile, d->colorprofiles) {
        KoColorSpaceRegistry::instance()->removeProfile(profile);
        delete profile;
    }
    delete d;
}

const KoColorProfile *KoColorSpaceFactory::colorProfile(const QByteArray &rawData, KoColorSpaceFactory::ProfileRegistrationInterface *registrationInterface) const
{
    KoColorProfile* colorProfile = createColorProfile(rawData);
    if (colorProfile && colorProfile->valid()) {
        if (const KoColorProfile* existingProfile = registrationInterface->profileByName(colorProfile->name())) {
            delete colorProfile;
            return existingProfile;
        }
        registrationInterface->registerNewProfile(colorProfile);
        d->colorprofiles.append(colorProfile);
    }
    return colorProfile;
}

const KoColorSpace *KoColorSpaceFactory::grabColorSpace(const KoColorProfile * profile)
{
    QMutexLocker l(&d->mutex);
    Q_ASSERT(profile);
    auto it = d->availableColorspaces.find(profile->name());
    KoColorSpace* cs;

    if (it == d->availableColorspaces.end()) {
        cs = createColorSpace(profile);
        KIS_ASSERT_X(cs != nullptr, "KoColorSpaceFactory::grabColorSpace", "createColorSpace returned nullptr.");
        if (cs) {
            d->availableColorspaces[profile->name()] = cs;
        }
    }
    else {
        cs = it.value();
    }

    return cs;
}

