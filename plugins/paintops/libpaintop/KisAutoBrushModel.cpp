/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisAutoBrushModel.h"

#include <KisLager.h>
#include <lager/constant.hpp>
#include <lager/lenses.hpp>


KisAutoBrushModel::KisAutoBrushModel(lager::cursor<CommonData> commonData, lager::cursor<AutoBrushData> autoBrushData, lager::cursor<qreal> commonBrushSizeData)
    : m_commonData(commonData),
      m_autoBrushData(autoBrushData),
      m_commonBrushSizeData(commonBrushSizeData),
      LAGER_QT(diameter) {m_commonBrushSizeData},
      LAGER_QT(ratio) {m_autoBrushData[&AutoBrushData::generator][&AutoBrushGeneratorData::ratio]},
      LAGER_QT(horizontalFade) {m_autoBrushData[&AutoBrushData::generator][&AutoBrushGeneratorData::horizontalFade]},
      LAGER_QT(verticalFade) {m_autoBrushData[&AutoBrushData::generator][&AutoBrushGeneratorData::verticalFade]},
      LAGER_QT(spikes) {m_autoBrushData[&AutoBrushData::generator][&AutoBrushGeneratorData::spikes]},
      LAGER_QT(antialiasEdges) {m_autoBrushData[&AutoBrushData::generator][&AutoBrushGeneratorData::antialiasEdges]},
      LAGER_QT(shape) {m_autoBrushData[&AutoBrushData::generator][&AutoBrushGeneratorData::shape]
                  .zoom(kislager::lenses::do_static_cast<AutoBrushGeneratorShape, int>)},
      LAGER_QT(type) {m_autoBrushData[&AutoBrushData::generator][&AutoBrushGeneratorData::type]
                  .zoom(kislager::lenses::do_static_cast<AutoBrushGeneratorType, int>)},
      LAGER_QT(curveString) {m_autoBrushData[&AutoBrushData::generator][&AutoBrushGeneratorData::curveString]},
      LAGER_QT(randomness) {m_autoBrushData[&AutoBrushData::randomness]
                  .zoom(kislager::lenses::scale<qreal>(100.0))},
      LAGER_QT(density) {m_autoBrushData[&AutoBrushData::density]
                  .zoom(kislager::lenses::scale<qreal>(100.0))},
      LAGER_QT(angle) {m_commonData[&CommonData::angle]
                  .zoom(kislager::lenses::scale<qreal>(180.0 / M_PI))},
      LAGER_QT(spacing) {m_commonData[&CommonData::spacing]},
      LAGER_QT(useAutoSpacing) {m_commonData[&CommonData::useAutoSpacing]},
      LAGER_QT(autoSpacingCoeff) {m_commonData[&CommonData::autoSpacingCoeff]},
      LAGER_QT(aggregatedSpacing) {lager::with(LAGER_QT(spacing),
                                               LAGER_QT(useAutoSpacing),
                                               LAGER_QT(autoSpacingCoeff))
                  .xform(zug::map(ToSpacingState{}),
                         zug::map(FromSpacingState{}))}
{
}

AutoBrushData KisAutoBrushModel::bakedOptionData() const
{
    AutoBrushData data = m_autoBrushData.get();
    data.generator.diameter = m_commonBrushSizeData.get();
    return data;
}
