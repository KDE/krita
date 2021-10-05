/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoResourceCacheStorage.h"

#include <QHash>
#include <QString>
#include <QVariant>

#include "kis_assert.h"
#include "kis_debug.h"

struct KoResourceCacheStorage::Private
{
    QHash<QString, QVariant> map;
};

KoResourceCacheStorage::KoResourceCacheStorage()
    : m_d(new Private)
{
}

KoResourceCacheStorage::~KoResourceCacheStorage()
{
}

QVariant KoResourceCacheStorage::fetch(const QString &key) const
{
    return m_d->map.value(key, QVariant());
}

void KoResourceCacheStorage::put(const QString &key, const QVariant &value)
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(!m_d->map.contains(key));
    m_d->map.insert(key, value);
}
