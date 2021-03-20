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
#include <kis_paint_action_type_option.h>

#include <kis_pressure_rotation_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_rate_option.h>
#include <kis_curve_option_widget.h>
#include <kis_brush_option_widget.h>
#include "kis_spray_shape_dynamics.h"
#include <kis_airbrush_option_widget.h>
#include <kis_compositeop_option.h>

KisSprayPaintOpSettingsWidget:: KisSprayPaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
    , m_sprayArea(new KisSprayOpOption())
{
    addPaintOpOption(m_sprayArea, i18n("Spray Area"));
    addPaintOpOption(new KisSprayShapeOption(), i18n("Spray shape"));
    addPaintOpOption(new KisBrushOptionWidget(), i18n("Brush Tip"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption(), i18n("Transparent"), i18n("Opaque")), i18n("Opacity"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption(), i18n("0%"), i18n("100%")), i18n("Size"));
    addPaintOpOption(new KisCompositeOpOption(true), i18n("Blending Mode"));

    addPaintOpOption(new KisSprayShapeDynamicsOption(), i18n("Shape dynamics"));
    addPaintOpOption(new KisColorOption(), i18n("Color options"));

    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption(), i18n("-180°"), i18n("180°")), i18n("Rotation"));
    addPaintOpOption(new KisAirbrushOptionWidget(false), i18n("Airbrush"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRateOption(), i18n("0%"), i18n("100%")), i18n("Rate"));
    addPaintOpOption(new KisPaintActionTypeOption(), i18n("Painting Mode"));
}

KisSprayPaintOpSettingsWidget::~ KisSprayPaintOpSettingsWidget()
{
}

KisPropertiesConfigurationSP  KisSprayPaintOpSettingsWidget::configuration() const
{
    KisSprayPaintOpSettings* config = new KisSprayPaintOpSettings(resourcesInterface());
    config->setOptionsWidget(const_cast<KisSprayPaintOpSettingsWidget*>(this));
    config->setProperty("paintop", "spraybrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
