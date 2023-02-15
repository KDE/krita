/*
 *  SPDX-FileCopyrightText: 2008, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <kis_curve_paintop_settings_widget.h>
#include <kis_properties_configuration.h>
#include <kis_curve_paintop_settings.h>

#include <KisPaintingModeOptionWidget.h>
#include <KisCurveOpOptionWidget.h>
#include <KisPaintOpOptionWidgetUtils.h>
#include <KisCompositeOpOptionWidget.h>
#include <KisStandardOptionData.h>
#include <KisCurveStandardOptionData.h>

KisCurvePaintOpSettingsWidget:: KisCurvePaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    namespace kpowu = KisPaintOpOptionWidgetUtils;

    addPaintOpOption(kpowu::createOptionWidget<KisCurveOpOptionWidget>());
    addPaintOpOption(kpowu::createOpacityOptionWidget());
    addPaintOpOption(kpowu::createCurveOptionWidget(KisLineWidthOptionData(), KisPaintOpOption::GENERAL, i18n("0%"), i18n("100%")));
    addPaintOpOption(kpowu::createCurveOptionWidget(KisCurvesOpacityOptionData(), KisPaintOpOption::GENERAL, i18n("0%"), i18n("100%")));
    addPaintOpOption(kpowu::createOptionWidget<KisCompositeOpOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisPaintingModeOptionWidget>());
}

KisCurvePaintOpSettingsWidget::~ KisCurvePaintOpSettingsWidget()
{
}


KisPropertiesConfigurationSP  KisCurvePaintOpSettingsWidget::configuration() const
{
    KisCurvePaintOpSettings* config = new KisCurvePaintOpSettings(resourcesInterface());
    config->setProperty("paintop", "curvebrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

