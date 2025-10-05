/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisPredefinedBrushModel.h"

#include <KisGlobalResourcesInterface.h>
#include <kis_predefined_brush_factory.h>
#include <lager/lenses/tuple.hpp>
#include <KisZug.h>
#include <KisLager.h>

namespace {

auto brushSizeLens = lager::lenses::getset(
    [](const std::tuple<QSize, qreal> &x) -> qreal { return std::get<0>(x).width() * std::get<1>(x); },
    [](std::tuple<QSize, qreal> x, qreal brushSize) -> std::tuple<QSize, qreal> {
        return std::make_tuple(std::get<0>(x), brushSize / std::get<0>(x).width());
    });

ComboBoxState calcApplicationSwitchState(enumBrushType brushType, bool supportsHSLBrushTips, enumBrushApplication application)
{
    QStringList values;
    QStringList toolTips;
    values << i18n("Alpha mask");
    toolTips << i18nc("@info:tooltip", "Luminosity of the brush tip image is used as alpha channel for the stroke");
    if (brushType == IMAGE || brushType == PIPE_IMAGE) {
        values << i18n("Color image");
        toolTips << i18nc("@info:tooltip", "The brush tip image is painted as it is");
        if (supportsHSLBrushTips) {
            values << i18n("Lightness map");
            toolTips << i18nc("@info:tooltip", "Luminosity of the brush tip image is used as lightness correction for the painting color. Alpha channel of the brush tip image is used as alpha for the final stroke");
            values << i18n("Gradient map");
            toolTips << i18nc("@info:tooltip", "The brush tip maps its value to the currently selected gradient. Alpha channel of the brush tip image is used as alpha for the final stroke");
        }
    }


    int currentValue = std::clamp((int)application, 0, (int)values.size() - 1);
    return ComboBoxState{values, currentValue, values.size() > 1, toolTips};
}

QString calcBrushDetails(PredefinedBrushData data)
{
    QString brushTypeString = "";

    QString animatedBrushTipSelectionMode;

    if (data.brushType == INVALID) {
        brushTypeString = i18n("Invalid");
    } else if (data.brushType == MASK) {
        brushTypeString = i18n("Mask");
    } else if (data.brushType == IMAGE) {
        brushTypeString = i18n("Image");
    } else if (data.brushType == PIPE_MASK ) {
        brushTypeString = i18n("Animated mask"); // GIH brush
        animatedBrushTipSelectionMode = data.parasiteSelection;
    } else if (data.brushType == PIPE_IMAGE ) {
        brushTypeString = i18n("Animated image");
    }

    const QString brushDetailsText = QString("%1 (%2 × %3) %4")
            .arg(brushTypeString)
            .arg(data.baseSize.width())
            .arg(data.baseSize.height())
            .arg(animatedBrushTipSelectionMode);

    return brushDetailsText;
}

auto effectiveResourceData = lager::lenses::getset(
    [](const PredefinedBrushData &predefinedDataArg) {
        if (predefinedDataArg.resourceSignature != KoResourceSignature()) {
            return predefinedDataArg;
        }

        CommonData commonData;

        /// NOTE: we cannot just pass the data by value because of the
        /// bug in lager: https://github.com/arximboldi/lager/issues/160
        PredefinedBrushData predefinedData = predefinedDataArg;

        auto source = KisGlobalResourcesInterface::instance()->source<KisBrush>(ResourceType::Brushes);

        KisBrushSP fallbackResource = source.fallbackResource();
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(fallbackResource, predefinedData);

        KisPredefinedBrushFactory::loadFromBrushResource(commonData, predefinedData, fallbackResource);

        /// NOTE: we discard `commonData` returned from the loaded brush, because
        /// we expect spacing and size information to be shared between auto and
        /// predefined brushes. If the user wants to load the settings embedded
        /// into the brush itself he/she should reset it explicitly via the GUI
        /// controls.

        return predefinedData;
    },
    [](const PredefinedBrushData&,
       const PredefinedBrushData &y) {

        return y;
    });

} // namespace


