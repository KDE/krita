/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_DEFORM_OPTION_MODEL_H
#define KIS_DEFORM_OPTION_MODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisDeformOptionData.h"
#include "KisWidgetConnectionUtils.h"

class KisDeformOptionModel : public QObject
{
    Q_OBJECT
public:
    KisDeformOptionModel(lager::cursor<KisDeformOptionData> optionData);

    lager::cursor<KisDeformOptionData> optionData;

    LAGER_QT_CURSOR(qreal, deformAmount);
    LAGER_QT_CURSOR(bool, deformUseBilinear);
    LAGER_QT_CURSOR(bool, deformUseCounter);
    LAGER_QT_CURSOR(bool, deformUseOldData);
    LAGER_QT_CURSOR(int, deformAction);
};

#endif // KIS_DEFORM_OPTION_MODEL_H
