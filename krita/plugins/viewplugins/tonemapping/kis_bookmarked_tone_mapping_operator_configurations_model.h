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

#ifndef _KIS_BOOKMARKED_TONE_MAPPING_OPERATOR_CONFIGURATIONS_MODEL_H_
#define _KIS_BOOKMARKED_TONE_MAPPING_OPERATOR_CONFIGURATIONS_MODEL_H_

#include <kis_bookmarked_configurations_model.h>
#include <kis_types.h>

class KisToneMappingOperator;
class KisPropertiesConfiguration;

class  KisBookmarkedToneMappingOperatorConfigurationsModel : public KisBookmarkedConfigurationsModel {
    public:
        KisBookmarkedToneMappingOperatorConfigurationsModel(KisPaintDeviceSP thumb, KisToneMappingOperator* op);
        ~KisBookmarkedToneMappingOperatorConfigurationsModel();
        KisPropertiesConfiguration* configuration(const QModelIndex &index) const;
    private:
        struct Private;
        Private* const d;
};

#endif
