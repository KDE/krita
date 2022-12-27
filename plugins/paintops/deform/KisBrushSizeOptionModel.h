/*
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_BRUSHSIZE_OPTION_MODEL_H
#define KIS_BRUSHSIZE_OPTION_MODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisBrushSizeOptionData.h"
#include "KisWidgetConnectionUtils.h"

class KisBrushSizeOptionModel : public QObject
{
    Q_OBJECT
public:
    KisBrushSizeOptionModel(lager::cursor<KisBrushSizeOptionData> optionData);

    lager::cursor<KisBrushSizeOptionData> optionData;

    LAGER_QT_CURSOR(qreal, brushDiameter);
    LAGER_QT_CURSOR(qreal, brushAspect);
    LAGER_QT_CURSOR(qreal, brushRotation);
    LAGER_QT_CURSOR(qreal, brushScale);
    LAGER_QT_CURSOR(qreal, brushSpacing);
    LAGER_QT_CURSOR(qreal, brushDensity);
    LAGER_QT_CURSOR(qreal, brushJitterMovement);
    LAGER_QT_CURSOR(bool, brushJitterMovementEnabled);
};

#endif // KIS_BRUSHSIZE_OPTION_MODEL_H
