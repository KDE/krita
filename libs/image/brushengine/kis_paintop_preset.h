/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KIS_PAINTOP_PRESET_H
#define KIS_PAINTOP_PRESET_H

#include <QPointer>

#include <KoResource.h>
#include "KoID.h"

#include "kis_types.h"
#include "kis_shared.h"
#include "kritaimage_export.h"
#include <brushengine/kis_uniform_paintop_property.h>
#include <KisPaintOpPresetUpdateProxy.h>

class KoCanvasResourcesInterface;
using KoCanvasResourcesInterfaceSP = QSharedPointer<KoCanvasResourcesInterface>;

class KoResourceCacheInterface;
using KoResourceCacheInterfaceSP = QSharedPointer<KoResourceCacheInterface>;

/**
 * A KisPaintOpPreset contains a particular set of settings
 * associated with a paintop, like brush, paintopsettings.
 * A new property in this class is to make it dirty. That means the
 * user can now temporarily save any tweaks in the Preset throughout
 * the session. The Dirty Preset setting/unsetting is handled
 * by KisPaintOpPresetSettings
 *
 * KisPaintOpPreset is a **serialized** representation. That is,
 * it stores only "metadata" needed to paint with a paintop. This
 * metadata can be used to create painting c++-objects (e.g.
 * brushes) or caches or embedded resources, **but** these objects
 * are never stored inside the preset itself.
 *
 * It is done intentionally, because our GUI elements work with
 * "metadata" only. They just read/write strings into the XML
 * document, they don't know anything about the internal structure
 * of the preset's C++ object. Therefore, if we stored any caches/
 * resources inside the preset, they would become out-of-sync with
 * the XML representation quite easily.
 *
 * To overcome this issue, we use a set of provider-like
 * interfaces to store caches/resources outside the preset:
 *
 * 1) KisResourcesInterface is the interface that lets the preset
 *    to fetch dependent resources from the cenralized storage.
 *    Theoretically, the preset could create dependent resources
 *    from its XML representation internally all the time, but
 *    there are two reasond against that:
 *
 *       1) That is inefficient to load the brushes and/or textures
 *          for every stroke. It can create significant delay in the
 *          beginning of every stroke.
 *
 *       2) When we show an embedded resource in the GUI, it would
 *          be nice to know the resource's 'resourceId', so we could
 *          display it correctly. If we load the embedded resource
 *          every time, 'resourceId' field would be empty.
 *
 *    Abstract KisResourcesInterface also allows us to replace a link
 *    to the centralized storage with a snapshot of cloned resources,
 *    when we start rendering data in the non-gui thread.
 *
 * 2) KoCanvasResourcesInterface is the interface that lets the preset
 *    link to the global storage of "canvas resources", like current
 *    gradient, current fg color and current bg color.
 *
 *    When we start a stroke, we make a snapshot of the current state
 *    of the required canvas resources and replace the link to the
 *    global storage with the created local snapshot.
 *
 *    Please take it into account that some of the embedded/linked
 *    resources privided by KisResourcesInterface may **change**
 *    while cloning. It happens when these resources "bake" the state
 *    of canvas resources into themselves (e.g. gradient may change
 *    the "fg-stop" with real fg color).
 *
 * 3) KoResourceCacheInterface is an interface that provides a storage
 *    for the internal preset's caches (e.g. a brush with a pregenerated
 *    pyramid or a pregenerated template for a rotated pattern).
 *
 *    The cache interface link is reset when any change is made to
 *    the preset's metadata (which, effectively, resets all the caches).
 *
 *    The cache link should also be reset manually if the preset
 *    depends on some canvas resource (e.g. fg-color) and this
 *    canvas resource has changed. That is tracked by
 *    KisPresetShadowUpdater.
 */

class KRITAIMAGE_EXPORT KisPaintOpPreset : public KoResource
{
public:

    /**
     * @brief The UpdatedPostponer class
     * @see KisPaintOpPresetUpdateProxy::postponeSettingsChanges()
     */
    class KRITAIMAGE_EXPORT UpdatedPostponer{
    public:
        UpdatedPostponer(KisPaintOpPresetSP preset);

        ~UpdatedPostponer();

    private:
        QPointer<KisPaintOpPresetUpdateProxy> m_updateProxy;
    };

public:

    KisPaintOpPreset();

    KisPaintOpPreset(const QString& filename);

    ~KisPaintOpPreset() override;

