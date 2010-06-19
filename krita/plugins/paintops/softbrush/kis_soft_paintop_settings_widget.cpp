/*
 *  Copyright (c) 2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_soft_paintop_settings_widget.h"

#include "kis_softop_option.h"
#include "kis_soft_paintop_settings.h"

#include <kis_paintop_options_widget.h>
#include <kis_paint_action_type_option.h>

#include <kis_curve_option_widget.h>
#include <kis_brush_size_option.h>

#include <kis_pressure_rotation_option.h>

#include "kis_hsv_option.h"
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_airbrush_option.h>

KisSoftPaintOpSettingsWidget:: KisSoftPaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpOptionsWidget(parent)
{
    m_paintActionTypeOption = new KisPaintActionTypeOption();
    m_softOption =  new KisSoftOpOption();
    m_sizeOption = new KisBrushSizeOption();
    m_sizeOption->setSpacing(0.1);
    m_hsvOption = new KisHSVOption();
    
    addPaintOpOption(m_sizeOption);
    addPaintOpOption(m_softOption);
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption()));
    addPaintOpOption(m_hsvOption);
    addPaintOpOption(new KisAirbrushOption());
    addPaintOpOption(m_paintActionTypeOption);
}

KisSoftPaintOpSettingsWidget::~ KisSoftPaintOpSettingsWidget()
{
}

KisPropertiesConfiguration*  KisSoftPaintOpSettingsWidget::configuration() const
{
    KisSoftPaintOpSettings* config = new KisSoftPaintOpSettings();
    config->setOptionsWidget( const_cast<KisSoftPaintOpSettingsWidget*>( this ) );
    config->setProperty("paintop", "softbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

void KisSoftPaintOpSettingsWidget::changePaintOpSize(qreal x, qreal y)
{
    if (qAbs(x) > qAbs(y)){
        m_sizeOption->setDiameter( m_sizeOption->diameter() + qRound(x) );
    }
    else // vertical drag
    {
        // we can do something different
    }
}

