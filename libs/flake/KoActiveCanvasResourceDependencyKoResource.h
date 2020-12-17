/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
