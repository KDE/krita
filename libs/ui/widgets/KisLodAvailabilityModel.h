/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISLODAVAILABILITYMODEL_H
#define KISLODAVAILABILITYMODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisLodAvailabilityData.h"
#include <kis_paintop_lod_limitations.h>

class KisLodAvailabilityModel : public QObject
{
    Q_OBJECT
public:
    enum AvailabilityState {
        Available = 0,
        Limited,
        BlockedByThreshold,
        BlockedFully
    };

    using AvailabilityStatePack =
        std::tuple<AvailabilityState, KisPaintopLodLimitations, bool>;
public:
    KisLodAvailabilityModel(lager::cursor<KisLodAvailabilityData> data,
                            lager::reader<qreal> effectiveBrushSize,
                            lager::reader<KisPaintopLodLimitations> lodLimitations);

    lager::cursor<KisLodAvailabilityData> data;
    lager::reader<qreal> effectiveBrushSize;
    lager::reader<KisPaintopLodLimitations> lodLimitations;

    LAGER_QT_CURSOR(bool, isLodUserAllowed);
    LAGER_QT_READER(bool, isLodSizeThresholdSupported);
    LAGER_QT_CURSOR(qreal, lodSizeThreshold);

    LAGER_QT_READER(AvailabilityStatePack, availabilityState);
    LAGER_QT_READER(bool, effectiveLodAvailable);

Q_SIGNALS:
    void sigConfigurationItemChanged();
};

#endif // KISLODAVAILABILITYMODEL_H
