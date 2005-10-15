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

#include "colorsfilters.h"
#include "kis_brightness_contrast_filter.h"
#include "kis_perchannel_filter.h"

typedef KGenericFactory<ColorsFilters> ColorsFiltersFactory;
K_EXPORT_COMPONENT_FACTORY( kritacolorsfilters, ColorsFiltersFactory( "krita" ) )

ColorsFilters::ColorsFilters(QObject *parent, const char *name, const QStringList &)
        : KParts::Plugin(parent, name)
{
    setInstance(ColorsFiltersFactory::instance());

     kdDebug(DBG_AREA_PLUGINS) << "ColorsFilters plugin. Class: "
           << className()
           << ", Parent: "
           << parent -> className()
           << "\n";


    if ( parent->inherits("KisFactory") )
    {
        KisFilterRegistry::instance()->add(new KisBrightnessContrastFilter());
        KisFilterRegistry::instance()->add(new KisAutoContrast());
        //KisFilterRegistry::instance()->add(new KisGammaCorrectionFilter());
        //KisFilterRegistry::instance()->add(new KisPerChannelFilter());
        KisFilterRegistry::instance()->add(new KisDesaturateFilter());
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
    setProgressTotalSteps(rect.width() * rect.height() * 2);
    Q_INT32 pixelsProcessed = 0;

    KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
    KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);

    // Number of channels in this device except alpha
     Q_INT32 depth = src -> colorSpace() -> nColorChannels();


    // initialize
    Q_UINT8* maxvalues = new Q_UINT8[depth];
    Q_UINT8* minvalues = new Q_UINT8[depth];
    memset(maxvalues, 0, depth * sizeof(Q_UINT8));
    memset(minvalues, OPACITY_OPAQUE, depth * sizeof(Q_UINT8));
    
    Q_UINT8** lut = new Q_UINT8*[depth];

    for (int i = 0; i < depth; i++) {
        lut[i] = new Q_UINT8[Q_UINT8_MAX+1];
        memset(lut[i], 0, (Q_UINT8_MAX+1) * sizeof(Q_UINT8));
    }

    while (!srcIt.isDone() && !cancelRequested())
    {
        if (srcIt.isSelected()) {
            Q_UINT8 opacity;

            QColor color;
            src -> colorSpace() -> toQColor(srcIt.rawData(), &color, &opacity);

            // skip non-opaque pixels
            if (src -> colorSpace() -> hasAlpha() && opacity != OPACITY_OPAQUE) {
                ++srcIt;
                continue;
            }

            for (int i = 0; i < depth; i++) {
		// XXX: Move to colorspace -- not independent
                Q_UINT8 index = srcIt.rawData()[i];
                if( index > maxvalues[i])
                    maxvalues[i] = index;
                if( index < minvalues[i])
                    minvalues[i] = index;
            }
        }
        ++srcIt;

        pixelsProcessed++;
        setProgress(pixelsProcessed);
    }

    if (cancelRequested()) {
        setProgressDone();
        return;
    }
    
    // build the LUT
    for (int i = 0; i < depth; i++) {
        Q_UINT8 diff = maxvalues[i] - minvalues[i];
        if (diff != 0) {
            for (int j = minvalues[i]; j <= maxvalues[i]; j++) {
                lut[i][j] = Q_UINT8_MAX * (j - minvalues[i]) / diff;
            }
        } else {
            lut[i][minvalues[i]] = minvalues[i];
        }
    }

    // apply

    srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(),rect.height(), true);

    while (!srcIt.isDone()  && !cancelRequested()) {
        if (srcIt.isSelected()) {
            Q_UINT8* dstData = dstIt.rawData();
            Q_UINT8* srcData = srcIt.rawData();

            // Iterate through all channels except alpha
            for (int i = 0; i < depth; ++i) {
                dstData[i] = lut[i][srcData[i]];
            }
        }
        ++dstIt;
        ++srcIt;

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

KisDesaturateFilter::KisDesaturateFilter()
    : KisFilter(id(), "adjust", "&Desaturate")
{
}

//XXX: This filter should write to dst, too!
void KisDesaturateFilter::process(KisPaintDeviceImplSP src, KisPaintDeviceImplSP dst, KisFilterConfiguration* /*config*/, const QRect& rect)
{
    KisColorAdjustment *adj = src->colorSpace()->createDesaturateAdjustment();

    KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
    KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);

    setProgressTotalSteps(rect.width() * rect.height());
    Q_INT32 pixelsProcessed = 0;

    while( ! srcIt.isDone()  && !cancelRequested())
    {
        Q_UINT32 npix=0, maxpix = srcIt.nConseqPixels();
        Q_UINT8 selectedness = srcIt.selectedness();
        // The idea here is to handle stretches of completely selected and completely unselected pixels.
        // Partially selected pixels are handled on pixel at a time.
        switch(selectedness)
        {
            case MIN_SELECTED:
                while(srcIt.selectedness()==MIN_SELECTED && maxpix)
                {
                    --maxpix;
                    ++srcIt;
                    ++npix;
                }
                dstIt+=npix;
                pixelsProcessed += npix;
                break;

            case MAX_SELECTED:
            {
                const Q_UINT8 *firstPixel = srcIt.oldRawData();
                while(srcIt.selectedness()==MAX_SELECTED && maxpix)
                {
                    --maxpix;
                    ++srcIt;
                    ++npix;
                }
                // desaturate
                src->colorSpace()->applyAdjustment(firstPixel, dstIt.rawData(), adj, npix);
                pixelsProcessed += npix;
                dstIt += npix;
                break;
            }

            default:
                // adjust, but since it's partially selected we also only partially adjust
                src->colorSpace()->applyAdjustment(srcIt.oldRawData(), dstIt.rawData(), adj, 1);
                const Q_UINT8 *pixels[2] = {srcIt.oldRawData(), dstIt.rawData()};
                Q_UINT8 weights[2] = {MAX_SELECTED - selectedness, selectedness};
                src->colorSpace()->mixColors(pixels, weights, 1, dstIt.rawData());
                ++srcIt;
                ++dstIt;
                pixelsProcessed++;
                break;
        }
        setProgress(pixelsProcessed);
    }

    setProgressDone();
}
