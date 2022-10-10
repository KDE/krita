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
#include <kis_pressure_hsv_option.h>
#include "kis_texture_option.h"
#include "kis_pressure_texture_strength_option.h"
#include <KisMaskingBrushOption.h>

#include <KisPaintopSettingsIds.h>
#include "kis_brush_option_widget.h"
#include "KisCurveOptionWidget2.h"
#include "KisSpacingOptionWidget.h"
#include "KisMirrorOptionWidget.h"
#include "KisSharpnessOptionWidget.h"
#include "KisScatterOptionWidget.h"
#include "KisAirbrushOptionWidget.h"
#include "KisCompositeOpOptionWidget.h"
#include "KisPaintingModeOptionWidget.h"
#include "KisColorSourceOptionWidget.h"
#include "KisLightnessStrengthOptionWidget.h"
#include "KisTextureOptionWidget.h"
#include "KisSizeOptionWidget.h"

#include <KisCurveOptionData.h>
#include <lager/state.hpp>
#include <KisZug.h>

#include <KisStandardOptionData.h>


struct KisBrushOpSettingsWidget::Private
{
    Private()
        : sizeOptionData({true, ""}),
          maskingOpacityOptionData({true, KisPaintOpUtils::MaskingBrushPresetPrefix}),
          maskingSizeOptionData({true, KisPaintOpUtils::MaskingBrushPresetPrefix}),
          maskingFlowOptionData({true, KisPaintOpUtils::MaskingBrushPresetPrefix}),
          maskingRatioOptionData({KisPaintOpUtils::MaskingBrushPresetPrefix}),
          maskingRotationOptionData({KisPaintOpUtils::MaskingBrushPresetPrefix}),
          maskingMirrorOptionData({KisPaintOpUtils::MaskingBrushPresetPrefix}),
          maskingScatterOptionData({KisPaintOpUtils::MaskingBrushPresetPrefix})
    {
    }

    lager::state<KisCompositeOpOptionData, lager::automatic_tag> compositeOpOptionData;
    lager::state<KisOpacityOptionData, lager::automatic_tag> opacityOptionData;
    lager::state<KisSizeOptionData, lager::automatic_tag> sizeOptionData;
    lager::state<KisFlowOptionData, lager::automatic_tag> flowOptionData;
    lager::state<KisRatioOptionData, lager::automatic_tag> ratioOptionData;
    lager::state<KisSoftnessOptionData, lager::automatic_tag> softnessOptionData;
    lager::state<KisRotationOptionData, lager::automatic_tag> rotationOptionData;
    lager::state<KisSpacingOptionData, lager::automatic_tag> spacingOptionData;
    lager::state<KisMirrorOptionData, lager::automatic_tag> mirrorOptionData;
    lager::state<KisSharpnessOptionData, lager::automatic_tag> sharpnessOptionData;
    lager::state<KisScatterOptionData, lager::automatic_tag> scatterOptionData;
    lager::state<KisLightnessStrengthOptionData, lager::automatic_tag> lightnessStrengthOptionData;

    lager::state<KisColorSourceOptionData, lager::automatic_tag> colorSourceOptionData;
    lager::state<KisDarkenOptionData, lager::automatic_tag> darkenOptionData;
    lager::state<KisMixOptionData, lager::automatic_tag> mixOptionData;
    lager::state<KisHueOptionData, lager::automatic_tag> hueOptionData;
    lager::state<KisSaturationOptionData, lager::automatic_tag> saturationOptionData;
    lager::state<KisValueOptionData, lager::automatic_tag> valueOptionData;
    lager::state<KisAirbrushOptionData, lager::automatic_tag> airbrushOptionData;
    lager::state<KisRateOptionData, lager::automatic_tag> rateOptionData;
    lager::state<KisPaintingModeOptionData, lager::automatic_tag> paintingModeOptionData;

    lager::state<KisTextureOptionData, lager::automatic_tag> textureOptionData;
    lager::state<KisStrengthOptionData, lager::automatic_tag> strengthOptionData;

    lager::state<KisOpacityOptionData, lager::automatic_tag> maskingOpacityOptionData;
    lager::state<KisSizeOptionData, lager::automatic_tag> maskingSizeOptionData;
    lager::state<KisFlowOptionData, lager::automatic_tag> maskingFlowOptionData;
    lager::state<KisRatioOptionData, lager::automatic_tag> maskingRatioOptionData;
    lager::state<KisRotationOptionData, lager::automatic_tag> maskingRotationOptionData;
    lager::state<KisMirrorOptionData, lager::automatic_tag> maskingMirrorOptionData;
    lager::state<KisScatterOptionData, lager::automatic_tag> maskingScatterOptionData;
};

