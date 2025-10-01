/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef SVGTEXTTOOLOPTIONSMODEL_H
#define SVGTEXTTOOLOPTIONSMODEL_H

#include <QObject>
#include <lager/cursor.hpp>
#include <lager/state.hpp>
#include <lager/extra/qt.hpp>
#include <SvgTextToolOptionsData.h>

class SvgTextToolOptionsModel : public QObject
{
    Q_OBJECT
public:
    SvgTextToolOptionsModel(lager::cursor<SvgTextToolOptionsData> _data = lager::make_state(SvgTextToolOptionsData(), lager::automatic_tag{}), QObject *parent = nullptr);

    lager::cursor<SvgTextToolOptionsData> data;
    LAGER_QT_CURSOR(bool, useCurrentTextProperties);
    LAGER_QT_CURSOR(QString, cssStylePresetName);
    LAGER_QT_CURSOR(bool, useVisualBidiCursor);
    LAGER_QT_CURSOR(bool, pasteRichtTextByDefault);

public Q_SLOTS:
    void saveOptions();
    void loadOptions();
    void resetOptions();

Q_SIGNALS:
    void optionsChanged();
};

#endif // SVGTEXTTOOLOPTIONSMODEL_H
