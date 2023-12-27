/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_CURVE_OP_OPTION_MODEL_H
#define KIS_CURVE_OP_OPTION_MODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisCurveOpOptionData.h"
#include "KisWidgetConnectionUtils.h"

class KisCurveOpOptionModel : public QObject
{
    Q_OBJECT
public:
    KisCurveOpOptionModel(lager::cursor<KisCurveOpOptionData> optionData);

    lager::cursor<KisCurveOpOptionData> optionData;

    LAGER_QT_CURSOR(bool, curvePaintConnectionLine);
    LAGER_QT_CURSOR(bool, curveSmoothing);
    LAGER_QT_CURSOR(qreal, curveStrokeHistorySize);
    LAGER_QT_CURSOR(qreal, curveLineWidth);
    LAGER_QT_CURSOR(qreal, curveCurvesOpacity);
    
};

#endif // KIS_CURVE_OP_OPTION_MODEL_H
