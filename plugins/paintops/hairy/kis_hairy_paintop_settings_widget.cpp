/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_hairy_paintop_settings_widget.h"
#include "kis_hairy_paintop_settings.h"

#include "KisHairyBristleOptionWidget.h"
#include "KisHairyInkOptionWidget.h"

#include <KisPaintOpOptionWidgetUtils.h>
#include <KisCompositeOpOptionWidget.h>
#include <KisStandardOptionData.h>
#include <KisSizeOptionWidget.h>
#include <KisPaintingModeOptionWidget.h>
#include <kis_brush_option_widget.h>

KisHairyPaintOpSettingsWidget:: KisHairyPaintOpSettingsWidget(QWidget* parent)
    : KisBrushBasedPaintopOptionWidget(KisBrushOptionWidgetFlag::None, parent)
{
    namespace kpowu = KisPaintOpOptionWidgetUtils;
    addPaintOpOption(kpowu::createOptionWidgetWithLodLimitations<KisHairyBristleOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisHairyInkOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisCompositeOpOptionWidget>());
    addPaintOpOption(kpowu::createOpacityOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisSizeOptionWidget>());
    addPaintOpOption(kpowu::createRotationOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisPaintingModeOptionWidget>());

    KisBrushOptionWidget *brushWidget = brushOptionWidget();
    QStringList hiddenOptions;
    hiddenOptions << "KisBrushChooser/lblSpacing"
                  << "KisBrushChooser/Spacing"
                  << "KisBrushChooser/ColorAsMask"
                  << "KisAutoBrushWidget/btnAntiAliasing"
                  << "KisAutoBrushWidget/grpFade"
                  << "KisAutoBrushWidget/lblDensity"
                  << "KisAutoBrushWidget/density"
                  << "KisAutoBrushWidget/lblSpacing"
                  << "KisAutoBrushWidget/spacingWidget"
                  << "KisAutoBrushWidget/lblRandomness"
                  << "KisAutoBrushWidget/inputRandomness"
                     ;
    brushWidget->hideOptions(hiddenOptions);
}

KisHairyPaintOpSettingsWidget::~ KisHairyPaintOpSettingsWidget()
{
}

KisPropertiesConfigurationSP  KisHairyPaintOpSettingsWidget::configuration() const
{
    KisHairyPaintOpSettings* config = new KisHairyPaintOpSettings(resourcesInterface());
    config->setProperty("paintop", "hairybrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
