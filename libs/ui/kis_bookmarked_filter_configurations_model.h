/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_BOOKMARKED_FILTER_CONFIGURATIONS_MODEL_H_
#define _KIS_BOOKMARKED_FILTER_CONFIGURATIONS_MODEL_H_

#include "kis_bookmarked_configurations_model.h"

#include "kis_types.h"

#include <kis_serializable_configuration.h>

/**
 * Use this model to get the list of configuration for a Filter.
 */
class KisBookmarkedFilterConfigurationsModel : public KisBookmarkedConfigurationsModel
{
    Q_OBJECT
public:
    /**
     * @param thumb a 100x100 thumbnail used to preview the filters
     * @param filter the filter
     */
    KisBookmarkedFilterConfigurationsModel(KisPaintDeviceSP thumb, KisFilterSP filter);
    ~KisBookmarkedFilterConfigurationsModel() override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    /**
     * @return the filter configuration
     */
    KisFilterConfigurationSP configuration(const QModelIndex &index) const;
private:
    struct Private;
    Private* const d;
};

#endif
