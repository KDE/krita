/*
 * This file is part of the KDE project
 *
 * Copyright (c) Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#ifndef _KIS_OILPAINT_FILTER_H_
#define _KIS_OILPAINT_FILTER_H_

#include "kis_filter.h"
#include <kdebug.h>

class KisOilPaintFilterConfiguration : public KisFilterConfiguration
{
    public:
                KisOilPaintFilterConfiguration(Q_UINT32 brushSize, Q_UINT32 smooth) : m_brushSize(brushSize), m_smooth(smooth) {};
    public:
                inline Q_UINT32 brushSize() { return m_brushSize; };
                inline Q_UINT32 smooth() {return m_smooth; };
        private:
                Q_UINT32 m_brushSize;
                Q_UINT32 m_smooth;
};

class KisOilPaintFilter : public KisFilter
{
public:
    KisOilPaintFilter();
public:
    virtual void process(KisPaintDeviceImplSP,KisPaintDeviceImplSP, KisFilterConfiguration* , const QRect&);
    static inline KisID id() { return KisID("oilpaint", i18n("Oilpaint")); };
    virtual bool supportsPainting() { return true; }
    virtual bool supportsPreview() { return true; }
    virtual std::list<KisFilterConfiguration*> listOfExamplesConfiguration(KisPaintDeviceImplSP dev);
    public:
    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceImplSP dev);
    virtual KisFilterConfiguration* configuration(QWidget*, KisPaintDeviceImplSP dev);
private:
    void OilPaint(KisPaintDeviceImplSP src, int x, int y, int w, int h, int BrushSize, int Smoothness);
    uint MostFrequentColor(KisPaintDeviceImplSP, const QRect& bounds, int X, int Y, int Radius, int Intensity);
    // Function to calcule the color intensity and return the luminance (Y)
    // component of YIQ color model.
    inline uint GetIntensity(uint Red, uint Green, uint Blue) { return ((uint)(Red * 0.3 + Green * 0.59 + Blue * 0.11)); }
};

#endif
