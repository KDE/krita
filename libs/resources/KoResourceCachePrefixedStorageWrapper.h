/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KORESOURCECACHEPREFIXEDSTORAGEWRAPPER_H
#define KORESOURCECACHEPREFIXEDSTORAGEWRAPPER_H

#include <KoResourceCacheInterface.h>
#include <QString>

/**
 * A simple wrapper class that converts all the passed cache keys
 * into a prefixed notation: "key" -> "prefix/key".
 *
 * This wrapper is usually needed when handing embedded objects
 * using the same cache storage, e.g. masking brush preset, which
 * is stored inside a normal preset under a prefix
 * ("MaskingBrush/Preset/")
 */
class KRITARESOURCES_EXPORT KoResourceCachePrefixedStorageWrapper : public KoResourceCacheInterface
{
public:
    KoResourceCachePrefixedStorageWrapper(const QString &prefix, KoResourceCacheInterfaceSP baseInterface);

    QVariant fetch(const QString &key) const override;
    void put(const QString &key, const QVariant &value) override;

private:
    QString m_prefix;
    KoResourceCacheInterfaceSP m_baseInterface;

};

#endif // KORESOURCECACHEPREFIXEDSTORAGEWRAPPER_H
