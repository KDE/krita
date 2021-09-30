/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2008, 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_DEFORM_PAINTOP_SETTINGS_H_
#define KIS_DEFORM_PAINTOP_SETTINGS_H_

#include <QScopedPointer>
#include <brushengine/kis_paintop_settings.h>
#include <kis_types.h>
#include <kis_outline_generation_policy.h>

class KisDeformPaintOpSettings : public KisOutlineGenerationPolicy<KisPaintOpSettings>
{

public:
    KisDeformPaintOpSettings(KisResourcesInterfaceSP resourcesInterface);
    ~KisDeformPaintOpSettings() override;

    void setPaintOpSize(qreal value) override;
    qreal paintOpSize() const override;

    QPainterPath brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom) override;

    bool paintIncremental() override;
    bool isAirbrushing() const override;

    QList<KisUniformPaintOpPropertySP> uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};
#endif
