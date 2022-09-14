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
#include "kis_brush_option_widget.h"
#include "KisCurveOptionWidget2.h"

#include <KisCurveOptionData.h>
#include <lager/state.hpp>

struct KisBrushOpSettingsWidget::Private
{
    Private()
        : flowOptionData(KisCurveOptionData(
                             KoID("Flow", i18n("Flow")),
                             KisPaintOpOption::GENERAL,
                             true, false, false,
                             0.1, 2.0))
    {
    }

    lager::state<KisCurveOptionData, lager::automatic_tag> flowOptionData;
};

KisBrushOpSettingsWidget::KisBrushOpSettingsWidget(QWidget* parent)
    : KisBrushBasedPaintopOptionWidget(KisBrushOptionWidgetFlag::SupportsPrecision |
                                       KisBrushOptionWidgetFlag::SupportsHSLBrushMode,
                                       parent),
      m_d(new Private)
{
    setObjectName("brush option widget");

    // Brush tip options
    addPaintOpOption(new KisCompositeOpOption(true));
    addPaintOpOption(new KisFlowOpacityOptionWidget());
    addPaintOpOption(new KisCurveOptionWidget2(m_d->flowOptionData, i18n("0%"), i18n("100%"), 0, 100, i18n("%")));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption(), i18n("0%"), i18n("100%")));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRatioOption(), i18n("0%"), i18n("100%")));
    addPaintOpOption(new KisPressureSpacingOptionWidget());
    addPaintOpOption(new KisPressureMirrorOptionWidget());

    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSoftnessOption(), i18n("Soft"), i18n("Hard")));
    addPaintOpOption(new KisPressureSharpnessOptionWidget());
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption(), i18n("-180°"), i18n("180°")));
    m_lightnessStrengthOptionWidget = new KisPressureLightnessStrengthOptionWidget();
    addPaintOpOption(m_lightnessStrengthOptionWidget);
    addPaintOpOption(new KisPressureScatterOptionWidget());

    // Colors options
    addPaintOpOption(new KisColorSourceOptionWidget());
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureDarkenOption(), i18n("0.0"), i18n("1.0")));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureMixOption(), i18nc("Background painting color", "Background"), i18nc("Foreground painting color", "Foreground")));
    addPaintOpOption(
        new KisCurveOptionWidget(KisPressureHSVOption::createHueOption(), KisPressureHSVOption::hueMinLabel(), KisPressureHSVOption::huemaxLabel()));
    addPaintOpOption(new KisCurveOptionWidget(KisPressureHSVOption::createSaturationOption(),
                                              KisPressureHSVOption::saturationMinLabel(),
                                              KisPressureHSVOption::saturationmaxLabel()));
    addPaintOpOption(
        new KisCurveOptionWidget(KisPressureHSVOption::createValueOption(), KisPressureHSVOption::valueMinLabel(), KisPressureHSVOption::valuemaxLabel()));
    addPaintOpOption(new KisAirbrushOptionWidget(false));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRateOption(), i18n("0%"), i18n("100%")));

    KisPaintActionTypeOption *actionTypeOption = new KisPaintActionTypeOption();
    addPaintOpOption(actionTypeOption);

    addPaintOpOption(new KisTextureOption(SupportsLightnessMode | SupportsGradientMode));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureTextureStrengthOption(), i18n("Weak"), i18n("Strong")));

    KisMaskingBrushOption *maskingOption = new KisMaskingBrushOption(brushOptionWidget()->effectiveBrushSize());
    addPaintOpOption(maskingOption);

    connect(maskingOption, SIGNAL(sigCheckedChanged(bool)),
            actionTypeOption, SLOT(slotForceWashMode(bool)));

    {
        KisCurveOption *maskingSizeOption = new KisPressureSizeOption();
        maskingSizeOption->setChecked(false);

        addPaintOpOption(
            new KisPrefixedPaintOpOptionWrapper<KisCurveOptionWidget>(KisPaintOpUtils::MaskingBrushPresetPrefix, maskingSizeOption, i18n("0%"), i18n("100%")),
            KisPaintOpOption::MASKING_BRUSH);
    }

    addPaintOpOption(new KisPrefixedPaintOpOptionWrapper<KisFlowOpacityOptionWidget>(KisPaintOpUtils::MaskingBrushPresetPrefix),
                     KisPaintOpOption::MASKING_BRUSH);

    KisCurveOption *maskingFlowOption = new KisPressureFlowOption();
    maskingFlowOption->setChecked(false);
    KisCurveOption *maskingRatioOption = new KisPressureRatioOption();
    maskingRatioOption->setChecked(false);

    addPaintOpOption(
        new KisPrefixedPaintOpOptionWrapper<KisCurveOptionWidget>(KisPaintOpUtils::MaskingBrushPresetPrefix, maskingFlowOption, i18n("0%"), i18n("100%")),
        KisPaintOpOption::MASKING_BRUSH);

    addPaintOpOption(
        new KisPrefixedPaintOpOptionWrapper<KisCurveOptionWidget>(KisPaintOpUtils::MaskingBrushPresetPrefix, maskingRatioOption, i18n("0%"), i18n("100%")),
        KisPaintOpOption::MASKING_BRUSH);

    addPaintOpOption(new KisPrefixedPaintOpOptionWrapper<KisPressureMirrorOptionWidget>(KisPaintOpUtils::MaskingBrushPresetPrefix),
                     KisPaintOpOption::MASKING_BRUSH);

    addPaintOpOption(new KisPrefixedPaintOpOptionWrapper<KisCurveOptionWidget>(KisPaintOpUtils::MaskingBrushPresetPrefix,
                                                                               new KisPressureRotationOption(),
                                                                               i18n("-180°"),
                                                                               i18n("180°")),
                     KisPaintOpOption::MASKING_BRUSH);

    addPaintOpOption(new KisPrefixedPaintOpOptionWrapper<KisPressureScatterOptionWidget>(KisPaintOpUtils::MaskingBrushPresetPrefix),
                     KisPaintOpOption::MASKING_BRUSH);
}

KisBrushOpSettingsWidget::~KisBrushOpSettingsWidget()
{
}

KisPropertiesConfigurationSP KisBrushOpSettingsWidget::configuration() const
{
    KisBrushBasedPaintOpSettingsSP config = new KisBrushOpSettings(resourcesInterface());
    config->setProperty("paintop", "paintbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

void KisBrushOpSettingsWidget::notifyPageChanged()
{
    m_lightnessStrengthOptionWidget->setEnabled(this->brushOptionWidget()->preserveLightness());
}
