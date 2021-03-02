/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PAINTOP_REGISTRY_H_
#define KIS_PAINTOP_REGISTRY_H_

#include <QObject>

#include "KoGenericRegistry.h"

#include "kis_paintop.h"
#include <brushengine/kis_paintop_factory.h>
#include "kis_types.h"
#include <brushengine/kis_paintop_settings.h>
#include <brushengine/kis_paintop_preset.h>
#include <kis_threaded_text_rendering_workaround.h>

#include <kritaimage_export.h>


class KisPaintOp;
class KisPainter;

/**
 * Manages the loading and creating of all paintop plugins.
 */
class KRITAIMAGE_EXPORT KisPaintOpRegistry : public QObject, public KoGenericRegistry<KisPaintOpFactory*>
{

    Q_OBJECT

public:
    KisPaintOpRegistry();
    ~KisPaintOpRegistry() override;

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND
    void preinitializePaintOpIfNeeded(const KisPaintOpPresetSP preset);
#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */

    /**
     * Create and return a paintop based on the given preset. A preset defines
     * a paintop, a settings object and possible a brush tip.
     */
    KisPaintOp* paintOp(const KisPaintOpPresetSP preset, KisPainter * painter, KisNodeSP node, KisImageSP image) const;

    KisInterstrokeDataFactory* createInterstrokeDataFactory(KisPaintOpPresetSP preset) const;

    /**
     * Create and return an (abstracted) configuration widget
     * for using the specified paintop with the specified input device,
     * with the specified parent as widget parent. Returns 0 if there
     * are no settings available for the given device.
     */
    KisPaintOpSettingsSP createSettings(const KoID& id, KisResourcesInterfaceSP resourcesInterface) const;

    /**
     * @return a default preset for the given paintop.
     */
    KisPaintOpPresetSP defaultPreset(const KoID& id, KisResourcesInterfaceSP resourcesInterface) const;

    // Get the icon to show in the user interface
    QIcon icon(const KoID & id) const;

    /**
     * This function return a list of all the keys in KoID format by using the name() method
     * on the objects stored in the registry.
     */
    QList<KoID> listKeys() const;

public:

    static KisPaintOpRegistry* instance();

private:

    KisPaintOpRegistry(const KisPaintOpRegistry&);
    KisPaintOpRegistry operator=(const KisPaintOpRegistry&);
    void initRegistry();


    // So the settings can get a paintop to render their sample image
    friend class KisPaintOpSettings;

    /**
     * Return a newly created paintop. You are responsible for deleting
     */
    KisPaintOp * paintOp(const QString& id, const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image) const;

};

#endif // KIS_PAINTOP_REGISTRY_H_

