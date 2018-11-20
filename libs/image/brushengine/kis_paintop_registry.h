/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

    /**
     * Create and return an (abstracted) configuration widget
     * for using the specified paintop with the specified input device,
     * with the specified parent as widget parent. Returns 0 if there
     * are no settings available for the given device.
     */
    KisPaintOpSettingsSP createSettings(const KoID& id) const;

    /**
     * @return a default preset for the given paintop.
     */
    KisPaintOpPresetSP defaultPreset(const KoID& id) const;

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

