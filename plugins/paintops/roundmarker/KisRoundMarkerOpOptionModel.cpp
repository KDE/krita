/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisRoundMarkerOpOptionModel.h"

using namespace KisWidgetConnectionUtils;

KisRoundMarkerOpOptionModel::KisRoundMarkerOpOptionModel(lager::cursor<KisRoundMarkerOpOptionData> _optionData)
    : optionData(_optionData)    
    , LAGER_QT(diameter) {_optionData[&KisRoundMarkerOpOptionData::diameter]}
    , LAGER_QT(spacing) {_optionData[&KisRoundMarkerOpOptionData::spacing]}
    , LAGER_QT(useAutoSpacing) {_optionData[&KisRoundMarkerOpOptionData::useAutoSpacing]}
    , LAGER_QT(autoSpacingCoeff) {_optionData[&KisRoundMarkerOpOptionData::autoSpacingCoeff]}
    , LAGER_QT(aggregatedSpacing) {lager::with(LAGER_QT(spacing),
                                          LAGER_QT(useAutoSpacing),
                                          LAGER_QT(autoSpacingCoeff))
             .xform(zug::map(ToSpacingState{}),
                    zug::map(FromSpacingState{}))}
{
}
