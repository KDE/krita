/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSprayShapeDynamicsOptionModel.h"

#include <KisLager.h>


KisSprayShapeDynamicsOptionModel::KisSprayShapeDynamicsOptionModel(lager::cursor<KisSprayShapeDynamicsOptionData> _optionData)
    : optionData(_optionData)
    , LAGER_QT(enabled) {_optionData[&KisSprayShapeDynamicsOptionData::enabled]}
    , LAGER_QT(randomSize) {_optionData[&KisSprayShapeDynamicsOptionData::randomSize]}
    , LAGER_QT(fixedRotation) {_optionData[&KisSprayShapeDynamicsOptionData::fixedRotation]}
    , LAGER_QT(randomRotation) {_optionData[&KisSprayShapeDynamicsOptionData::randomRotation]}
    , LAGER_QT(followCursor) {_optionData[&KisSprayShapeDynamicsOptionData::followCursor]}
    , LAGER_QT(followDrawingAngle) {_optionData[&KisSprayShapeDynamicsOptionData::followDrawingAngle]}
    , LAGER_QT(fixedAngle) {_optionData[&KisSprayShapeDynamicsOptionData::fixedAngle].zoom(kislager::lenses::do_static_cast<quint16, qreal>)}
    , LAGER_QT(randomRotationWeight) {_optionData[&KisSprayShapeDynamicsOptionData::randomRotationWeight]}
    , LAGER_QT(followCursorWeight) {_optionData[&KisSprayShapeDynamicsOptionData::followCursorWeight]}
    , LAGER_QT(followDrawingAngleWeight) {_optionData[&KisSprayShapeDynamicsOptionData::followDrawingAngleWeight]}
{
}
