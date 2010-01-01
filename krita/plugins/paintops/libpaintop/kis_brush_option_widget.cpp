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
#include "kis_brush_option_widget.h"
#include <klocale.h>

#include <kis_image.h>

#include "kis_brush_selection_widget.h"
#include "kis_brush.h"


KisBrushOptionWidget::KisBrushOptionWidget()
        : KisPaintOpOption(i18n("Brush Tip"))
{
    m_checkable = false;
    m_brushSelectionWidget = new KisBrushSelectionWidget();
    connect(m_brushSelectionWidget, SIGNAL(sigBrushChanged()), SLOT(brushChanged()));
    m_brushSelectionWidget->hide();
    setConfigurationPage(m_brushSelectionWidget);
    m_brushOption.setBrush(brush());
}

KisBrushSP KisBrushOptionWidget::brush() const
{
    return m_brushSelectionWidget->brush();
}

void KisBrushOptionWidget::setAutoBrush(bool on)
{
    m_brushSelectionWidget->setAutoBrush(on);
}

void KisBrushOptionWidget::setPredefinedBrushes(bool on)
{
    m_brushSelectionWidget->setPredefinedBrushes(on);
}

void KisBrushOptionWidget::setCustomBrush(bool on)
{
    m_brushSelectionWidget->setCustomBrush(on);
}

void KisBrushOptionWidget::setTextBrush(bool on)
{
    m_brushSelectionWidget->setTextBrush(on);
}

void KisBrushOptionWidget::setImage(KisImageWSP image)
{
    m_brushSelectionWidget->setImage(image);
}


void KisBrushOptionWidget::writeOptionSetting(KisPropertiesConfiguration* settings) const
{
    m_brushOption.writeOptionSetting(settings);
}

void KisBrushOptionWidget::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_brushOption.readOptionSetting(setting);
    m_brushSelectionWidget->setCurrentBrush(m_brushOption.brush());
}


void KisBrushOptionWidget::setAutoBrushDiameter(qreal diameter)
{
    m_brushSelectionWidget->setAutoBrushDiameter(diameter);
}


qreal KisBrushOptionWidget::autoBrushDiameter()
{
    return m_brushSelectionWidget->autoBrushDiameter();
}

void KisBrushOptionWidget::brushChanged()
{
    m_brushOption.setBrush(brush());
    emit sigSettingChanged();
}