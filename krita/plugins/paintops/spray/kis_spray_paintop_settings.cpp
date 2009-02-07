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
#include "kis_spray_paintop_settings.h"

#include <KoColorSpaceRegistry.h>
#include <KoColor.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_paintop_registry.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paint_information.h>
#include <kis_paint_action_type_option.h>

#include "kis_spray_paintop_settings_widget.h"
#include "kis_sprayop_option.h"

KisSprayPaintOpSettings::KisSprayPaintOpSettings(KisSprayPaintOpSettingsWidget* settingsWidget)
        : KisPaintOpSettings(settingsWidget)
{
    m_options = settingsWidget;
    m_options->writeConfiguration( this );
}

KisPaintOpSettingsSP KisSprayPaintOpSettings::clone() const
{
    KisPaintOpSettings* settings =
        static_cast<KisPaintOpSettings*>( m_options->configuration() );
    return settings;
}


bool KisSprayPaintOpSettings::paintIncremental()
{
    return m_options->m_paintActionTypeOption->paintActionType() == BUILDUP;
}



void KisSprayPaintOpSettings::fromXML(const QDomElement& elt)
{
    KisPaintOpSettings::fromXML( elt );
    m_options->setConfiguration( this );
}

void KisSprayPaintOpSettings::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    KisPropertiesConfiguration * settings = m_options->configuration();
    settings->KisPropertiesConfiguration::toXML( doc, rootElt );
    delete settings;
}

int KisSprayPaintOpSettings::radius() const
{
    return m_options->m_sprayOption->radius();
}

qreal KisSprayPaintOpSettings::coverage() const
{
    return m_options->m_sprayOption->coverage();
}

qreal KisSprayPaintOpSettings::amount() const
{
    return m_options->m_sprayOption->amount();
}


bool KisSprayPaintOpSettings::jitterMovement() const
{
    return m_options->m_sprayOption->jitterMovement();
}

bool KisSprayPaintOpSettings::jitterSize() const
{
    return m_options->m_sprayOption->jitterSize();
}

bool KisSprayPaintOpSettings::useParticles() const
{
    return m_options->m_sprayOption->useParticles();
}

