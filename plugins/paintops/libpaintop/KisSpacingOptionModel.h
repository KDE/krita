/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSPACINGOPTIONMODEL_H
#define KISSPACINGOPTIONMODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisSpacingOptionData.h"

class KisSpacingOptionModel : public QObject
{
    Q_OBJECT
public:
    KisSpacingOptionModel(lager::cursor<KisSpacingOptionMixIn> optionData);
    lager::cursor<KisSpacingOptionMixIn> spacingOptionData;
    LAGER_QT_CURSOR(bool, useSpacingUpdates);
    LAGER_QT_CURSOR(bool, isotropicSpacing);
};

#endif // KISSPACINGOPTIONMODEL_H
