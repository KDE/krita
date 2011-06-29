/*
 *  Copyright (c) 2008,2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_deform_paintop_settings.h"
#include "kis_deform_paintop_settings_widget.h"
#include "kis_deform_option.h"

#include <kis_paintop_options_widget.h>
#include <kis_brush_size_option.h>

#include <kis_pressure_rotation_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_curve_option_widget.h>
#include <kis_airbrush_option.h>
#include <kis_compositeop_option.h>

KisDeformPaintOpSettingsWidget::KisDeformPaintOpSettingsWidget(QWidget* parent)
        : KisPaintOpOptionsWidget(parent)
{
    m_deformOption = new KisDeformOption();
    m_brushSizeOption = new KisBrushSizeOption();
    m_brushSizeOption->setDiameter(200);

    addPaintOpOption(m_brushSizeOption);
    addPaintOpOption(m_deformOption);
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption()));
    addPaintOpOption(new KisCompositeOpOption(true));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption()));
    addPaintOpOption(new KisAirbrushOption());
}

KisDeformPaintOpSettingsWidget::~ KisDeformPaintOpSettingsWidget()
{
}


void KisDeformPaintOpSettingsWidget::changePaintOpSize(qreal x, qreal y)
{
    // if the movement is more left<->right then up<->down
    if (qAbs(x) > qAbs(y)){
        m_brushSizeOption->setDiameter( m_brushSizeOption->diameter() + qRound(x) );
    }
    else // vice-versa
    {
        // we can do something different, e.g. change deform mode or ...
    }
}


QSizeF KisDeformPaintOpSettingsWidget::paintOpSize() const
{
    qreal height = m_brushSizeOption->diameter() * m_brushSizeOption->brushAspect();
    return QSizeF(m_brushSizeOption->diameter(), height);
}


KisPropertiesConfiguration* KisDeformPaintOpSettingsWidget::configuration() const
{
    KisDeformPaintOpSettings* config = new KisDeformPaintOpSettings();
    config->setOptionsWidget(const_cast<KisDeformPaintOpSettingsWidget*>(this));
    config->setProperty("paintop","deformBrush");
    writeConfiguration(config);
    return config;
}

