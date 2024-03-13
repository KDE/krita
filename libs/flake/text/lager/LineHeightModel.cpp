/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "LineHeightModel.h"

namespace  {

auto getValue =  lager::lenses::getset (
            [] (const KoSvgText::LineHeightInfo &data) -> qreal {
    if (data.isNumber) {
        return data.value;
    } else {
        return data.length.value;
    }
}, [] (KoSvgText::LineHeightInfo data, const qreal &val) -> KoSvgText::LineHeightInfo {
        if (data.isNumber) {
        data.value = val;
        } else {
            data.length.value = val;
        }
        return data;
});

auto getUnit =  lager::lenses::getset (
            [] (const KoSvgText::LineHeightInfo &data) -> LineHeightModel::LineHeightType {
    if (data.isNumber) {
        return LineHeightModel::Lines;
    } else {
        switch(data.length.unit) {
        case KoSvgText::CssLengthPercentage::Absolute:
            return LineHeightModel::LineHeightType::Absolute;
        case KoSvgText::CssLengthPercentage::Em:
            return LineHeightModel::LineHeightType::Em;
        case KoSvgText::CssLengthPercentage::Ex:
            return LineHeightModel::LineHeightType::Ex;
        case KoSvgText::CssLengthPercentage::Percentage:
            return LineHeightModel::LineHeightType::Percentage;
        }
        return LineHeightModel::LineHeightType::Absolute;
    }
}, [] (KoSvgText::LineHeightInfo data, const LineHeightModel::LineHeightType &val) -> KoSvgText::LineHeightInfo {
        if (val == LineHeightModel::Lines) {
            data.isNumber = true;
        } else {
        switch(val) {
        case LineHeightModel::LineHeightType::Absolute:
            data.length.unit = KoSvgText::CssLengthPercentage::Absolute;
        break;
        case LineHeightModel::LineHeightType::Em:
            data.length.unit = KoSvgText::CssLengthPercentage::Em;
        break;
        case LineHeightModel::LineHeightType::Ex:
            data.length.unit = KoSvgText::CssLengthPercentage::Ex;
        break;
        case LineHeightModel::LineHeightType::Percentage:
            data.length.unit = KoSvgText::CssLengthPercentage::Percentage;
        break;
        default:
        break;
        }
        }
        return data;

});
}

LineHeightModel::LineHeightModel(lager::cursor<KoSvgText::LineHeightInfo> _data)
    : data(_data)
    , LAGER_QT(isNormal) {data[&KoSvgText::LineHeightInfo::isNormal]}
    , LAGER_QT(value) {data.zoom(getValue)}
    , LAGER_QT(unit) {data.zoom(getUnit)}
{

}
