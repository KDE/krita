/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgTextToolOptionsModel.h"

SvgTextToolOptionsModel::SvgTextToolOptionsModel(const QString &configName, lager::cursor<SvgTextToolOptionsData> _data, QObject *parent)
    : QObject{parent}
    , data(_data)
    , LAGER_QT(useCurrentTextProperties) {data[&SvgTextToolOptionsData::useCurrentTextProperties]}
    , LAGER_QT(cssStylePresetName) {data[&SvgTextToolOptionsData::cssStylePresetName]}
    , LAGER_QT(useVisualBidiCursor) {data[&SvgTextToolOptionsData::useVisualBidiCursor]}
    , LAGER_QT(pasteRichtTextByDefault) {data[&SvgTextToolOptionsData::pasteRichtTextByDefault]}
    , m_configName(configName)
{
    lager::watch(data, std::bind(&SvgTextToolOptionsModel::optionsChanged, this));
    connect(this, SIGNAL(optionsChanged()), this, SLOT(saveOptions()));
    loadOptions();
}

void SvgTextToolOptionsModel::setConfigName(const QString &configName)
{
    if (m_configName != configName) {
        m_configName = configName;
        loadOptions();
    }
}

void SvgTextToolOptionsModel::saveOptions()
{
    SvgTextToolOptionsData _d = data.get();
    _d.writeConfig(m_configName);
}

void SvgTextToolOptionsModel::loadOptions()
{
    if (m_configName.isEmpty()) return;
    SvgTextToolOptionsData _d = data.get();
    _d.loadConfig(m_configName);
    data.set(_d);
}

void SvgTextToolOptionsModel::resetOptions()
{
    SvgTextToolOptionsData _d = data.get();
    _d.resetConfig();
    data.set(_d);
}
