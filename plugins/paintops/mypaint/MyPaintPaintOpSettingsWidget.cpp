/*
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "MyPaintPaintOpSettingsWidget.h"

#include <kis_airbrush_option_widget.h>
#include <kis_paintop_settings_widget.h>
#include <klocalizedstring.h>

#include "MyPaintCurveOption.h"
#include "MyPaintCurveOptionWidget.h"
#include "MyPaintPaintOpOption.h"
#include "MyPaintPaintOpSettings.h"

KisMyPaintOpSettingsWidget:: KisMyPaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    KisMyPaintOpOption *basicOption = new KisMyPaintOpOption();
    m_baseOption = basicOption;    

    KisMyPaintCurveOptionWidget *radiusOption = new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("radius_logarithmic", KisPaintOpOption::GENERAL, false, 2.0, 0.01, 6.0), "0", "100");
    m_radiusWidget = radiusOption;

    KisMyPaintCurveOptionWidget *hardnessOption = new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("hardness", KisPaintOpOption::GENERAL, false, 0.8, 0.02, 1), "0", "100");
    m_hardnessWidget = hardnessOption;

    KisMyPaintCurveOptionWidget *opacityOption = new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("opaque", KisPaintOpOption::GENERAL, false, 1, 0, 1), "0", "100");
    m_opacityWidget = opacityOption;

    addPaintOpOption(basicOption, i18nc("Option Category", "Basic"));
    addPaintOpOption(radiusOption, i18n("Radius Logarithmic"), i18nc("Option Category", "Basic"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("radius_by_random", KisPaintOpOption::GENERAL, false, 0.0, 0, 1.50), "0", "100"), i18n("Radius by Random"), i18nc("Option Category", "Basic"));
    addPaintOpOption(hardnessOption, i18n("Hardness"), i18nc("Option Category", "Basic"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("anti_aliasing", KisPaintOpOption::GENERAL, false, 0.0, 0, 1), "0", "100"), i18n("Anti Aliasing"), i18nc("Option Category", "Basic"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("elliptical_dab_angle", KisPaintOpOption::GENERAL, false, 0.0, 0, 180), "0", "100"), i18n("Elliptical Dab Angle"), i18nc("Option Category", "Basic"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("elliptical_dab_ratio", KisPaintOpOption::GENERAL, false, 1, 1, 10), "0", "100"), i18n("Elliptical Dab Ratio"), i18nc("Option Category", "Basic"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("direction_filter", KisPaintOpOption::GENERAL, false, 2, 0, 10), "0", "100"), i18n("Direction Filter"), i18nc("Option Category", "Basic"));

    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("change_color_h", KisPaintOpOption::GENERAL, false, 0.0, -2, 2), "0", "100"), i18n("Change Color H"), i18nc("Option Category", "Color"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("change_color_l", KisPaintOpOption::GENERAL, false, 0.0, -2, 2), "0", "100"), i18n("Change Color L"), i18nc("Option Category", "Color"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("change_color_v", KisPaintOpOption::GENERAL, false, 0.0, -2, 2), "0", "100"), i18n("Change Color V"), i18nc("Option Category", "Color"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("change_color_hsl_s", KisPaintOpOption::GENERAL, false, 0.0, -2, 2), "0", "100"), i18n("Change Color HSL S"), i18nc("Option Category", "Color"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("change_color_hsv_s", KisPaintOpOption::GENERAL, false, 0.0, -2, 2), "0", "100"), i18n("Change Color HSV S"), i18nc("Option Category", "Color"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("colorize", KisPaintOpOption::GENERAL, false, 0.0, 0, 1), "0", "100"), i18n("Colorize"), i18nc("Option Category", "Color"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("speed1_gamma", KisPaintOpOption::GENERAL, false, 4, -8, 8), "0", "100"), i18n("Fine Speed Gamma"), i18nc("Option Category", "Speed"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("speed2_gamma", KisPaintOpOption::GENERAL, false, 4, -8, 8), "0", "100"), i18n("Gross Speed Gamma"), i18nc("Option Category", "Speed"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("speed1_slowness", KisPaintOpOption::GENERAL, false, 4, -8, 8), "0", "100"), i18n("Fine Speed Slowness"), i18nc("Option Category", "Speed"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("speed2_slowness", KisPaintOpOption::GENERAL, false, 4, -8, 8), "0", "100"), i18n("Gross Speed Slowness"), i18nc("Option Category", "Speed"));

    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("offset_by_speed", KisPaintOpOption::GENERAL, false, 0.0, -3, 3), "0", "100"), i18n("Offset By Speed"), i18nc("Option Category", "Speed"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("offset_by_random", KisPaintOpOption::GENERAL, false, 0.0, -3, 3), "0", "100"), i18n("Offset By Random"), i18nc("Option Category", "Speed"));

    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("dabs_per_actual_radius", KisPaintOpOption::GENERAL, false, 2, 0, 6), "0", "100"), i18n("Dabs Per Actual Radius"), i18nc("Option Category", "Dabs"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("dabs_per_second", KisPaintOpOption::GENERAL, false, 0, 0, 80), "0", "100"), i18n("Dabs per Second"), i18nc("Option Category", "Dabs"));
    addPaintOpOption(opacityOption, i18n("Opaque"), i18nc("Option Category", "Opacity"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("opaque_linearize", KisPaintOpOption::GENERAL, false, 0.9, 0, 3), "0", "100"), i18n("Opaque Linearize"), i18nc("Option Category", "Opacity"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("opaque_multiply", KisPaintOpOption::GENERAL, false, 0, 0, 2), "0", "100"), i18n("Opaque Multiply"), i18nc("Option Category", "Opacity"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("slow_tracking_per_dab", KisPaintOpOption::GENERAL, false, 0, 0, 10), "0", "100"), i18n("Slow tracking per dab"), i18nc("Option Category", "Tracking"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("slow_tracking", KisPaintOpOption::GENERAL, false, 0, 0, 10), "0", "100"), i18n("Slow Tracking"), i18nc("Option Category", "Tracking"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("tracking_noise", KisPaintOpOption::GENERAL, false, 0, 0, 12), "0", "100"), i18n("Tracking Noise"), i18nc("Option Category", "Tracking"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("smudge", KisPaintOpOption::GENERAL, false, 0, 0, 1), "0", "100"), i18n("Smudge"), i18nc("Option Category", "Smudge"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("smudge_length", KisPaintOpOption::GENERAL, false, 0.5, 0, 1), "0", "100"), i18n("Smudge Length"), i18nc("Option Category", "Smudge"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("smudge_radius_log", KisPaintOpOption::GENERAL, false, 0, -1.6, 1.6), "0", "100"), i18n("Smudge Radius Log"), i18nc("Option Category", "Smudge"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("stroke_duration_logarithmic", KisPaintOpOption::GENERAL, false, 4, -1, 7), "0", "100"), i18n("Stroke Duration log"), i18nc("Option Category", "Stroke"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("stroke_holdtime", KisPaintOpOption::GENERAL, false, 0, 0, 10), "0", "100"), i18n("Stroke Holdtime"), i18nc("Option Category", "Stroke"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("stroke_threshold", KisPaintOpOption::GENERAL, false, 0, 0, 0.50), "0", "100"), i18n("Stroke Threshold"), i18nc("Option Category", "Stroke"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("custom_input", KisPaintOpOption::GENERAL, false, 0, -5, 5), "0", "100"), i18n("Custom Input"), i18nc("Option Category", "Custom"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption("custom_input_slowness", KisPaintOpOption::GENERAL, false, 0, 0, 10), "0", "100"), i18n("Custom Input Slowness"), i18nc("Option Category", "Custom"));
    addPaintOpOption(new KisAirbrushOptionWidget(false), i18n("Airbrush"));

    connect(radiusOption->slider(), SIGNAL(valueChanged(qreal)), SLOT(updateBaseOptionRadius(qreal)));
    connect(hardnessOption->slider(), SIGNAL(valueChanged(qreal)), SLOT(updateBaseOptionHardness(qreal)));
    connect(opacityOption->slider(), SIGNAL(valueChanged(qreal)), SLOT(updateBaseOptionOpacity(qreal)));

    connect(basicOption->radiusSlider(), SIGNAL(valueChanged(qreal)), SLOT(updateRadiusOptionOpacity(qreal)));
    connect(basicOption->hardnessSlider(), SIGNAL(valueChanged(qreal)), SLOT(updateHardnessOptionOpacity(qreal)));
    connect(basicOption->opacitySlider(), SIGNAL(valueChanged(qreal)), SLOT(updateOpacityOptionOpacity(qreal)));
}

KisMyPaintOpSettingsWidget::~ KisMyPaintOpSettingsWidget()
{
}

KisPropertiesConfigurationSP  KisMyPaintOpSettingsWidget::configuration() const
{
    KisMyPaintOpSettings* config = new KisMyPaintOpSettings(resourcesInterface());
    config->setOptionsWidget(const_cast<KisMyPaintOpSettingsWidget*>(this));
    config->setProperty("paintop", "mypaintbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

void KisMyPaintOpSettingsWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
}

void KisMyPaintOpSettingsWidget::refreshBaseOption()
{    
    m_baseOption->refresh();
}

void KisMyPaintOpSettingsWidget::updateBaseOptionRadius(qreal value) {

    m_baseOption->radiusSlider()->blockSignals(true);
    m_baseOption->radiusSlider()->setValue(value);
    m_baseOption->radiusSlider()->blockSignals(false);    
}

void KisMyPaintOpSettingsWidget::updateBaseOptionHardness(qreal value) {

    m_baseOption->hardnessSlider()->blockSignals(true);
    m_baseOption->hardnessSlider()->setValue(value);
    m_baseOption->hardnessSlider()->blockSignals(false);
}

void KisMyPaintOpSettingsWidget::updateBaseOptionOpacity(qreal value) {

    m_baseOption->opacitySlider()->blockSignals(true);
    m_baseOption->opacitySlider()->setValue(value);
    m_baseOption->opacitySlider()->blockSignals(false);
}

void KisMyPaintOpSettingsWidget::updateRadiusOptionOpacity(qreal value) {

    m_radiusWidget->slider()->blockSignals(true);
    m_radiusWidget->slider()->setValue(value);
    m_radiusWidget->slider()->blockSignals(false);    
    refreshBaseOption();
}

void KisMyPaintOpSettingsWidget::updateHardnessOptionOpacity(qreal value) {

    m_hardnessWidget->slider()->blockSignals(true);
    m_hardnessWidget->slider()->setValue(value);
    m_hardnessWidget->slider()->blockSignals(false);
    refreshBaseOption();
}

void KisMyPaintOpSettingsWidget::updateOpacityOptionOpacity(qreal value) {

    m_opacityWidget->slider()->blockSignals(true);
    m_opacityWidget->slider()->setValue(value);
    m_opacityWidget->slider()->blockSignals(false);
    refreshBaseOption();
}
