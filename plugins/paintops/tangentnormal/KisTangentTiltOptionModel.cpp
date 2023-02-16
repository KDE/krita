/*
 * SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KisTangentTiltOptionModel.h"

#include <KisLager.h>

using namespace KisWidgetConnectionUtils;

KisTangentTiltOptionModel::KisTangentTiltOptionModel(lager::cursor<KisTangentTiltOptionData> _optionData)
    : optionData(_optionData)    
    , LAGER_QT(redChannel) {_optionData[&KisTangentTiltOptionData::redChannel]}
    , LAGER_QT(greenChannel) {_optionData[&KisTangentTiltOptionData::greenChannel]}
    , LAGER_QT(blueChannel) {_optionData[&KisTangentTiltOptionData::blueChannel]}
    , LAGER_QT(directionType) {_optionData[&KisTangentTiltOptionData::directionType].zoom(kislager::lenses::do_static_cast<TangentTiltDirectionType, int>)}
    , LAGER_QT(elevationSensitivity) {_optionData[&KisTangentTiltOptionData::elevationSensitivity]}
    , LAGER_QT(mixValue) {_optionData[&KisTangentTiltOptionData::mixValue]}
{
}
