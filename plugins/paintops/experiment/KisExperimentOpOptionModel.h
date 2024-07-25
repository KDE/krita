/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_EXPERIMENT_OP_OPTION_MODEL_H
#define KIS_EXPERIMENT_OP_OPTION_MODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisExperimentOpOptionData.h"
#include "KisWidgetConnectionUtils.h"

class KisExperimentOpOptionModel : public QObject
{
    Q_OBJECT
public:
    KisExperimentOpOptionModel(lager::cursor<KisExperimentOpOptionData> optionData);

    lager::cursor<KisExperimentOpOptionData> optionData;

    LAGER_QT_CURSOR(bool, isDisplacementEnabled);
    LAGER_QT_CURSOR(qreal, displacement);
    
    LAGER_QT_CURSOR(bool, isSpeedEnabled);
    LAGER_QT_CURSOR(qreal, speed);
    
    LAGER_QT_CURSOR(bool, isSmoothingEnabled);
    LAGER_QT_CURSOR(qreal, smoothing);
    
    LAGER_QT_CURSOR(bool, windingFill);
    LAGER_QT_CURSOR(bool, hardEdge);
    
    LAGER_QT_CURSOR(int, fillType);
    
};

#endif // KIS_EXPERIMENT_OP_OPTION_MODEL_H
