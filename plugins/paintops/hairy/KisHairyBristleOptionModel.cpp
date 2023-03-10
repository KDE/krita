/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisHairyBristleOptionModel.h"

using namespace KisWidgetConnectionUtils;

KisHairyBristleOptionModel::KisHairyBristleOptionModel(lager::cursor<KisHairyBristleOptionData> _optionData)
    : optionData(_optionData)
    , LAGER_QT(useMousePressure) {_optionData[&KisHairyBristleOptionData::useMousePressure]}
    , LAGER_QT(scaleFactor) {_optionData[&KisHairyBristleOptionData::scaleFactor]}
    , LAGER_QT(randomFactor) {_optionData[&KisHairyBristleOptionData::randomFactor]}
    , LAGER_QT(shearFactor) {_optionData[&KisHairyBristleOptionData::shearFactor]}
    , LAGER_QT(densityFactor) {_optionData[&KisHairyBristleOptionData::densityFactor]}
    , LAGER_QT(threshold) {_optionData[&KisHairyBristleOptionData::threshold]}
    , LAGER_QT(antialias) {_optionData[&KisHairyBristleOptionData::antialias]}
    , LAGER_QT(useCompositing) {_optionData[&KisHairyBristleOptionData::useCompositing]}
    , LAGER_QT(connectedPath) {_optionData[&KisHairyBristleOptionData::connectedPath]}
{
}
