/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_brushop_settings_widget.h"
#include <KisBrushOpSettings.h>
#include <kis_pressure_darken_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_flow_option.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_ratio_option.h>
#include <kis_paint_action_type_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_mix_option.h>
#include <kis_pressure_lightness_strength_option.h>
#include <kis_pressure_lightness_strength_option_widget.h>
#include <kis_curve_option_widget.h>
#include <kis_pressure_hsv_option.h>
#include <kis_airbrush_option_widget.h>
#include <kis_pressure_scatter_option_widget.h>
#include <kis_pressure_softness_option.h>
#include <kis_pressure_sharpness_option_widget.h>
#include <kis_color_source_option_widget.h>
#include <kis_compositeop_option.h>
#include <kis_pressure_flow_opacity_option_widget.h>
#include <kis_pressure_spacing_option_widget.h>
#include <kis_pressure_rate_option.h>
#include "kis_texture_option.h"
#include <kis_pressure_mirror_option_widget.h>
#include "kis_pressure_texture_strength_option.h"
#include <KisMaskingBrushOption.h>

#include <KisPrefixedPaintOpOptionWrapper.h>
#include <KisPaintopSettingsIds.h>

KisBrushOpSettingsWidget::KisBrushOpSettingsWidget(QWidget* parent)
    : KisBrushBasedPaintopOptionWidget(parent)
{
    setObjectName("brush option widget");
    setPrecisionEnabled(true);
    setHSLBrushTipEnabled(true);

    // Brush tip options
    addPaintOpOption(new KisCompositeOpOption(true), i18n("Blending Mode"));
    addPaintOpOption(new KisFlowOpacityOptionWidget(), i18n("Opacity"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureFlowOption(), i18n("0%"), i18n("100%")), i18n("Flow"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption(), i18n("0%"), i18n("100%")), i18n("Size"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRatioOption(), i18n("0%"), i18n("100%")), i18n("Ratio"));
    addPaintOpOption(new KisPressureSpacingOptionWidget(), i18n("Spacing"));
    addPaintOpOption(new KisPressureMirrorOptionWidget(), i18n("Mirror"));


    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSoftnessOption(), i18n("Soft"), i18n("Hard")), i18n("Softness"));
    addPaintOpOption(new KisPressureSharpnessOptionWidget(), i18n("Sharpness"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption(), i18n("-180°"), i18n("180°")), i18n("Rotation"));
    m_lightnessStrengthOptionWidget = new KisPressureLightnessStrengthOptionWidget();
    addPaintOpOption(m_lightnessStrengthOptionWidget, i18n("Lightness Strength"));
    addPaintOpOption(new KisPressureScatterOptionWidget(), i18n("Scatter"));

    // Colors options
    addPaintOpOption(new KisColorSourceOptionWidget(), i18nc("Color source", "Source"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureDarkenOption(), i18n("0.0"), i18n("1.0")), i18n("Darken"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureMixOption(), i18n("Background"), i18n("Foreground")), i18nc("Mixing of colors", "Mix"));
    addPaintOpOption(new KisCurveOptionWidget(KisPressureHSVOption::createHueOption(), KisPressureHSVOption::hueMinLabel(), KisPressureHSVOption::huemaxLabel()), i18n("Hue"));
    addPaintOpOption(new KisCurveOptionWidget(KisPressureHSVOption::createSaturationOption(), KisPressureHSVOption::saturationMinLabel(), KisPressureHSVOption::saturationmaxLabel()), i18n("Saturation"));
    addPaintOpOption(new KisCurveOptionWidget(KisPressureHSVOption::createValueOption(), KisPressureHSVOption::valueMinLabel(), KisPressureHSVOption::valuemaxLabel()), i18nc("Label of Brightness value in brush settings", "Value"));
    addPaintOpOption(new KisAirbrushOptionWidget(false), i18n("Airbrush"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRateOption(), i18n("0%"), i18n("100%")), i18n("Rate"));


    KisPaintActionTypeOption *actionTypeOption = new KisPaintActionTypeOption();
    addPaintOpOption(actionTypeOption, i18n("Painting Mode"));

    addPaintOpOption(new KisTextureOption(SupportsLightnessMode | SupportsGradientMode), i18n("Pattern"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureTextureStrengthOption(), i18n("Weak"), i18n("Strong")), i18n("Strength"));

    KisMaskingBrushOption::MasterBrushSizeAdapter sizeAdapter =
        [this] () { return this->brush()->userEffectiveSize(); };

    KisMaskingBrushOption *maskingOption = new KisMaskingBrushOption(sizeAdapter);
    addPaintOpOption(maskingOption, i18n("Brush Tip"));

    connect(maskingOption, SIGNAL(sigCheckedChanged(bool)),
            actionTypeOption, SLOT(slotForceWashMode(bool)));

    {
        KisCurveOption *maskingSizeOption = new KisPressureSizeOption();
        maskingSizeOption->setChecked(false);

        addPaintOpOption(
                    new KisPrefixedPaintOpOptionWrapper<KisCurveOptionWidget>(
                        KisPaintOpUtils::MaskingBrushPresetPrefix,
                        maskingSizeOption,
                        i18n("0%"), i18n("100%")),
                    i18n("Size"), KisPaintOpOption::MASKING_BRUSH);
    }

    addPaintOpOption(
        new KisPrefixedPaintOpOptionWrapper<KisFlowOpacityOptionWidget>(
            KisPaintOpUtils::MaskingBrushPresetPrefix),
        i18n("Opacity"), KisPaintOpOption::MASKING_BRUSH);

    KisCurveOption *maskingFlowOption = new KisPressureFlowOption();
    maskingFlowOption->setChecked(false);
    KisCurveOption *maskingRatioOption = new KisPressureRatioOption();
    maskingRatioOption->setChecked(false);

    addPaintOpOption(
                new KisPrefixedPaintOpOptionWrapper<KisCurveOptionWidget>(
                    KisPaintOpUtils::MaskingBrushPresetPrefix,
                    maskingFlowOption,
                    i18n("0%"), i18n("100%")),
                i18n("Flow"), KisPaintOpOption::MASKING_BRUSH);

    addPaintOpOption(
                new KisPrefixedPaintOpOptionWrapper<KisCurveOptionWidget>(
                    KisPaintOpUtils::MaskingBrushPresetPrefix,
                    maskingRatioOption,
                    i18n("0%"), i18n("100%")),
                i18n("Ratio"), KisPaintOpOption::MASKING_BRUSH);

    addPaintOpOption(
        new KisPrefixedPaintOpOptionWrapper<KisPressureMirrorOptionWidget>(
            KisPaintOpUtils::MaskingBrushPresetPrefix),
        i18n("Mirror"), KisPaintOpOption::MASKING_BRUSH);

    addPaintOpOption(
        new KisPrefixedPaintOpOptionWrapper<KisCurveOptionWidget>(
            KisPaintOpUtils::MaskingBrushPresetPrefix,
            new KisPressureRotationOption(), i18n("-180°"), i18n("180°")),
        i18n("Rotation"), KisPaintOpOption::MASKING_BRUSH);

    addPaintOpOption(
        new KisPrefixedPaintOpOptionWrapper<KisPressureScatterOptionWidget>(
            KisPaintOpUtils::MaskingBrushPresetPrefix),
        i18n("Scatter"), KisPaintOpOption::MASKING_BRUSH);
}

KisBrushOpSettingsWidget::~KisBrushOpSettingsWidget()
{
}

KisPropertiesConfigurationSP KisBrushOpSettingsWidget::configuration() const
{
    KisBrushBasedPaintOpSettingsSP config = new KisBrushOpSettings(resourcesInterface());
    config->setOptionsWidget(const_cast<KisBrushOpSettingsWidget*>(this));
    config->setProperty("paintop", "paintbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

void KisBrushOpSettingsWidget::notifyPageChanged()
{
    KisBrushSP brush = this->brush();
    m_lightnessStrengthOptionWidget->setEnabled(brush->preserveLightness());
}
