/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TextTransformModel.h"
#include <KisLager.h>

TextTransformModel::TextTransformModel(lager::cursor<KoSvgText::TextTransformInfo> _data)
    : data(_data)
    , LAGER_QT(capitals){data[&KoSvgText::TextTransformInfo::capitals].zoom(kislager::lenses::do_static_cast<KoSvgText::TextTransform, int>)}
    , LAGER_QT(fullWidth){data[&KoSvgText::TextTransformInfo::fullWidth]}
    , LAGER_QT(fullSizeKana){data[&KoSvgText::TextTransformInfo::fullSizeKana]}
{

}