KisBrushOpSettingsWidget::KisBrushOpSettingsWidget(QWidget* parent, KisResourcesInterfaceSP resourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    : KisBrushBasedPaintopOptionWidget(KisBrushOptionWidgetFlag::SupportsPrecision |
                                       KisBrushOptionWidgetFlag::SupportsHSLBrushMode,
                                       parent),
      m_d(new Private)
{
    // TODO: pass into KisPaintOpSettingsWidget!
    Q_UNUSED(canvasResourcesInterface);

    setObjectName("brush option widget");

    // Brush tip options
    addPaintOpOption(new KisCompositeOpOptionWidget(m_d->compositeOpOptionData));
    addPaintOpOptionData(m_d->opacityOptionData, KisPaintOpOption::GENERAL, i18n("Transparent"), i18n("Opaque"));
    addPaintOpOptionData(m_d->flowOptionData, KisPaintOpOption::GENERAL);
    addPaintOpOption(new KisSizeOptionWidget(m_d->sizeOptionData));
    addPaintOpOptionData(m_d->ratioOptionData, KisPaintOpOption::GENERAL);
    addPaintOpOption(new KisSpacingOptionWidget(m_d->spacingOptionData), KisPaintOpOption::GENERAL);
    addPaintOpOption(new KisMirrorOptionWidget(m_d->mirrorOptionData), KisPaintOpOption::GENERAL);

    addPaintOpOptionData(m_d->softnessOptionData, KisPaintOpOption::GENERAL, i18n("Soft"), i18n("Hard"));
    addPaintOpOptionData(m_d->rotationOptionData, KisPaintOpOption::GENERAL, i18n("-180°"), i18n("180°"));
    addPaintOpOption(new KisSharpnessOptionWidget(m_d->sharpnessOptionData), KisPaintOpOption::GENERAL);

    addPaintOpOption(new KisLightnessStrengthOptionWidget(m_d->lightnessStrengthOptionData, brushOptionWidget()->lightnessModeEnabled()));

    addPaintOpOption(new KisScatterOptionWidget(m_d->scatterOptionData), KisPaintOpOption::GENERAL);

    // Colors options
    addPaintOpOption(new KisColorSourceOptionWidget(m_d->colorSourceOptionData));
    addPaintOpOptionData(m_d->darkenOptionData, KisPaintOpOption::COLOR, i18n("0.0"), i18n("1.0"));
    addPaintOpOptionData(m_d->mixOptionData, KisPaintOpOption::COLOR, i18nc("Background painting color", "Background"), i18n("Foreground painting color", "Foreground"));

    addPaintOpOptionData(m_d->hueOptionData,
                         KisPaintOpOption::COLOR,
                         KisPressureHSVOption::hueMinLabel(),
                         KisPressureHSVOption::hueMaxLabel(),
                         -180, 180, i18n("°"));

    addPaintOpOptionData(m_d->saturationOptionData,
                         KisPaintOpOption::COLOR,
                         KisPressureHSVOption::saturationMinLabel(),
                         KisPressureHSVOption::saturationMaxLabel());

    addPaintOpOptionData(m_d->valueOptionData,
                         KisPaintOpOption::COLOR,
                         KisPressureHSVOption::valueMinLabel(),
                         KisPressureHSVOption::valueMaxLabel());

    addPaintOpOption(new KisAirbrushOptionWidget(m_d->airbrushOptionData));
    addPaintOpOptionData(m_d->rateOptionData, KisPaintOpOption::COLOR);

    KisMaskingBrushOption *maskingOption = new KisMaskingBrushOption(brushOptionWidget()->effectiveBrushSize());
    addPaintOpOption(new KisPaintingModeOptionWidget(m_d->paintingModeOptionData, maskingOption->maskingBrushEnabledReader()));

    addPaintOpOption(new KisTextureOptionWidget(m_d->textureOptionData, resourcesInterface, SupportsLightnessMode | SupportsGradientMode));
    addPaintOpOptionData(m_d->strengthOptionData, KisPaintOpOption::COLOR, i18n("Weak"), i18n("Strong"));

    addPaintOpOption(maskingOption);

    addPaintOpOptionData(m_d->maskingOpacityOptionData, KisPaintOpOption::MASKING_BRUSH);
    addPaintOpOptionData(m_d->maskingSizeOptionData, KisPaintOpOption::MASKING_BRUSH);
    addPaintOpOptionData(m_d->maskingFlowOptionData, KisPaintOpOption::MASKING_BRUSH);
    addPaintOpOptionData(m_d->maskingRatioOptionData, KisPaintOpOption::MASKING_BRUSH);
    addPaintOpOptionData(m_d->maskingRotationOptionData, KisPaintOpOption::MASKING_BRUSH, i18n("-180°"), i18n("180°"));

    addPaintOpOption(new KisMirrorOptionWidget(
                         lager::cursor<KisMirrorOptionData>(m_d->maskingMirrorOptionData)),
                     KisPaintOpOption::MASKING_BRUSH);

    addPaintOpOption(new KisScatterOptionWidget(
                         lager::cursor<KisScatterOptionData>(m_d->maskingScatterOptionData)),
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

template<typename Data, typename... Args>
void KisBrushOpSettingsWidget::addPaintOpOptionData(Data &data, Args... args)
{
    addPaintOpOption(new KisCurveOptionWidget2(data.zoom(kiszug::lenses::to_base<KisCurveOptionData>),
                                                         args...));
}
