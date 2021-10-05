/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoResourceCachePrefixedStorageWrapper.h"

#include <QVariant>


KoResourceCachePrefixedStorageWrapper::KoResourceCachePrefixedStorageWrapper(const QString &prefix, KoResourceCacheInterfaceSP baseInterface)
    : m_prefix(prefix),
      m_baseInterface(baseInterface)
{
}

QVariant KoResourceCachePrefixedStorageWrapper::fetch(const QString &key) const
{
    return m_baseInterface->fetch(m_prefix + key);
}

void KoResourceCachePrefixedStorageWrapper::put(const QString &key, const QVariant &value)
{
    m_baseInterface->put(m_prefix + key, value);
}
