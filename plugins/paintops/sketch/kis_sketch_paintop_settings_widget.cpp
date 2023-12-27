/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_sketch_paintop_settings_widget.h"
#include "kis_sketch_paintop_settings.h"

#include <kis_paintop_settings_widget.h>
#include <KisPaintOpOptionWidgetUtils.h>

#include "KisSketchOpOptionWidget.h"
#include <KisCompositeOpOptionWidget.h>
#include <KisStandardOptionData.h>
#include "KisSizeOptionWidget.h"
#include "KisSketchStandardOptionData.h"
#include <KisAirbrushOptionWidget.h>
#include <KisPaintingModeOptionWidget.h>


KisSketchPaintOpSettingsWidget::KisSketchPaintOpSettingsWidget(QWidget* parent)
    : KisBrushBasedPaintopOptionWidget(KisBrushOptionWidgetFlag::None, parent)
{
    namespace kpowu = KisPaintOpOptionWidgetUtils;

    addPaintOpOption(kpowu::createOptionWidgetWithLodLimitations<KisSketchOpOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisCompositeOpOptionWidget>());
    addPaintOpOption(kpowu::createOpacityOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisSizeOptionWidget>());
    addPaintOpOption(kpowu::createRotationOptionWidget());
    addPaintOpOption(kpowu::createCurveOptionWidget(KisLineWidthOptionData(), KisPaintOpOption::GENERAL, i18n("0%"), i18n("100%")));
    addPaintOpOption(kpowu::createCurveOptionWidget(KisOffsetScaleOptionData(), KisPaintOpOption::GENERAL, i18n("0%"), i18n("100%")));
    addPaintOpOption(kpowu::createCurveOptionWidget(KisDensityOptionData(), KisPaintOpOption::GENERAL, i18n("0%"), i18n("100%")));
    addPaintOpOption(kpowu::createOptionWidget<KisAirbrushOptionWidget>(KisAirbrushOptionData(), false));
    addPaintOpOption(kpowu::createRateOptionWidget());

    KisPaintingModeOptionData defaultModeData;
    defaultModeData.paintingMode = enumPaintingMode::BUILDUP;
    addPaintOpOption(kpowu::createOptionWidget<KisPaintingModeOptionWidget>(defaultModeData));
}

KisSketchPaintOpSettingsWidget::~ KisSketchPaintOpSettingsWidget()
{
}

KisPropertiesConfigurationSP  KisSketchPaintOpSettingsWidget::configuration() const
{
    KisSketchPaintOpSettingsSP config = new KisSketchPaintOpSettings(resourcesInterface());
    config->setProperty("paintop", "sketchbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

