/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_hatching_paintop_settings_widget.h"

#include "kis_hatching_paintop_settings.h"

#include "KisHatchingStandardOptions.h"

#include <kis_brush_option_widget.h>
#include <kis_paintop_settings_widget.h>
#include <KisPaintOpOptionWidgetUtils.h>

#include "KisHatchingOptionsWidget.h"
#include "KisHatchingPreferencesWidget.h"
#include <KisStandardOptionData.h>
#include <KisCompositeOpOptionWidget.h>
#include "KisSizeOptionWidget.h"
#include "KisMirrorOptionWidget.h"
#include <KisPaintingModeOptionWidget.h>
#include "KisTextureOptionWidget.h"

KisHatchingPaintOpSettingsWidget:: KisHatchingPaintOpSettingsWidget(QWidget* parent, KisResourcesInterfaceSP resourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    : KisBrushBasedPaintopOptionWidget(KisBrushOptionWidgetFlag::SupportsPrecision, parent)
{
    Q_UNUSED(canvasResourcesInterface)
    namespace kpowu = KisPaintOpOptionWidgetUtils;

    //-------Adding widgets to the screen------------

    addPaintOpOption(kpowu::createOptionWidgetWithLodLimitations<KisHatchingOptionsWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisHatchingPreferencesWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisCompositeOpOptionWidget>());
    addPaintOpOption(kpowu::createCurveOptionWidget(KisSeparationOptionData(), KisPaintOpOption::GENERAL, i18n("0.0"), i18n("1.0")));
    addPaintOpOption(kpowu::createCurveOptionWidget(KisThicknessOptionData(), KisPaintOpOption::GENERAL, i18n("0.0"), i18n("1.0")));
    addPaintOpOption(kpowu::createCurveOptionWidget(KisAngleOptionData(), KisPaintOpOption::GENERAL, i18n("0.0"), i18n("1.0")));
    addPaintOpOption(kpowu::createCurveOptionWidget(KisCrosshatchingOptionData(), KisPaintOpOption::GENERAL, i18n("0.0"), i18n("1.0")));
    addPaintOpOption(kpowu::createOpacityOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisSizeOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisMirrorOptionWidget>());

    addPaintOpOption(kpowu::createOptionWidget<KisPaintingModeOptionWidget>());

    addPaintOpOption(kpowu::createOptionWidget<KisTextureOptionWidget>(KisTextureOptionData(), resourcesInterface));
    addPaintOpOption(kpowu::createStrengthOptionWidget());

}

KisHatchingPaintOpSettingsWidget::~ KisHatchingPaintOpSettingsWidget()
{
}

KisPropertiesConfigurationSP  KisHatchingPaintOpSettingsWidget::configuration() const
{
    KisHatchingPaintOpSettingsSP config = new KisHatchingPaintOpSettings(resourcesInterface());
    config->setProperty("paintop", "hatchingbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
