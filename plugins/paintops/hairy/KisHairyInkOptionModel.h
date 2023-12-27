/*
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_HAIRYINK_OPTION_MODEL_H
#define KIS_HAIRYINK_OPTION_MODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisHairyInkOptionData.h"
#include "KisWidgetConnectionUtils.h"

class KisHairyInkOptionModel : public QObject
{
    Q_OBJECT
public:
    KisHairyInkOptionModel(lager::cursor<KisHairyInkOptionData> optionData);

    lager::cursor<KisHairyInkOptionData> optionData;

    LAGER_QT_CURSOR(bool, inkDepletionEnabled);
    LAGER_QT_CURSOR(int, inkAmount);
    LAGER_QT_CURSOR(QString, inkDepletionCurve);
    LAGER_QT_CURSOR(bool, useSaturation);
    LAGER_QT_CURSOR(bool, useOpacity);
    LAGER_QT_CURSOR(bool, useWeights);
    LAGER_QT_CURSOR(int, pressureWeight);
    LAGER_QT_CURSOR(int, bristleLengthWeight);
    LAGER_QT_CURSOR(int, bristleInkAmountWeight);
    LAGER_QT_CURSOR(int, inkDepletionWeight);
    LAGER_QT_CURSOR(bool, useSoakInk);
};

#endif // KIS_HAIRYINK_OPTION_MODEL_H
