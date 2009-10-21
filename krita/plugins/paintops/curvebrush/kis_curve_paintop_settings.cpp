/*
 *  Copyright (c) 2008,2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include <kis_curve_paintop_settings.h>
#include <kis_curve_paintop_settings_widget.h>

KisCurvePaintOpSettings::KisCurvePaintOpSettings()
        : m_options(0)
{
}


KisPaintOpSettingsSP KisCurvePaintOpSettings::clone() const
{
    KisPaintOpSettings* settings =
        static_cast<KisPaintOpSettings*>(m_options->configuration());
    return settings;
}

bool KisCurvePaintOpSettings::paintIncremental()
{
    return false;
}

int KisCurvePaintOpSettings::minimalDistance() const
{
    return m_options->minimalDistance();
}


int KisCurvePaintOpSettings::curveAction() const
{
    return m_options->curveAction();
}

int KisCurvePaintOpSettings::interval() const
{
    return m_options->interval();
}


void KisCurvePaintOpSettings::fromXML(const QDomElement& elt)
{
    KisPaintOpSettings::fromXML(elt);
    m_options->setConfiguration(this);
}

void KisCurvePaintOpSettings::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    KisPropertiesConfiguration * settings = m_options->configuration();
    settings->KisPropertiesConfiguration::toXML(doc, rootElt);
    delete settings;
}

