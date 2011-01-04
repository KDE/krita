/* This file is part of the KDE project
 * Copyright (C) Silvio Heinrich <plassy@web.de>, (C) 2011
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

#include "kis_pressure_composite_option.h"


#include <klocale.h>

#include <kis_painter.h>
#include <widgets/kis_curve_widget.h>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>

KisPressureCompositeOption::KisPressureCompositeOption()
    : KisCurveOption(i18n("Color"), "Color", KisPaintOpOption::brushCategory(), false)
{
    setMinimumLabel(i18n("No Color"));
    setMaximumLabel(i18n("Full Color"));
}

void KisPressureCompositeOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    KisCurveOption::writeOptionSetting(setting);
    setting->setProperty("CompositeOp", m_compositeOp);
    setting->setProperty("CompositeRateValue", m_rate);
}

void KisPressureCompositeOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOption::readOptionSetting(setting);
    m_compositeOp = setting->getString("CompositeOp");
    m_rate        = setting->getDouble("CompositeRateValue");
    
    if(m_compositeOp == "") //TODO: test if compositeOp is valid instead of just testing for an empty string
        m_compositeOp = COMPOSITE_OVER;
}


void KisPressureCompositeOption::applyOpacityRate(KisPainter* painter, const KisPaintInformation& info, qreal scaleMin, qreal scaleMax) const
{
    if(!isChecked())
        return;
    
    qreal  rate    = scaleMin + (scaleMax - scaleMin) * m_rate; // scale m_rate into the range scaleMin - scaleMax
    quint8 opacity = qBound(OPACITY_TRANSPARENT_U8, (quint8)(rate * computeValue(info) * 255.0), OPACITY_OPAQUE_U8);
    
    painter->setOpacity(opacity);
}

void KisPressureCompositeOption::applyCompositeOp(KisPainter* painter) const
{
    if(isChecked())
        painter->setCompositeOp(m_compositeOp);
}
