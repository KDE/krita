/* This file is part of the KDE project
   Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kis_brightness_contrast_filter.h"
#include "kis_multi_integer_filter_widget.h"
#include "kis_strategy_colorspace.h"
#include "kis_paint_device.h"
#include "kis_iterators_pixel.h"

KisBrightnessContrastFilterConfiguration::KisBrightnessContrastFilterConfiguration(Q_INT32 nbrightness, Q_INT32 ncontrast) :
	m_brightness(nbrightness),
	m_contrast(ncontrast)
{
}

KisBrightnessContrastFilter::KisBrightnessContrastFilter(KisView * view ) : KisFilter( name(), view )
{
	
}

KisFilterConfigurationWidget* KisBrightnessContrastFilter::createConfigurationWidget(QWidget* parent)
{
	vKisIntegerWidgetParam param;
	param.push_back( KisIntegerWidgetParam( -100, 100, 0, "Brightness" ) );
	param.push_back( KisIntegerWidgetParam( -100, 100, 0, "Contrast" ) );
	return new KisMultiIntegerFilterWidget(this, parent, name().ascii(), name().ascii(), param );
}

KisFilterConfiguration* KisBrightnessContrastFilter::configuration(KisFilterConfigurationWidget* nwidget)
{
	KisMultiIntegerFilterWidget* widget = (KisMultiIntegerFilterWidget*) nwidget;
	if( widget == 0 )
	{
		return new KisBrightnessContrastFilterConfiguration( 0, 0 );
	} else {
		return new KisBrightnessContrastFilterConfiguration( widget->valueAt( 0 ), widget->valueAt( 1 ) );
	}
}


void KisBrightnessContrastFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* config, const QRect& rect,KisTileCommand* ktc)
{
	KisBrightnessContrastFilterConfiguration* configBC = (KisBrightnessContrastFilterConfiguration*) config;
	KisIteratorLinePixel srcLineIt = src->iteratorPixelSelectionBegin(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() );
	KisIteratorLinePixel dstLineIt = dst->iteratorPixelSelectionBegin(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() );
	KisIteratorLinePixel lastLine = src->iteratorPixelSelectionEnd(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() + rect.height() - 1);
	Q_INT32 depth = src->depth() - 1;
	double contrast = (100.0 + configBC->contrast()) / 100;
	contrast *= contrast;
	while( srcLineIt <= lastLine )
	{
		KisIteratorPixel srcPixelIt = *srcLineIt;
		KisIteratorPixel dstPixelIt = *dstLineIt;
		KisIteratorPixel lastPixel = srcLineIt.end();
		while( srcPixelIt <= lastPixel )
		{
			for( int i = 0; i < depth; i++)
			{
			// change the brightness
				int nd = srcPixelIt.oldValue()[ i ] + configBC->brightness();
				nd = (int)(((nd - QUANTUM_MAX / 2 ) * contrast) + QUANTUM_MAX / 2);
				dstPixelIt[i] = QMAX( 0, QMIN( QUANTUM_MAX, nd ) );
			}
			++srcPixelIt;
			++dstPixelIt;
		}
		++srcLineIt;
		++dstLineIt;
	}
}
