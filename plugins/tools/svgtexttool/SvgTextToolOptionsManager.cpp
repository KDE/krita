/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgTextToolOptionsManager.h"

struct SvgTextToolOptionsManager::Private
{
    SvgTextToolOptionsModel *model{new SvgTextToolOptionsModel()};

    bool showDebug = false;
    bool showCharacterDebug = true;
    bool showLineDebug = false;

    int textType = 0;

    bool textPropertiesDockerOpen = false;
    bool showTextPropertyButton = false;

    bool typeSettingMode = false;
};

SvgTextToolOptionsManager::SvgTextToolOptionsManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{

}

SvgTextToolOptionsManager::~SvgTextToolOptionsManager()
{

}

SvgTextToolOptionsModel *SvgTextToolOptionsManager::optionsModel() const
{
    return d->model;
}

void SvgTextToolOptionsManager::setOptionsModel(SvgTextToolOptionsModel *model)
{
    if (d->model == model) return;
    d->model = model;
    Q_EMIT optionsModelChanged();
}

bool SvgTextToolOptionsManager::showDebug() const
{
    return d->showDebug;
}

void SvgTextToolOptionsManager::setShowDebug(const bool show)
{
    if (d->showDebug == show) return;
    d->showDebug = show;
    Q_EMIT showDebugChanged();
}

bool SvgTextToolOptionsManager::showCharacterDebug() const
{
    return d->showCharacterDebug;
}

void SvgTextToolOptionsManager::setShowCharacterDebug(const bool show)
{
    if (d->showCharacterDebug == show) return;
    d->showCharacterDebug = show;
    Q_EMIT showDebugCharacterChanged();
}

bool SvgTextToolOptionsManager::showLineDebug() const
{
    return d->showLineDebug;
}

void SvgTextToolOptionsManager::setShowLineDebug(const bool show)
{
    if (d->showLineDebug == show) return;
    d->showLineDebug = show;
    Q_EMIT showLineDebugChanged();
}

int SvgTextToolOptionsManager::textType() const
{
    return d->textType;
}
void SvgTextToolOptionsManager::convertToTextType(const int type)
{
    if (d->textType == type) return;
    d->textType = type;
    Q_EMIT convertTextType(type);
}

bool SvgTextToolOptionsManager::textPropertiesOpen() const
{
    return d->textPropertiesDockerOpen;
}

void SvgTextToolOptionsManager::setTextPropertiesOpen(const bool open)
{
    if (d->textPropertiesDockerOpen == open) return;
    d->textPropertiesDockerOpen = open;
    Q_EMIT openTextPropertiesDocker(open);
}

bool SvgTextToolOptionsManager::showTextPropertyButton() const
{
    return d->showTextPropertyButton;
}

void SvgTextToolOptionsManager::setShowTextPropertyButton(const bool show)
{
    if (d->showTextPropertyButton == show) return;
    d->showTextPropertyButton = show;
    Q_EMIT showTextPropertyButtonChanged();
}

bool SvgTextToolOptionsManager::typeSettingMode() const
{
    return d->typeSettingMode;
}

void SvgTextToolOptionsManager::setTypeSettingMode(const bool activate)
{
    if (d->typeSettingMode == activate) return;
    d->typeSettingMode = activate;
    Q_EMIT typeSettingModeChanged();
}

void SvgTextToolOptionsManager::emitOpenTextEditor()
{
    Q_EMIT openTextEditor();
}

void SvgTextToolOptionsManager::emitGlyphPalette()
{
    Q_EMIT openGlyphPalette();
}
