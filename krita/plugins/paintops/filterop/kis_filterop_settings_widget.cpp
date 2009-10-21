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

#include "kis_filterop_settings_widget.h"
#include "kis_filterop_settings.h"

#include <kis_properties_configuration.h>
#include <kis_brush_option.h>
#include <filter/kis_filter.h>
#include <kis_image.h>
#include <kis_paint_device.h>

#include <kis_paintop_options_widget.h>
#include <kis_pressure_size_option.h>
#include <kis_filter_option.h>

KisFilterOpSettingsWidget::KisFilterOpSettingsWidget(QWidget* parent)
        : KisPaintOpOptionsWidget(parent)
{
    setObjectName("filter option widget");

    m_brushOption = new KisBrushOption();
    m_sizeOption = new KisPressureSizeOption();
    m_filterOption = new KisFilterOption();

    addPaintOpOption(m_brushOption);
    addPaintOpOption(m_sizeOption);
    addPaintOpOption(m_filterOption);

}

KisFilterOpSettingsWidget::~KisFilterOpSettingsWidget()
{
    delete m_brushOption;
    delete m_sizeOption;
    delete m_filterOption;
}

KisPropertiesConfiguration* KisFilterOpSettingsWidget::configuration() const
{
    KisFilterOpSettings *config = new KisFilterOpSettings();
    config->setOptionsWidget(const_cast<KisFilterOpSettingsWidget*>(this));
    config->setProperty("paintop", "filter"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

void KisFilterOpSettingsWidget::setImage(KisImageWSP image)
{
    m_brushOption->setImage(image);
}

#include "kis_filterop_settings_widget.moc"
