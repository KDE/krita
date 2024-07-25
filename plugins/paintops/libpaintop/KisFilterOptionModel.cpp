/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisFilterOptionModel.h"

#include <kis_filter.h>
#include <kis_filter_configuration.h>
#include <kis_filter_registry.h>
#include <KisGlobalResourcesInterface.h>

namespace {

auto effectiveFilterStateLens = lager::lenses::getset(
    [](const FilterState &x) {
        if (!std::get<0>(x).isEmpty()) {
           return x;
        }

        KisFilterSP fallback = KisFilterRegistry::instance()->fallbackFilter();

        return std::make_tuple(
                    fallback->id(),
                    fallback->defaultConfiguration(KisGlobalResourcesInterface::instance())->toXML());
    },
    [](FilterState, const FilterState &y) {
        return y;
    });

} // namespace


KisFilterOptionModel::KisFilterOptionModel(lager::cursor<KisFilterOptionData> _optionData)
    : optionData(_optionData)
    , LAGER_QT(filterId) {optionData[&KisFilterOptionData::filterId]}
    , LAGER_QT(filterConfig) {optionData[&KisFilterOptionData::filterConfig]}
    , LAGER_QT(effectiveFilterState) {lager::with(LAGER_QT(filterId), LAGER_QT(filterConfig)).zoom(effectiveFilterStateLens)}
    , LAGER_QT(smudgeMode) {optionData[&KisFilterOptionData::smudgeMode]}
{
}

KisFilterOptionData KisFilterOptionModel::bakedOptionData() const
{
    KisFilterOptionData data = *optionData;

    std::tie(data.filterId, data.filterConfig) =
        effectiveFilterState();

    return data;
}



