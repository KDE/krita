/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISFILTEROPTIONMODEL_H
#define KISFILTEROPTIONMODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisFilterOptionData.h"

using FilterState = std::tuple<QString, QString>;

class KisFilterOptionModel : public QObject
{
    Q_OBJECT
public:

    KisFilterOptionModel(lager::cursor<KisFilterOptionData> optionData);
    lager::cursor<KisFilterOptionData> optionData;
    LAGER_QT_CURSOR(QString, filterId);
    LAGER_QT_CURSOR(QString, filterConfig);
    LAGER_QT_CURSOR(FilterState, effectiveFilterState);
    LAGER_QT_CURSOR(bool, smudgeMode);

    KisFilterOptionData bakedOptionData() const;
};

#endif // KISFILTEROPTIONMODEL_H
