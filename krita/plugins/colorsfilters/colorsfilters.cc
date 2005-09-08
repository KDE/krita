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
#include <kis_abstract_colorspace.h>

#include "colorsfilters.h"
#include "kis_brightness_contrast_filter.h"

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
        KisFilterRegistry::instance()->add(new KisGammaCorrectionFilter());
        KisFilterRegistry::instance()->add(new KisColorAdjustmentFilter());
        KisFilterRegistry::instance()->add(new KisDesaturateFilter());
    }
}

ColorsFilters::~ColorsFilters()
{
}

//==================================================================

KisColorAdjustmentFilter::KisColorAdjustmentFilter() :
    KisIntegerPerChannelFilter(id(), "adjust", "&Color Adjustment...", -255, 255, 0)
{
}

/**
 * XXX: This filter should write to dst, too!
 */
void KisColorAdjustmentFilter::process(KisPaintDeviceImplSP src, KisPaintDeviceImplSP dst, KisFilterConfiguration* config, const QRect& rect)
{
    KisIntegerPerChannelFilterConfiguration* configPC = (KisIntegerPerChannelFilterConfiguration*) config;
    KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
    KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);
    Q_INT32 depth = src->nChannels() - 1;

    setProgressTotalSteps(rect.width() * rect.height());
    Q_INT32 pixelsProcessed = 0;

    while( ! srcIt.isDone() && !cancelRequested())
    {
        if (srcIt.isSelected()) {
            KisPixelRO data = srcIt.oldPixel();
            KisPixel dstData = dstIt.pixel();
            for( int i = 0; i < depth; i++)
            {
		// XXX: Move to colorspace -- not independent
                KisQuantum d = srcIt[ configPC->channel( i ) ];
                Q_INT32 s = configPC->valueFor( i );
                if( d < -s  ) dstData[ configPC->channel( i ) ] = 0;
                else if( d > QUANTUM_MAX - s) dstData[ configPC->channel( i ) ] = QUANTUM_MAX;
                else dstData[ configPC->channel( i ) ] = d + s;
            }
        }
        ++dstIt;
        ++srcIt;

        pixelsProcessed++;
        setProgress(pixelsProcessed);
    }

    setProgressDone();
}


//==================================================================

KisGammaCorrectionFilter::KisGammaCorrectionFilter()
    : KisDoublePerChannelFilter(id(), "adjust", "&Gamma Correction...", 0.1, 6.0, 1.0)
{
}

// XXX: This filter should write to dst, too!
void KisGammaCorrectionFilter::process(KisPaintDeviceImplSP src, KisPaintDeviceImplSP dst, KisFilterConfiguration* config, const QRect& rect)
{
    KisDoublePerChannelFilterConfiguration* configPC = (KisDoublePerChannelFilterConfiguration*) config;
    KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
    KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);
    Q_INT32 depth = src->nChannels() - 1;

    setProgressTotalSteps(rect.width() * rect.height());
    Q_INT32 pixelsProcessed = 0;

    while( ! srcIt.isDone() && !cancelRequested())
    {
        if (srcIt.isSelected()) {
            for( int i = 0; i < depth; i++)
            {
		// XXX: Move to colorspace -- not independent
                QUANTUM sd = srcIt.oldRawData()[ configPC->channel( i ) ];
                KisQuantum dd = dstIt[ configPC->channel( i ) ];
                dd = (QUANTUM)( QUANTUM_MAX * pow( ((float)sd)/QUANTUM_MAX, 1.0 / configPC->valueFor( i ) ) );
            }
        }
        ++dstIt;
        ++srcIt;

        pixelsProcessed++;
        setProgress(pixelsProcessed);
    }

    setProgressDone();
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
    QUANTUM* maxvalues = new QUANTUM[depth];
    QUANTUM* minvalues = new QUANTUM[depth];
    memset(maxvalues, 0, depth * sizeof(QUANTUM));
    memset(minvalues, OPACITY_OPAQUE, depth * sizeof(QUANTUM));
    
    QUANTUM** lut = new QUANTUM*[depth];

    for (int i = 0; i < depth; i++) {
        lut[i] = new QUANTUM[QUANTUM_MAX+1];
        memset(lut[i], 0, (QUANTUM_MAX+1) * sizeof(QUANTUM));
    }

    while (!srcIt.isDone() && !cancelRequested())
    {
        if (srcIt.isSelected()) {
            QUANTUM opacity;

            QColor color;
            src -> colorSpace() -> toQColor(srcIt.rawData(), &color, &opacity);

            // skip non-opaque pixels
            if (src -> colorSpace() -> hasAlpha() && opacity != OPACITY_OPAQUE) {
                ++srcIt;
                continue;
            }

            for (int i = 0; i < depth; i++) {
		// XXX: Move to colorspace -- not independent
                QUANTUM index = srcIt.rawData()[i];
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

    srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(),rect.height(), true);

    while (!srcIt.isDone()  && !cancelRequested()) {
        if (srcIt.isSelected()) {
            QUANTUM* dstData = dstIt.rawData();
            QUANTUM* srcData = srcIt.rawData();

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
    KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
    KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);
    
    KisAbstractColorSpace * scs = src -> colorSpace();
    KisProfileSP profile = src -> profile();

    setProgressTotalSteps(rect.width() * rect.height());
    Q_INT32 pixelsProcessed = 0;

    while( ! srcIt.isDone()  && !cancelRequested())
    {
        if (srcIt.isSelected()) {
            QColor c;

            const Q_UINT8 * srcData = srcIt.oldRawData();
            // Try to be colorspace independent
            scs -> toQColor(srcData, &c, profile);
            ;
            /* I thought of using the HSV model, but GIMP seems to use
                HSL for desaturating. Better use the gimp model for now
                (HSV produces a lighter image than HSL) */

//             Q_INT32 lightness = ( QMAX(QMAX(c.red(), c.green()), c.blue())
//                     + QMIN(QMIN(c.red(), c.green()), c.blue()) ) / 2;

            // XXX: BSAR: Doesn't this doe the same but better?
		// XXX: Move to colorspace -- not independent
            Q_INT32 lightness = qGray(c.red(), c.green(), c.blue());
            scs -> fromQColor(QColor(lightness, lightness, lightness), dstIt.rawData(), profile);
        }
        ++srcIt;
        ++dstIt;

        pixelsProcessed++;
        setProgress(pixelsProcessed);
    }
    setProgressDone();
}