KisPredefinedBrushModel::KisPredefinedBrushModel(lager::cursor<CommonData> commonData,
                                                 lager::cursor<PredefinedBrushData> predefinedBrushData,
                                                 lager::cursor<qreal> commonBrushSizeData,
                                                 bool supportsHSLBrushTips)
    : m_commonData(commonData),
      m_predefinedBrushData(predefinedBrushData),
      m_supportsHSLBrushTips(supportsHSLBrushTips),
      m_commonBrushSizeData(commonBrushSizeData),
      m_effectivePredefinedData(m_predefinedBrushData.zoom(effectiveResourceData)),
      LAGER_QT(resourceSignature) {m_effectivePredefinedData[&PredefinedBrushData::resourceSignature]},
      LAGER_QT(baseSize) {m_effectivePredefinedData[&PredefinedBrushData::baseSize]},
      LAGER_QT(brushSize) {m_commonBrushSizeData},
      LAGER_QT(application) {m_effectivePredefinedData[&PredefinedBrushData::application]
                  .zoom(kislager::lenses::do_static_cast<enumBrushApplication, int>)},
      LAGER_QT(hasColorAndTransparency) {m_effectivePredefinedData[&PredefinedBrushData::hasColorAndTransparency]},
      LAGER_QT(autoAdjustMidPoint) {m_effectivePredefinedData[&PredefinedBrushData::autoAdjustMidPoint]},
      LAGER_QT(adjustmentMidPoint) {m_effectivePredefinedData[&PredefinedBrushData::adjustmentMidPoint]
                  .zoom(kislager::lenses::do_static_cast<quint8, int>)},
      LAGER_QT(brightnessAdjustment) {m_effectivePredefinedData[&PredefinedBrushData::brightnessAdjustment]
                  .xform(kiszug::map_multiply<qreal>(100.0) | kiszug::map_round,
                         kiszug::map_static_cast<qreal> | kiszug::map_multiply<qreal>(0.01))},
      LAGER_QT(contrastAdjustment) {m_effectivePredefinedData[&PredefinedBrushData::contrastAdjustment]
                  .xform(kiszug::map_multiply<qreal>(100.0) | kiszug::map_round,
                         kiszug::map_static_cast<qreal> | kiszug::map_multiply<qreal>(0.01))},
      LAGER_QT(angle) {m_commonData[&CommonData::angle]
                  .zoom(kislager::lenses::scale<qreal>(180.0 / M_PI))},
      LAGER_QT(spacing) {m_commonData[&CommonData::spacing]},
      LAGER_QT(useAutoSpacing) {m_commonData[&CommonData::useAutoSpacing]},
      LAGER_QT(autoSpacingCoeff) {m_commonData[&CommonData::autoSpacingCoeff]},
      LAGER_QT(aggregatedSpacing) {lager::with(LAGER_QT(spacing),
                                               LAGER_QT(useAutoSpacing),
                                               LAGER_QT(autoSpacingCoeff))
                  .xform(zug::map(ToSpacingState{}),
                         zug::map(FromSpacingState{}))},
      LAGER_QT(applicationSwitchState){lager::with(
                  m_effectivePredefinedData[&PredefinedBrushData::brushType],
                  m_supportsHSLBrushTips,
                  m_effectivePredefinedData[&PredefinedBrushData::application])
                  .xform(zug::map(&calcApplicationSwitchState))},
      LAGER_QT(adjustmentsEnabled){LAGER_QT(applicationSwitchState)[&ComboBoxState::currentIndex]
                  .xform(kiszug::map_greater<int>(1))},
      LAGER_QT(brushName) {LAGER_QT(resourceSignature)[&KoResourceSignature::name]},
      LAGER_QT(brushDetails) {m_effectivePredefinedData.map(&calcBrushDetails)},
      LAGER_QT(lightnessModeEnabled)
          {LAGER_QT(applicationSwitchState)
                [&ComboBoxState::currentIndex].
                  xform(kiszug::map_equal<int>(LIGHTNESSMAP))}
{
}


PredefinedBrushData KisPredefinedBrushModel::bakedOptionData() const
{
    PredefinedBrushData data = m_effectivePredefinedData.get();
    data.application =
        static_cast<enumBrushApplication>(
            LAGER_QT(applicationSwitchState)->currentIndex);
    data.scale = m_commonBrushSizeData.get() / data.baseSize.width();

    return data;

}

enumBrushApplication KisPredefinedBrushModel::effectiveBrushApplication(PredefinedBrushData predefinedData, bool supportsHSLBrushTips)
{
    return static_cast<enumBrushApplication>(calcApplicationSwitchState(predefinedData.brushType, supportsHSLBrushTips, predefinedData.application).currentIndex);
}

qreal KisPredefinedBrushModel::effectiveBrushSize(PredefinedBrushData predefinedData)
{
    return lager::view(brushSizeLens, std::make_tuple(predefinedData.baseSize, predefinedData.scale));
}

void KisPredefinedBrushModel::setEffectiveBrushSize(PredefinedBrushData &predefinedData, qreal value)
{
    std::tie(predefinedData.baseSize, predefinedData.scale) =
        lager::set(brushSizeLens, std::make_tuple(predefinedData.baseSize, predefinedData.scale), value);
}
