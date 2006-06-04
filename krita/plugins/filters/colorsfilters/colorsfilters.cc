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

#include <QSlider>
#include <QPoint>
#include <QColor>

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
#include <KoColorSpace.h>
#include <kis_painter.h>
#include <kis_selection.h>
#include "kis_histogram.h"
#include "kis_basic_histogram_producers.h"
#include "colorsfilters.h"
#include "kis_brightness_contrast_filter.h"
#include "kis_perchannel_filter.h"

typedef KGenericFactory<ColorsFilters> ColorsFiltersFactory;
K_EXPORT_COMPONENT_FACTORY( kritacolorsfilters, ColorsFiltersFactory( "krita" ) )

ColorsFilters::ColorsFilters(QObject *parent, const QStringList &)
        : KParts::Plugin(parent)
{
    setInstance(ColorsFiltersFactory::instance());

    if (parent->inherits("KisFilterRegistry")) {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(KisFilterSP(new KisBrightnessContrastFilter()));
        manager->add(KisFilterSP(new KisAutoContrast()));
        manager->add(KisFilterSP(new KisPerChannelFilter()));
        manager->add(KisFilterSP(new KisDesaturateFilter()));
    }
}

ColorsFilters::~ColorsFilters()
{
}


//==================================================================


KisAutoContrast::KisAutoContrast() : KisFilter(id(), "adjust", i18n("&Auto Contrast"))
{
}

bool KisAutoContrast::workWith(KoColorSpace* cs)
{
    return (cs->getProfile() != 0);
}

void KisAutoContrast::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* , const QRect& rect)
{
    // initialize
    KisHistogramProducerSP producer = KisHistogramProducerSP(new KisGenericLabHistogramProducer());
    KisHistogram histogram(src, producer, LINEAR);
    int minvalue = int(255*histogram.calculations().getMin() + 0.5);
    int maxvalue = int(255*histogram.calculations().getMax() + 0.5);

    if(maxvalue>255)
        maxvalue= 255;

    histogram.setChannel(0);
    int twoPercent = int(0.005*histogram.calculations().getCount());
    int pixCount = 0;
    int binnum = 0;

    while(binnum<histogram.producer()->numberOfBins())
    {
        pixCount += histogram.getValue(binnum);
        if(pixCount > twoPercent)
        {
            minvalue = binnum;
            break;
        }
        binnum++;
    }
    pixCount = 0;
    binnum = histogram.producer()->numberOfBins()-1;
    while(binnum>0)
    {
        pixCount += histogram.getValue(binnum);
       if(pixCount > twoPercent)
        {
            maxvalue = binnum;
            break;
        }
        binnum--;
    }
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
            qint32 val = (i-minvalue)/diff;

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

    KisSelectionSP dstSel;
    if (dst != src) {
        KisPainter gc(dst);
        gc.bitBlt(rect.x(), rect.y(), COMPOSITE_COPY, src, rect.x(), rect.y(), rect.width(), rect.height());
        gc.end();
        if (src->hasSelection()) {
            dstSel = dst->selection();
            dst->setSelection(src->selection());
        }
    }

    // apply
    KoColorAdjustment *adj = src->colorSpace()->createBrightnessContrastAdjustment(cfg->transfer);

    KisRectIteratorPixel iter = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );

    setProgressTotalSteps(rect.width() * rect.height());
    qint32 pixelsProcessed = 0;

    while( ! iter.isDone()  && !cancelRequested())
    {
        quint32 npix=0, maxpix = iter.nConseqPixels();
        quint8 selectedness = iter.selectedness();
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
                quint8 *firstPixel = iter.rawData();
                while(iter.selectedness()==MAX_SELECTED && maxpix)
                {
                    if (maxpix != 0)
                        ++iter;
                    ++npix;
                }
                // adjust
                src->colorSpace()->applyAdjustment(firstPixel, firstPixel, adj, npix);
                pixelsProcessed += npix;
                ++iter;
                break;
            }

            default:
                // adjust, but since it's partially selected we also only partially adjust
                src->colorSpace()->applyAdjustment(iter.oldRawData(), iter.rawData(), adj, 1);
                const quint8 *pixels[2] = {iter.oldRawData(), iter.rawData()};
                quint8 weights[2] = {MAX_SELECTED - selectedness, selectedness};
                src->colorSpace()->mixColors(pixels, weights, 2, iter.rawData());
                ++iter;
                pixelsProcessed++;
                break;
        }
        setProgress(pixelsProcessed);
    }
    // Restore selection
    if (src != dst && src->hasSelection()) {
        dst->setSelection(dstSel);
    }
    delete adj;
    setProgressDone();
}


//==================================================================

KisDesaturateFilter::KisDesaturateFilter()
    : KisFilter(id(), "adjust", i18n("&Desaturate"))
{
    m_lastCS = 0;
    m_adj = 0;

}

KisDesaturateFilter::~KisDesaturateFilter()
{
    delete m_adj;
}

bool KisDesaturateFilter::workWith(KoColorSpace* cs)
{
    return (cs->getProfile() != 0);
}

void KisDesaturateFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* /*config*/, const QRect& rect)
{
    if (dst != src) {
        KisPainter gc(dst);
        gc.bitBlt(rect.x(), rect.y(), COMPOSITE_COPY, src, rect.x(), rect.y(), rect.width(), rect.height());
        gc.end();
    }

    if (m_adj == 0 || (m_lastCS && m_lastCS != src->colorSpace())) {
        m_adj = src->colorSpace()->createDesaturateAdjustment();
        m_lastCS = src->colorSpace();
    }

    KisRectIteratorPixel iter = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );

    setProgressTotalSteps(rect.width() * rect.height());
    qint32 pixelsProcessed = 0;

    while( ! iter.isDone()  && !cancelRequested())
    {
        quint32 npix=0, maxpix = iter.nConseqPixels();
        quint8 selectedness = iter.selectedness();
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
                quint8 *firstPixel = iter.rawData();
                while(iter.selectedness()==MAX_SELECTED && maxpix)
                {
                    --maxpix;
                    if (maxpix != 0)
                        ++iter;
                    ++npix;
                }
                // adjust
                src->colorSpace()->applyAdjustment(firstPixel, firstPixel, m_adj, npix);
                pixelsProcessed += npix;
                ++iter;
                break;
            }

            default:
                // adjust, but since it's partially selected we also only partially adjust
                src->colorSpace()->applyAdjustment(iter.oldRawData(), iter.rawData(), m_adj, 1);
                const quint8 *pixels[2] = {iter.oldRawData(), iter.rawData()};
                quint8 weights[2] = {MAX_SELECTED - selectedness, selectedness};
                src->colorSpace()->mixColors(pixels, weights, 2, iter.rawData());
                ++iter;
                pixelsProcessed++;
                break;
        }
        setProgress(pixelsProcessed);
    }
    setProgressDone();
}
