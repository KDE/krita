/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisPredefinedBrushModel.h"

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
    values << i18n("Alpha Mask");
    toolTips << i18nc("@info:tooltip", "Luminosity of the brush tip image is used as alpha channel for the stroke");
    if (brushType == IMAGE || brushType == PIPE_IMAGE) {
        values << i18n("Color Image");
        toolTips << i18nc("@info:tooltip", "The brush tip image is painted as it is");
        if (supportsHSLBrushTips) {
            values << i18n("Lightness Map");
            toolTips << i18nc("@info:tooltip", "Luminosity of the brush tip image is used as lightness correction for the painting color. Alpha channel of the brush tip image is used as alpha for the final stroke");
            values << i18n("Gradient Map");
            toolTips << i18nc("@info:tooltip", "The brush tip maps its value to the currently selected gradient. Alpha channel of the brush tip image is used as alpha for the final stroke");
        }
    }


    int currentValue = std::clamp(static_cast<int>(application), 0, values.size() - 1);
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
        brushTypeString = i18n("Animated Mask"); // GIH brush
        animatedBrushTipSelectionMode = data.parasiteSelection;
    } else if (data.brushType == PIPE_IMAGE ) {
        brushTypeString = i18n("Animated Image");
    }

    const QString brushDetailsText = QString("%1 (%2 x %3) %4")
            .arg(brushTypeString)
            .arg(data.baseSize.width())
            .arg(data.baseSize.height())
            .arg(animatedBrushTipSelectionMode);

    return brushDetailsText;
}

}

KisPredefinedBrushModel::KisPredefinedBrushModel(lager::cursor<CommonData> commonData,
                                                 lager::cursor<PredefinedBrushData> predefinedBrushData,
                                                 lager::cursor<qreal> commonBrushSizeData,
                                                 bool supportsHSLBrushTips)
    : m_commonData(commonData),
      m_predefinedBrushData(predefinedBrushData),
      m_supportsHSLBrushTips(supportsHSLBrushTips),
      m_commonBrushSizeData(commonBrushSizeData),
      LAGER_QT(resourceSignature) {m_predefinedBrushData[&PredefinedBrushData::resourceSignature]},
      LAGER_QT(baseSize) {m_predefinedBrushData[&PredefinedBrushData::baseSize]},
      LAGER_QT(brushSize) {m_commonBrushSizeData},
      LAGER_QT(application) {m_predefinedBrushData[&PredefinedBrushData::application]
                  .zoom(kiszug::lenses::do_static_cast<enumBrushApplication, int>)},
      LAGER_QT(hasColorAndTransparency) {m_predefinedBrushData[&PredefinedBrushData::hasColorAndTransparency]},
      LAGER_QT(autoAdjustMidPoint) {m_predefinedBrushData[&PredefinedBrushData::autoAdjustMidPoint]},
      LAGER_QT(adjustmentMidPoint) {m_predefinedBrushData[&PredefinedBrushData::adjustmentMidPoint]
                  .zoom(kiszug::lenses::do_static_cast<quint8, int>)},
      LAGER_QT(brightnessAdjustment) {m_predefinedBrushData[&PredefinedBrushData::brightnessAdjustment]
                  .xform(kiszug::map_mupliply<qreal>(100.0) | kiszug::map_round,
                         kiszug::map_static_cast<qreal> | kiszug::map_mupliply<qreal>(0.01))},
      LAGER_QT(contrastAdjustment) {m_predefinedBrushData[&PredefinedBrushData::contrastAdjustment]
                  .xform(kiszug::map_mupliply<qreal>(100.0) | kiszug::map_round,
                         kiszug::map_static_cast<qreal> | kiszug::map_mupliply<qreal>(0.01))},
      LAGER_QT(angle) {m_commonData[&CommonData::angle]
                  .zoom(kiszug::lenses::scale<qreal>(180.0 / M_PI))},
      LAGER_QT(spacing) {m_commonData[&CommonData::spacing]},
      LAGER_QT(useAutoSpacing) {m_commonData[&CommonData::useAutoSpacing]},
      LAGER_QT(autoSpacingCoeff) {m_commonData[&CommonData::autoSpacingCoeff]},
      LAGER_QT(aggregatedSpacing) {lager::with(LAGER_QT(spacing),
                                               LAGER_QT(useAutoSpacing),
                                               LAGER_QT(autoSpacingCoeff))
                  .xform(zug::map(ToSpacingState{}),
                         zug::map(FromSpacingState{}))},
      LAGER_QT(applicationSwitchState){lager::with(
                  m_predefinedBrushData[&PredefinedBrushData::brushType],
                  m_supportsHSLBrushTips,
                  m_predefinedBrushData[&PredefinedBrushData::application])
                  .xform(zug::map(&calcApplicationSwitchState))},
      LAGER_QT(adjustmentsEnabled){LAGER_QT(applicationSwitchState)[&ComboBoxState::currentIndex]
                  .xform(kiszug::map_greater<int>(1))},
      LAGER_QT(brushName) {LAGER_QT(resourceSignature)[&KoResourceSignature::name]},
      LAGER_QT(brushDetails) {m_predefinedBrushData.map(&calcBrushDetails)},
      LAGER_QT(lightnessModeEnabled)
          {LAGER_QT(applicationSwitchState)
                [&ComboBoxState::currentIndex].
                  xform(kiszug::map_equal<int>(LIGHTNESSMAP))}
{
}


PredefinedBrushData KisPredefinedBrushModel::bakedOptionData() const
{
    PredefinedBrushData data = m_predefinedBrushData.get();
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
