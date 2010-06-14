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
    QHash<QString, QList<KoColorSpace*> > availableColorspaces;
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
    foreach(const QList<KoColorSpace*>& cl, d->availableColorspaces) {
        count += cl.size();
    }
    for(QHash<KoColorSpace*, QString>::const_iterator it = d->stackInformation.begin();
        it != d->stackInformation.end(); ++it)
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

KoColorSpace* KoColorSpaceFactory::grabColorSpace(const KoColorProfile * profile)
{
    QMutexLocker l(&d->mutex);
    Q_ASSERT(profile);
    QList<KoColorSpace*>& csList = d->availableColorspaces[profile->name()];
    if (!csList.isEmpty()) {
        KoColorSpace* cs = csList.back();
        csList.pop_back();
        Q_ASSERT(!d->availableColorspaces[profile->name()].contains(cs));
        return cs;
    }
    KoColorSpace* cs = createColorSpace(profile);
    d->colorspaces.push_back(cs);
#ifndef NDEBUG
    d->stackInformation[cs] = kBacktrace();
#endif
    return cs;
}

void KoColorSpaceFactory::releaseColorSpace(KoColorSpace * colorspace)
{
    QMutexLocker l(&d->mutex);
    // TODO it is probably worth to avoid caching too many color spaces
    const KoColorProfile* profile = colorspace->profile();
    Q_ASSERT(d->colorspaces.contains(colorspace));
    Q_ASSERT(!d->availableColorspaces[profile->name()].contains(colorspace));
    d->availableColorspaces[profile->name()].push_back(colorspace);
#ifndef NDEBUG
    d->stackInformation.remove(colorspace);
#endif
}
