/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SPRAY_OP_OPTION_MODEL_H
#define KIS_SPRAY_OP_OPTION_MODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisSprayOpOptionData.h"
#include "KisWidgetConnectionUtils.h"

class KisSprayOpOptionModel : public QObject
{
    Q_OBJECT
public:
    KisSprayOpOptionModel(lager::cursor<KisSprayOpOptionData> optionData);

    lager::cursor<KisSprayOpOptionData> optionData;
    
    LAGER_QT_CURSOR(int, diameter);
    LAGER_QT_CURSOR(qreal, aspect);
    LAGER_QT_CURSOR(qreal, brushRotation);
    LAGER_QT_CURSOR(qreal, scale);
    LAGER_QT_CURSOR(qreal, spacing);
    LAGER_QT_CURSOR(bool, jitterMovement);
    LAGER_QT_CURSOR(qreal, jitterAmount);
    LAGER_QT_CURSOR(bool, useDensity);
    LAGER_QT_READER(bool, isNumParticlesVisible);
    LAGER_QT_CURSOR(int, particleCount);
    LAGER_QT_CURSOR(qreal, coverage);
    
    LAGER_QT_CURSOR(int, angularDistributionType);
    LAGER_QT_CURSOR(QString, angularDistributionCurve);
    LAGER_QT_CURSOR(int, angularDistributionCurveRepeat);
    LAGER_QT_CURSOR(int, radialDistributionType);
    LAGER_QT_CURSOR(qreal, radialDistributionStdDeviation);
    LAGER_QT_CURSOR(qreal, radialDistributionClusteringAmount);
    LAGER_QT_CURSOR(QString, radialDistributionCurve);
    LAGER_QT_CURSOR(int, radialDistributionCurveRepeat);
    LAGER_QT_CURSOR(bool, radialDistributionCenterBiased);
};

#endif // KIS_SPRAY_OP_OPTION_MODEL_H
