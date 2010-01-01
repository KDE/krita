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

#include "kis_complexop_settings_widget.h"
#include "kis_complexop_settings.h"
#include <widgets/kis_curve_widget.h>
#include <kis_properties_configuration.h>
#include <kis_brush_option_widget.h>
#include <kis_paintop_options_widget.h>
#include <kis_pressure_darken_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_paint_action_type_option.h>
#include <kis_bidirectional_mixing_option.h>
#include <kis_curve_option_widget.h>

KisComplexOpSettingsWidget::KisComplexOpSettingsWidget(QWidget* parent)
        : KisPaintOpOptionsWidget(parent)
{
    setObjectName("mixing brush option widget");

    m_brushOption = new KisBrushOptionWidget;
    m_bidiOption = new KisBidirectionalMixingOption;

    addPaintOpOption(m_brushOption);
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureDarkenOption()));
    addPaintOpOption(new KisPaintActionTypeOption());
    addPaintOpOption(m_bidiOption);

}

KisComplexOpSettingsWidget::~KisComplexOpSettingsWidget()
{
    delete m_brushOption;
    delete m_bidiOption;
}

KisPropertiesConfiguration* KisComplexOpSettingsWidget::configuration() const
{
    KisComplexOpSettings *config = new KisComplexOpSettings();
    config->setOptionsWidget(const_cast<KisComplexOpSettingsWidget*>(this));
    config->setProperty("paintop", "complex"); // XXX: make this a const id string
    return config;
}

#include "kis_complexop_settings_widget.moc"
