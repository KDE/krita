/*
 * SPDX-FileCopyrightText: 2009 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_grid_paintop_settings_widget.h"

#include "kis_grid_paintop_settings.h"
#include "KisGridShapeOptionWidget.h"
#include "KisGridOpOptionWidget.h"


#include <KisColorOptionWidget.h>

#include <kis_paintop_settings_widget.h>
#include <KisPaintingModeOptionWidget.h>
#include <KisPaintOpOptionWidgetUtils.h>
#include <klocalizedstring.h>
#include <KisCompositeOpOptionWidget.h>

KisGridPaintOpSettingsWidget:: KisGridPaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    namespace kpowu = KisPaintOpOptionWidgetUtils;
    addPaintOpOption(kpowu::createOptionWidget<KisGridOpOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisGridShapeOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisCompositeOpOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisColorOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisPaintingModeOptionWidget>());
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
