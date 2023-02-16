/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisAirbrushOptionModel.h"

KisAirbrushOptionModel::KisAirbrushOptionModel(lager::cursor<KisAirbrushOptionData> optionData)
    : airbrushOptionData(optionData)
    , LAGER_QT(isChecked) {airbrushOptionData[&KisAirbrushOptionData::isChecked]}
    , LAGER_QT(airbrushRate) {airbrushOptionData[&KisAirbrushOptionData::airbrushRate]}
    , LAGER_QT(ignoreSpacing) {airbrushOptionData[&KisAirbrushOptionData::ignoreSpacing]}
{
}
