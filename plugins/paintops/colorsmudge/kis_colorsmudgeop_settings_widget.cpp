/*
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_colorsmudgeop_settings_widget.h"
#include "kis_brush_based_paintop_settings.h"
#include "kis_overlay_mode_option.h"
#include "kis_rate_option.h"
#include "kis_smudge_option_widget.h"
#include "kis_brush_option_widget.h"
#include "kis_smudge_radius_option.h"

#include <kis_properties_configuration.h>
#include <kis_paintop_settings_widget.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_ratio_option.h>
#include <kis_curve_option_widget.h>
#include <kis_pressure_paint_thickness_option.h>
#include <kis_pressure_paint_thickness_option_widget.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_scatter_option_widget.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_gradient_option.h>
#include <kis_airbrush_option_widget.h>
#include <kis_compositeop_option.h>
#include <kis_pressure_spacing_option_widget.h>
#include <kis_pressure_rate_option.h>
#include "kis_texture_option.h"
#include <kis_pressure_mirror_option_widget.h>
#include "kis_pressure_texture_strength_option.h"
#include "kis_pressure_hsv_option.h"
#include "kis_colorsmudgeop_settings.h"
#include "kis_signals_blocker.h"


KisColorSmudgeOpSettingsWidget::KisColorSmudgeOpSettingsWidget(QWidget* parent):
    KisBrushBasedPaintopOptionWidget(parent)
{
    setObjectName("brush option widget");
    setPrecisionEnabled(true);
    setHSLBrushTipEnabled(true);

    addPaintOpOption(new KisCompositeOpOption(true));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption(), i18n("Transparent"), i18n("Opaque")));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption(), i18n("0%"), i18n("100%")));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRatioOption(), i18n("0%"), i18n("100%")));
    addPaintOpOption(new KisPressureSpacingOptionWidget());
    addPaintOpOption(new KisPressureMirrorOptionWidget());

    m_smudgeOptionWidget = new KisSmudgeOptionWidget();
    addPaintOpOption(m_smudgeOptionWidget);

    m_radiusStrengthOptionWidget = new KisCurveOptionWidget(new KisSmudgeRadiusOption(), i18n("0.0"), i18n("1.0"));
    addPaintOpOption(m_radiusStrengthOptionWidget);

    addPaintOpOption(new KisCurveOptionWidget(
        new KisRateOption(KoID("ColorRate", i18nc("Color rate of active Foreground color", "Color Rate")), KisPaintOpOption::GENERAL, false),
        i18n("0.0"),
        i18n("1.0")));
    m_paintThicknessOptionWidget = new KisPressurePaintThicknessOptionWidget();
    addPaintOpOption(m_paintThicknessOptionWidget);

    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption(), i18n("-180°"), i18n("180°")));
    addPaintOpOption(new KisPressureScatterOptionWidget());
    m_overlayOptionWidget = new KisOverlayModeOptionWidget();
    addPaintOpOption(m_overlayOptionWidget);
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureGradientOption(), i18n("0%"), i18n("100%")));
    addPaintOpOption(
        new KisCurveOptionWidget(KisPressureHSVOption::createHueOption(), KisPressureHSVOption::hueMinLabel(), KisPressureHSVOption::huemaxLabel()));
    addPaintOpOption(new KisCurveOptionWidget(KisPressureHSVOption::createSaturationOption(),
                                              KisPressureHSVOption::saturationMinLabel(),
                                              KisPressureHSVOption::saturationmaxLabel()));
    addPaintOpOption(
        new KisCurveOptionWidget(KisPressureHSVOption::createValueOption(), KisPressureHSVOption::valueMinLabel(), KisPressureHSVOption::valuemaxLabel()));
    addPaintOpOption(new KisAirbrushOptionWidget(false));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRateOption(), i18n("0%"), i18n("100%")));

    addPaintOpOption(new KisTextureOption());
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureTextureStrengthOption(), i18n("Weak"), i18n("Strong")));

    const KisBrushOptionWidget* brushOption = brushOptionWidget();
    connect(brushOption, SIGNAL(sigSettingChanged()), SLOT(slotBrushOptionChanged()));
}

KisColorSmudgeOpSettingsWidget::~KisColorSmudgeOpSettingsWidget() { }

KisPropertiesConfigurationSP KisColorSmudgeOpSettingsWidget::configuration() const
{
    KisColorSmudgeOpSettingsSP config = new KisColorSmudgeOpSettings(resourcesInterface());
    config->setProperty("paintop", "colorsmudge");
    writeConfiguration(config);
    return config;
}

void KisColorSmudgeOpSettingsWidget::notifyPageChanged()
{
    KisBrushSP brush = this->brush();
    bool pierced =  brush ? brush->isPiercedApprox() : false;
    m_smudgeOptionWidget->updateBrushPierced(pierced);

    //If brush is a mask, it can use either engine, but if its not, it must use the new engine
    if (brush) {
        m_smudgeOptionWidget->setUseNewEngineCheckboxEnabled(brush->brushApplication() == ALPHAMASK);
        m_paintThicknessOptionWidget->setEnabled(brush->preserveLightness());
        m_overlayOptionWidget->setEnabled(brush->brushApplication() != LIGHTNESSMAP);
        m_radiusStrengthOptionWidget->updateRange(0.0, m_smudgeOptionWidget->useNewEngine() ? 1.0 : 3.0);
    }
}

void KisColorSmudgeOpSettingsWidget::slotBrushOptionChanged() {
    notifyPageChanged();
}

void KisColorSmudgeOpSettingsWidget::fixNewEngineOption() const
{
    KisBrushSP brush = const_cast<KisColorSmudgeOpSettingsWidget*>(this)->brush();

    if (brush) {
        if (brush->brushApplication() != ALPHAMASK) {
            KisSignalsBlocker b(m_smudgeOptionWidget);
            m_smudgeOptionWidget->setUseNewEngine(true);
        }
    }
}

void KisColorSmudgeOpSettingsWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    KisBrushBasedPaintopOptionWidget::setConfiguration(config);
    fixNewEngineOption();
}

void KisColorSmudgeOpSettingsWidget::writeConfiguration(KisPropertiesConfigurationSP config) const
{
    fixNewEngineOption();
    KisBrushBasedPaintopOptionWidget::writeConfiguration(config);
}
