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
#include "KisPaintOpPresetUpdateProxy.h"
#include "kis_curve_option_uniform_property.h"
#include "kis_smudge_radius_option.h"
#include "kis_pressure_paint_thickness_option.h"

QList<KisUniformPaintOpPropertySP> KisColorSmudgeOpSettings::uniformProperties(KisPaintOpSettingsSP settings, QPointer<KisPaintOpPresetUpdateProxy> updateProxy)
{
    QList<KisUniformPaintOpPropertySP> props =
        listWeakToStrong(m_d->uniformProperties);

    if (props.isEmpty()) {
        {
            KisComboBasedPaintOpPropertyCallback *prop = new KisComboBasedPaintOpPropertyCallback(KoID("smudge_mode", i18n("Smudge Mode")), settings, 0);

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

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }

        {
            KisCurveOptionUniformProperty *prop =
                new KisCurveOptionUniformProperty(
                    "smudge_length",
                    new KisSmudgeOption(),
                    settings, 0);

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
        {
            KisCurveOptionUniformProperty *prop =
                new KisCurveOptionUniformProperty(
                    "smudge_radius",
                    new KisSmudgeRadiusOption(),
                    settings, 0);

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }

        {
            KisCurveOptionUniformProperty *prop =
                new KisCurveOptionUniformProperty("smudge_color_rate",
                                                  new KisRateOption(KoID("ColorRate", i18n("Color Rate")), KisPaintOpOption::GENERAL, false),
                                                  settings,
                                                  0);

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }

        {
            KisUniformPaintOpPropertyCallback *prop =
                new KisUniformPaintOpPropertyCallback(KisUniformPaintOpPropertyCallback::Bool, KoID("smudge_smear_alpha", i18n("Smear Alpha")), settings, 0);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisSmudgeOption option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(option.getSmearAlpha());
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisSmudgeOption option;
                    option.readOptionSetting(prop->settings().data());
                    option.setSmearAlpha(prop->value().toBool());
                    option.writeOptionSetting(prop->settings().data());
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }

        {
            KisCurveOptionUniformProperty *prop =
                new KisCurveOptionUniformProperty(
                    "smudge_paint_thickness_rate",
                    new KisPressurePaintThicknessOption(),
                    settings, 0);

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }

        {
            KisComboBasedPaintOpPropertyCallback *prop =
                new KisComboBasedPaintOpPropertyCallback(KoID("smudge_paint_thickness_mode", i18n("Paint Thickness Mode")), settings, 0);

            QList<QString> modes;
            modes << i18n("Overwrite");
            modes << i18n("Paint over");

            prop->setItems(modes);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisPressurePaintThicknessOption option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(int(option.getThicknessMode()) - 1);
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisPressurePaintThicknessOption option;
                    option.readOptionSetting(prop->settings().data());
                    option.setThicknessMode(KisPressurePaintThicknessOption::ThicknessMode(prop->value().toInt() + 1));
                    option.writeOptionSetting(prop->settings().data());
                });

            QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
    }

    return KisBrushBasedPaintOpSettings::uniformProperties(settings, updateProxy) + props;
}

