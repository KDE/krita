/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISAIRBRUSHOPTIONMODEL_H
#define KISAIRBRUSHOPTIONMODEL_H

#include <QObject>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisAirbrushOptionData.h"


class KisAirbrushOptionModel : public QObject
{
    Q_OBJECT
public:
    KisAirbrushOptionModel(lager::cursor<KisAirbrushOptionData> optionData);
    lager::cursor<KisAirbrushOptionData> airbrushOptionData;

    LAGER_QT_CURSOR(bool, isChecked);
    LAGER_QT_CURSOR(qreal, airbrushRate);
    LAGER_QT_CURSOR(bool, ignoreSpacing);
};

#endif // KISAIRBRUSHOPTIONMODEL_H
