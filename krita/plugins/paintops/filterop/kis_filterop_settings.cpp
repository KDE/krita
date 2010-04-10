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

#include "kis_filterop_settings.h"

#include <QDomDocument>

#include "kis_filterop_settings_widget.h"

#include <kis_filter_option.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_registry.h>
#include <filter/kis_filter_configuration.h>
#include <kis_node.h>
#include <kis_image.h>
#include <kis_types.h>
#include <kis_paint_device.h>

KisFilterOpSettings::KisFilterOpSettings()
{
    setPropertyNotSaved(FILTER_CONFIGURATION);
}

KisFilterOpSettings::~KisFilterOpSettings()
{
}

bool KisFilterOpSettings::paintIncremental()
{
    return true; // We always paint on the existing data
}

void KisFilterOpSettings::setNode(KisNodeSP node)
{
    KisFilterOpSettingsWidget* options = dynamic_cast<KisFilterOpSettingsWidget*>(optionsWidget());

    KisPaintOpSettings::setNode(node);
    if (options) {
        options->m_filterOption->setNode(node);
    }
}

void KisFilterOpSettings::setImage(KisImageWSP image)
{
    KisFilterOpSettingsWidget* options = dynamic_cast<KisFilterOpSettingsWidget*>(optionsWidget());

    if (options) {
        options->m_filterOption->setImage(image);
    }
}

KisFilterConfiguration* KisFilterOpSettings::filterConfig() const
{
    if (hasProperty(FILTER_ID)) {
        KisFilterSP filter = KisFilterRegistry::instance()->get(getString(FILTER_ID));
        Q_ASSERT(filter);
        if(filter) {
            KisFilterConfiguration* configuration = filter->factoryConfiguration(0);
            configuration->fromXML(getString(FILTER_CONFIGURATION));
            return configuration;
        }
    }
    return 0;
}

void KisFilterOpSettings::toXML(QDomDocument& doc, QDomElement& root) const
{
    KisPaintOpSettings::toXML(doc, root);
    KisFilterConfiguration* configuration = filterConfig();
    if (configuration) {
        QDomElement e = doc.createElement("filterconfig");
        configuration->toXML(doc, e);
        root.appendChild(e);
    }
    delete configuration;
}

void KisFilterOpSettings::fromXML(const QDomElement& e)
{
    KisPaintOpSettings::fromXML(e);
    QDomElement element = e.firstChildElement("filterconfig");
    if (hasProperty(FILTER_ID)) {
        KisFilterSP filter = KisFilterRegistry::instance()->get(getString(FILTER_ID));
        Q_ASSERT(filter);
        if(filter) {
            KisFilterConfiguration* configuration = filter->factoryConfiguration(0);
            configuration->fromXML(element);
            setProperty(FILTER_CONFIGURATION, configuration->toXML());
            delete configuration;
        }
    }
}

