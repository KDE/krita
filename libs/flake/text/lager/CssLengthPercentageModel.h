/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef CSSLENGTHPERCENTAGEMODEL_H
#define CSSLENGTHPERCENTAGEMODEL_H

#include <QObject>
#include <KoSvgText.h>
#include <lager/state.hpp>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include <kritaflake_export.h>

class KRITAFLAKE_EXPORT CssLengthPercentageModel : public QObject
{
    Q_OBJECT
public:
    CssLengthPercentageModel(lager::cursor<KoSvgText::CssLengthPercentage> _data = lager::make_state(KoSvgText::CssLengthPercentage(), lager::automatic_tag{}));

    lager::cursor<KoSvgText::CssLengthPercentage> length;

    LAGER_QT_CURSOR(qreal, value);
    LAGER_QT_CURSOR(int, unitType);
};

#endif // CSSLENGTHPERCENTAGEMODEL_H
