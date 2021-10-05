/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KORESOURCECACHEINTERFACE_H
#define KORESOURCECACHEINTERFACE_H

#include "kritaresources_export.h"

#include <QSharedPointer>

class QString;
class QVariant;


class KRITARESOURCES_EXPORT KoResourceCacheInterface
{
public:
    virtual ~KoResourceCacheInterface();

    virtual QVariant fetch(const QString &key) const = 0;
    virtual void put(const QString &key, const QVariant &value) = 0;
};

using KoResourceCacheInterfaceSP = QSharedPointer<KoResourceCacheInterface>;

Q_DECLARE_METATYPE(KoResourceCacheInterfaceSP)

#endif // KORESOURCECACHEINTERFACE_H
