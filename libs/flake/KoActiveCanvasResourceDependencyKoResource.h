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
#ifndef KOACTIVECANVASRESOURCEDEPENDENCYKORESOURCE_H
#define KOACTIVECANVASRESOURCEDEPENDENCYKORESOURCE_H

#include <KoActiveCanvasResourceDependency.h>
#include <QVariant>

/**
 * A canvas resource dependency for KoResource-based canvas resources. It relies
 * on the presence of KoResource::requiredCanvasResources().
 */
template <typename ResourceType>
class KoActiveCanvasResourceDependencyKoResource : public KoActiveCanvasResourceDependency
{
    using ResourceTypeSP = QSharedPointer<ResourceType>;

public:
    KoActiveCanvasResourceDependencyKoResource(int sourceKey, int targetKey)
        : KoActiveCanvasResourceDependency(sourceKey, targetKey)
    {
    }

    bool shouldUpdateSource(QVariant &source, const QVariant &target) override
    {
        Q_UNUSED(target);

        bool needsResourceUpdate = false;

        ResourceTypeSP sourceResource = source.value<ResourceTypeSP>();

        if (sourceResource && sourceResource->requiredCanvasResources().contains(targetKey())) {
            needsResourceUpdate = true;
        }

        return needsResourceUpdate;
    }
};

#endif // KOACTIVECANVASRESOURCEDEPENDENCYKORESOURCE_H
