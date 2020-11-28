/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_colorsmudgeop_settings.h"

#include "kis_smudge_option.h"

struct KisColorSmudgeOpSettings::Private
{
    QList<KisUniformPaintOpPropertyWSP> uniformProperties;
};

KisColorSmudgeOpSettings::KisColorSmudgeOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisBrushBasedPaintOpSettings(resourcesInterface),
      m_d(new Private)
{
}

KisColorSmudgeOpSettings::~KisColorSmudgeOpSettings()
{
}

#include <brushengine/kis_slider_based_paintop_property.h>
#include <brushengine/kis_combo_based_paintop_property.h>
#include "kis_paintop_preset.h"
#include "kis_paintop_settings_update_proxy.h"
#include "kis_curve_option_uniform_property.h"
#include "kis_smudge_radius_option.h"

QList<KisUniformPaintOpPropertySP> KisColorSmudgeOpSettings::uniformProperties(KisPaintOpSettingsSP settings)
{
    QList<KisUniformPaintOpPropertySP> props =
        listWeakToStrong(m_d->uniformProperties);

    if (props.isEmpty()) {
        {
            KisComboBasedPaintOpPropertyCallback *prop =
                new KisComboBasedPaintOpPropertyCallback(
                    "smudge_mode",
                    i18n("Smudge Mode"),
                    settings, 0);

            QList<QString> modes;
            modes << i18n("Smearing");
            modes << i18n("Dulling");

            prop->setItems(modes);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisSmudgeOption option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(int(option.getMode()));
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisSmudgeOption option;
                    option.readOptionSetting(prop->settings().data());
                    option.setMode(KisSmudgeOption::Mode(prop->value().toInt()));
                    option.writeOptionSetting(prop->settings().data());
                });

            QObject::connect(updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }

        {
            KisCurveOptionUniformProperty *prop =
                new KisCurveOptionUniformProperty(
                    "smudge_length",
                    new KisSmudgeOption(),
                    settings, 0);

            QObject::connect(updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
        {
            KisCurveOptionUniformProperty *prop =
                new KisCurveOptionUniformProperty(
                    "smudge_radius",
                    new KisSmudgeRadiusOption(),
                    settings, 0);

            QObject::connect(updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }

        {
            KisCurveOptionUniformProperty *prop =
                new KisCurveOptionUniformProperty(
                    "smudge_color_rate",
                    new KisRateOption("ColorRate", KisPaintOpOption::GENERAL, false),
                    settings, 0);

            QObject::connect(updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
    }

    return KisBrushBasedPaintOpSettings::uniformProperties(settings) + props;
}

