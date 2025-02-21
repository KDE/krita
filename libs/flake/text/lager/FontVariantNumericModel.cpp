/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "FontVariantNumericModel.h"
#include <KisLager.h>

FontVariantNumericModel::FontVariantNumericModel(lager::cursor<KoSvgText::FontFeatureNumeric> _data)
    : data(_data)
    , LAGER_QT(figureStyle) {data[&KoSvgText::FontFeatureNumeric::style].zoom(kislager::lenses::do_static_cast<KoSvgText::NumericFigureStyle, int>)}
    , LAGER_QT(figureSpacing) {data[&KoSvgText::FontFeatureNumeric::spacing].zoom(kislager::lenses::do_static_cast<KoSvgText::NumericFigureSpacing, int>)}
    , LAGER_QT(fractions) {data[&KoSvgText::FontFeatureNumeric::fractions].zoom(kislager::lenses::do_static_cast<KoSvgText::NumericFractions, int>)}
    , LAGER_QT(ordinals) {data[&KoSvgText::FontFeatureNumeric::ordinals]}
    , LAGER_QT(slashedZero) {data[&KoSvgText::FontFeatureNumeric::slashedZero]}
{

}
