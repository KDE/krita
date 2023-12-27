/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSketchOpOptionModel.h"

#include <KisLager.h>

using namespace KisWidgetConnectionUtils;

KisSketchOpOptionModel::KisSketchOpOptionModel(lager::cursor<KisSketchOpOptionData> _optionData)
    : optionData(_optionData)
    , LAGER_QT(offset) {_optionData[&KisSketchOpOptionData::offset]}
    , LAGER_QT(probability) {_optionData[&KisSketchOpOptionData::probability]
                .zoom(kislager::lenses::scale<qreal>(100.0))}
    , LAGER_QT(simpleMode) {_optionData[&KisSketchOpOptionData::simpleMode]}
    , LAGER_QT(makeConnection) {_optionData[&KisSketchOpOptionData::makeConnection]}
    , LAGER_QT(magnetify) {_optionData[&KisSketchOpOptionData::magnetify]}
    , LAGER_QT(randomRGB) {_optionData[&KisSketchOpOptionData::randomRGB]}
    , LAGER_QT(randomOpacity) {_optionData[&KisSketchOpOptionData::randomOpacity]}
    , LAGER_QT(distanceOpacity) {_optionData[&KisSketchOpOptionData::distanceOpacity]}
    , LAGER_QT(distanceDensity) {_optionData[&KisSketchOpOptionData::distanceDensity]}
    , LAGER_QT(antiAliasing) {_optionData[&KisSketchOpOptionData::antiAliasing]}
    , LAGER_QT(lineWidth) {_optionData[&KisSketchOpOptionData::lineWidth]}
{
}
