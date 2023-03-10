/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisScatterOptionModel.h"

KisScatterOptionModel::KisScatterOptionModel(lager::cursor<KisScatterOptionMixIn> optionData)
    : scatterOptionData(optionData)
    , LAGER_QT(axisX) {scatterOptionData[&KisScatterOptionMixIn::axisX]}
    , LAGER_QT(axisY) {scatterOptionData[&KisScatterOptionMixIn::axisY]}
{
}
