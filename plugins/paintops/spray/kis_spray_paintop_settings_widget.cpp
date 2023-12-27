/*
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_spray_paintop_settings_widget.h"

#include "kis_spray_paintop_settings.h"

#include <KisColorOptionWidget.h>
#include <kis_paintop_settings_widget.h>

#include <KisPaintingModeOptionWidget.h>
#include <KisPaintOpOptionWidgetUtils.h>
#include <kis_brush_option_widget.h>
#include <KisAirbrushOptionWidget.h>
#include <KisPaintOpOptionWidgetUtils.h>
#include <KisSizeOptionWidget.h>
#include <KisStandardOptionData.h>
#include <KisCompositeOpOptionWidget.h>
#include <KisSprayOpOptionWidget.h>
#include <KisSprayShapeDynamicsOptionWidget.h>
#include <KisSprayShapeOptionWidget.h>




KisSprayPaintOpSettingsWidget:: KisSprayPaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    namespace kpowu = KisPaintOpOptionWidgetUtils;


	KisSprayOpOptionWidget* sprayOpWidget = kpowu::createOptionWidget<KisSprayOpOptionWidget>();
	
    addPaintOpOption(sprayOpWidget);
    
    addPaintOpOption(kpowu::createOptionWidget<KisSprayShapeOptionWidget>(KisSprayShapeOptionData(), sprayOpWidget->diameter(), sprayOpWidget->scale()));
    addPaintOpOption(new KisBrushOptionWidget(KisBrushOptionWidgetFlag::None));
    addPaintOpOption(kpowu::createOpacityOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisSizeOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisCompositeOpOptionWidget>());

    addPaintOpOption(kpowu::createOptionWidget<KisSprayShapeDynamicsOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisColorOptionWidget>());

    addPaintOpOption(kpowu::createRotationOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisAirbrushOptionWidget>());
    addPaintOpOption(kpowu::createRateOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisPaintingModeOptionWidget>());
}

KisSprayPaintOpSettingsWidget::~ KisSprayPaintOpSettingsWidget()
{
}

KisPropertiesConfigurationSP  KisSprayPaintOpSettingsWidget::configuration() const
{
    KisSprayPaintOpSettings* config = new KisSprayPaintOpSettings(resourcesInterface());
    config->setProperty("paintop", "spraybrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
