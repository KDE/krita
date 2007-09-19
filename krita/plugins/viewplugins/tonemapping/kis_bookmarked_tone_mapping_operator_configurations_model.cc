/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_bookmarked_tone_mapping_operator_configurations_model.h"

#include <kis_paint_device.h>
#include <kis_properties_configuration.h>

#include "kis_tone_mapping_operator.h"

struct KisBookmarkedToneMappingOperatorConfigurationsModel::Private {
    KisToneMappingOperator* toneMappingOperator;
    KisPaintDeviceSP thumbnail;
};

KisBookmarkedToneMappingOperatorConfigurationsModel::KisBookmarkedToneMappingOperatorConfigurationsModel(KisPaintDeviceSP thumb, KisToneMappingOperator* op)
    : KisBookmarkedConfigurationsModel(op->bookmarkManager()), d(new Private)
{
    d->toneMappingOperator = op;
    d->thumbnail = thumb;
}

KisBookmarkedToneMappingOperatorConfigurationsModel::~KisBookmarkedToneMappingOperatorConfigurationsModel()
{
    delete d;
}

KisPropertiesConfiguration* KisBookmarkedToneMappingOperatorConfigurationsModel::configuration(const QModelIndex &index) const
{
    KisPropertiesConfiguration* config = dynamic_cast<KisPropertiesConfiguration*>(KisBookmarkedConfigurationsModel::configuration(index));
    if(config) return config;
    return new KisPropertiesConfiguration;
}
