/*
 * This file is part of the KDE project
 *
 * Copyright (c) Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
*/

#ifndef _KIS_OILPAINT_FILTER_H_
#define _KIS_OILPAINT_FILTER_H_

#include "kis_filter.h"
#include "kis_view.h"
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
                KisOilPaintFilter(KisView * view);
        public:
	       virtual void process(KisPaintDeviceSP,KisPaintDeviceSP, KisFilterConfiguration* , const QRect&, KisTileCommand* );
                static inline QString name() { return "Oilpaint"; };
        public:
		virtual KisFilterConfigurationWidget* createConfigurationWidget(QWidget* parent);
		virtual KisFilterConfiguration* configuration(KisFilterConfigurationWidget*);
        private:
                void OilPaint(QUANTUM* data, int w, int h, int BrushSize, int Smoothness, KisProgressDisplayInterface *m_progress);
                inline uint MostFrequentColor(uchar* Bits, int Width, int Height, int X, 
                                  int Y, int Radius, int Intensity);                           
                // Function to calcule the color intensity and return the luminance (Y)
                // component of YIQ color model.
                inline uint GetIntensity(uint Red, uint Green, uint Blue)
                { return ((uint)(Red * 0.3 + Green * 0.59 + Blue * 0.11)); } 
};

#endif
