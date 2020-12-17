/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOACTIVECANVASRESOURCEDEPENDENCY_H
#define KOACTIVECANVASRESOURCEDEPENDENCY_H

#include <QScopedPointer>
#include <QSharedPointer>
#include "kritaflake_export.h"


/**
 * @brief A representation of dependency between different canvas
 * resources stored in KoReosurceManager.
 *
 * A resource dependency looks like that
 *
 *     source ----->----- depends on ----->----- target
 *
 * , that is, when 'target' changes 'source' should be updated.
 */
class KRITAFLAKE_EXPORT KoActiveCanvasResourceDependency
{
public:
    KoActiveCanvasResourceDependency(int sourceKey, int targetKey);
    virtual ~KoActiveCanvasResourceDependency();

    /**
     * @return the resource type that should be updated when targetKey() is changed
     */
    int sourceKey() const;

    /**
     * @return the resource type that should cause updates sourceKey()
     */
    int targetKey() const;

    /**
     * @return true if \p source does really depend on \p target and the
     * manager should emit notification about \p target's change
     */
    virtual bool shouldUpdateSource(QVariant &source, const QVariant &target) = 0;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

typedef QSharedPointer<KoActiveCanvasResourceDependency> KoActiveCanvasResourceDependencySP;

#endif // KOACTIVECANVASRESOURCEDEPENDENCY_H
