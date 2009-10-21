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
#include "kis_dyna_paintop_settings_widget.h"

#include "kis_dynaop_option.h"
#include "kis_dyna_paintop_settings.h"

#include <kis_paintop_options_widget.h>
#include <kis_paint_action_type_option.h>

KisDynaPaintOpSettingsWidget:: KisDynaPaintOpSettingsWidget(QWidget* parent)
        : KisPaintOpOptionsWidget(parent)
{
    m_paintActionTypeOption = new KisPaintActionTypeOption();
    m_dynaOption =  new KisDynaOpOption();

    addPaintOpOption(m_dynaOption);
    addPaintOpOption(m_paintActionTypeOption);
}

KisDynaPaintOpSettingsWidget::~ KisDynaPaintOpSettingsWidget()
{
    delete m_dynaOption;
    delete m_paintActionTypeOption;
}

KisPropertiesConfiguration*  KisDynaPaintOpSettingsWidget::configuration() const
{
    KisDynaPaintOpSettings* config = new KisDynaPaintOpSettings();
    config->setOptionsWidget(const_cast<KisDynaPaintOpSettingsWidget*>(this));

    config->setProperty("paintop", "dynabrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
