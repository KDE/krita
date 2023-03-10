/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_COLOR_OPTION_MODEL_H
#define KIS_COLOR_OPTION_MODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisColorOptionData.h"
#include "KisWidgetConnectionUtils.h"

class PAINTOP_EXPORT KisColorOptionModel : public QObject
{
    Q_OBJECT
public:
    KisColorOptionModel(lager::cursor<KisColorOptionData> optionData);

    lager::cursor<KisColorOptionData> optionData;
    
    LAGER_QT_CURSOR(bool, useRandomHSV);
    LAGER_QT_CURSOR(bool, useRandomOpacity);
    LAGER_QT_CURSOR(bool, sampleInputColor);
    
    LAGER_QT_CURSOR(bool, fillBackground);
    LAGER_QT_CURSOR(bool, colorPerParticle);
    LAGER_QT_CURSOR(bool, mixBgColor);
    
    LAGER_QT_CURSOR(int, hue);
    LAGER_QT_CURSOR(int, saturation);
    LAGER_QT_CURSOR(int, value);
    
};

#endif // KIS_COLOR_OPTION_MODEL_H
