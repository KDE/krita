/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_bookmarked_filter_configurations_model.h"

#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_paint_device.h>
#include <KisGlobalResourcesInterface.h>

struct KisBookmarkedFilterConfigurationsModel::Private {
    KisPaintDeviceSP thumb;
    KisFilterSP filter;
};

KisBookmarkedFilterConfigurationsModel::KisBookmarkedFilterConfigurationsModel(KisPaintDeviceSP thumb, KisFilterSP filter)
        : KisBookmarkedConfigurationsModel(filter->bookmarkManager()), d(new Private)
{
    d->thumb = thumb;
    d->filter = filter;
}

KisBookmarkedFilterConfigurationsModel::~KisBookmarkedFilterConfigurationsModel()
{
    delete d;
}


QVariant KisBookmarkedFilterConfigurationsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    return KisBookmarkedConfigurationsModel::data(index, role);

}

KisFilterConfigurationSP KisBookmarkedFilterConfigurationsModel::configuration(const QModelIndex &index) const
{
    KisFilterConfigurationSP config = dynamic_cast<KisFilterConfiguration*>(KisBookmarkedConfigurationsModel::configuration(index).data());
    if (config) return config;
    return d->filter->defaultConfiguration(KisGlobalResourcesInterface::instance());
}

