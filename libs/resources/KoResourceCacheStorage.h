/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KORESOURCECACHESTORAGE_H
#define KORESOURCECACHESTORAGE_H

#include <KoResourceCacheInterface.h>
#include <QScopedPointer>

class KRITARESOURCES_EXPORT KoResourceCacheStorage : public KoResourceCacheInterface
{
public:
    KoResourceCacheStorage();
    ~KoResourceCacheStorage();

    QVariant fetch(const QString &key) const override;
    void put(const QString &key, const QVariant &value) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KORESOURCECACHESTORAGE_H
