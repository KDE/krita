/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_PARTICLEOP_OPTION_MODEL_H
#define KIS_PARTICLEOP_OPTION_MODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisParticleOpOptionData.h"
#include "KisWidgetConnectionUtils.h"

class KisParticleOpOptionModel : public QObject
{
    Q_OBJECT
public:
    KisParticleOpOptionModel(lager::cursor<KisParticleOpOptionData> optionData);

    lager::cursor<KisParticleOpOptionData> optionData;

    LAGER_QT_CURSOR(int, particleCount);
    LAGER_QT_CURSOR(int, particleIterations);
    LAGER_QT_CURSOR(qreal, particleGravity);
    LAGER_QT_CURSOR(qreal, particleWeight);
    LAGER_QT_CURSOR(qreal, particleScaleX);
    LAGER_QT_CURSOR(qreal, particleScaleY);
};

#endif // KIS_PARTICLEOP_OPTION_MODEL_H
