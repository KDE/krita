/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
#include "mypaint_paintop_settings.h"

#include <KoColorSpaceRegistry.h>
#include <KoViewConverter.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_paintop_registry.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paint_information.h>

#include <KoColor.h>

#include "mypaint_paintop_settings_widget.h"

MyPaintSettings::MyPaintSettings()
        : m_options(0)
{
}


KisPaintOpSettingsSP MyPaintSettings::clone() const
{
    KisPaintOpSettings* settings =
        dynamic_cast<KisPaintOpSettings*>( m_options->configuration() );
    return settings;
}

void MyPaintSettings::fromXML(const QDomElement& elt)
{
    // First, call the parent class fromXML to make sure all the
    // properties are saved to the map
    KisPaintOpSettings::fromXML( elt );
    // Then load the properties for all widgets
    m_options->setConfiguration( this );
}

void MyPaintSettings::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    // First, make sure all the option widgets have saved their state
    // to the property configuration
    KisPropertiesConfiguration * settings = m_options->configuration();
    // Then call the parent class fromXML
    settings->KisPropertiesConfiguration::toXML( doc, rootElt );
    delete settings;
}

MyPaintBrushResource* MyPaintSettings::brush() const
{
    return m_options->brush();;
}
