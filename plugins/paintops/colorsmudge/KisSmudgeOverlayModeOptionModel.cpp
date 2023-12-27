/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSmudgeOverlayModeOptionModel.h"

KisSmudgeOverlayModeOptionModel::KisSmudgeOverlayModeOptionModel(lager::cursor<KisSmudgeOverlayModeOptionData> _optionData,
                                                                 lager::reader<bool> _overlayModeAllowed)
    : optionData(_optionData)
    , overlayModeAllowed(_overlayModeAllowed)
    , LAGER_QT(isChecked) {optionData[&KisSmudgeOverlayModeOptionData::isChecked]}
{
}

KisSmudgeOverlayModeOptionData KisSmudgeOverlayModeOptionModel::bakedOptionData() const
{
    KisSmudgeOverlayModeOptionData data = optionData.get();
    data.isChecked &= overlayModeAllowed.get();
    return data;
}
