/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_ROUNDMARKER_OP_OPTION_MODEL_H
#define KIS_ROUNDMARKER_OP_OPTION_MODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisRoundMarkerOpOptionData.h"
#include "KisWidgetConnectionUtils.h"

class KisRoundMarkerOpOptionModel : public QObject
{
    Q_OBJECT
public:
    KisRoundMarkerOpOptionModel(lager::cursor<KisRoundMarkerOpOptionData> optionData);

    lager::cursor<KisRoundMarkerOpOptionData> optionData;

    LAGER_QT_CURSOR(qreal, diameter);
    LAGER_QT_CURSOR(qreal, spacing);
    LAGER_QT_CURSOR(bool, useAutoSpacing);
    LAGER_QT_CURSOR(qreal, autoSpacingCoeff);
    LAGER_QT_CURSOR(SpacingState, aggregatedSpacing);
};

#endif // KIS_ROUNDMARKER_OP_OPTION_MODEL_H
