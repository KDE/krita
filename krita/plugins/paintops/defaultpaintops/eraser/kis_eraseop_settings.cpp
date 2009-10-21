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

#include "kis_eraseop_settings.h"
#include "kis_eraseop_settings_widget.h"

#include <kis_brush_option.h>
#include <kis_paintop_options_widget.h>
#include <kis_pressure_darken_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_paint_action_type_option.h>

KisEraseOpSettings::KisEraseOpSettings()
        : m_options(0)
{
}

KisEraseOpSettings::~KisEraseOpSettings()
{
}

bool KisEraseOpSettings::paintIncremental()
{
    return true;
}

void KisEraseOpSettings::fromXML(const QDomElement& elt)
{
    // First, call the parent class fromXML to make sure all the
    // properties are saved to the map
    KisPaintOpSettings::fromXML(elt);

    // Then load the properties for all widgets
    m_options->setConfiguration(this);
}

void KisEraseOpSettings::toXML(QDomDocument& doc, QDomElement& rootElt) const
{

    // First, make sure all the option widgets have saved their state
    // to the property configuration
    KisPropertiesConfiguration * settings = m_options->configuration();

    // Then call the parent class fromXML
    settings->KisPropertiesConfiguration::toXML(doc, rootElt);

    delete settings;
}


KisPaintOpSettingsSP KisEraseOpSettings::clone() const
{

    KisPaintOpSettings* settings = dynamic_cast<KisPaintOpSettings*>(m_options->configuration());
    return settings;

}
