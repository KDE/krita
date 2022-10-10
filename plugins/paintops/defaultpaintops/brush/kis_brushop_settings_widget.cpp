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
#include <KisZug.h>

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
#include <KisPaintOpOptionUtils.h>

KisBrushOpSettingsWidget::KisBrushOpSettingsWidget(QWidget* parent, KisResourcesInterfaceSP resourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    : KisBrushBasedPaintopOptionWidget(KisBrushOptionWidgetFlag::SupportsPrecision |
                                       KisBrushOptionWidgetFlag::SupportsHSLBrushMode,
                                       parent)
{
    // TODO: pass into KisPaintOpSettingsWidget!
    Q_UNUSED(canvasResourcesInterface);

    setObjectName("brush option widget");

    namespace kpou = KisPaintOpOptionUtils;

    // Brush tip options
    addPaintOpOption(kpou::createOptionWidget<KisCompositeOpOptionWidget>());
    addPaintOpOption(kpou::createOpacityOptionWidget());
    addPaintOpOption(kpou::createFlowOptionWidget());
    addPaintOpOption(kpou::createOptionWidget<KisSizeOptionWidget>());
    addPaintOpOption(kpou::createRatioOptionWidget());
    addPaintOpOption(kpou::createOptionWidget<KisSpacingOptionWidget>());
    addPaintOpOption(kpou::createOptionWidget<KisMirrorOptionWidget>());

    addPaintOpOption(kpou::createSoftnessOptionWidget());
    addPaintOpOption(kpou::createRotationOptionWidget());

    addPaintOpOption(kpou::createOptionWidget<KisSharpnessOptionWidget>());
    addPaintOpOption(kpou::createOptionWidget<KisLightnessStrengthOptionWidget>(KisLightnessStrengthOptionData(), brushOptionWidget()->lightnessModeEnabled()));

    addPaintOpOption(kpou::createOptionWidget<KisScatterOptionWidget>());

    // Colors options
    addPaintOpOption(kpou::createOptionWidget<KisColorSourceOptionWidget>());
    addPaintOpOption(kpou::createDarkenOptionWidget());
    addPaintOpOption(kpou::createMixOptionWidget());
    addPaintOpOption(kpou::createHueOptionWidget());
    addPaintOpOption(kpou::createSaturationOptionWidget());
    addPaintOpOption(kpou::createValueOptionWidget());

    addPaintOpOption(kpou::createOptionWidget<KisAirbrushOptionWidget>());
    addPaintOpOption(kpou::createRateOptionWidget());

    KisMaskingBrushOption *maskingOption = new KisMaskingBrushOption(brushOptionWidget()->effectiveBrushSize());
    addPaintOpOption(kpou::createOptionWidget<KisPaintingModeOptionWidget>(KisPaintingModeOptionData(), maskingOption->maskingBrushEnabledReader()));

    addPaintOpOption(kpou::createOptionWidget<KisTextureOptionWidget>(KisTextureOptionData(), resourcesInterface, SupportsLightnessMode | SupportsGradientMode));
    addPaintOpOption(kpou::createStrengthOptionWidget());

    addPaintOpOption(maskingOption);

    addPaintOpOption(kpou::createMaskingOpacityOptionWidget());
    addPaintOpOption(kpou::createOptionWidget<KisSizeOptionWidget>(KisSizeOptionData(true, KisPaintOpUtils::MaskingBrushPresetPrefix), KisPaintOpOption::MASKING_BRUSH));
    addPaintOpOption(kpou::createMaskingFlowOptionWidget());
    addPaintOpOption(kpou::createMaskingRatioOptionWidget());
    addPaintOpOption(kpou::createMaskingRotationOptionWidget());
    addPaintOpOption(kpou::createOptionWidget<KisMirrorOptionWidget>(KisMirrorOptionData(KisPaintOpUtils::MaskingBrushPresetPrefix), KisPaintOpOption::MASKING_BRUSH));
    addPaintOpOption(kpou::createOptionWidget<KisScatterOptionWidget>(KisScatterOptionData(KisPaintOpUtils::MaskingBrushPresetPrefix), KisPaintOpOption::MASKING_BRUSH));
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
