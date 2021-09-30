/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISBRUSHOPSETTINGS_H
#define KISBRUSHOPSETTINGS_H

#include "kis_brush_based_paintop_settings.h"


class KisBrushOpSettings : public KisBrushBasedPaintOpSettings
{
public:
    KisBrushOpSettings(KisResourcesInterfaceSP resourcesInterface);
    ~KisBrushOpSettings();

    bool needsAsynchronousUpdates() const override;
    QList<KisUniformPaintOpPropertySP> uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISBRUSHOPSETTINGS_H
