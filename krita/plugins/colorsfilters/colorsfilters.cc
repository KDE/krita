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

#include <math.h>

#include <stdlib.h>

#include <qslider.h>
#include <qpoint.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_doc.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_tile_command.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kistile.h>
#include <kistilemgr.h>
#include <kis_iterators_pixel.h>

// #include <kmessagebox.h>

#include "colorsfilters.h"
#include "kis_brightness_contrast_filter.h"

#define min(x,y) ((x)<(y)?(x):(y))

typedef KGenericFactory<ColorsFilters> ColorsFiltersFactory;
K_EXPORT_COMPONENT_FACTORY( kritacolorsfilters, ColorsFiltersFactory( "krita" ) )

ColorsFilters::ColorsFilters(QObject *parent, const char *name, const QStringList &)
		: KParts::Plugin(parent, name)
{
	setInstance(ColorsFiltersFactory::instance());

	kdDebug() << "ColorsFilters plugin. Class: " 
		  << className() 
		  << ", Parent: " 
		  << parent -> className()
		  << "\n";


	KisBrightnessContrastFilter* kbc = new KisBrightnessContrastFilter();
	(void) new KAction(i18n("&Brightness / Contrast..."), 0, 0, kbc, SLOT(slotActivated()), actionCollection(), "brightnesscontrast");
	KisGammaCorrectionFilter* kgc = new KisGammaCorrectionFilter();
	(void) new KAction(i18n("&Gamma Correction..."), 0, 0, kgc, SLOT(slotActivated()), actionCollection(), "gammacorrection");
	KisColorAdjustementFilter* kfca = new KisColorAdjustementFilter();
	(void) new KAction(i18n("&Color Adjustment..."), 0, 0, kfca, SLOT(slotActivated()), actionCollection(), "coloradjustment");
	KisDesaturateFilter* kdf = new KisDesaturateFilter();
	(void) new KAction(i18n("&Desaturate"), 0, 0, kdf, SLOT(slotActivated()), actionCollection(), "desaturate");
	if ( !parent->inherits("KisView") )
	{
		m_view = 0;
	} else {
		m_view = (KisView*) parent;
	}
}

ColorsFilters::~ColorsFilters()
{
}

KisColorAdjustementFilter::KisColorAdjustementFilter() : KisPerChannelFilter("Color adjustment", -255, 255, 0)
{
}

void KisColorAdjustementFilter::process(KisPaintDeviceSP device, KisFilterConfiguration* config, const QRect& rect,KisTileCommand* ktc)
{
	KisPerChannelFilterConfiguration* configPC = (KisPerChannelFilterConfiguration*) config;
	KisIteratorLinePixel lineIt = device->iteratorPixelSelectionBegin(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() );
	KisIteratorLinePixel lastLine = device->iteratorPixelSelectionEnd(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() + rect.height() - 1);
	Q_INT32 depth = device->depth() - 1;
	while( lineIt <= lastLine )
	{
		KisIteratorPixel pixelIt = *lineIt;
		KisIteratorPixel lastPixel = lineIt.end();
		while( pixelIt <= lastPixel )
		{
			KisPixelRepresentation data = pixelIt;
			for( int i = 0; i < depth; i++)
			{
				KisQuantum d = pixelIt[ configPC->channel( i ) ];
				Q_INT32 s = configPC->valueFor( i );
				if( d < -s  ) d = 0;
				else if( d > QUANTUM_MAX - s) d = QUANTUM_MAX;
				else d = d + s;
			}
			++pixelIt;
		}
		++lineIt;
	}
}

KisGammaCorrectionFilter::KisGammaCorrectionFilter() : KisPerChannelFilter("Gamma adjustement", 1, 600, 1)
{
}

void KisGammaCorrectionFilter::process(KisPaintDeviceSP device, KisFilterConfiguration* config, const QRect& rect,KisTileCommand* ktc)
{
	KisPerChannelFilterConfiguration* configPC = (KisPerChannelFilterConfiguration*) config;
	KisIteratorLinePixel lineIt = device->iteratorPixelSelectionBegin(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() );
	KisIteratorLinePixel lastLine = device->iteratorPixelSelectionEnd(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() + rect.height() - 1);
	Q_INT32 depth = device->depth() - 1;
	while( lineIt <= lastLine )
	{
		KisIteratorPixel pixelIt = *lineIt;
		KisIteratorPixel lastPixel = lineIt.end();
		while( pixelIt <= lastPixel )
		{
			KisPixelRepresentation data = pixelIt;
			for( int i = 0; i < depth; i++)
			{
				KisQuantum d = pixelIt[ configPC->channel( i ) ];
				d = ( QUANTUM_MAX * pow( ((float)d)/QUANTUM_MAX, 1.0 / configPC->valueFor( i ) ) );
			}
			++pixelIt;
		}
		++lineIt;
	}
}

KisDesaturateFilter::KisDesaturateFilter() : KisFilter("Desaturate")
{
}

void KisDesaturateFilter::process(KisPaintDeviceSP device, KisFilterConfiguration* config, const QRect& rect,KisTileCommand* ktc)
{
	if(colorStrategy()->name() != "RGBA")
		return;
	KisIteratorLinePixel lineIt = device->iteratorPixelSelectionBegin(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() );
	KisIteratorLinePixel lastLine = device->iteratorPixelSelectionEnd(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() + rect.height() - 1);
	Q_INT32 depth = device->depth() - 1;
	while( lineIt <= lastLine )
	{
		KisIteratorPixel pixelIt = *lineIt;
		KisIteratorPixel lastPixel = lineIt.end();
		while( pixelIt <= lastPixel )
		{
			KisPixelRepresentation data = pixelIt;
			/* I thought of using the HSV model, but GIMP seems to use
					HSL for desaturating. Better use the gimp model for now 
					(HSV produces a lighter image than HSL) */
			Q_INT32 lightness = ( QMAX(QMAX(data[0], data[1]), data[2])
													+ QMIN(QMIN(data[0], data[1]), data[2]) ) / 2; 
			data[0] = lightness;
			data[1] = lightness;
			data[2] = lightness;
			++pixelIt;
		}
		++lineIt;
	}
}
