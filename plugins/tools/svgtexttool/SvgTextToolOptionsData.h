/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef SVGTEXTTOOLOPTIONSDATA_H
#define SVGTEXTTOOLOPTIONSDATA_H

#include <QString>

struct SvgTextToolOptionsData
{
    bool useCurrentTextProperties = true;
    QString cssStylePresetName = QString();

    bool useVisualBidiCursor = false;

    bool pasteRichtTextByDefault = false;

    void writeConfig(const QString &toolId);
    void loadConfig(const QString &toolId);
    void resetConfig();
};

#endif // SVGTEXTTOOLOPTIONSDATA_H
