/*
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisBrushSizeOptionModel.h"

#include <KisLager.h>

using namespace KisWidgetConnectionUtils;

KisBrushSizeOptionModel::KisBrushSizeOptionModel(lager::cursor<KisBrushSizeOptionData> _optionData)
    : optionData(_optionData)
    , LAGER_QT(brushDiameter) {_optionData[&KisBrushSizeOptionData::brushDiameter]}
    , LAGER_QT(brushAspect) {_optionData[&KisBrushSizeOptionData::brushAspect]}
    , LAGER_QT(brushRotation) {_optionData[&KisBrushSizeOptionData::brushRotation]}
    , LAGER_QT(brushScale) {_optionData[&KisBrushSizeOptionData::brushScale]}
    , LAGER_QT(brushSpacing) {_optionData[&KisBrushSizeOptionData::brushSpacing]}
    , LAGER_QT(brushDensity) {_optionData[&KisBrushSizeOptionData::brushDensity]
                .zoom(kislager::lenses::scale<qreal>(100.0))}
    , LAGER_QT(brushJitterMovement) {_optionData[&KisBrushSizeOptionData::brushJitterMovement]}
    , LAGER_QT(brushJitterMovementEnabled) {_optionData[&KisBrushSizeOptionData::brushJitterMovementEnabled]}
{
}
