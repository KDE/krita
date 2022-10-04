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
#include <KisPaintingModeOptionWidget.h>
#include <KisPaintOpOptionUtils.h>
#include <kis_compositeop_option.h>
#include <klocalizedstring.h>

KisGridPaintOpSettingsWidget:: KisGridPaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    addPaintOpOption(new KisGridOpOption());
    addPaintOpOption(new KisGridShapeOption());
    addPaintOpOption(new KisCompositeOpOption(true));
    addPaintOpOption(new KisColorOption());
    namespace kpou = KisPaintOpOptionUtils;
    addPaintOpOption(kpou::createOptionWidget<KisPaintingModeOptionWidget>());
}

KisGridPaintOpSettingsWidget::~ KisGridPaintOpSettingsWidget()
{
}

KisPropertiesConfigurationSP  KisGridPaintOpSettingsWidget::configuration() const
{
    KisGridPaintOpSettings* config = new KisGridPaintOpSettings(resourcesInterface());
    config->setProperty("paintop", "gridbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
