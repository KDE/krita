/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_GRID_SHAPE_OPTION_MODEL_H
#define KIS_GRID_SHAPE_OPTION_MODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisGridShapeOptionData.h"
#include "KisWidgetConnectionUtils.h"

class KisGridShapeOptionModel : public QObject
{
    Q_OBJECT
public:
    KisGridShapeOptionModel(lager::cursor<KisGridShapeOptionData> optionData);

    lager::cursor<KisGridShapeOptionData> optionData;
    
    LAGER_QT_CURSOR(int, shape);
    
};

#endif // KIS_GRID_SHAPE_OPTION_MODEL_H
