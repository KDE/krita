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
    setMinimumLabel(i18n("Full Color"));
    setMaximumLabel(i18n("No Color"));
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
    m_rate        = setting->getInt("CompositeRateValue");
    
    if(m_compositeOp == "") //TODO: test if compositeOp is valid instead of just testing for an empty string
        m_compositeOp = COMPOSITE_OVER;
}

QString KisPressureCompositeOption::apply(KisPainter* painter, qint8 opacity, const KisPaintInformation& info) const
{
    if(!isChecked())
        return painter->compositeOp()->id();
    
    QString oldCompositeOp = painter->compositeOp()->id();
    
    opacity = (m_rate * 255) / 100;
    opacity = qBound((qint32)OPACITY_TRANSPARENT_U8,
                     (qint32)(double(opacity) * computeValue(info) / PRESSURE_DEFAULT),
                     (qint32)OPACITY_OPAQUE_U8);
    
    //qreal  opacity1 = (qreal)(painter->opacity() * computeValue(info));
    //quint8 opacity2 = (quint8)qRound(qBound<qreal>(OPACITY_TRANSPARENT_U8, opacity1, OPACITY_OPAQUE_U8));
    
    painter->setCompositeOp(m_compositeOp);
    painter->setOpacity(opacity);

    return oldCompositeOp;
}

