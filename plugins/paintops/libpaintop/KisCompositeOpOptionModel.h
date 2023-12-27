/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCOMPOSITEOPOPTIONMODEL_H
#define KISCOMPOSITEOPOPTIONMODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisCompositeOpOptionData.h"


class KisCompositeOpOptionModel : public QObject
{
    Q_OBJECT
public:
    KisCompositeOpOptionModel(lager::cursor<KisCompositeOpOptionData> optionData);
    lager::cursor<KisCompositeOpOptionData> optionData;

    LAGER_QT_CURSOR(QString, compositeOpId);
    LAGER_QT_CURSOR(bool, eraserMode);
};

#endif // KISCOMPOSITEOPOPTIONMODEL_H
