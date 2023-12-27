/*
 *  SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tangent_normal_paintop_settings_widget.h"
#include "kis_brush_based_paintop_settings.h"
#include "KisTangentTiltOptionWidget.h"

#include <kis_properties_configuration.h>
#include <KisStandardOptionData.h>
#include <KisPaintOpOptionWidgetUtils.h>

#include <KisCompositeOpOptionWidget.h>
#include "KisSizeOptionWidget.h"
#include "KisSpacingOptionWidget.h"
#include "KisMirrorOptionWidget.h"
#include "KisSharpnessOptionWidget.h"
#include "KisScatterOptionWidget.h"
#include "KisAirbrushOptionWidget.h"
#include "KisPaintingModeOptionWidget.h"
#include <KisTextureOptionWidget.h>

KisTangentNormalPaintOpSettingsWidget::KisTangentNormalPaintOpSettingsWidget(QWidget* parent, KisResourcesInterfaceSP resourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface):
    KisBrushBasedPaintopOptionWidget(KisBrushOptionWidgetFlag::SupportsPrecision |
                                     KisBrushOptionWidgetFlag::SupportsHSLBrushMode, parent)
{
    Q_UNUSED(canvasResourcesInterface)
    namespace kpowu = KisPaintOpOptionWidgetUtils;

    setObjectName("brush option widget");

    addPaintOpOption(kpowu::createOptionWidget<KisCompositeOpOptionWidget>());
    addPaintOpOption(kpowu::createOpacityOptionWidget());
    addPaintOpOption(kpowu::createFlowOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisSizeOptionWidget>());

    addPaintOpOption(kpowu::createOptionWidget<KisTangentTiltOptionWidget>());

    addPaintOpOption(kpowu::createOptionWidget<KisSpacingOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisMirrorOptionWidget>());

    addPaintOpOption(kpowu::createSoftnessOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisSharpnessOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisScatterOptionWidget>());
    addPaintOpOption(kpowu::createRotationOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisAirbrushOptionWidget>());
    addPaintOpOption(kpowu::createRateOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisPaintingModeOptionWidget>());

    addPaintOpOption(kpowu::createOptionWidget<KisTextureOptionWidget>(KisTextureOptionData(), resourcesInterface));
    addPaintOpOption(kpowu::createStrengthOptionWidget());
}

KisTangentNormalPaintOpSettingsWidget::~KisTangentNormalPaintOpSettingsWidget() { }

KisPropertiesConfigurationSP KisTangentNormalPaintOpSettingsWidget::configuration() const
{
    KisBrushBasedPaintOpSettingsSP config = new KisBrushBasedPaintOpSettings(resourcesInterface());
    config->setProperty("paintop", "tangentnormal");
    writeConfiguration(config);
    return config;
}


