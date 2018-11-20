/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_colorsmudgeop_settings.h"

#include "kis_smudge_option.h"

struct KisColorSmudgeOpSettings::Private
{
    QList<KisUniformPaintOpPropertyWSP> uniformProperties;
};

KisColorSmudgeOpSettings::KisColorSmudgeOpSettings()
    : m_d(new Private)
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

