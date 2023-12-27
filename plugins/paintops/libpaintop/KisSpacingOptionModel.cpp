/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSpacingOptionModel.h"

KisSpacingOptionModel::KisSpacingOptionModel(lager::cursor<KisSpacingOptionMixIn> optionData)
    : spacingOptionData(optionData)
    , LAGER_QT(useSpacingUpdates) {spacingOptionData[&KisSpacingOptionMixIn::useSpacingUpdates]}
    , LAGER_QT(isotropicSpacing) {spacingOptionData[&KisSpacingOptionMixIn::isotropicSpacing]}
{
}
