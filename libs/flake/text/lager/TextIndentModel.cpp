/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TextIndentModel.h"

TextIndentModel::TextIndentModel(lager::cursor<KoSvgText::TextIndentInfo> _textData)
    : data(_textData)
    , lengthData(data[&KoSvgText::TextIndentInfo::length])
    , lengthModel(lengthData)
    , LAGER_QT(hanging){data[&KoSvgText::TextIndentInfo::hanging]}
    , LAGER_QT(eachLine){data[&KoSvgText::TextIndentInfo::eachLine]}
{
    lager::watch(lengthData, std::bind(&TextIndentModel::lengthChanged, this));
}

CssLengthPercentageModel *TextIndentModel::length()
{
    return &this->lengthModel;
}
