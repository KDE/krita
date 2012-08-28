/* This file is part of the KDE project
 * 
 * Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_RATE_OPTION_H
#define KIS_RATE_OPTION_H

#include "kis_curve_option.h"
#include <kis_paint_information.h>
#include <kis_types.h>

class KisPropertiesConfiguration;
class KisPainter;

class KisRateOption: public KisCurveOption
{
public:
    KisRateOption(const QString& name, const QString& label="", bool checked=true, const QString& category=KisPaintOpOption::brushCategory());
    
    /**
     * Set the opacity of the painter based on the rate
     * and the curve (if checked)
     */
    void apply(KisPainter& painter, const KisPaintInformation& info, qreal scaleMin=0.0, qreal scaleMax=1.0, qreal multiplicator=1.0) const;
    
    void setRate(qreal rate) { KisCurveOption::setValue(rate); }
    qreal getRate() const { return KisCurveOption::value(); }
};

#endif // KIS_RATE_OPTION_H