    KisPaintOpPreset(const KisPaintOpPreset &rhs);
    KisPaintOpPreset &operator=(const KisPaintOpPreset &rhs) = delete;
    KoResourceSP clone() const override;

    /// set the id of the paintop plugin
    void setPaintOp(const KoID & paintOp);

    /// return the id of the paintop plugin
    KoID paintOp() const;

    QString name() const override;

    /// replace the current settings object with the specified settings
    void setSettings(KisPaintOpSettingsSP settings);

    /// return the settings that define this paintop preset
    KisPaintOpSettingsSP settings() const;

    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;

    bool saveToDevice(QIODevice* dev) const override;

    QPair<QString, QString> resourceType() const override
    {
        return QPair<QString, QString>(ResourceType::PaintOpPresets, "");
    }

    void updateLinkedResourcesMetaData(KisResourcesInterfaceSP resourcesInterface) override;

    void toXML(QDomDocument& doc, QDomElement& elt) const;

    void fromXML(const QDomElement& elt, KisResourcesInterfaceSP resourcesInterface);

    bool removable() const {
        return true;
    }

    QString defaultFileExtension() const override {
        return ".kpp";
    }

    QPointer<KisPaintOpPresetUpdateProxy> updateProxy() const;
    QPointer<KisPaintOpPresetUpdateProxy> updateProxyNoCreate() const;

    QList<KisUniformPaintOpPropertySP> uniformProperties();

    /**
     * @return true if this preset demands a secondary masked brush running
     *         alongside it
     */
    bool hasMaskingPreset() const;

    /**
     * @return a newly created preset of the masked brush that should be run
     *         alongside the current brush
     */
    KisPaintOpPresetSP createMaskingPreset() const;

    /**
     * @return resource interface that is used by KisPaintOpSettings object for
     * loading linked resources
     */
    KisResourcesInterfaceSP resourcesInterface() const;

    /**
     * Set resource interface that will be used by KisPaintOpSettings object for
     * loading linked resources
     */
    void setResourcesInterface(KisResourcesInterfaceSP resourcesInterface);

    /**
     * Returns canvas resources interface associated with the current preset.
     *
     * In contrast to resourcesInterface() the canvas resources interface may
     * be null, because the preset is created without any canvas resources.
     * The resources are assigned to the preset only when the ser starts to
     * paint with it.
     *
     * The preset has no default canvas resources interface, because canvas
     * resources are unique per-canvas, but the presets are unique per-
     * application. Therefore association between the preset and canvas
     * resources interface would be ambiguous.
     */
    KoCanvasResourcesInterfaceSP canvasResourcesInterface() const;

    /**
     * Sets canvas resources interface used for initializing the preset
     *
     * @see canvasResourcesInterface()
     */
    void setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface);

    /**
     * \see KisRequiredResourcesOperators::createLocalResourcesSnapshot
     */
    void createLocalResourcesSnapshot(KisResourcesInterfaceSP globalResourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface);

    /**
     * \see KisRequiredResourcesOperators::hasLocalResourcesSnapshot
     */
    bool hasLocalResourcesSnapshot() const;

    /**
     * \see KisRequiredResourcesOperators::cloneWithResourcesSnapshot
     */
    KisPaintOpPresetSP cloneWithResourcesSnapshot(KisResourcesInterfaceSP globalResourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface) const;


    QList<KoResourceLoadResult> linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const override;

    QList<KoResourceLoadResult> embeddedResources(KisResourcesInterfaceSP globalResourcesInterface) const override;

    QList<int> requiredCanvasResources() const override;

    /**
     * Set resource cache object generated for this preset (or its
     * clone). After calling this method with non-null cache interface
     * all the internal caches of this preset will be initialized.
     * The cache interface itself will be stored inside the preset
     * and will be preserved while cloning the preset.
     *
     * Changing any property of the preset will reset both, internal
     * caches and cache interface to null.
     *
     * Calling this method with nullptr resets internal caches. It is
     * needed, e.g. when some canvas resources have changed (\see
     * requiredCanvasResources()).
     */
    void setResourceCacheInterface(KoResourceCacheInterfaceSP cacheInterface);

    /**
     * Return the saved cache interface or null if not available
     */
    KoResourceCacheInterfaceSP resourceCacheInterface() const;

    /**
     * Recalculates all the internal caches and stores them in the cache
     * interface.
     */
    void regenerateResourceCache(KoResourceCacheInterfaceSP cacheInterface);

private:

    struct Private;
    Private * const d;
};

Q_DECLARE_METATYPE(KisPaintOpPresetSP)

#endif
