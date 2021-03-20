/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoColorProfileStorage.h"

#include <QHash>
#include <QReadWriteLock>
#include <QString>

#include "KoColorSpaceFactory.h"
#include "KoColorProfile.h"
#include "kis_assert.h"


struct KoColorProfileStorage::Private {
    QHash<QString, KoColorProfile * > profileMap;
    QHash<QByteArray, KoColorProfile * > profileUniqueIdMap;
    QHash<QString, QString> profileAlias;
    QReadWriteLock lock;

    void populateUniqueIdMap();
};

KoColorProfileStorage::KoColorProfileStorage()
    : d(new Private)
{

}

KoColorProfileStorage::~KoColorProfileStorage()
{
}

void KoColorProfileStorage::addProfile(KoColorProfile *profile)
{
    QWriteLocker locker(&d->lock);

    if (profile->valid()) {
        d->profileMap[profile->name()] = profile;
        if (!d->profileUniqueIdMap.isEmpty()) {
            d->profileUniqueIdMap.insert(profile->uniqueId(), profile);
        }
    }
}

void KoColorProfileStorage::addProfile(const KoColorProfile *profile)
{
    addProfile(profile->clone());
}

void KoColorProfileStorage::removeProfile(KoColorProfile *profile)
{
    QWriteLocker locker(&d->lock);

    d->profileMap.remove(profile->name());
    if (!d->profileUniqueIdMap.isEmpty()) {
        d->profileUniqueIdMap.remove(profile->uniqueId());
    }
}

bool KoColorProfileStorage::containsProfile(const KoColorProfile *profile)
{
    QReadLocker l(&d->lock);
    return d->profileMap.contains(profile->name());
}

void KoColorProfileStorage::addProfileAlias(const QString &name, const QString &to)
{
    QWriteLocker l(&d->lock);
    d->profileAlias[name] = to;
}

QString KoColorProfileStorage::profileAlias(const QString &name) const
{
    QReadLocker l(&d->lock);
    return d->profileAlias.value(name, name);
}

const KoColorProfile *KoColorProfileStorage::profileByName(const QString &name) const
{
    QReadLocker l(&d->lock);
    return d->profileMap.value(d->profileAlias.value(name, name), 0);
}

void KoColorProfileStorage::Private::populateUniqueIdMap()
{
    QWriteLocker l(&lock);
    profileUniqueIdMap.clear();

    for (auto it = profileMap.constBegin();
         it != profileMap.constEnd();
         ++it) {

        KoColorProfile *profile = it.value();
        QByteArray id = profile->uniqueId();

        if (!id.isEmpty()) {
            profileUniqueIdMap.insert(id, profile);
        }
    }
}


const KoColorProfile *KoColorProfileStorage::profileByUniqueId(const QByteArray &id) const
{
    QReadLocker l(&d->lock);
    if (d->profileUniqueIdMap.isEmpty()) {
        l.unlock();
        d->populateUniqueIdMap();
        l.relock();
    }
    return d->profileUniqueIdMap.value(id, 0);

}

QList<const KoColorProfile *> KoColorProfileStorage::profilesFor(const KoColorSpaceFactory *csf) const
{
    QList<const KoColorProfile *>  profiles;
    if (!csf) return profiles;

    QReadLocker l(&d->lock);

    QHash<QString, KoColorProfile * >::ConstIterator it;
    for (it = d->profileMap.constBegin(); it != d->profileMap.constEnd(); ++it) {
        KoColorProfile *  profile = it.value();
        if (csf->profileIsCompatible(profile)) {
            Q_ASSERT(profile);
            //         if (profile->colorSpaceSignature() == csf->colorSpaceSignature()) {
            profiles.push_back(profile);
        }
    }
    return profiles;
}
