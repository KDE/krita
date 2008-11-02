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
#include <KoColorSpaceRegistry.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_paintop_registry.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paint_information.h>

#include <KoColor.h>
#include <qdebug.h>

#include <kis_deform_paintop_settings_widget.h>
#include <kis_deform_paintop_settings.h>

KisDeformPaintOpSettings::KisDeformPaintOpSettings(KisDeformPaintOpSettingsWidget* settingsWidget)
        : KisPaintOpSettings(settingsWidget)
{
    m_options = settingsWidget;
    // Initialize with the default settings from the widget
    m_options->writeConfiguration( this );
}


KisPaintOpSettingsSP KisDeformPaintOpSettings::clone() const
{
    KisPaintOpSettings* settings =
        static_cast<KisPaintOpSettings*>( m_options->configuration() );
    return settings;
}

bool KisDeformPaintOpSettings::paintIncremental()
{
    return true;
}

int KisDeformPaintOpSettings::radius() const
{
    return m_options->radius();
}


double KisDeformPaintOpSettings::deformAmount() const
{
    return m_options->deformAmount();
}

bool KisDeformPaintOpSettings::bilinear() const
{
    return m_options->bilinear();
}

bool KisDeformPaintOpSettings::useMovementPaint() const
{
    return m_options->useMovementPaint();
}

int KisDeformPaintOpSettings::deformAction() const
{
    return m_options->deformAction();
}

void KisDeformPaintOpSettings::fromXML(const QDomElement& elt)
{
    // First, call the parent class fromXML to make sure all the
    // properties are saved to the map
    KisPaintOpSettings::fromXML( elt );
    // Then load the properties for all widgets
    m_options->setConfiguration( this );
}

void KisDeformPaintOpSettings::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    // First, make sure all the option widgets have saved their state
    // to the property configuration
    KisPropertiesConfiguration * settings = m_options->configuration();
    // Then call the parent class fromXML
    settings->KisPropertiesConfiguration::toXML( doc, rootElt );
    delete settings;
}

