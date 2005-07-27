/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 * ported from Gimp, Copyright (C) 1997 Eiichi Takamori <taka@ma1.seikyou.ne.jp>
 * original pixelize.c for GIMP 0.54 by Tracy Scott
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

#include <stdlib.h>
#include <vector>

#include <qpoint.h>
#include <qspinbox.h>
#include <qptrvector.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <knuminput.h>

#include <kis_doc.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_progress_display_interface.h>

#include "kis_multi_integer_filter_widget.h"
#include "kis_separate_channels.h"
#include "kis_strategy_colorspace.h"
#include "kis_colorspace_registry.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

KisSeparateChannels::KisSeparateChannels() : KisFilter(id(), "artistic", "&Separate channels...")
{
}

void KisSeparateChannels::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* configuration, const QRect& rect)
{
        Q_INT32 x = rect.x(), y = rect.y();
        Q_INT32 width = rect.width();
        Q_INT32 height = rect.height();
        
        //read the filter configuration values from the KisFilterConfiguration object
        Q_UINT32 pixelWidth = ((KisSeparateChannelsConfiguration*)configuration)->pixelWidth();
        Q_UINT32 pixelHeight = ((KisSeparateChannelsConfiguration*)configuration)->pixelHeight();
        
        //pixelize(src, dst, x, y, width, height, pixelWidth, pixelHeight);
	separateChannels(src, dst, rect);
}

void KisSeparateChannels::separateChannels(KisPaintDeviceSP src, KisPaintDeviceSP dst, const QRect& rect)
{
	Q_UINT32 numberOfChannels = src -> nChannels();
	KisStrategyColorSpaceSP colorStrategy = src -> colorStrategy();
	vKisChannelInfoSP channels = colorStrategy -> channels();

	setProgressTotalSteps( numberOfChannels );
        setProgressStage(i18n("Applying separate channels..."),0);
	
	vKisLayerSP layers;
	
	vKisChannelInfoSP_cit begin = channels.begin();
	vKisChannelInfoSP_cit end = channels.end();
	
	int i = 0;
	for (vKisChannelInfoSP_cit it = begin; it != end; ++it)
	{
		KisChannelInfoSP ch = (*it);
		
		KisLayerSP dev = new KisLayer( KisColorSpaceRegistry::instance() -> get( "GRAYA" ), ch->name());
		kdDebug() << "Adding layer: " << ch->name() << endl;	
		layers.push_back(dev);
	
		KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);
		// XXX: Casper is going to make sure that these iterators align!
		KisRectIteratorPixel dstIt = dev->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true);

		while( ! srcIt.isDone() )
		{	
			if(srcIt.isSelected())
			{
				dstIt.rawData()[0] = srcIt.oldRawData()[i];
				dstIt.rawData()[1] = OPACITY_OPAQUE;
			}
			++dstIt;
			++srcIt;
		}
		++i;
	}

	KisImageSP img = src -> image();
	
	vKisLayerSP_it it;

	for ( it = layers.begin(); it != layers.end(); ++it ) {
		KisLayerSP layer = (*it);
		img->add( layer, -1);	
	}
	kdDebug() << "Number of Layers: " << img->nlayers() << endl;


	setProgressDone();
}

