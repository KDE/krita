/*
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_spray_paintop_settings_widget.h"

#include <KoViewConverter.h>
#include <KoColorSpaceRegistry.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_paintop_registry.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paint_information.h>

#include <KoColor.h>

#include "kis_sprayop_option.h"
#include "kis_spray_paintop_settings.h"
#include "kis_spray_shape_option.h"
#include "kis_spray_color_option.h"

#include <kis_paintop_options_widget.h>
#include <kis_paint_action_type_option.h>

KisSprayPaintOpSettingsWidget:: KisSprayPaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpOptionsWidget(parent)
{
    m_paintActionTypeOption = new KisPaintActionTypeOption();
    m_sprayOption =  new KisSprayOpOption();

    m_sprayShapeOption = new KisSprayShapeOption();
    m_sprayColorOption = new KisSprayColorOption();

    addPaintOpOption(m_sprayOption);
    addPaintOpOption(m_sprayShapeOption);
    addPaintOpOption(m_sprayColorOption);
    addPaintOpOption(m_paintActionTypeOption);
}

KisSprayPaintOpSettingsWidget::~ KisSprayPaintOpSettingsWidget()
{
    delete m_sprayOption;
    delete m_paintActionTypeOption;
}

KisPropertiesConfiguration*  KisSprayPaintOpSettingsWidget::configuration() const
{
    KisSprayPaintOpSettings* config = new KisSprayPaintOpSettings( const_cast<KisSprayPaintOpSettingsWidget*>( this ) );
    config->setProperty("paintop", "spraybrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
