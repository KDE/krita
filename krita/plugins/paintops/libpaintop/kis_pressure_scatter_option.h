/* 
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_PRESSURE_SCATTER_OPTION_H
#define KIS_PRESSURE_SCATTER_OPTION_H

#include "kis_curve_option.h"
#include <kis_paint_information.h>
#include <krita_export.h>
#include <kis_types.h>


const QString SCATTER_X = "Scattering/AxisX";
const QString SCATTER_Y = "Scattering/AxisY";
const QString SCATTER_AMOUNT = "Scattering/Amount";

/**
 * Scatters the position of the dab
 */
class PAINTOP_EXPORT KisPressureScatterOption : public KisCurveOption
{
public:
    KisPressureScatterOption();

    /**
    * Set the 
    */
    QPointF apply(const KisPaintInformation& info, qreal diameter) const;

    virtual void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    virtual void readOptionSetting(const KisPropertiesConfiguration* setting);
    
    void enableAxisY(bool enable);
    void enableAxisX(bool enable);
    bool isAxisXEnabled();
    bool isAxisYEnabled();
    void setScatterAmount(qreal amount);
    qreal scatterAmount();
    
private:
    bool m_axisX;
    bool m_axisY;
};

#endif
