/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TEXTTRANSFORMMODEL_H
#define TEXTTRANSFORMMODEL_H

#include <QObject>
#include <KoSvgText.h>
#include <lager/state.hpp>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include <kritaflake_export.h>

class KRITAFLAKE_EXPORT TextTransformModel : public QObject
{
    Q_OBJECT
public:
    explicit TextTransformModel(lager::cursor<KoSvgText::TextTransformInfo> _data = lager::make_state(KoSvgText::TextTransformInfo(), lager::automatic_tag{}));

    lager::cursor<KoSvgText::TextTransformInfo> data;

    LAGER_QT_CURSOR(int, capitals);
    LAGER_QT_CURSOR(bool, fullWidth);
    LAGER_QT_CURSOR(bool, fullSizeKana);

};

#endif // TEXTTRANSFORMMODEL_H
