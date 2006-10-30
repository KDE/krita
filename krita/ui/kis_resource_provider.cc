/*
 *  Copyright (c) 2006 Boudewijn Rempt  <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_resource_provider.h"

#include "kis_brush.h"
#include "kis_pattern.h"
#include "kis_gradient.h"
#include "kis_view2.h"

KoCanvasBase * KisResourceProvider::canvas() const
{
    return m_view->canvasBase();
}

KoColor KisResourceProvider::bgColor() const
{
    return m_bgColor;
}

KoColor KisResourceProvider::fgColor() const
{
    return m_fgColor;
}

float KisResourceProvider::HDRExposure() const
{
    return m_HDRExposure;
}

void KisResourceProvider::setHDRExposure(float exposure)
{
    m_HDRExposure = exposure;
}


KisBrush * KisResourceProvider::currentBrush() const
{
    return m_brush;
}


KisPattern * KisResourceProvider::currentPattern() const
{
    return m_pattern;
}


KisGradient * KisResourceProvider::currentGradient() const
{
    return m_gradient;
}



KoID KisResourceProvider::currentPaintop() const
{
    return m_paintop;
}


const KisPaintOpSettings * KisResourceProvider::currentPaintopSettings() const
{
    return m_paintopSettings;
}



void KisResourceProvider::brushActivated(KisResource *brush)
{

    m_brush = dynamic_cast<KisBrush*>(brush);

    if (m_brush )
    {
        emit brushChanged(m_brush);
    }
}

void KisResourceProvider::patternActivated(KisResource *pattern)
{
    m_pattern = dynamic_cast<KisPattern*>(pattern);

    if (m_pattern) {
        emit patternChanged(m_pattern);
    }
}

void KisResourceProvider::gradientActivated(KisResource *gradient)
{

    m_gradient = dynamic_cast<KisGradient*>(gradient);

    if (m_gradient) {
        emit gradientChanged(m_gradient);
    }
}

void KisResourceProvider::paintopActivated(const KoID & paintop, const KisPaintOpSettings *paintopSettings)
{
    if (paintop.id().isNull() || paintop.id().isEmpty()) {
        return;
    }

    m_paintop = paintop;
    m_paintopSettings = paintopSettings;
    emit paintopChanged(m_paintop, paintopSettings);
}

void KisResourceProvider::setBGColor(const KoColor& c)
{
    m_bgColor = c;
    emit sigBGColorChanged( c );
}

void KisResourceProvider::setFGColor(const KoColor& c)
{
    m_fgColor = c;
    emit sigFGColorChanged( c );
}

void KisResourceProvider::slotSetFGColor(const KoColor& c)
{
    m_fgColor = c;
    emit sigFGColorChanged( c );
}

void KisResourceProvider::slotSetBGColor(const KoColor& c)
{
    m_bgColor = c;
    emit sigBGColorChanged( c );
}

#include "kis_resource_provider.moc"
