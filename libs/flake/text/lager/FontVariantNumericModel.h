/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef FONTVARIANTNUMERICMODEL_H
#define FONTVARIANTNUMERICMODEL_H

#include <QObject>

#include <KoSvgText.h>
#include <lager/state.hpp>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include <kritaflake_export.h>

class KRITAFLAKE_EXPORT FontVariantNumericModel : public QObject
{
    Q_OBJECT
public:
    FontVariantNumericModel(lager::cursor<KoSvgText::FontFeatureNumeric> _data = lager::make_state(KoSvgText::FontFeatureNumeric(), lager::automatic_tag{}));

    lager::cursor<KoSvgText::FontFeatureNumeric> data;

    LAGER_QT_CURSOR(int, figureStyle);
    LAGER_QT_CURSOR(int, figureSpacing);
    LAGER_QT_CURSOR(int, fractions);
    LAGER_QT_CURSOR(bool, ordinals);
    LAGER_QT_CURSOR(bool, slashedZero);

};

#endif // FONTVARIANTNUMERICMODEL_H
