/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_GRID_OP_OPTION_MODEL_H
#define KIS_GRID_OP_OPTION_MODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisGridOpOptionData.h"
#include "KisWidgetConnectionUtils.h"

class KisGridOpOptionModel : public QObject
{
    Q_OBJECT
public:
    KisGridOpOptionModel(lager::cursor<KisGridOpOptionData> optionData);

    lager::cursor<KisGridOpOptionData> optionData;
    
    LAGER_QT_CURSOR(int, diameter);
    LAGER_QT_CURSOR(int, grid_width);
    LAGER_QT_CURSOR(int, grid_height);
    
    LAGER_QT_CURSOR(qreal, horizontal_offset);
    LAGER_QT_CURSOR(qreal, vertical_offset);
    LAGER_QT_CURSOR(int, grid_division_level);
    
    LAGER_QT_CURSOR(bool, grid_pressure_division);
    LAGER_QT_CURSOR(qreal, grid_scale);
    LAGER_QT_CURSOR(qreal, grid_vertical_border);
    
    LAGER_QT_CURSOR(qreal, grid_horizontal_border);
    LAGER_QT_CURSOR(bool, grid_random_border);
    
    
};

#endif // KIS_GRID_OP_OPTION_MODEL_H
