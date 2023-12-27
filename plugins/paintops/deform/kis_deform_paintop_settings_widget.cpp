/*
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_deform_paintop_settings.h"
#include "kis_deform_paintop_settings_widget.h"
#include "KisDeformOptionWidget.h"

#include <kis_paintop_settings_widget.h>
#include "KisBrushSizeOptionWidget.h"

#include <KisStandardOptionData.h>
#include <KisSizeOptionWidget.h>
#include <KisAirbrushOptionWidget.h>
#include <KisPaintOpOptionWidgetUtils.h>
#include <KisCompositeOpOptionWidget.h>

KisDeformPaintOpSettingsWidget::KisDeformPaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    namespace kpowu = KisPaintOpOptionWidgetUtils;

    addPaintOpOption(kpowu::createOptionWidget<KisBrushSizeOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidgetWithLodLimitations<KisDeformOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisCompositeOpOptionWidget>());
    addPaintOpOption(kpowu::createOpacityOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisSizeOptionWidget>());
    addPaintOpOption(kpowu::createRotationOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisAirbrushOptionWidget>());
    addPaintOpOption(kpowu::createRateOptionWidget());
}

KisDeformPaintOpSettingsWidget::~ KisDeformPaintOpSettingsWidget()
{
}


KisPropertiesConfigurationSP KisDeformPaintOpSettingsWidget::configuration() const
{
    KisDeformPaintOpSettings* config = new KisDeformPaintOpSettings(resourcesInterface());
    config->setProperty("paintop", "deformBrush");
    writeConfiguration(config);
    return config;
}

