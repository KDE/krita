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
#include <kis_iterators_pixel.h>
#include <kis_pixel.h>
#include <kis_colorspace.h>

#include "kis_histogram.h"
#include "kis_basic_histogram_producers.h"
#include "colorsfilters.h"
#include "kis_brightness_contrast_filter.h"
#include "kis_perchannel_filter.h"

typedef KGenericFactory<ColorsFilters> ColorsFiltersFactory;
K_EXPORT_COMPONENT_FACTORY( kritacolorsfilters, ColorsFiltersFactory( "krita" ) )

ColorsFilters::ColorsFilters(QObject *parent, const char *name, const QStringList &)
        : KParts::Plugin(parent, name)
{
    setInstance(ColorsFiltersFactory::instance());

     kdDebug(41006) << "ColorsFilters plugin. Class: "
           << className()
           << ", Parent: "
           << parent -> className()
           << "\n";


    if (parent->inherits("KisFilterRegistry")) {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(new KisBrightnessContrastFilter());
        manager->add(new KisAutoContrast());
        manager->add(new KisPerChannelFilter());
        manager->add(new KisDesaturateFilter());
    }
}

ColorsFilters::~ColorsFilters()
{
}


//==================================================================


KisAutoContrast::KisAutoContrast() : KisFilter(id(), "adjust", "&Auto Contrast")
{
}

// XXX: This filter should write to dst, too!
void KisAutoContrast::process(KisPaintDeviceImplSP src, KisPaintDeviceImplSP dst, KisFilterConfiguration* , const QRect& rect)
{
    // initialize
    KisHistogramProducerSP producer = new KisGenericLightnessHistogramProducer();
    KisHistogram histogram(src, producer, LINEAR);
    int minvalue = int(255*histogram.calculations().getMin() + 0.5);
    int maxvalue = int(255*histogram.calculations().getMax() + 0.5);

    if(maxvalue>255)
        maxvalue= 255;

    // build the transferfunction
    int diff = maxvalue - minvalue;

    KisBrightnessContrastFilterConfiguration * cfg = new KisBrightnessContrastFilterConfiguration();

    for(int i=0; i <255; i++)
        cfg->transfer[i] = 0xFFFF;

    if (diff != 0)
    {
        for(int i=0; i <minvalue; i++)
            cfg->transfer[i] = 0x0;
        for(int i=minvalue; i <maxvalue; i++)
        {
            Q_INT32 val = (i-minvalue)/diff;

            val = int((0xFFFF * (i-minvalue)) / diff);
            if(val >0xFFFF)
                val=0xFFFF;
            if(val <0)
                val = 0;

            cfg->transfer[i] = val;
        }
        for(int i=maxvalue; i <256; i++)
            cfg->transfer[i] = 0xFFFF;
    }

    // apply
    KisColorAdjustment *adj = src->colorSpace()->createBrightnessContrastAdjustment(cfg->transfer);

    KisRectIteratorPixel iter = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );

    setProgressTotalSteps(rect.width() * rect.height());
    Q_INT32 pixelsProcessed = 0;

    while( ! iter.isDone()  && !cancelRequested())
    {
        Q_UINT32 npix=0, maxpix = iter.nConseqPixels();
        Q_UINT8 selectedness = iter.selectedness();
        // The idea here is to handle stretches of completely selected and completely unselected pixels.
        // Partially selected pixels are handled one pixel at a time.
        switch(selectedness)
        {
            case MIN_SELECTED:
                while(iter.selectedness()==MIN_SELECTED && maxpix)
                {
                    --maxpix;
                    ++iter;
                    ++npix;
                }
                pixelsProcessed += npix;
                break;

            case MAX_SELECTED:
            {
                Q_UINT8 *firstPixel = iter.rawData();
                while(iter.selectedness()==MAX_SELECTED && maxpix)
                {
                    --maxpix;
                    ++iter;
                    ++npix;
                }
                // adjust
                src->colorSpace()->applyAdjustment(firstPixel, firstPixel, adj, npix);
                pixelsProcessed += npix;
                break;
            }

            default:
                // adjust, but since it's partially selected we also only partially adjust
                src->colorSpace()->applyAdjustment(iter.oldRawData(), iter.rawData(), adj, 1);
                const Q_UINT8 *pixels[2] = {iter.oldRawData(), iter.rawData()};
                Q_UINT8 weights[2] = {MAX_SELECTED - selectedness, selectedness};
                src->colorSpace()->mixColors(pixels, weights, 2, iter.rawData());
                ++iter;
                pixelsProcessed++;
                break;
        }
        setProgress(pixelsProcessed);
    }

    setProgressDone();
}


//==================================================================

KisDesaturateFilter::KisDesaturateFilter()
    : KisFilter(id(), "adjust", "&Desaturate")
{
}

void KisDesaturateFilter::process(KisPaintDeviceImplSP src, KisPaintDeviceImplSP dst, KisFilterConfiguration* /*config*/, const QRect& rect)
{
    KisColorAdjustment *adj = src->colorSpace()->createDesaturateAdjustment();

    KisRectIteratorPixel iter = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );

    setProgressTotalSteps(rect.width() * rect.height());
    Q_INT32 pixelsProcessed = 0;

    while( ! iter.isDone()  && !cancelRequested())
    {
        Q_UINT32 npix=0, maxpix = iter.nConseqPixels();
        Q_UINT8 selectedness = iter.selectedness();
        // The idea here is to handle stretches of completely selected and completely unselected pixels.
        // Partially selected pixels are handled one pixel at a time.
        switch(selectedness)
        {
            case MIN_SELECTED:
                while(iter.selectedness()==MIN_SELECTED && maxpix)
                {
                    --maxpix;
                    ++iter;
                    ++npix;
                }
                pixelsProcessed += npix;
                break;

            case MAX_SELECTED:
            {
                Q_UINT8 *firstPixel = iter.rawData();
                while(iter.selectedness()==MAX_SELECTED && maxpix)
                {
                    --maxpix;
                    ++iter;
                    ++npix;
                }
                // adjust
                src->colorSpace()->applyAdjustment(firstPixel, firstPixel, adj, npix);
                pixelsProcessed += npix;
                break;
            }

            default:
                // adjust, but since it's partially selected we also only partially adjust
                src->colorSpace()->applyAdjustment(iter.oldRawData(), iter.rawData(), adj, 1);
                const Q_UINT8 *pixels[2] = {iter.oldRawData(), iter.rawData()};
                Q_UINT8 weights[2] = {MAX_SELECTED - selectedness, selectedness};
                src->colorSpace()->mixColors(pixels, weights, 2, iter.rawData());
                ++iter;
                pixelsProcessed++;
                break;
        }
        setProgress(pixelsProcessed);
    }

    setProgressDone();
}
