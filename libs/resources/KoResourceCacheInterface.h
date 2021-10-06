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


/**
 * @brief a provider-like interface class for sharing caches between multiple resources
 *
 * Some resources, e.g. KisPaintOpPreset, may require some computational-
 * intensive work to be done to start being useful. This work is not
 * serialized into the file format, so after we load the preset and
 * before we can start using it we need to spend some time on regenerating
 * some caches. For KisPaintOpPreset such caches are: KisQImagePyramid
 * and outline.
 *
 * Instead of generating this cache before every stroke we can
 * pregenerate it in advance and share among multiple presets.
 *
 * And here comes KoResourceCacheInterface. It provides abstract
 * interface for creating a cache and passing it into other presets.
 *
 * Usage:
 *
 *        \code{.cpp}
 *
 *        // in the GUI thread
 *        KisPaintOpPresetSP tempPreset =
 *            preset->cloneWithResourcesSnapshot(resourcesInterface, canvasResourcesInterface);
 *
 *        // in the non-GUI background thread [1]
 *        KoResourceCacheInterfaceSP cacheInterface =
 *           new KoResourceCacheStorage();
 *        tempPreset->regenerateResourceCache(cacheInterface);
 *
 *        // back in the GUI thread [2]
 *        preset->setResourceCacheInterface(cacheInterface);
 *
 *        // now the GUI-scope preset has all the caches prepared,
 *        // therefore, when we clone it to do real painting, it
 *        // will have all the caches ready
 *
 *        \endcode
 *
 * [1] - please take it into account that we cannot access the original
 *       `preset` from the non-GUI thread, because it may require resource
 *       database access, which is impossible in non-GUI thread. Therefore
 *       we must first create a copy of this resource with all the external
 *       resources linked to it, and only after that pass it to the non-gui
 *       thread
 *
 * [2] - there is also a cache-validity complication. We have generated the
 *       cache using a snapshot of a specific state of `resourcesInterface` and
 *       `canvasResourcesInterface`. Therefore, if any of these change, we
 *       must reset the cache manually! See code in KisPresetShadowUpdater
 *       for example implementation.
 *
 */
class KRITARESOURCES_EXPORT KoResourceCacheInterface
{
public:
    virtual ~KoResourceCacheInterface();

    /// fetch a cached object from the cache using \p key
    virtual QVariant fetch(const QString &key) const = 0;

    /// store a cached object \p value into the cache using \p key
    /// WARNING: storing an object twice with the same \p key is
    ///          considered as invalid operation and will assert!
    ///          This behavior is intentional to avoid cache key
    ///          aliasing.
    virtual void put(const QString &key, const QVariant &value) = 0;
};

using KoResourceCacheInterfaceSP = QSharedPointer<KoResourceCacheInterface>;

Q_DECLARE_METATYPE(KoResourceCacheInterfaceSP)

#endif // KORESOURCECACHEINTERFACE_H
