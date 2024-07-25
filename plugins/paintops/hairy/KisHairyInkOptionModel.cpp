/*
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisHairyInkOptionModel.h"

using namespace KisWidgetConnectionUtils;

KisHairyInkOptionModel::KisHairyInkOptionModel(lager::cursor<KisHairyInkOptionData> _optionData)
    : optionData(_optionData)
    , LAGER_QT(inkDepletionEnabled) {_optionData[&KisHairyInkOptionData::inkDepletionEnabled]}
    , LAGER_QT(inkAmount) {_optionData[&KisHairyInkOptionData::inkAmount]}
    , LAGER_QT(inkDepletionCurve) {_optionData[&KisHairyInkOptionData::inkDepletionCurve]}
    , LAGER_QT(useSaturation) {_optionData[&KisHairyInkOptionData::useSaturation]}
    , LAGER_QT(useOpacity) {_optionData[&KisHairyInkOptionData::useOpacity]}
    , LAGER_QT(useWeights) {_optionData[&KisHairyInkOptionData::useWeights]}
    , LAGER_QT(pressureWeight) {_optionData[&KisHairyInkOptionData::pressureWeight]}
    , LAGER_QT(bristleLengthWeight) {_optionData[&KisHairyInkOptionData::bristleLengthWeight]}
    , LAGER_QT(bristleInkAmountWeight) {_optionData[&KisHairyInkOptionData::bristleInkAmountWeight]}
    , LAGER_QT(inkDepletionWeight) {_optionData[&KisHairyInkOptionData::inkDepletionWeight]}
    , LAGER_QT(useSoakInk) {_optionData[&KisHairyInkOptionData::useSoakInk]}
{
}
