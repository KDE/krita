/*
 * Copyright (c) 2004, 2009 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_color_transformation_filter.h"

#include <KoColorTransformation.h>
#include <KoProgressUpdater.h>

#include <kis_iterators_pixel.h>
#include <kis_processing_information.h>
#include <kis_paint_device.h>
#include <kis_selection.h>

#ifndef NDEBUG
#include <QTime>
#endif

KisColorTransformationFilter::KisColorTransformationFilter(const KoID& id, const KoID & category, const QString & entry) : KisFilter(id, category, entry)
{
}

KisColorTransformationFilter::~KisColorTransformationFilter()
{
}

void KisColorTransformationFilter::process(KisConstProcessingInformation srcInfo,
                        KisProcessingInformation dstInfo,
                        const QSize& size,
                        const KisFilterConfiguration* config,
                        KoUpdater* progressUpdater
                    ) const
{
    const KisPaintDeviceSP src = srcInfo.paintDevice();
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();
    QPoint srcTopLeft = srcInfo.topLeft();
    Q_UNUSED(config);
    Q_ASSERT(!src.isNull());
    Q_ASSERT(!dst.isNull());

    if (progressUpdater) {
        progressUpdater->setRange(0, size.height());
    }

    const KoColorSpace * cs = src->colorSpace();

    KoColorTransformation* inverter = createTransformation(cs, config);
    if(!inverter) return;

#ifndef NDEBUG
    QTime t;
    t.start();
#endif

#if 0
// Method one: iterate and check every pixel for selectedness. It is
// only slightly slower than the next method and the code is very
// clear. Note that using nextRow() instead of recreating the iterators
// for every row makes a huge difference.

    KisHLineConstIteratorPixel srcIt = src->createHLineConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width(), srcInfo.selection());
    KisHLineIteratorPixel dstIt = dst->createHLineIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), dstInfo.selection());

    for (int row = 0; row < size.height() && !(progressUpdater && progressUpdater->interrupted()); ++row) {
        while (!srcIt.isDone() && !(progressUpdater && progressUpdater->interrupted())) {
            if (srcIt.isSelected()) {
                inverter->transform(srcIt.oldRawData(), dstIt.rawData(), 1);
            }
            ++srcIt;
            ++dstIt;
        }
        srcIt.nextRow();
        dstIt.nextRow();
        if (progressUpdater) progressUpdater->setValue(row);
    }
#ifndef NDEBUG
    dbgPlugins << "Per-pixel isSelected():" << t.elapsed() << " ms";
#endif




#ifndef NDEBUG
    t.restart();
#endif

#endif

    bool hasSelection = srcInfo.selection();

// Method two: check the number of consecutive pixels the iterators
// points to. Take as large stretches of unselected pixels as possible
// and pass those to the color space transform object in one go. It's
// quite a bit speedier, with the speed improvement more noticeable
// the less happens inside the color transformation.

    KisHLineConstIteratorPixel srcIt = src->createHLineConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width());
    KisHLineIteratorPixel dstIt = dst->createHLineIterator(dstTopLeft.x(), dstTopLeft.y(), size.width());

    for (int row = 0; row < size.height(); ++row) {
        while (! srcIt.isDone()) {
            int srcItConseq = srcIt.nConseqHPixels();
            int dstItConseq = dstIt.nConseqHPixels();
            int conseqPixels = qMin(srcItConseq, dstItConseq);

            int pixels = 0;

            if (hasSelection) {
                // Get largest horizontal row of selected pixels

                while (srcIt.isSelected() && pixels < conseqPixels) {
                    ++pixels;
                }
                inverter->transform(srcIt.oldRawData(), dstIt.rawData(), pixels);

                // We apparently found a non-selected pixels, or the row
                // was done; get the stretch of non-selected pixels
                while (!srcIt.isSelected() && pixels < conseqPixels) {
                    ++ pixels;
                }
            } else {
                pixels = conseqPixels;
                inverter->transform(srcIt.oldRawData(), dstIt.rawData(), pixels);
            }

            // Update progress
            srcIt += pixels;
            dstIt += pixels;
        }
        srcIt.nextRow();
        dstIt.nextRow();
    }
#ifndef NDEBUG
    dbgPlugins << "Consecutive pixels:" << t.elapsed() << " ms";
#endif

    delete inverter;
    //if(progressUpdater) progressUpdater->setProgress( 100 );

    // Two inversions make no inversion? No -- because we're reading
    // from the oldData in both loops without saving the transaction
    // in between, both inversion loops invert the original image.
}
