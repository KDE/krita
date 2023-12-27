/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisColorOptionModel.h"


KisColorOptionModel::KisColorOptionModel(lager::cursor<KisColorOptionData> _optionData)
    : optionData(_optionData)
    , LAGER_QT(useRandomHSV) {_optionData[&KisColorOptionData::useRandomHSV]}
    , LAGER_QT(useRandomOpacity) {_optionData[&KisColorOptionData::useRandomOpacity]}
    , LAGER_QT(sampleInputColor) {_optionData[&KisColorOptionData::sampleInputColor]}
    
    , LAGER_QT(fillBackground) {_optionData[&KisColorOptionData::fillBackground]}
    , LAGER_QT(colorPerParticle) {_optionData[&KisColorOptionData::colorPerParticle]}
    , LAGER_QT(mixBgColor) {_optionData[&KisColorOptionData::mixBgColor]}
    
    , LAGER_QT(hue) {_optionData[&KisColorOptionData::hue]}
    , LAGER_QT(saturation) {_optionData[&KisColorOptionData::saturation]}
    , LAGER_QT(value) {_optionData[&KisColorOptionData::value]}
{
}
