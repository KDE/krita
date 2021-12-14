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

    KisMyPaintCurveOptionWidget *radiusOption = new KisMyPaintCurveOptionWidget(
        new KisMyPaintCurveOption(KoID("radius_logarithmic", i18n("Radius Logarithmic")), KisPaintOpOption::GENERAL, false, 2.0, 0.01, 6.0),
        "0",
        "100");
    m_radiusWidget = radiusOption;

    KisMyPaintCurveOptionWidget *hardnessOption =
        new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption(KoID("hardness", i18n("Hardness")), KisPaintOpOption::GENERAL, false, 0.8, 0.02, 1),
                                        "0",
                                        "100");
    m_hardnessWidget = hardnessOption;

    KisMyPaintCurveOptionWidget *opacityOption =
        new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption(KoID("opaque", i18n("Opaque")), KisPaintOpOption::GENERAL, false, 1, 0, 1), "0", "100");
    m_opacityWidget = opacityOption;

    addPaintOpOption(basicOption);
    addPaintOpOption(radiusOption, i18nc("Option Category", "Basic"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(
                         new KisMyPaintCurveOption(KoID("radius_by_random", i18n("Radius by Random")), KisPaintOpOption::GENERAL, false, 0.0, 0, 1.50),
                         "0",
                         "100"),
                     i18nc("Option Category", "Basic"));
    addPaintOpOption(hardnessOption, i18nc("Option Category", "Basic"));
    addPaintOpOption(
        new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption(KoID("anti_aliasing", i18n("Anti Aliasing")), KisPaintOpOption::GENERAL, false, 0.0, 0, 1),
                                        "0",
                                        "100"),
        i18nc("Option Category", "Basic"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(
                         new KisMyPaintCurveOption(KoID("elliptical_dab_angle", i18n("Elliptical Dab Angle")), KisPaintOpOption::GENERAL, false, 0.0, 0, 180),
                         "0",
                         "100"),
                     i18nc("Option Category", "Basic"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(
                         new KisMyPaintCurveOption(KoID("elliptical_dab_ratio", i18n("Elliptical Dab Ratio")), KisPaintOpOption::GENERAL, false, 1, 1, 10),
                         "0",
                         "100"),
                     i18nc("Option Category", "Basic"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(
                         new KisMyPaintCurveOption(KoID("direction_filter", i18n("Direction Filter")), KisPaintOpOption::GENERAL, false, 2, 0, 10),
                         "0",
                         "100"),
                     i18nc("Option Category", "Basic"));

    addPaintOpOption(new KisAirbrushOptionWidget(false), i18n("Airbrush"));
    addPaintOpOption(
        new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption(KoID("change_color_h", i18n("Change Color H")), KisPaintOpOption::GENERAL, false, 0.0, -2, 2),
                                        "0",
                                        "100"),
        i18nc("Option Category", "Color"));
    addPaintOpOption(
        new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption(KoID("change_color_l", i18n("Change Color L")), KisPaintOpOption::GENERAL, false, 0.0, -2, 2),
                                        "0",
                                        "100"),
        i18nc("Option Category", "Color"));
    addPaintOpOption(
        new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption(KoID("change_color_v", i18n("Change Color V")), KisPaintOpOption::GENERAL, false, 0.0, -2, 2),
                                        "0",
                                        "100"),
        i18nc("Option Category", "Color"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(
                         new KisMyPaintCurveOption(KoID("change_color_hsl_s", i18n("Change Color HSL S")), KisPaintOpOption::GENERAL, false, 0.0, -2, 2),
                         "0",
                         "100"),
                     i18nc("Option Category", "Color"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(
                         new KisMyPaintCurveOption(KoID("change_color_hsv_s", i18n("Change Color HSV S")), KisPaintOpOption::GENERAL, false, 0.0, -2, 2),
                         "0",
                         "100"),
                     i18nc("Option Category", "Color"));
    addPaintOpOption(
        new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption(KoID("colorize", i18n("Colorize")), KisPaintOpOption::GENERAL, false, 0.0, 0, 1), "0", "100"),
        i18nc("Option Category", "Color"));
    addPaintOpOption(
        new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption(KoID("speed1_gamma", i18n("Fine Speed Gamma")), KisPaintOpOption::GENERAL, false, 4, -8, 8),
                                        "0",
                                        "100"),
        i18nc("Option Category", "Speed"));
    addPaintOpOption(
        new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption(KoID("speed2_gamma", i18n("Gross Speed Gamma")), KisPaintOpOption::GENERAL, false, 4, -8, 8),
                                        "0",
                                        "100"),
        i18nc("Option Category", "Speed"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(
                         new KisMyPaintCurveOption(KoID("speed1_slowness", i18n("Fine Speed Slowness")), KisPaintOpOption::GENERAL, false, 4, -8, 8),
                         "0",
                         "100"),
                     i18nc("Option Category", "Speed"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(
                         new KisMyPaintCurveOption(KoID("speed2_slowness", i18n("Gross Speed Slowness")), KisPaintOpOption::GENERAL, false, 4, -8, 8),
                         "0",
                         "100"),
                     i18nc("Option Category", "Speed"));

    addPaintOpOption(new KisMyPaintCurveOptionWidget(
                         new KisMyPaintCurveOption(KoID("offset_by_speed", i18n("Offset By Speed")), KisPaintOpOption::GENERAL, false, 0.0, -3, 3),
                         "0",
                         "100"),
                     i18nc("Option Category", "Speed"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(
                         new KisMyPaintCurveOption(KoID("offset_by_random", i18n("Offset By Random")), KisPaintOpOption::GENERAL, false, 0.0, -3, 3),
                         "0",
                         "100"),
                     i18nc("Option Category", "Speed"));

    addPaintOpOption(new KisMyPaintCurveOptionWidget(
                         new KisMyPaintCurveOption(KoID("dabs_per_actual_radius", i18n("Dabs Per Actual Radius")), KisPaintOpOption::GENERAL, false, 2, 0, 6),
                         "0",
                         "100"),
                     i18nc("Option Category", "Dabs"));
    addPaintOpOption(
        new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption(KoID("dabs_per_second", i18n("Dabs per Second")), KisPaintOpOption::GENERAL, false, 0, 0, 80),
                                        "0",
                                        "100"),
        i18nc("Option Category", "Dabs"));
    addPaintOpOption(opacityOption, i18nc("Option Category", "Opacity"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(
                         new KisMyPaintCurveOption(KoID("opaque_linearize", i18n("Opaque Linearize")), KisPaintOpOption::GENERAL, false, 0.9, 0, 3),
                         "0",
                         "100"),
                     i18nc("Option Category", "Opacity"));
    addPaintOpOption(
        new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption(KoID("opaque_multiply", i18n("Opaque Multiply")), KisPaintOpOption::GENERAL, false, 0, 0, 2),
                                        "0",
                                        "100"),
        i18nc("Option Category", "Opacity"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(
                         new KisMyPaintCurveOption(KoID("slow_tracking_per_dab", i18n("Slow tracking per dab")), KisPaintOpOption::GENERAL, false, 0, 0, 10),
                         "0",
                         "100"),
                     i18nc("Option Category", "Tracking"));
    addPaintOpOption(
        new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption(KoID("slow_tracking", i18n("Slow Tracking")), KisPaintOpOption::GENERAL, false, 0, 0, 10),
                                        "0",
                                        "100"),
        i18nc("Option Category", "Tracking"));
    addPaintOpOption(
        new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption(KoID("tracking_noise", i18n("Tracking Noise")), KisPaintOpOption::GENERAL, false, 0, 0, 12),
                                        "0",
                                        "100"),
        i18nc("Option Category", "Tracking"));
    addPaintOpOption(
        new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption(KoID("smudge", i18n("Smudge")), KisPaintOpOption::GENERAL, false, 0, 0, 1), "0", "100"),
        i18nc("Option Category", "Smudge"));
    addPaintOpOption(
        new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption(KoID("smudge_length", i18n("Smudge Length")), KisPaintOpOption::GENERAL, false, 0.5, 0, 1),
                                        "0",
                                        "100"),
        i18nc("Option Category", "Smudge"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(
                         new KisMyPaintCurveOption(KoID("smudge_radius_log", i18n("Smudge Radius Log")), KisPaintOpOption::GENERAL, false, 0, -1.6, 1.6),
                         "0",
                         "100"),
                     i18nc("Option Category", "Smudge"));
    addPaintOpOption(
        new KisMyPaintCurveOptionWidget(
            new KisMyPaintCurveOption(KoID("stroke_duration_logarithmic", i18n("Stroke Duration log")), KisPaintOpOption::GENERAL, false, 4, -1, 7),
            "0",
            "100"),
        i18nc("Option Category", "Stroke"));
    addPaintOpOption(
        new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption(KoID("stroke_holdtime", i18n("Stroke Holdtime")), KisPaintOpOption::GENERAL, false, 0, 0, 10),
                                        "0",
                                        "100"),
        i18nc("Option Category", "Stroke"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(
                         new KisMyPaintCurveOption(KoID("stroke_threshold", i18n("Stroke Threshold")), KisPaintOpOption::GENERAL, false, 0, 0, 0.50),
                         "0",
                         "100"),
                     i18nc("Option Category", "Stroke"));
    addPaintOpOption(
        new KisMyPaintCurveOptionWidget(new KisMyPaintCurveOption(KoID("custom_input", i18n("Custom Input")), KisPaintOpOption::GENERAL, false, 0, -5, 5),
                                        "0",
                                        "100"),
        i18nc("Option Category", "Custom"));
    addPaintOpOption(new KisMyPaintCurveOptionWidget(
                         new KisMyPaintCurveOption(KoID("custom_input_slowness", i18n("Custom Input Slowness")), KisPaintOpOption::GENERAL, false, 0, 0, 10),
                         "0",
                         "100"),
                     i18nc("Option Category", "Custom"));

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
