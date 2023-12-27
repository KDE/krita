/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCOLORSOURCEOPTIONMODEL_H
#define KISCOLORSOURCEOPTIONMODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisColorSourceOptionData.h"


class KisColorSourceOptionModel : public QObject
{
    Q_OBJECT
public:
    KisColorSourceOptionModel(lager::cursor<KisColorSourceOptionData> optionData);
    lager::cursor<KisColorSourceOptionData> optionData;

    LAGER_QT_CURSOR(int, type);
};

#endif // KISCOLORSOURCEOPTIONMODEL_H
