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

#include "kis_smudge_option.h"

#include <klocale.h>

#include <kis_painter.h>
#include <widgets/kis_curve_widget.h>

#include <KoColor.h>
#include <KoColorSpace.h>

KisSmudgeOption::KisSmudgeOption(const QString& name, const QString& label, bool checked, const QString& category):
    KisRateOption(name, label, checked, category),
    mMode(SMEARING_MODE) { }

void KisSmudgeOption::apply(KisPainter& painter, const KisPaintInformation& info, qreal scaleMin, qreal scaleMax, qreal multiplicator) const
{
    if(!isChecked()) {
        painter.setOpacity((quint8)(scaleMax * 255.0));
        return;
    }
    
    qreal  rate    = scaleMin + (scaleMax - scaleMin) * multiplicator * computeValue(info); // scale m_rate into the range scaleMin - scaleMax
    quint8 opacity = qBound(OPACITY_TRANSPARENT_U8, (quint8)(rate * 255.0), OPACITY_OPAQUE_U8);
    
    painter.setOpacity(opacity);
}

void KisSmudgeOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    KisRateOption::writeOptionSetting(setting);
    setting->setProperty(name() + "Mode", mMode);
}

void KisSmudgeOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisRateOption::readOptionSetting(setting);
    
    if(setting->hasProperty(name() + "Mode"))
        mMode = (Mode)setting->getInt(name() + "Mode", mMode);
    else
        mMode = SMEARING_MODE;
}