void KisSeparateChannels::pixelize(KisPaintDeviceSP src, KisPaintDeviceSP dst, int startx, int starty, int width, int height, int pixelWidth, int pixelHeight)
{
        Q_INT32 pixelSize = src -> pixelSize();
        QMemArray<Q_INT32> average(  pixelSize );
        Q_UINT8* bufRow; 
        Q_UINT8* buf;
        Q_INT32 count;                
        bool hasAlpha = src -> hasAlpha();
        Q_INT32 rowstride; 

        //calculate the total number of pixels
        Q_INT32 numX=0;
        Q_INT32 numY=0;
        
        for (Q_INT32 x = startx; x < startx + width; x += pixelWidth - (x % pixelWidth))
        {
                numX++;
        }
        for (Q_INT32 y = starty; y < starty + height; y += pixelHeight - (y % pixelHeight))
        {
                numY++;
        }       
        
        setProgressTotalSteps( numX * numY );
        setProgressStage(i18n("Applying separate channels..."),0);
        
        Q_INT32 numberOfPixelsProcessed = 0;
        
        for (Q_INT32 y = starty; y < starty + height; y += pixelHeight - (y % pixelHeight))
        {
                Q_INT32 h = pixelHeight - (y % pixelHeight);
                h = MIN(h, starty + height - y);

                for (Q_INT32 x = startx; x < startx + width; x += pixelWidth - (x % pixelWidth))
                {
                        Q_INT32 w = pixelWidth - (x % pixelWidth);
                        w = MIN(w, startx + width - x);
                        
                        for (Q_INT32 i = 0; i < pixelSize; i++)
                        {
                                average[i] = 0;
                        }
                        count = 0;

                        //read
                        KisRectIteratorPixel srcIt = src->createRectIterator(x, y, w, h, false);
                        if ( hasAlpha )
                        {
                                while( ! srcIt.isDone() )
                                {
                                        if(srcIt.isSelected())
                                        {
                                                Q_INT32 alpha = srcIt.oldRawData()[pixelSize-1];
        
                                                average[pixelSize-1] += alpha;
                                                for (Q_INT32 i = 0; i < pixelSize-1; i++)
                                                {        
                                                        average[i] += srcIt.oldRawData()[i] * alpha;
                                                }      
                                                count++;
                                        }
                                        ++srcIt;
                                }
                        }
                        else
                        {
                                while( ! srcIt.isDone() )
                                {
                                        if(srcIt.isSelected())
                                        {
                                                for (Q_INT32 i = 0; i < pixelSize; i++)
                                                {        
                                                        average[i] += srcIt.oldRawData()[i];
                                                }  
                                                count++;
                                        }
                                        ++srcIt;
                                }
                        }

                        //average
                        if (count > 0)
                        {
                                if ( hasAlpha )
                                        {
                                        Q_INT32 alpha = average[pixelSize-1];
                        
                                        if ((average[pixelSize-1] = alpha / count))
                                        {
                                        for (Q_INT32 i = 0; i < pixelSize-1; i++)
                                                average[i] /= alpha;
                                        }
                                }
                                else
                                {
                                        for (Q_INT32 i = 0; i < pixelSize; i++)
                                        average[i] /= count;
                                }
                        }
                        //write
                        srcIt = src->createRectIterator(x, y, w, h, false);
                        KisRectIteratorPixel dstIt = dst->createRectIterator(x, y, w, h, true );
                        while( ! srcIt.isDone() )
                        {
                                if(srcIt.isSelected())
                                {
                                        for( int i = 0; i < pixelSize; i++)
                                        {
                                                dstIt.rawData()[i] = average[i];
                                        }
                                }
                                ++srcIt;
                                ++dstIt;
                        }
                numberOfPixelsProcessed++;
                setProgress(numberOfPixelsProcessed);
                }
        }
		
	setProgressDone();
}

KisFilterConfigWidget * KisSeparateChannels::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev)
{
	vKisIntegerWidgetParam param;
	param.push_back( KisIntegerWidgetParam( 2, 40, 10, i18n("Pixelwidth") ) );
	param.push_back( KisIntegerWidgetParam( 2, 40, 10, i18n("Pixelheight") ) );
	return new KisMultiIntegerFilterWidget(parent, id().id().ascii(), id().id().ascii(), param );
}

KisFilterConfiguration* KisSeparateChannels::configuration(QWidget* nwidget, KisPaintDeviceSP dev)
{
	KisMultiIntegerFilterWidget* widget = (KisMultiIntegerFilterWidget*) nwidget;
	if( widget == 0 )
	{
		return new KisSeparateChannelsConfiguration( 10, 10);
	} else {
		return new KisSeparateChannelsConfiguration( widget->valueAt( 0 ), widget->valueAt( 1 ) );
	}
}
