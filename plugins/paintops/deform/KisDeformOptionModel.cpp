/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDeformOptionModel.h"

#include <KisLager.h>

using namespace KisWidgetConnectionUtils;

KisDeformOptionModel::KisDeformOptionModel(lager::cursor<KisDeformOptionData> _optionData)
    : optionData(_optionData)
    , LAGER_QT(deformAmount) {_optionData[&KisDeformOptionData::deformAmount]}
    , LAGER_QT(deformUseBilinear) {_optionData[&KisDeformOptionData::deformUseBilinear]}
    , LAGER_QT(deformUseCounter) {_optionData[&KisDeformOptionData::deformUseCounter]}
    , LAGER_QT(deformUseOldData) {_optionData[&KisDeformOptionData::deformUseOldData]}
    , LAGER_QT(deformAction) {_optionData[&KisDeformOptionData::deformAction].zoom(kislager::lenses::do_static_cast<DeformModes, int>)}
{
}
