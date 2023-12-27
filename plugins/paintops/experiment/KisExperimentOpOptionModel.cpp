/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisExperimentOpOptionModel.h"

#include <KisLager.h>


KisExperimentOpOptionModel::KisExperimentOpOptionModel(lager::cursor<KisExperimentOpOptionData> _optionData)
    : optionData(_optionData)
    , LAGER_QT(isDisplacementEnabled) {_optionData[&KisExperimentOpOptionData::isDisplacementEnabled]}
    , LAGER_QT(displacement) {_optionData[&KisExperimentOpOptionData::displacement]}
    , LAGER_QT(isSpeedEnabled) {_optionData[&KisExperimentOpOptionData::isSpeedEnabled]}
    , LAGER_QT(speed) {_optionData[&KisExperimentOpOptionData::speed]}
    , LAGER_QT(isSmoothingEnabled) {_optionData[&KisExperimentOpOptionData::isSmoothingEnabled]}
    , LAGER_QT(smoothing) {_optionData[&KisExperimentOpOptionData::smoothing]}
    , LAGER_QT(windingFill) {_optionData[&KisExperimentOpOptionData::windingFill]}
    , LAGER_QT(hardEdge) {_optionData[&KisExperimentOpOptionData::hardEdge]}
    , LAGER_QT(fillType) {_optionData[&KisExperimentOpOptionData::fillType].zoom(kislager::lenses::do_static_cast<ExperimentFillType, int>)}
    
{
}
