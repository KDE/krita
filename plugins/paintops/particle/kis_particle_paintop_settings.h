/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PARTICLE_PAINTOP_SETTINGS_H_
#define KIS_PARTICLE_PAINTOP_SETTINGS_H_

#include <QScopedPointer>
#include <brushengine/kis_no_size_paintop_settings.h>
#include <kis_types.h>

class KisParticlePaintOpSettings : public KisNoSizePaintOpSettings
{

public:

    KisParticlePaintOpSettings(KisResourcesInterfaceSP resourcesInterface);
    ~KisParticlePaintOpSettings() override;

    bool lodSizeThresholdSupported() const override;
    bool paintIncremental() override;

    QList<KisUniformPaintOpPropertySP> uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif
