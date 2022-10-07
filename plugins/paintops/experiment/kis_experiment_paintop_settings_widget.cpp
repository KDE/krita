/*
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_experiment_paintop_settings_widget.h"

#include "kis_experimentop_option.h"
#include "kis_experiment_paintop_settings.h"

#include <kis_color_option.h>
#include <kis_paintop_settings_widget.h>

#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_curve_option_widget.h>
#include <KisPaintOpOptionUtils.h>
#include <KisCompositeOpOptionWidget.h>

KisExperimentPaintOpSettingsWidget:: KisExperimentPaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    namespace kpou = KisPaintOpOptionUtils;

    addPaintOpOption(new KisExperimentOpOption());
    addPaintOpOption(kpou::createOptionWidget<KisCompositeOpOptionWidget>());
    //addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption(), i18n("Transparent"), i18n("Opaque")), i18n("Opacity"));
}

KisExperimentPaintOpSettingsWidget::~ KisExperimentPaintOpSettingsWidget()
{
}

KisPropertiesConfigurationSP  KisExperimentPaintOpSettingsWidget::configuration() const
{
    KisExperimentPaintOpSettings* config = new KisExperimentPaintOpSettings(resourcesInterface());
    config->setProperty("paintop", "experimentbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

