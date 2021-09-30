/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ROUNDMARKEROP_SETTINGS_H
#define __KIS_ROUNDMARKEROP_SETTINGS_H

#include <QScopedPointer>
#include <kis_paintop_settings.h>
#include <kis_outline_generation_policy.h>


class KisRoundMarkerOpSettings : public KisOutlineGenerationPolicy<KisPaintOpSettings>
{
public:
    KisRoundMarkerOpSettings(KisResourcesInterfaceSP resourcesInterface);
    ~KisRoundMarkerOpSettings() override;

    bool paintIncremental() override;

    qreal paintOpSize() const override;
    void setPaintOpSize(qreal value) override;

    bool isAirbrushing() const override
    {
        return false;
    }

    qreal airbrushInterval() const override
    {
        return 1000.0;
    }

    QPainterPath brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom) override;

    QList<KisUniformPaintOpPropertySP> uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_ROUNDMARKEROP_SETTINGS_H */
