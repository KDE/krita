/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisBrushOpSettings.h"

struct KisBrushOpSettings::Private
{
    QList<KisUniformPaintOpPropertyWSP> uniformProperties;
};

KisBrushOpSettings::KisBrushOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisBrushBasedPaintOpSettings(resourcesInterface),
      m_d(new Private)
{
}

KisBrushOpSettings::~KisBrushOpSettings()
{
}

bool KisBrushOpSettings::needsAsynchronousUpdates() const
{
    return true;
}

#include "kis_paintop_preset.h"
#include "KisPaintOpPresetUpdateProxy.h"
#include "kis_curve_option_uniform_property.h"
#include "kis_pressure_lightness_strength_option.h"

QList<KisUniformPaintOpPropertySP> KisBrushOpSettings::uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy)
{
    QList<KisUniformPaintOpPropertySP> props = listWeakToStrong(m_d->uniformProperties);

    if (props.isEmpty()) {
        {
            KisCurveOptionUniformProperty *prop =
                new KisCurveOptionUniformProperty(
                    "lightness_strength",
                    new KisPressureLightnessStrengthOption(),
                    settings, 0);

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
    }

    return KisBrushBasedPaintOpSettings::uniformProperties(settings, updateProxy) + props;
}
