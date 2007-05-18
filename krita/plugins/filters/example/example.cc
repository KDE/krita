/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include <QPoint>
#include <QTime>

#include <klocale.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <KoColorTransformation.h>

#include <kis_types.h>
#include <kis_selection.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_filter_registry.h>
#include <kis_global.h>

// #include <kmessagebox.h>

#include "example.h"

typedef KGenericFactory<KritaExample> KritaExampleFactory;
K_EXPORT_COMPONENT_FACTORY( kritaexample, KritaExampleFactory( "krita" ) )

KritaExample::KritaExample(QObject *parent, const QStringList &)
        : KParts::Plugin(parent)
{
    setComponentData(KritaExampleFactory::componentData());


    if (parent->inherits("KisFilterRegistry")) {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(KisFilterSP(new KisFilterInvert()));
    }
}

KritaExample::~KritaExample()
{
}

KisFilterInvert::KisFilterInvert() : KisFilter(id(), "adjust", i18n("&Invert"))
{
}

void KisFilterInvert::process(const KisPaintDeviceSP src, const QPoint& srcTopLeft, KisPaintDeviceSP dst, const QPoint& dstTopLeft, const QSize& size, KisFilterConfiguration* /*config*/)
{
    Q_ASSERT(!src.isNull());
    Q_ASSERT(!dst.isNull());

    KisHLineIteratorPixel dstIt = dst->createHLineIterator(dstTopLeft.x(), dstTopLeft.y(), size.width());
    KisHLineConstIteratorPixel srcIt = src->createHLineConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width());

    int pixelsProcessed = 0;
    setProgressTotalSteps(size.width() * size.height());

    KoColorSpace * cs = src->colorSpace();

    KoColorTransformation* inverter = cs->createInvertTransformation();

// Method one: iterate and check every pixel for selectedness. It is
// only slightly slower than the next method and the code is very
// clear. Note that using nextRow() instead of recreating the iterators
// for every row makes a huge difference.
#if 0
    for ( int row = 0; row < size.height(); ++row ) {
        while ( !srcIt.isDone() ) {
            if ( srcIt.isSelected() ) {
                inverter->transform( srcIt.oldRawData(), dstIt.rawData(), 1);
            }
            ++srcIt;
            ++dstIt;

        }
        srcIt.nextRow();
        dstIt.nextRow();
    }


#endif

    bool hasSelection = src->hasSelection();

// Method two: check the number of consecutive pixels the iterators
// points to. Take as large stretches of unselected pixels as possible
// and pass those to the color space transform object in one go. The
// speed advantage in this case is slight: littlecms takes most of the
// time, not the iteration.
    for (int row = 0; row < size.height(); ++row) {
        while( ! srcIt.isDone() )
        {
            int srcItConseq = srcIt.nConseqHPixels();
            int dstItConseq = srcIt.nConseqHPixels();
            int conseqPixels = qMin( srcItConseq, dstItConseq );

            int pixels = 0;

            if ( hasSelection ) {
                // Get largest horizontal row of selected pixels

                while ( srcIt.isSelected() && pixels < conseqPixels ) {
                    ++pixels;
                }
                inverter->transform( srcIt.oldRawData(), dstIt.rawData(), pixels);

                // We apparently found a non-selected pixels, or the row
                // was done; get the stretch of non-selected pixels
                while ( !srcIt.isSelected() && pixels < conseqPixels ) {
                    ++ pixels;
                }
            }
            else {
                pixels = conseqPixels;
                inverter->transform( srcIt.oldRawData(), dstIt.rawData(), pixels );
            }

            // Update progress
            srcIt += pixels;
            dstIt += pixels;
        }
        srcIt.nextRow();
        dstIt.nextRow();
    }

// Method three (incomplete, the case where there's a selection still
// needs to be handled). It should be the fastest one because we cache
// whole scanlines. In fact, it isn't faster than method 2 at all.
#if 0
    quint32 w = size.width();
    quint32 h = size.height();
    quint32 length = w * src->pixelSize();

    // Allocate the buffers for the scanlines
    quint8 * srcPixels = new quint8[length];
    quint8 * selPixels = 0;
    if ( hasSelection )
         selPixels = new quint8[w * src->selection()->pixelSize()];
    quint8 * dstPixels = new quint8[length];

    for ( int row = 0; row < h; ++row ) {
        // Prepare the scanlines
        const_cast<KisPaintDevice*>( src.data() )->readBytes( srcPixels, dstTopLeft.x(), dstTopLeft.y() + row, w, 1 );
        memset( dstPixels, 0, length );

        // If there's a selection, iterate over the selection to find
        // the stretches of non selected pixels and selected pixels.
        if ( hasSelection ) {
            const_cast<KisSelection*>( src->selection().data() )->readBytes( selPixels, dstTopLeft.x(), dstTopLeft.y() + row, size.width(), 1 );
            // XXX: iterate over the selection bytes to determine
            // the selected stretches
        }
        else {
            inverter->transform( srcPixels, dstPixels, w );
        }
        dst->writeBytes( dstPixels, dstTopLeft.x(), dstTopLeft.y() + row, w, 1 );

    }
#endif

    delete inverter;
    setProgressDone(); // Must be called even if you don't really support progression
}
