/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "CssLengthPercentageModel.h"
#include <KisLager.h>

CssLengthPercentageModel::CssLengthPercentageModel(lager::cursor<KoSvgText::CssLengthPercentage> _data)
    : length(_data)
    , LAGER_QT(value) {length[&KoSvgText::CssLengthPercentage::value]}
, LAGER_QT(unitType) {length[&KoSvgText::CssLengthPercentage::unit].zoom(kislager::lenses::do_static_cast<KoSvgText::CssLengthPercentage::UnitType, int>)}
{}
