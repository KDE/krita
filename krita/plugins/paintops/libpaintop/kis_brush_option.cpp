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
#include "kis_brush_option.h"
#include "kis_brush_selection_widget.h"
#include <klocale.h>
#include "kis_brush.h"

KisBrushOption::KisBrushOption()
        : KisPaintOpOption(i18n("Brush Tip"))
{
    m_checkable = false;
    m_brushSelectionWidget = new KisBrushSelectionWidget();
    connect(m_brushSelectionWidget, SIGNAL(sigBrushChanged()), SIGNAL(sigSettingChanged()));
    m_brushSelectionWidget->hide();
    setConfigurationPage(m_brushSelectionWidget);
}

KisBrush* KisBrushOption::brush() const
{
    return m_brushSelectionWidget->brush();
}


void KisBrushOption::setAutoBrush( bool on )
{
    m_brushSelectionWidget->setAutoBrush( on );
}

void KisBrushOption::setPredefinedBrushes( bool on )
{
    m_brushSelectionWidget->setPredefinedBrushes( on );
}

void KisBrushOption::setCustomBrush( bool on )
{
    m_brushSelectionWidget->setCustomBrush( on );
}

void KisBrushOption::setTextBrush( bool on )
{
    m_brushSelectionWidget->setTextBrush( on );
}


void KisBrushOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
#ifdef __GNUC__
#warning "KisBrushOption::writeOptionSetting: define a serialization format for potato stamps"
#endif


}

void KisBrushOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
#ifdef __GNUC__
#warning "KisBrushOption::readOptionSetting: define a serialization format for potato stamps & a way to  set the active potato stamp correctly"
#endif
}
