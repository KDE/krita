/*
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_spray_paintop_settings_widget.h"

#include "kis_sprayop_option.h"
#include "kis_spray_paintop_settings.h"
#include "kis_spray_shape_option.h"

#include <kis_color_option.h>
#include <kis_paintop_settings_widget.h>

#include <KisPaintingModeOptionWidget.h>
#include <KisPaintOpOptionUtils.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_rate_option.h>
#include <kis_curve_option_widget.h>
#include <kis_brush_option_widget.h>
#include "kis_spray_shape_dynamics.h"
#include <KisAirbrushOptionWidget.h>
#include <KisPaintOpOptionUtils.h>
#include <KisCompositeOpOptionWidget.h>


KisSprayPaintOpSettingsWidget:: KisSprayPaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    namespace kpou = KisPaintOpOptionUtils;

    addPaintOpOption(new KisSprayOpOption());
    addPaintOpOption(new KisSprayShapeOption());
    addPaintOpOption(new KisBrushOptionWidget(KisBrushOptionWidgetFlag::None));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption(), i18n("Transparent"), i18n("Opaque")));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption(), i18n("0%"), i18n("100%")));
    addPaintOpOption(kpou::createOptionWidget<KisCompositeOpOptionWidget>());

    addPaintOpOption(new KisSprayShapeDynamicsOption());
    addPaintOpOption(new KisColorOption());

    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption(), i18n("-180°"), i18n("180°")));
    addPaintOpOption(kpou::createOptionWidget<KisAirbrushOptionWidget>());
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRateOption(), i18n("0%"), i18n("100%")));
    addPaintOpOption(kpou::createOptionWidget<KisPaintingModeOptionWidget>());
}

KisSprayPaintOpSettingsWidget::~ KisSprayPaintOpSettingsWidget()
{
}

KisPropertiesConfigurationSP  KisSprayPaintOpSettingsWidget::configuration() const
{
    KisSprayPaintOpSettings* config = new KisSprayPaintOpSettings(resourcesInterface());
    config->setProperty("paintop", "spraybrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
