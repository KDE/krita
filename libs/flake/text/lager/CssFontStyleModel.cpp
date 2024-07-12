/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "CssFontStyleModel.h"

namespace  {

auto getValue =  lager::lenses::getset (
            [] (const KoSvgText::CssFontStyleData &data) -> qreal {
    if (data.style == QFont::StyleOblique) {
        if (data.slantValue.isAuto) {
            return 14;
        } else {
            return data.slantValue.customValue;
        }
    }
    return 0;
}, [] (KoSvgText::CssFontStyleData data, const qreal &val) -> KoSvgText::CssFontStyleData {
        if (data.style == QFont::StyleOblique) {
            data.slantValue.isAuto = false;
            data.slantValue = val;
        }
        return data;
});

auto getStyle =  lager::lenses::getset (
            [] (const KoSvgText::CssFontStyleData &data) -> CssFontStyleModel::FontStyle {
    return CssFontStyleModel::FontStyle(data.style);
}, [] (KoSvgText::CssFontStyleData data, const CssFontStyleModel::FontStyle &val) -> KoSvgText::CssFontStyleData {
        data.style = QFont::Style(val);
        if (data.style != QFont::StyleOblique) {
            data.slantValue.isAuto = true;
            data.slantValue = 0;
        }
        return data;
});
}

CssFontStyleModel::CssFontStyleModel(lager::cursor<KoSvgText::CssFontStyleData> _data)
    : data(_data)
    , LAGER_QT(style) {data.zoom(getStyle)}
    , LAGER_QT(value) {data.zoom(getValue)}
{

}
