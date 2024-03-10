/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoSvgTextPropertiesModel.h"
#include "KoSvgTextProperties.h"
#include <lager/constant.hpp>

namespace {

auto createTextProperty = lager::lenses::getset(
            [](const KoSvgTextPropertyData value) {
    return value.commonProperties.propertyOrDefault(KoSvgTextProperties::WritingModeId);
},
    [](KoSvgTextPropertyData value, const QVariant &variant) {
        value.commonProperties.setProperty(KoSvgTextProperties::WritingModeId, variant);
        return value;
    }
        );
}

KoSvgTextPropertiesModel::KoSvgTextPropertiesModel(lager::cursor<KoSvgTextPropertyData> _textData)
    : textData(_textData)
    , LAGER_QT(writingMode) {lager::with(_textData).zoom(createTextProperty)}
{
}
