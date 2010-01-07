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
#include "kis_paintop_factory.h"
#include "kis_types.h"
#include "kis_paintop_settings.h"
#include "kis_paintop_preset.h"

#include <krita_export.h>

class QWidget;

class KisPaintOp;
class KisPainter;
class KoColorSpace;
class KoInputDevice;

/**
 * Manages the loading and creating of all paintop plugins.
 */
class KRITAIMAGE_EXPORT KisPaintOpRegistry : public QObject, public KoGenericRegistry<KisPaintOpFactory*>
{

    Q_OBJECT

public:
    virtual ~KisPaintOpRegistry();

    /**
     * Create and return a paintop based on the given preset. A preset defines
     * a paintop, a settings object and possible a brush tip.
     */
    KisPaintOp* paintOp(const KisPaintOpPresetSP preset, KisPainter * painter, KisImageWSP image) const;

    /**
     * Create and return an (abstracted) configuration widget
     * for using the specified paintop with the specified input device,
     * with the specified parent as widget parent. Returns 0 if there
     * are no settings available for the given device.
     */
    KisPaintOpSettingsSP settings(const KoID& id, KisImageWSP image = 0) const;

    /**
     * @return a default preset for the given paintop.
     */
    KisPaintOpPresetSP defaultPreset(const KoID& id, KisImageWSP image = 0) const;

    // Whether we should show this paintop in the toolchest
    bool userVisible(const KoID & id, const KoColorSpace* cs) const;

    // Get the name of the icon to show in the toolchest
    QString pixmap(const KoID & id) const;

    /**
     * This function return a list of all the keys in KoID format by using the name() method
     * on the objects stored in the registry.
     */
    QList<KoID> listKeys() const;

public:

    static KisPaintOpRegistry* instance();

private:

    KisPaintOpRegistry();
    KisPaintOpRegistry(const KisPaintOpRegistry&);
    KisPaintOpRegistry operator=(const KisPaintOpRegistry&);


    // So the settings can get a paintop to render their sample image
    friend class KisPaintOpSettings;

    /**
     * Return a newly created paintop. You are responsible for deleting
     */
    KisPaintOp * paintOp(const QString& id, const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageWSP image = 0) const;

};

#endif // KIS_PAINTOP_REGISTRY_H_

