/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SPRAY_SHAPE_DYNAMICS_OPTION_MODEL_H
#define KIS_SPRAY_SHAPE_DYNAMICS_OPTION_MODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisSprayShapeDynamicsOptionData.h"
#include "KisWidgetConnectionUtils.h"

class KisSprayShapeDynamicsOptionModel : public QObject
{
    Q_OBJECT
public:
    KisSprayShapeDynamicsOptionModel(lager::cursor<KisSprayShapeDynamicsOptionData> optionData);

    lager::cursor<KisSprayShapeDynamicsOptionData> optionData;

    LAGER_QT_CURSOR(bool, enabled);
    LAGER_QT_CURSOR(bool, randomSize);
    
    LAGER_QT_CURSOR(bool, fixedRotation);
    LAGER_QT_CURSOR(bool, randomRotation);
    
    LAGER_QT_CURSOR(bool, followCursor);
    LAGER_QT_CURSOR(bool, followDrawingAngle);
    
    LAGER_QT_CURSOR(qreal, fixedAngle);
    LAGER_QT_CURSOR(qreal, randomRotationWeight);
    
    LAGER_QT_CURSOR(qreal, followCursorWeight);
    LAGER_QT_CURSOR(qreal, followDrawingAngleWeight);
};

#endif // KIS_SPRAY_SHAPE_DYNAMICS_OPTION_MODEL_H
