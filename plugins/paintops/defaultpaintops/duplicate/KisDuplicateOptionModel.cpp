/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisDuplicateOptionModel.h"

KisDuplicateOptionModel::KisDuplicateOptionModel(lager::cursor<KisDuplicateOptionData> optionData)
    : optionData(optionData)
    , LAGER_QT(healing) {optionData[&KisDuplicateOptionData::healing]}
    , LAGER_QT(correctPerspective) {optionData[&KisDuplicateOptionData::correctPerspective]}
    , LAGER_QT(moveSourcePoint) {optionData[&KisDuplicateOptionData::moveSourcePoint]}
    , LAGER_QT(resetSourcePoint) {optionData[&KisDuplicateOptionData::resetSourcePoint]}
    , LAGER_QT(cloneFromProjection) {optionData[&KisDuplicateOptionData::cloneFromProjection]}
{
}
