/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisRoundMarkerOpOptionModel.h"

#include <KisZug.h>

using namespace KisWidgetConnectionUtils;

KisRoundMarkerOpOptionModel::KisRoundMarkerOpOptionModel(lager::cursor<KisRoundMarkerOpOptionData> _optionData)
    : optionData(_optionData)    
    , LAGER_QT(diameter) {_optionData[&KisRoundMarkerOpOptionData::diameter]}
    , LAGER_QT(spacing) {_optionData[&KisRoundMarkerOpOptionData::spacing]}
    , LAGER_QT(use_auto_spacing) {_optionData[&KisRoundMarkerOpOptionData::use_auto_spacing]}
    , LAGER_QT(auto_spacing_coeff) {_optionData[&KisRoundMarkerOpOptionData::auto_spacing_coeff]}
    , LAGER_QT(aggregatedSpacing) {lager::with(LAGER_QT(spacing),
                                          LAGER_QT(use_auto_spacing),
                                          LAGER_QT(auto_spacing_coeff))
             .xform(zug::map(ToSpacingState{}),
                    zug::map(FromSpacingState{}))}
{
}
