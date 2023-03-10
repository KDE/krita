/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisParticleOpOptionModel.h"

using namespace KisWidgetConnectionUtils;

KisParticleOpOptionModel::KisParticleOpOptionModel(lager::cursor<KisParticleOpOptionData> _optionData)
    : optionData(_optionData)
    , LAGER_QT(particleCount) {_optionData[&KisParticleOpOptionData::particleCount]}
    , LAGER_QT(particleIterations) {_optionData[&KisParticleOpOptionData::particleIterations]}
    , LAGER_QT(particleGravity) {_optionData[&KisParticleOpOptionData::particleGravity]}
    , LAGER_QT(particleWeight) {_optionData[&KisParticleOpOptionData::particleWeight]}
    , LAGER_QT(particleScaleX) {_optionData[&KisParticleOpOptionData::particleScaleX]}
    , LAGER_QT(particleScaleY) {_optionData[&KisParticleOpOptionData::particleScaleY]}
{
}
