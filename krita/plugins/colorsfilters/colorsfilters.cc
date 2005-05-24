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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <math.h>

#include <stdlib.h>
#include <string.h>

#include <qslider.h>
#include <qpoint.h>
#include <qcolor.h>

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
#include <kis_types.h>
#include <kis_view.h>
#include <kis_iterators_pixel.h>
#include <kis_pixel.h>
#include <kis_strategy_colorspace.h>
// #include <kmessagebox.h>

#include "colorsfilters.h"
#include "kis_brightness_contrast_filter.h"

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


	if ( !parent->inherits("KisView") )
	{
		return;
	} else {
		m_view = (KisView*) parent;
	}

	KisFilterSP kbc = createFilter<KisBrightnessContrastFilter>(m_view);
	(void) new KAction(i18n("&Brightness / Contrast..."), 0, 0, kbc, SLOT(slotActivated()), actionCollection(), "brightnesscontrast");
	
	KisFilterSP kac = createFilter<KisAutoContrast>(m_view);
	(void) new KAction(i18n("&Auto Contrast"), 0, 0, kac, SLOT(slotActivated()), actionCollection(), "autocontrast");

	KisFilterSP kgc = createFilter<KisGammaCorrectionFilter>(m_view);
	(void) new KAction(i18n("&Gamma Correction..."), 0, 0, kgc, SLOT(slotActivated()), actionCollection(), "gammacorrection");

	KisFilterSP kfca = createFilter<KisColorAdjustmentFilter>(m_view);
	(void) new KAction(i18n("&Color Adjustment..."), 0, 0, kfca, SLOT(slotActivated()), actionCollection(), "coloradjustment");
	
	KisFilterSP kdf = createFilter<KisDesaturateFilter>(m_view);
	(void) new KAction(i18n("&Desaturate"), 0, 0, kdf, SLOT(slotActivated()), actionCollection(), "desaturate");
}

ColorsFilters::~ColorsFilters()
{
}

//==================================================================

KisColorAdjustmentFilter::KisColorAdjustmentFilter(KisView * view) :
	KisIntegerPerChannelFilter(view, id(), -255, 255, 0)
{
}

void KisColorAdjustmentFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* config, const QRect& rect)
{
	KisIntegerPerChannelFilterConfiguration* configPC = (KisIntegerPerChannelFilterConfiguration*) config;
	KisRectIteratorPixel rectIt = src->createRectIterator(rect.x(), rect.y(), rect.width(),rect.height(), true);
	Q_INT32 depth = src->nChannels() - 1;

	setProgressTotalSteps(rect.width() * rect.height());
	Q_INT32 pixelsProcessed = 0;

	while( ! rectIt.isDone() && !cancelRequested())
	{
		if (rectIt.isSelected()) {
			KisPixelRO data = rectIt.oldPixel();
			KisPixel dstData = rectIt.pixel();
			for( int i = 0; i < depth; i++)
			{
				KisQuantum d = rectIt[ configPC->channel( i ) ];
				Q_INT32 s = configPC->valueFor( i );
				if( d < -s  ) dstData[ configPC->channel( i ) ] = 0;
				else if( d > QUANTUM_MAX - s) dstData[ configPC->channel( i ) ] = QUANTUM_MAX;
				else dstData[ configPC->channel( i ) ] = d + s;
			}
		}
		++rectIt;

		pixelsProcessed++;
		setProgress(pixelsProcessed);
	}

	setProgressDone();
}


//==================================================================

KisGammaCorrectionFilter::KisGammaCorrectionFilter(KisView * view)
	: KisDoublePerChannelFilter(view, id(), 0.1, 6.0, 1.0)
{
}

void KisGammaCorrectionFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* config, const QRect& rect)
{
	KisDoublePerChannelFilterConfiguration* configPC = (KisDoublePerChannelFilterConfiguration*) config;
	KisRectIteratorPixel rectIt = src->createRectIterator(rect.x(), rect.y(), rect.width(),rect.height(), true);
	Q_INT32 depth = src->nChannels() - 1;

	setProgressTotalSteps(rect.width() * rect.height());
	Q_INT32 pixelsProcessed = 0;

	while( ! rectIt.isDone() && !cancelRequested())
	{
		if (rectIt.isSelected()) {
			for( int i = 0; i < depth; i++)
			{
				QUANTUM sd = rectIt.oldRawData()[ configPC->channel( i ) ];
				KisQuantum dd = rectIt[ configPC->channel( i ) ];
				dd = (QUANTUM)( QUANTUM_MAX * pow( ((float)sd)/QUANTUM_MAX, 1.0 / configPC->valueFor( i ) ) );
			}
		}
		++rectIt;

		pixelsProcessed++;
		setProgress(pixelsProcessed);
	}

	setProgressDone();
}

