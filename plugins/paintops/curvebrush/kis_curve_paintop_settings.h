/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_CURVE_PAINTOP_SETTINGS_H_
#define KIS_CURVE_PAINTOP_SETTINGS_H_

#include <QScopedPointer>
#include <brushengine/kis_paintop_settings.h>

class KisCurvePaintOpSettings : public KisPaintOpSettings
{

public:
    KisCurvePaintOpSettings(KisResourcesInterfaceSP resourcesInterface);
    ~KisCurvePaintOpSettings() override;

    void setPaintOpSize(qreal value) override;
    qreal paintOpSize() const override;

    bool paintIncremental() override;

    QList<KisUniformPaintOpPropertySP> uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};
#endif
