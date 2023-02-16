/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisLodAvailabilityModel.h"

#include <lager/lenses/tuple.hpp>
#include <KisZug.h>

namespace {
KisLodAvailabilityModel::AvailabilityStatePack
calcLodAvailabilityState(const KisLodAvailabilityData &data, qreal effectiveBrushSize, const KisPaintopLodLimitations &l) {
    KisLodAvailabilityModel::AvailabilityState state = KisLodAvailabilityModel::Available;

    if (!l.blockers.isEmpty()) {
        state = KisLodAvailabilityModel::BlockedFully;
    } else if (data.isLodSizeThresholdSupported &&
               effectiveBrushSize < data.lodSizeThreshold) {

        state = KisLodAvailabilityModel::BlockedByThreshold;
    } else if (!l.limitations.isEmpty()) {
        state = KisLodAvailabilityModel::Limited;
    }

    return std::make_tuple(state, l, data.isLodUserAllowed);
}
}

KisLodAvailabilityModel::KisLodAvailabilityModel(lager::cursor<KisLodAvailabilityData> _data, lager::reader<qreal> _effectiveBrushSize, lager::reader<KisPaintopLodLimitations> _lodLimitations)
    : data(_data)
    , effectiveBrushSize(_effectiveBrushSize)
    , lodLimitations(_lodLimitations)
    , LAGER_QT(isLodUserAllowed) {data[&KisLodAvailabilityData::isLodUserAllowed]}
    , LAGER_QT(isLodSizeThresholdSupported) {data[&KisLodAvailabilityData::isLodSizeThresholdSupported]}
    , LAGER_QT(lodSizeThreshold) {data[&KisLodAvailabilityData::lodSizeThreshold]}
    , LAGER_QT(availabilityState) {lager::with(data, effectiveBrushSize, lodLimitations).map(&calcLodAvailabilityState)}
    , LAGER_QT(effectiveLodAvailable) {LAGER_QT(availabilityState)
            .zoom(lager::lenses::first)
            .xform(kiszug::map_less_equal<int>(static_cast<int>(Limited)))}
{
    data.watch(std::bind(&KisLodAvailabilityModel::sigConfigurationItemChanged, this));
}
