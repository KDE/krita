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
#include <KoColor.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_paintop_registry.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paint_information.h>
#include <kis_paint_action_type_option.h>

#include "kis_dyna_paintop_settings.h"
#include "kis_dyna_paintop_settings_widget.h"
#include "kis_dynaop_option.h"

KisDynaPaintOpSettings::KisDynaPaintOpSettings(KisDynaPaintOpSettingsWidget* settingsWidget)
        : KisPaintOpSettings(settingsWidget)
{
    m_options = settingsWidget;
    m_options->writeConfiguration( this );
}

KisPaintOpSettingsSP KisDynaPaintOpSettings::clone() const
{
    KisPaintOpSettings* settings =
        static_cast<KisPaintOpSettings*>( m_options->configuration() );
    return settings;
}


bool KisDynaPaintOpSettings::paintIncremental()
{
    return m_options->m_paintActionTypeOption->paintActionType() == BUILDUP;
}



void KisDynaPaintOpSettings::fromXML(const QDomElement& elt)
{
    KisPaintOpSettings::fromXML( elt );
    m_options->setConfiguration( this );
}

void KisDynaPaintOpSettings::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    KisPropertiesConfiguration * settings = m_options->configuration();
    settings->KisPropertiesConfiguration::toXML( doc, rootElt );
    delete settings;
}

int KisDynaPaintOpSettings::radius() const
{
    return m_options->m_dynaOption->radius();
}

qreal KisDynaPaintOpSettings::coverage() const
{
    return m_options->m_dynaOption->coverage();
}

qreal KisDynaPaintOpSettings::amount() const
{
    return m_options->m_dynaOption->amount();
}


bool KisDynaPaintOpSettings::jitterMovement() const
{
    return m_options->m_dynaOption->jitterMovement();
}

bool KisDynaPaintOpSettings::jitterSize() const
{
    return m_options->m_dynaOption->jitterSize();
}

bool KisDynaPaintOpSettings::useParticles() const
{
    return m_options->m_dynaOption->useParticles();
}

