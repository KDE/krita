/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_hairy_paintop_settings_widget.h"
#include "kis_hairy_paintop_settings.h"

#include "kis_hairy_ink_option.h"

#include <kis_paint_action_type_option.h>
#include "kis_hairy_bristle_option.h"
#include <kis_curve_option_widget.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_compositeop_option.h>
#include <kis_brush_option_widget.h>

KisHairyPaintOpSettingsWidget:: KisHairyPaintOpSettingsWidget(QWidget* parent)
    : KisBrushBasedPaintopOptionWidget(parent)
{
    addPaintOpOption(new KisHairyBristleOption());
    addPaintOpOption(new KisHairyInkOption());
    addPaintOpOption(new KisCompositeOpOption(true));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption(), i18n("Transparent"), i18n("Opaque")));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption(), i18n("0%"), i18n("100%")));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption(), i18n("-180°"), i18n("180°")));
    addPaintOpOption(new KisPaintActionTypeOption());

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
