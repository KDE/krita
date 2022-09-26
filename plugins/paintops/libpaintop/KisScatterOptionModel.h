/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSCATTEROPTIONMODEL_H
#define KISSCATTEROPTIONMODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisScatterOptionData.h"

class KisScatterOptionModel : public QObject
{
    Q_OBJECT
public:
    KisScatterOptionModel(lager::cursor<KisScatterOptionMixIn> optionData);
    lager::cursor<KisScatterOptionMixIn> scatterOptionData;
    LAGER_QT_CURSOR(bool, axisX);
    LAGER_QT_CURSOR(bool, axisY);
};

#endif // KISSCATTEROPTIONMODEL_H
