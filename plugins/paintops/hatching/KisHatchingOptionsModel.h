/*
 *  SPDX-FileCopyrightText: 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_HATCHING_OPTIONS_MODEL_H
#define KIS_HATCHING_OPTIONS_MODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisHatchingOptionsData.h"
#include "KisWidgetConnectionUtils.h"

class KisHatchingOptionsModel : public QObject
{
    Q_OBJECT
public:
    KisHatchingOptionsModel(lager::cursor<KisHatchingOptionsData> optionData);

    lager::cursor<KisHatchingOptionsData> optionData;

    LAGER_QT_CURSOR(qreal, angle);
    LAGER_QT_CURSOR(qreal, separation);
    LAGER_QT_CURSOR(qreal, thickness);
    LAGER_QT_CURSOR(qreal, originX);
    LAGER_QT_CURSOR(qreal, originY);
    LAGER_QT_CURSOR(int, crosshatchingStyle);
    LAGER_QT_CURSOR(int, separationIntervals);
};

#endif // KIS_HATCHING_OPTIONS_MODEL_H
