/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgTextToolOptionsModel.h"

const QString SVG_TEXT_TOOL_ID = "SvgTextTool";

SvgTextToolOptionsModel::SvgTextToolOptionsModel(lager::cursor<SvgTextToolOptionsData> _data, QObject *parent)
    : QObject{parent}
    , data(_data)
    , LAGER_QT(useCurrentTextProperties) {data[&SvgTextToolOptionsData::useCurrentTextProperties]}
    , LAGER_QT(cssStylePresetName) {data[&SvgTextToolOptionsData::cssStylePresetName]}
    , LAGER_QT(useVisualBidiCursor) {data[&SvgTextToolOptionsData::useVisualBidiCursor]}
    , LAGER_QT(pasteRichtTextByDefault) {data[&SvgTextToolOptionsData::pasteRichtTextByDefault]}
{
    lager::watch(data, std::bind(&SvgTextToolOptionsModel::optionsChanged, this));
    connect(this, SIGNAL(optionsChanged()), this, SLOT(saveOptions()));
    loadOptions();
}

void SvgTextToolOptionsModel::saveOptions()
{
    SvgTextToolOptionsData _d = data.get();
    _d.writeConfig(SVG_TEXT_TOOL_ID);
}

void SvgTextToolOptionsModel::loadOptions()
{
    SvgTextToolOptionsData _d = data.get();
    _d.loadConfig(SVG_TEXT_TOOL_ID);
    data.set(_d);
}

void SvgTextToolOptionsModel::resetOptions()
{
    SvgTextToolOptionsData _d = data.get();
    _d.resetConfig();
    data.set(_d);
}
