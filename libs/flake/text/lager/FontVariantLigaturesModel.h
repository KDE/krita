/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef FONTVARIANTLIGATURESMODEL_H
#define FONTVARIANTLIGATURESMODEL_H

#include <QObject>

#include <KoSvgText.h>
#include <lager/state.hpp>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include <kritaflake_export.h>

class KRITAFLAKE_EXPORT FontVariantLigaturesModel : public QObject
{
    Q_OBJECT
public:
    FontVariantLigaturesModel(lager::cursor<KoSvgText::FontFeatureLigatures> _data = lager::make_state(KoSvgText::FontFeatureLigatures(), lager::automatic_tag{}));

    lager::cursor<KoSvgText::FontFeatureLigatures> data;

    LAGER_QT_CURSOR(bool, commonLigatures);
    LAGER_QT_CURSOR(bool, discretionaryLigatures);
    LAGER_QT_CURSOR(bool, historicalLigatures);
    LAGER_QT_CURSOR(bool, contextualAlternates);

};

#endif // FONTVARIANTLIGATURESMODEL_H
