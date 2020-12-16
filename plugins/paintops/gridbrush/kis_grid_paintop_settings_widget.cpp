/*
 * SPDX-FileCopyrightText: 2009 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_grid_paintop_settings_widget.h"

#include "kis_gridop_option.h"
#include "kis_grid_paintop_settings.h"
#include "kis_grid_shape_option.h"

#include <kis_color_option.h>

#include <kis_paintop_settings_widget.h>
#include <kis_paint_action_type_option.h>
#include <kis_compositeop_option.h>
#include <klocalizedstring.h>

KisGridPaintOpSettingsWidget:: KisGridPaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    m_gridOption =  new KisGridOpOption();
    m_gridShapeOption = new KisGridShapeOption();
    m_ColorOption = new KisColorOption();

    addPaintOpOption(m_gridOption, i18n("Brush size"));
    addPaintOpOption(m_gridShapeOption, i18n("Particle type"));
    addPaintOpOption(new KisCompositeOpOption(true), i18n("Blending Mode"));
    addPaintOpOption(m_ColorOption, i18n("Color options"));
    addPaintOpOption(new KisPaintActionTypeOption(), i18n("Painting Mode"));
}

KisGridPaintOpSettingsWidget::~ KisGridPaintOpSettingsWidget()
{
}

KisPropertiesConfigurationSP  KisGridPaintOpSettingsWidget::configuration() const
{
    KisGridPaintOpSettings* config = new KisGridPaintOpSettings(resourcesInterface());
    config->setOptionsWidget(const_cast<KisGridPaintOpSettingsWidget*>(this));
    config->setProperty("paintop", "gridbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
