/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOSHAPEBULKACTIONLOCK_H
#define KOSHAPEBULKACTIONLOCK_H

#include "kritaflake_export.h"

#include <KisAdaptedLock.h>
#include <QList>
#include <QRect>

class KoShape;
class KoShapeBulkActionInterface;

class KRITAFLAKE_EXPORT KoShapeBulkActionLockAdapter {
public:
    using Update = std::pair<KoShape*, QRectF>;
    using UpdatesList = std::vector<Update>;
public:
    KoShapeBulkActionLockAdapter(const QList<KoShape*> &shapes);
    void lock();
    void unlock();

    UpdatesList takeFinalUpdatesList();

private:
    bool tryAddBulkInterfaceShape(KoShape *shape);
    void addBulkInterfaceDependees(const QList<KoShape*> dependees);

private:
    QList<KoShapeBulkActionInterface*> m_bulkInterfaceShapes;
    QList<KoShape*> m_normalUpdateShapes;
    std::unordered_map<KoShape*, QRectF> m_finalUpdates;
};

class KRITAFLAKE_EXPORT KoShapeBulkActionLock : protected KisAdaptedLock<KoShapeBulkActionLockAdapter>
{
public:
    using BaseClass = KisAdaptedLock<KoShapeBulkActionLockAdapter>;
    using BaseClass::BaseClass;

    ~KoShapeBulkActionLock();

    using BaseClass::Update;
    using BaseClass::UpdatesList;

    using BaseClass::lock;

    [[nodiscard]] UpdatesList unlock() {
        BaseClass::unlock();
        return KoShapeBulkActionLockAdapter::takeFinalUpdatesList();
    }

    using BaseClass::owns_lock;
    using BaseClass::swap;
    using BaseClass::release;
    using BaseClass::operator bool;
};

#endif /* KOSHAPEBULKACTIONLOCK_H */
