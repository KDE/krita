/*
 *  SPDX-FileCopyrightText: 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisHatchingOptionsModel.h"

#include <KisLager.h>

using namespace KisWidgetConnectionUtils;

KisHatchingOptionsModel::KisHatchingOptionsModel(lager::cursor<KisHatchingOptionsData> _optionData)
    : optionData(_optionData)
    , LAGER_QT(angle) {_optionData[&KisHatchingOptionsData::angle]}
    , LAGER_QT(separation) {_optionData[&KisHatchingOptionsData::separation]}
    , LAGER_QT(thickness) {_optionData[&KisHatchingOptionsData::thickness]}
    , LAGER_QT(originX) {_optionData[&KisHatchingOptionsData::originX]}
    , LAGER_QT(originY) {_optionData[&KisHatchingOptionsData::originY]}
    , LAGER_QT(crosshatchingStyle) {_optionData[&KisHatchingOptionsData::crosshatchingStyle].zoom(kislager::lenses::do_static_cast<CrosshatchingType, int>)}
    , LAGER_QT(separationIntervals) {_optionData[&KisHatchingOptionsData::separationIntervals]}
{
}