//==================================================================


KisAutoContrast::KisAutoContrast(KisView* view) : KisFilter(id(), view)
{

}
void KisAutoContrast::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* , const QRect& rect)
{
	setProgressTotalSteps(rect.width() * rect.height() * 2);
	Q_INT32 pixelsProcessed = 0;

	KisRectIteratorPixel rectIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);

	// Number of channels in this device except alpha
 	Q_INT32 depth = src -> colorStrategy() -> nColorChannels();


	// initialize
	QUANTUM* maxvalues = new QUANTUM[depth];
	QUANTUM* minvalues = new QUANTUM[depth];
	memset(maxvalues, 0, depth * sizeof(QUANTUM));
	memset(minvalues, OPACITY_OPAQUE, depth * sizeof(QUANTUM));
	
	QUANTUM** lut = new QUANTUM*[depth];

	for (int i = 0; i < depth; i++) {
		lut[i] = new QUANTUM[QUANTUM_MAX+1];
		memset(lut[i], 0, (QUANTUM_MAX+1) * sizeof(QUANTUM));
	}

	while (!rectIt.isDone() && !cancelRequested())
	{
		if (rectIt.isSelected()) {
			QUANTUM opacity;

			QColor color;
			src -> colorStrategy() -> toQColor(rectIt.rawData(), &color, &opacity);

			// skip non-opaque pixels
			if (src -> colorStrategy() -> alpha() && opacity != OPACITY_OPAQUE) {
				++rectIt;
				continue;
			}

			for (int i = 0; i < depth; i++) {
				QUANTUM index = rectIt.rawData()[i];
				if( index > maxvalues[i])
					maxvalues[i] = index;
				if( index < minvalues[i])
					minvalues[i] = index;
			}
		}
		++rectIt;

		pixelsProcessed++;
		setProgress(pixelsProcessed);
	}

	if (cancelRequested()) {
		setProgressDone();
		return;
	}
	
	// build the LUT
	for (int i = 0; i < depth; i++) {
		QUANTUM diff = maxvalues[i] - minvalues[i];
		if (diff != 0) {
			for (int j = minvalues[i]; j <= maxvalues[i]; j++) {
				lut[i][j] = QUANTUM_MAX * (j - minvalues[i]) / diff;
			}
		} else {
			lut[i][minvalues[i]] = minvalues[i];
		}
	}

	// apply

	rectIt = src->createRectIterator(rect.x(), rect.y(), rect.width(),rect.height(), true);

	while (!rectIt.isDone()  && !cancelRequested()) {
		if (rectIt.isSelected()) {
			QUANTUM* dstData = rectIt.rawData();

			// Iterate through all channels except alpha
			for (int i = 0; i < depth; ++i) {
				dstData[i] = lut[i][dstData[i]];
			}
		}
		++rectIt;

		pixelsProcessed++;
		setProgress(pixelsProcessed);
	}
	
	// and delete everything
	delete[] maxvalues;
	delete[] minvalues;
	for (int i = 0; i < depth; i++) {
		delete[] lut[i];
	}
	delete[] lut;

	setProgressDone();
}


//==================================================================

KisDesaturateFilter::KisDesaturateFilter(KisView * view)
	: KisFilter(id(), view)
{
}

void KisDesaturateFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* /*config*/, const QRect& rect)
{
	KisRectIteratorPixel rectIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true);

	KisStrategyColorSpaceSP scs = src -> colorStrategy();
	KisProfileSP profile = src -> profile();

	setProgressTotalSteps(rect.width() * rect.height());
	Q_INT32 pixelsProcessed = 0;

	while( ! rectIt.isDone() && !cancelRequested())
	{
		if (rectIt.isSelected()) {
			QColor c;

			const Q_UINT8 * srcData = rectIt.oldRawData();
			// Try to be colorspace independent
			scs -> toQColor(srcData, &c, profile);
			;
			/* I thought of using the HSV model, but GIMP seems to use
				HSL for desaturating. Better use the gimp model for now
				(HSV produces a lighter image than HSL) */

// 			Q_INT32 lightness = ( QMAX(QMAX(c.red(), c.green()), c.blue())
// 					+ QMIN(QMIN(c.red(), c.green()), c.blue()) ) / 2;

			// XXX: BSAR: Doesn't this doe the same but better?
			Q_INT32 lightness = qGray(c.red(), c.green(), c.blue());
			scs -> nativeColor(QColor(lightness, lightness, lightness), rectIt.rawData(), profile);
		}
		++rectIt;

		pixelsProcessed++;
		setProgress(pixelsProcessed);
	}
	setProgressDone();
}
