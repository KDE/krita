/*
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_EXPERIMENT_PAINTOP_SETTINGS_H_
#define KIS_EXPERIMENT_PAINTOP_SETTINGS_H_

#include <brushengine/kis_no_size_paintop_settings.h>
#include <QScopedPointer>

class KisExperimentPaintOpSettings : public KisNoSizePaintOpSettings
{
public:
    KisExperimentPaintOpSettings(KisResourcesInterfaceSP resourcesInterface);
    ~KisExperimentPaintOpSettings() override;

    bool lodSizeThresholdSupported() const override;
    bool paintIncremental() override;
    QPainterPath brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom) override;

    QList<KisUniformPaintOpPropertySP> uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy) override;

private:

    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif
