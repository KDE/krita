/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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

#ifndef KIS_PRESSURE_COMPOSITE_OPTION_H
#define KIS_PRESSURE_COMPOSITE_OPTION_H

#include "kis_curve_option.h"
#include <kis_paint_information.h>
#include <krita_export.h>
#include <kis_types.h>

class QSlider;
class KisPropertiesConfiguration;
class KisPainter;

class PAINTOP_EXPORT KisPressureCompositeOption : public KisCurveOption
{
public:
    KisPressureCompositeOption();

    /**
     * Set the composite mode and opacity of the painter based on the pressure
     * and the curve (if checked) and return the old composite mode
     * of the painter.
     */
    QString apply(KisPainter* painter, qint8 opacity, const KisPaintInformation& info) const;

    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    void readOptionSetting(const KisPropertiesConfiguration* setting);
    
    void setCompositeOp(const QString& compositeOp) { m_compositeOp = compositeOp; }
    QString getCompositeOp() { return m_compositeOp; }
    
    void setRate(int rate) { m_rate = rate; }
    int getRate() { return m_rate; }
    
private:
    QString m_compositeOp;
    int     m_rate;
};

#endif // KIS_PRESSURE_COMPOSITE_OPTION_H
