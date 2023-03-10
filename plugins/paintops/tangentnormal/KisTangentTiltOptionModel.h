/*
 * SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KIS_TANGENTTILT_OPTION_MODEL_H
#define KIS_TANGENTTILT_OPTION_MODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisTangentTiltOptionData.h"
#include "KisWidgetConnectionUtils.h"

class KisTangentTiltOptionModel : public QObject
{
    Q_OBJECT
public:
    KisTangentTiltOptionModel(lager::cursor<KisTangentTiltOptionData> optionData);

    lager::cursor<KisTangentTiltOptionData> optionData;

    LAGER_QT_CURSOR(int, redChannel);
    LAGER_QT_CURSOR(int, greenChannel);
    LAGER_QT_CURSOR(int, blueChannel);
    LAGER_QT_CURSOR(int, directionType);
    LAGER_QT_CURSOR(double, elevationSensitivity);
    LAGER_QT_CURSOR(double, mixValue);
};

#endif // KIS_TANGENTTILT_OPTION_MODEL_H
