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

#include <lager/state.hpp>

#include <KisMaskingBrushOption.h>
#include <KisPaintopSettingsIds.h>
#include "kis_brush_option_widget.h"
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

#include <KisStandardOptionData.h>
#include <KisPaintOpOptionWidgetUtils.h>

KisBrushOpSettingsWidget::KisBrushOpSettingsWidget(QWidget* parent, KisResourcesInterfaceSP resourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    : KisBrushBasedPaintopOptionWidget(KisBrushOptionWidgetFlag::SupportsPrecision |
                                       KisBrushOptionWidgetFlag::SupportsHSLBrushMode,
                                       parent)
{
    // TODO: pass into KisPaintOpSettingsWidget!
    Q_UNUSED(canvasResourcesInterface);

    setObjectName("brush option widget");

    namespace kpowu = KisPaintOpOptionWidgetUtils;

    // Brush tip options
    addPaintOpOption(kpowu::createOptionWidget<KisCompositeOpOptionWidget>());
    addPaintOpOption(kpowu::createOpacityOptionWidget());
    addPaintOpOption(kpowu::createFlowOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisSizeOptionWidget>());
    addPaintOpOption(kpowu::createRatioOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisSpacingOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisMirrorOptionWidget>());

    addPaintOpOption(kpowu::createSoftnessOptionWidget());
    addPaintOpOption(kpowu::createRotationOptionWidget());

    addPaintOpOption(kpowu::createOptionWidget<KisSharpnessOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisLightnessStrengthOptionWidget>(KisLightnessStrengthOptionData(), brushOptionWidget()->lightnessModeEnabled()));

    addPaintOpOption(kpowu::createOptionWidget<KisScatterOptionWidget>());

    // Colors options
    addPaintOpOption(kpowu::createOptionWidget<KisColorSourceOptionWidget>());
    addPaintOpOption(kpowu::createDarkenOptionWidget());
    addPaintOpOption(kpowu::createMixOptionWidget());
    addPaintOpOption(kpowu::createHueOptionWidget());
    addPaintOpOption(kpowu::createSaturationOptionWidget());
    addPaintOpOption(kpowu::createValueOptionWidget());

    addPaintOpOption(kpowu::createOptionWidget<KisAirbrushOptionWidget>());
    addPaintOpOption(kpowu::createRateOptionWidget());

    KisMaskingBrushOption *maskingOption = new KisMaskingBrushOption(brushOptionWidget()->effectiveBrushSize());
    addPaintOpOption(kpowu::createOptionWidget<KisPaintingModeOptionWidget>(KisPaintingModeOptionData(), maskingOption->maskingBrushEnabledReader()));

    addPaintOpOption(kpowu::createOptionWidget<KisTextureOptionWidget>(KisTextureOptionData(), resourcesInterface, SupportsLightnessMode | SupportsGradientMode));
    addPaintOpOption(kpowu::createStrengthOptionWidget());

    addPaintOpOption(maskingOption);

    addPaintOpOption(kpowu::createMaskingOpacityOptionWidget());
    addPaintOpOption(kpowu::createMaskingFlowOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisSizeOptionWidget>(KisSizeOptionData(KisPaintOpUtils::MaskingBrushPresetPrefix), KisPaintOpOption::MASKING_BRUSH));
    addPaintOpOption(kpowu::createMaskingRatioOptionWidget());
    addPaintOpOption(kpowu::createMaskingRotationOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisMirrorOptionWidget>(KisMirrorOptionData(KisPaintOpUtils::MaskingBrushPresetPrefix), KisPaintOpOption::MASKING_BRUSH));
    addPaintOpOption(kpowu::createOptionWidget<KisScatterOptionWidget>(KisScatterOptionData(KisPaintOpUtils::MaskingBrushPresetPrefix), KisPaintOpOption::MASKING_BRUSH));
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
