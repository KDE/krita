/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_sketch_paintop_settings_widget.h"

#include "kis_sketchop_option.h"
#include "kis_sketch_paintop_settings.h"

#include <kis_curve_option_widget.h>
#include <kis_pressure_opacity_option.h>
#include <kis_paintop_options_widget.h>
#include <kis_paint_action_type_option.h>
#include <kis_airbrush_option.h>
#include <kis_pressure_size_option.h>
#include <kis_compositeop_option.h>

#include <QDomDocument>
#include <QDomElement>
#include <kis_pressure_rotation_option.h>
#include "kis_density_option.h"
#include "kis_linewidth_option.h"
#include "kis_offset_scale_option.h"

KisSketchPaintOpSettingsWidget::KisSketchPaintOpSettingsWidget(QWidget* parent)
        : KisBrushBasedPaintopOptionWidget(parent)
{
    m_sketchOption =  new KisSketchOpOption();

    addPaintOpOption(m_sketchOption);
    addPaintOpOption(new KisCompositeOpOption(true));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisLineWidthOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisOffsetScaleOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisDensityOption()));
    addPaintOpOption(new KisAirbrushOption(false));

    m_paintActionType = new KisPaintActionTypeOption();
    KisPropertiesConfiguration defaultSetting;
    defaultSetting.setProperty("PaintOpAction",BUILDUP);
    m_paintActionType->readOptionSetting(&defaultSetting);

    addPaintOpOption(m_paintActionType);

    KisPropertiesConfiguration* reconfigurationCourier = configuration();
    QDomDocument xMLAnalyzer("");
    xMLAnalyzer.setContent(reconfigurationCourier->getString("brush_definition") );

    QDomElement firstTag = xMLAnalyzer.documentElement();
    QDomElement firstTagsChild = firstTag.elementsByTagName("MaskGenerator").item(0).toElement();

    firstTagsChild.attributeNode("diameter").setValue("128");

    reconfigurationCourier->setProperty("brush_definition", xMLAnalyzer.toString() );
    setConfiguration(reconfigurationCourier);
    delete reconfigurationCourier;
}

KisSketchPaintOpSettingsWidget::~ KisSketchPaintOpSettingsWidget()
{
}

KisPropertiesConfiguration*  KisSketchPaintOpSettingsWidget::configuration() const
{
    KisSketchPaintOpSettings* config = new KisSketchPaintOpSettings();
    config->setOptionsWidget(const_cast<KisSketchPaintOpSettingsWidget*>(this));
    config->setProperty("paintop", "sketchbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

