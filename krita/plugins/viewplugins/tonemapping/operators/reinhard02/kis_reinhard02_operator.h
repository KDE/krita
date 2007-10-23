/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_REINHARD02_OPERATOR_H_
#define _KIS_REINHARD02_OPERATOR_H_

#include "kis_tone_mapping_operator.h"

class KisReinhard02Operator : public KisToneMappingOperator {
    public:
        KisReinhard02Operator();
        virtual KisToneMappingOperatorConfigurationWidget* createConfigurationWidget(QWidget*) const;
        virtual void toneMap(KisPaintDeviceSP, KisPropertiesConfiguration* config) const;
        virtual KoColorSpace* colorSpace() const ;
};

#endif
