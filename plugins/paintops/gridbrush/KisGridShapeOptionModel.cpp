/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisGridShapeOptionModel.h"


KisGridShapeOptionModel::KisGridShapeOptionModel(lager::cursor<KisGridShapeOptionData> _optionData)
    : optionData(_optionData)
    , LAGER_QT(shape) {_optionData[&KisGridShapeOptionData::shape]}
{
}
