/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSMUDGELENGTHOPTIONMODEL_H
#define KISSMUDGELENGTHOPTIONMODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisSmudgeLengthOptionData.h"
#include "KisWidgetConnectionUtils.h"

class KisSmudgeLengthOptionModel : public QObject
{
    Q_OBJECT
public:
    KisSmudgeLengthOptionModel(lager::cursor<KisSmudgeLengthOptionMixIn> optionData, lager::reader<bool> forceUseNewEngine);
    lager::cursor<KisSmudgeLengthOptionMixIn> optionData;

    LAGER_QT_CURSOR(int, mode);
    LAGER_QT_CURSOR(bool, smearAlpha);
    LAGER_QT_CURSOR(bool, useNewEngine);
    LAGER_QT_READER(CheckBoxState, useNewEngineState);

    KisSmudgeLengthOptionMixIn backedOptionData() const;
};

#endif // KISSMUDGELENGTHOPTIONMODEL_H
