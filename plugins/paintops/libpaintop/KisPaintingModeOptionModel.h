/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISPAINTINGMODEOPTIONMODEL_H
#define KISPAINTINGMODEOPTIONMODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisPaintingModeOptionData.h"
#include "KisWidgetConnectionUtils.h"

using ButtonGroupState = KisWidgetConnectionUtils::ButtonGroupState;

class KisPaintingModeOptionModel : public QObject
{
    Q_OBJECT
public:
    KisPaintingModeOptionModel(lager::cursor<KisPaintingModeOptionData> optionData, lager::reader<bool> maskingBrushEnabled);

    lager::cursor<KisPaintingModeOptionData> optionData;
    lager::reader<bool> maskingBrushEnabled;

    KisPaintingModeOptionData bakedOptionData() const;

    LAGER_QT_CURSOR(int, paintingMode);
    LAGER_QT_READER(int, effectivePaintingMode);
    LAGER_QT_READER(ButtonGroupState, paintingModeState);
};

#endif // KISPAINTINGMODEOPTIONMODEL_H
