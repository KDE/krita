/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_BOOKMARKED_FILTER_CONFIGURATIONS_MODEL_H_
#define _KIS_BOOKMARKED_FILTER_CONFIGURATIONS_MODEL_H_

#include "kis_bookmarked_configurations_model.h"

#include "kis_types.h"

class KisFilterConfiguration;

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
    ~KisBookmarkedFilterConfigurationsModel();
    /**
     * Calling this method with role == Qt::DecorationRole will return a 100x100
     * thumbnail.
     */
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    /**
     * @return the filter configuration
     */
    KisFilterConfiguration* configuration(const QModelIndex &index) const;
private slots:
    void previewUpdated(int i);
private:
    struct Private;
    Private* const d;
};

#endif
