/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SKETCHOP_OPTION_MODEL_H
#define KIS_SKETCHOP_OPTION_MODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisSketchOpOptionData.h"
#include "KisWidgetConnectionUtils.h"

class KisSketchOpOptionModel : public QObject
{
    Q_OBJECT
public:
    KisSketchOpOptionModel(lager::cursor<KisSketchOpOptionData> optionData);

    lager::cursor<KisSketchOpOptionData> optionData;

    LAGER_QT_CURSOR(qreal, offset);
    LAGER_QT_CURSOR(qreal, probability);
    LAGER_QT_CURSOR(bool, simpleMode);
    LAGER_QT_CURSOR(bool, makeConnection);
    LAGER_QT_CURSOR(bool, magnetify);
    LAGER_QT_CURSOR(bool, randomRGB);
    LAGER_QT_CURSOR(bool, randomOpacity);
    LAGER_QT_CURSOR(bool, distanceOpacity);
    LAGER_QT_CURSOR(bool, distanceDensity);
    LAGER_QT_CURSOR(bool, antiAliasing);
    LAGER_QT_CURSOR(int, lineWidth);
};

#endif // KIS_SKETCHOP_OPTION_MODEL_H
