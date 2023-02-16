/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_HAIRYBRISTLE_OPTION_MODEL_H
#define KIS_HAIRYBRISTLE_OPTION_MODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisHairyBristleOptionData.h"
#include "KisWidgetConnectionUtils.h"

class KisHairyBristleOptionModel : public QObject
{
    Q_OBJECT
public:
    KisHairyBristleOptionModel(lager::cursor<KisHairyBristleOptionData> optionData);

    lager::cursor<KisHairyBristleOptionData> optionData;

    LAGER_QT_CURSOR(bool, useMousePressure);
    LAGER_QT_CURSOR(double, scaleFactor);
    LAGER_QT_CURSOR(double, randomFactor);
    LAGER_QT_CURSOR(double, shearFactor);
    LAGER_QT_CURSOR(double, densityFactor);
    LAGER_QT_CURSOR(bool, threshold);
    LAGER_QT_CURSOR(bool, antialias);
    LAGER_QT_CURSOR(bool, useCompositing);
    LAGER_QT_CURSOR(bool, connectedPath);
};

#endif // KIS_HAIRYBRISTLE_OPTION_MODEL_H
