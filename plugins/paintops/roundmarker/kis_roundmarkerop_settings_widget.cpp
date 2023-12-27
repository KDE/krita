/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_roundmarkerop_settings_widget.h"
#include "kis_brush_based_paintop_settings.h"

#include <kis_properties_configuration.h>
#include <kis_paintop_settings_widget.h>
#include "kis_roundmarkerop_settings.h"
#include <KisRoundMarkerOpOptionWidget.h>
#include <KisPaintOpOptionWidgetUtils.h>
#include <KisCompositeOpOptionWidget.h>
#include "KisSizeOptionWidget.h"
#include "KisSpacingOptionWidget.h"


KisRoundMarkerOpSettingsWidget::KisRoundMarkerOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    namespace kpowu = KisPaintOpOptionWidgetUtils;

    setObjectName("roundmarker option widget");
    //setPrecisionEnabled(true);

    addPaintOpOption(kpowu::createOptionWidget<KisRoundMarkerOpOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisCompositeOpOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisSizeOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisSpacingOptionWidget>());
}

KisRoundMarkerOpSettingsWidget::~KisRoundMarkerOpSettingsWidget() { }

KisPropertiesConfigurationSP KisRoundMarkerOpSettingsWidget::configuration() const
{
    KisRoundMarkerOpSettings *config = new KisRoundMarkerOpSettings(resourcesInterface());
    config->setProperty("paintop", "roundmarker");
    writeConfiguration(config);
    return config;
}
