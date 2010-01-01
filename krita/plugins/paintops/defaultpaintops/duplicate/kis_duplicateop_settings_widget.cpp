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

#include "kis_duplicateop_settings_widget.h"
#include "kis_duplicateop_settings.h"
#include "kis_duplicateop_option.h"

#include <kis_image.h>
#include <kis_properties_configuration.h>
#include <kis_brush_option_widget.h>
#include <kis_paintop_options_widget.h>
#include <kis_pressure_size_option.h>
#include <kis_curve_option_widget.h>


KisDuplicateOpSettingsWidget::KisDuplicateOpSettingsWidget(QWidget* parent)
        : KisPaintOpOptionsWidget(parent)
{
    setObjectName("brush option widget");

    m_brushOption = new KisBrushOptionWidget;
    m_duplicateOption = new KisDuplicateOpOption;

    addPaintOpOption(m_brushOption);
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption()));
    addPaintOpOption(m_duplicateOption);
}

KisDuplicateOpSettingsWidget::~KisDuplicateOpSettingsWidget()
{
    delete m_brushOption;
    delete m_duplicateOption;
}

KisPropertiesConfiguration* KisDuplicateOpSettingsWidget::configuration() const
{
    KisDuplicateOpSettings *config =
        new KisDuplicateOpSettings(0);
    config->setOptionsWidget(const_cast<KisDuplicateOpSettingsWidget*>(this));
    config->setProperty("paintop", "duplicate"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

#include "kis_duplicateop_settings_widget.moc"
