/*
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_colorsmudgeop_settings_widget.h"
#include "kis_brush_based_paintop_settings.h"
#include "kis_brush_option_widget.h"

#include <kis_properties_configuration.h>
#include <kis_paintop_settings_widget.h>
#include "kis_colorsmudgeop_settings.h"
#include "kis_signals_blocker.h"
#include <KisAirbrushOptionWidget.h>
#include <KisPaintOpOptionWidgetUtils.h>
#include <KisTextureOptionWidget.h>
#include <KisStandardOptionData.h>
#include <KisCompositeOpOptionWidget.h>
#include <KisSpacingOptionWidget.h>
#include <KisSizeOptionWidget.h>
#include <KisMirrorOptionWidget.h>
#include <KisScatterOptionWidget.h>
#include <KisSmudgeLengthOptionWidget.h>
#include <KisPaintThicknessOptionWidget.h>
#include <KisSmudgeOverlayModeOptionWidget.h>
#include <KisBrushPropertiesModel.h>
#include <KisColorSmudgeStandardOptionData.h>
#include <KisSmudgeRadiusOptionData.h>
#include <KisZug.h>


struct KisColorSmudgeOpSettingsWidget::Private
{
    Private(lager::reader<KisBrushModel::BrushData> brushData,
            KisResourcesInterfaceSP resourcesInterface)
        : brushPropertiesModel(brushData, resourcesInterface)
    {
    }

    KisBrushPropertiesModel brushPropertiesModel;
};

KisColorSmudgeOpSettingsWidget::KisColorSmudgeOpSettingsWidget(QWidget* parent, KisResourcesInterfaceSP resourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    : KisBrushBasedPaintopOptionWidget(KisBrushOptionWidgetFlag::SupportsPrecision |
                                       KisBrushOptionWidgetFlag::SupportsHSLBrushMode,
                                       parent)
    , m_d(new Private(brushOptionWidget()->bakedBrushData(), resourcesInterface))
{
    Q_UNUSED(canvasResourcesInterface)
    namespace kpowu = KisPaintOpOptionWidgetUtils;

    setObjectName("brush option widget");

    addPaintOpOption(kpowu::createOptionWidget<KisCompositeOpOptionWidget>());
    addPaintOpOption(kpowu::createOpacityOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisSizeOptionWidget>());
    addPaintOpOption(kpowu::createRatioOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisSpacingOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisMirrorOptionWidget>());


    KisSmudgeLengthOptionWidget *smudgeLengthWidget =
        kpowu::createOptionWidget<KisSmudgeLengthOptionWidget>
            (KisSmudgeLengthOptionData(),
             m_d->brushPropertiesModel.isBrushPierced,
             m_d->brushPropertiesModel.brushApplication
                 .xform(kiszug::map_greater<int>(ALPHAMASK)));

    addPaintOpOption(smudgeLengthWidget);

    lager::reader<std::tuple<qreal, qreal>> rangeReader =
        smudgeLengthWidget->useNewEngine()
            .map([] (bool useNewEngine) {
                return std::make_tuple(0.0,
                                       useNewEngine ? 1.0 : 3.0);
            });

    KisCurveOptionWidget *smudgeRadiusWidget =
        kpowu::createCurveOptionWidget(KisSmudgeRadiusOptionData(),
                                      KisPaintOpOption::GENERAL,
                                      lager::make_constant(true),
                                      rangeReader);

    addPaintOpOption(smudgeRadiusWidget);

    addPaintOpOption(kpowu::createCurveOptionWidget(KisColorRateOptionData(), KisPaintOpOption::GENERAL));

    addPaintOpOption(kpowu::createOptionWidget<KisPaintThicknessOptionWidget>(KisPaintThicknessOptionData(), brushOptionWidget()->lightnessModeEnabled()));

    addPaintOpOption(kpowu::createRotationOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisScatterOptionWidget>());

    addPaintOpOption(kpowu::createOptionWidget<KisSmudgeOverlayModeOptionWidget>(
                         KisSmudgeOverlayModeOptionData(),
                         brushOptionWidget()->
                             lightnessModeEnabled()
                             .map(std::logical_not{})));

    addPaintOpOption(kpowu::createCurveOptionWidget(KisGradientOptionData(), KisPaintOpOption::GENERAL));

    addPaintOpOption(kpowu::createHueOptionWidget());
    addPaintOpOption(kpowu::createSaturationOptionWidget());
    addPaintOpOption(kpowu::createValueOptionWidget());

    addPaintOpOption(kpowu::createOptionWidget<KisAirbrushOptionWidget>());
    addPaintOpOption(kpowu::createRateOptionWidget());

    addPaintOpOption(kpowu::createOptionWidget<KisTextureOptionWidget>(KisTextureOptionData(), resourcesInterface));
    addPaintOpOption(kpowu::createCurveOptionWidget(KisStrengthOptionData(), KisPaintOpOption::COLOR, i18n("Weak"), i18n("Strong")));
}

KisColorSmudgeOpSettingsWidget::~KisColorSmudgeOpSettingsWidget() { }

KisPropertiesConfigurationSP KisColorSmudgeOpSettingsWidget::configuration() const
{
    KisColorSmudgeOpSettingsSP config = new KisColorSmudgeOpSettings(resourcesInterface());
    config->setProperty("paintop", "colorsmudge");
    writeConfiguration(config);
    return config;
}
