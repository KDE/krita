/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "KoColorSpaceFactory.h"

#include "DebugPigment.h"

#include <QMutexLocker>

#include "KoColorProfile.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"

struct KoColorSpaceFactory::Private {
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
        errorPigment << it.key()->id() << " still in used, and grabed in: ";
        errorPigment << it.value();
    }
    if( count != d->colorspaces.size())
    {
        errorPigment << (d->colorspaces.size() - count) << " colorspaces are still used";
    }
    Q_ASSERT(count == d->colorspaces.size());
#endif
    foreach(KoColorSpace* cs, d->colorspaces) {
        delete cs;
    }
    foreach(KoColorProfile* profile, d->colorprofiles) {
        KoColorSpaceRegistry::instance()->removeProfile(profile);
        delete profile;
    }
    delete d;
}

const KoColorProfile* KoColorSpaceFactory::colorProfile(const QByteArray& rawData) const
{
    KoColorProfile* colorProfile = createColorProfile(rawData);
    if (colorProfile && colorProfile->valid()) {
        if (const KoColorProfile* existingProfile = KoColorSpaceRegistry::instance()->profileByName(colorProfile->name())) {
            delete colorProfile;
            return existingProfile;
        }
        KoColorSpaceRegistry::instance()->addProfile(colorProfile);
        d->colorprofiles.append(colorProfile);
    }
    return colorProfile;
}

const KoColorSpace *KoColorSpaceFactory::grabColorSpace(const KoColorProfile * profile)
{
    QMutexLocker l(&d->mutex);
    Q_ASSERT(profile);
    KoColorSpace* cs = 0;
    if (!d->availableColorspaces.contains(profile->name())) {
        cs = createColorSpace(profile);
        d->availableColorspaces[profile->name()] = cs;
    } else {
        cs = d->availableColorspaces[profile->name()];
    }
    return cs;
}

