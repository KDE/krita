/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_brushop_settings_widget.h"
#include <kis_brush_based_paintop_settings.h>
#include <kis_pressure_darken_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_paint_action_type_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_mix_option.h>
#include <kis_curve_option_widget.h>
#include <kis_pressure_hsv_option.h>
#include <kis_airbrush_option.h>
#include <kis_pressure_mirror_option_widget.h>
#include <kis_pressure_scatter_option_widget.h>
#include <kis_pressure_softness_option.h>
#include <kis_pressure_sharpness_option_widget.h>
#include <kis_color_source_option_widget.h>
#include <kis_pressure_spacing_option.h>
#include <kis_compositeop_option.h>
#include <kis_pressure_flow_opacity_option_widget.h>


KisBrushOpSettingsWidget::KisBrushOpSettingsWidget(QWidget* parent)
        : KisBrushBasedPaintopOptionWidget(parent)
{
    setObjectName("brush option widget");
    setPrecisionEnabled(true);

    // Brush tip options
    addPaintOpOption(new KisCompositeOpOption(true));
    addPaintOpOption(new KisFlowOpacityOptionWidget());
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSpacingOption()));
    addPaintOpOption(new KisPressureMirrorOptionWidget());
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSoftnessOption()));
    addPaintOpOption(new KisPressureSharpnessOptionWidget());
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption()));
    addPaintOpOption(new KisPressureScatterOptionWidget());

    // Colors options
    addPaintOpOption(new KisColorSourceOptionWidget());
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureDarkenOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureMixOption()));
    addPaintOpOption(new KisCurveOptionWidget(KisPressureHSVOption::createHueOption()));
    addPaintOpOption(new KisCurveOptionWidget(KisPressureHSVOption::createSaturationOption()));
    addPaintOpOption(new KisCurveOptionWidget(KisPressureHSVOption::createValueOption()));
    addPaintOpOption(new KisAirbrushOption(false));
    addPaintOpOption(new KisPaintActionTypeOption());

    addTextureOptions();
}

KisBrushOpSettingsWidget::~KisBrushOpSettingsWidget()
{
}

KisPropertiesConfiguration* KisBrushOpSettingsWidget::configuration() const
{
    KisBrushBasedPaintOpSettings *config = new KisBrushBasedPaintOpSettings();
    config->setOptionsWidget(const_cast<KisBrushOpSettingsWidget*>(this));
    config->setProperty("paintop", "paintbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

#include "kis_brushop_settings_widget.moc"
