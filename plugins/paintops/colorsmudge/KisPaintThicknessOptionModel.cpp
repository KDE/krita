/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisPaintThicknessOptionModel.h"

namespace {

auto paintThicknessLens = lager::lenses::getset (
    [] (const KisPaintThicknessOptionData::ThicknessMode &mode) {
        return mode == KisPaintThicknessOptionData::OVERWRITE ?
            0 :
            1;
    },
    [] (KisPaintThicknessOptionData::ThicknessMode, int value) {
         return value == 0 ?
            KisPaintThicknessOptionData::OVERWRITE :
            KisPaintThicknessOptionData::OVERLAY;
    });

} // namespace

KisPaintThicknessOptionModel::KisPaintThicknessOptionModel(lager::cursor<KisPaintThicknessOptionMixIn> _optionData)
    : optionData(_optionData)
    , LAGER_QT(mode) {optionData[&KisPaintThicknessOptionMixIn::mode].zoom(paintThicknessLens)}
{
}
