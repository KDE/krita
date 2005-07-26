/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include <klocale.h>

#include <qlayout.h>

#include "kis_brightness_contrast_filter.h"
#include "wdg_brightness_contrast.h"
#include "kis_strategy_colorspace.h"
#include "kis_paint_device.h"
#include "kis_iterators_pixel.h"
#include "tiles/kis_iterator.h"

KisBrightnessContrastFilterConfiguration::KisBrightnessContrastFilterConfiguration(Q_INT32 nbrightness, Q_INT32 ncontrast) :
	m_brightness(nbrightness),
	m_contrast(ncontrast)
{
}

KisBrightnessContrastFilter::KisBrightnessContrastFilter()
	: KisFilter( id(), "adjust", "&Brightness/contrast...")
{

}

KisFilterConfigWidget * KisBrightnessContrastFilter::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP)
{
	KisFilterConfigWidget * w = new KisFilterConfigWidget(parent);
	QHBoxLayout * l = new QHBoxLayout(w);
	l->setAutoAdd(true);
	WdgBrightnessContrast * wbc = new WdgBrightnessContrast(w);
	return w;
}

KisFilterConfiguration* KisBrightnessContrastFilter::configuration(QWidget* nwidget, KisPaintDeviceSP)
{
	WdgBrightnessContrast* widget = (WdgBrightnessContrast*) nwidget;
	
	if ( widget == 0 )
	{
		return new KisBrightnessContrastFilterConfiguration( 0, 0 );
	} else {
		return new KisBrightnessContrastFilterConfiguration( 1, 1 );
	}
}

std::list<KisFilterConfiguration*> KisBrightnessContrastFilter::listOfExamplesConfiguration(KisPaintDeviceSP dev)
{
	std::list<KisFilterConfiguration*> list;
	list.insert(list.begin(), new KisBrightnessContrastFilterConfiguration( 0, 1 ));
	list.insert(list.begin(), new KisBrightnessContrastFilterConfiguration( 1, 0 ));
	return list;
}


void KisBrightnessContrastFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* config, const QRect& rect)
{
	KisBrightnessContrastFilterConfiguration* configBC = (KisBrightnessContrastFilterConfiguration*) config;

	KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
	KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);

	setProgressTotalSteps(rect.width() * rect.height());
	Q_INT32 pixelsProcessed = 0;

	while( ! srcIt.isDone()  && !cancelRequested())
	{
		// change the brightness and contrast
		src->colorStrategy()->adjustBrightnessContrast(srcIt.oldRawData(), dstIt.rawData(), configBC->brightness(),configBC->contrast(),1);
					
		++srcIt;
		++dstIt;

		pixelsProcessed++;
		setProgress(pixelsProcessed);
	}

	setProgressDone();
}
