/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisPaintingModeOptionModel.h"

#include <KisLager.h>

using ToControlState = KisWidgetConnectionUtils::ToControlState;

namespace {
int calcEffectivePaintingMode(enumPaintingMode mode, bool maskingBrushEnabled) {
    return static_cast<int>(maskingBrushEnabled ? enumPaintingMode::WASH : mode);
}
}

KisPaintingModeOptionModel::KisPaintingModeOptionModel(lager::cursor<KisPaintingModeOptionData> _optionData, lager::reader<bool> _maskingBrushEnabled)
    : optionData(_optionData)
    , maskingBrushEnabled(_maskingBrushEnabled)
    , LAGER_QT(paintingMode) {optionData[&KisPaintingModeOptionData::paintingMode].zoom(kislager::lenses::do_static_cast<enumPaintingMode, int>)}
    , LAGER_QT(effectivePaintingMode) {
        lager::with(optionData[&KisPaintingModeOptionData::paintingMode],
                    maskingBrushEnabled)
            .map(&calcEffectivePaintingMode)}
    , LAGER_QT(paintingModeState) {
        lager::with(LAGER_QT(effectivePaintingMode),
                    maskingBrushEnabled.map(std::logical_not{}))
            .map(ToControlState{})}
{
}

KisPaintingModeOptionData KisPaintingModeOptionModel::bakedOptionData() const
{
    KisPaintingModeOptionData data;
    data.paintingMode = static_cast<enumPaintingMode>(effectivePaintingMode());
    return data;
}
