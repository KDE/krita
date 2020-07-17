/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
