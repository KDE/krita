/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgTextToolOptionsData.h"

#include <KSharedConfig>
#include <KConfigGroup>

const QString USE_CURRENT_TEXT_PROPERTIES = "useCurrentTextProperties";
const QString CSS_STYLE_PRESET_NAME = "cssStylePresetName";
const QString USE_VISUAL_BIDI_CURSOR = "useVisualBidiCursor";
const QString PASTE_RICH_TEXT_BY_DEFAULT = "pasteRichtTextByDefault";

void SvgTextToolOptionsData::writeConfig(const QString &toolId)
{
    KConfigGroup configGroup = KSharedConfig::openConfig()->group(toolId);
    configGroup.writeEntry(USE_CURRENT_TEXT_PROPERTIES, useCurrentTextProperties);
    configGroup.writeEntry(CSS_STYLE_PRESET_NAME, cssStylePresetName);
    configGroup.writeEntry(USE_VISUAL_BIDI_CURSOR, useVisualBidiCursor);
    configGroup.writeEntry(PASTE_RICH_TEXT_BY_DEFAULT, pasteRichtTextByDefault);
}

void SvgTextToolOptionsData::loadConfig(const QString &toolId)
{
    KConfigGroup configGroup = KSharedConfig::openConfig()->group(toolId);
    useCurrentTextProperties = configGroup.readEntry<bool>(USE_CURRENT_TEXT_PROPERTIES, true);
    cssStylePresetName = configGroup.readEntry<QString>(CSS_STYLE_PRESET_NAME, QString());
    useVisualBidiCursor = configGroup.readEntry<bool>(USE_VISUAL_BIDI_CURSOR, false);
    pasteRichtTextByDefault = configGroup.readEntry<bool>(PASTE_RICH_TEXT_BY_DEFAULT, false);
}

void SvgTextToolOptionsData::resetConfig()
{
    useCurrentTextProperties = true;
    cssStylePresetName = QString();
    useVisualBidiCursor = false;
    pasteRichtTextByDefault = false;
}
