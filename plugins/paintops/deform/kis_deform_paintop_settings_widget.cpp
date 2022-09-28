/*
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_deform_paintop_settings.h"
#include "kis_deform_paintop_settings_widget.h"
#include "kis_deform_option.h"

#include <kis_paintop_settings_widget.h>
#include <kis_brush_size_option.h>

#include <kis_pressure_rotation_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_rate_option.h>
#include <kis_curve_option_widget.h>
#include <kis_compositeop_option.h>
#include <KisAirbrushOptionWidget.h>
#include <KisPaintOpOptionUtils.h>

KisDeformPaintOpSettingsWidget::KisDeformPaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    namespace kpou = KisPaintOpOptionUtils;

    KisBrushSizeOption *m_brushSizeOption = new KisBrushSizeOption();
    m_brushSizeOption->setDiameter(200);

    addPaintOpOption(m_brushSizeOption);
    addPaintOpOption(new KisDeformOption());
    addPaintOpOption(new KisCompositeOpOption(true));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption(), i18n("Transparent"), i18n("Opaque")));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption(), i18n("0%"), i18n("100%")));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption(), i18n("-180°"), i18n("180°")));
    addPaintOpOption(kpou::createOptionWidget<KisAirbrushOptionWidget>());
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRateOption(), i18n("0%"), i18n("100%")));
}

KisDeformPaintOpSettingsWidget::~ KisDeformPaintOpSettingsWidget()
{
}


KisPropertiesConfigurationSP KisDeformPaintOpSettingsWidget::configuration() const
{
    KisDeformPaintOpSettings* config = new KisDeformPaintOpSettings(resourcesInterface());
    config->setProperty("paintop", "deformBrush");
    writeConfiguration(config);
    return config;
}

