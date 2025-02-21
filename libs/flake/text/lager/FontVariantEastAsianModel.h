/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef FONTVARIANTEASTASIANMODEL_H
#define FONTVARIANTEASTASIANMODEL_H

#include <QObject>

#include <KoSvgText.h>
#include <lager/state.hpp>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include <kritaflake_export.h>

class KRITAFLAKE_EXPORT FontVariantEastAsianModel : public QObject
{
    Q_OBJECT
public:
    FontVariantEastAsianModel(lager::cursor<KoSvgText::FontFeatureEastAsian> _data = lager::make_state(KoSvgText::FontFeatureEastAsian(), lager::automatic_tag{}));

    lager::cursor<KoSvgText::FontFeatureEastAsian> data;

    LAGER_QT_CURSOR(int, variant);
    LAGER_QT_CURSOR(int, width);
    LAGER_QT_CURSOR(bool, ruby);

};

#endif // FONTVARIANTEASTASIANMODEL_H
