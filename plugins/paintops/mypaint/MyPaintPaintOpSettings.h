/*
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MY_PAINTOP_SETTINGS_H_
#define KIS_MY_PAINTOP_SETTINGS_H_

#include <QScopedPointer>

#include <brushengine/kis_no_size_paintop_settings.h>
#include <kis_types.h>

#include <kis_outline_generation_policy.h>
#include "MyPaintPaintOpSettingsWidget.h"


class KisMyPaintOpSettings : public KisOutlineGenerationPolicy<KisPaintOpSettings>
{
public:
    KisMyPaintOpSettings(KisResourcesInterfaceSP resourcesInterface);
    ~KisMyPaintOpSettings() override;

    void setPaintOpSize(qreal value) override;
    qreal paintOpSize() const override;

    void setPaintOpOpacity(qreal value) override;
    qreal paintOpOpacity() override;

    QPainterPath brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom) override;

    QString modelName() const override {
        return "airbrush";
    }

    bool paintIncremental() override;

private:
    Q_DISABLE_COPY(KisMyPaintOpSettings)

    struct Private;
    const QScopedPointer<Private> m_d;

};

typedef KisSharedPtr<KisMyPaintOpSettings> KisMyPaintOpSettingsSP;

#endif
