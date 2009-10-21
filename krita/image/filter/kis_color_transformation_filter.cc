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
#include <KoUpdater.h>

#include <kis_iterators_pixel.h>
#include <kis_processing_information.h>
#include <kis_paint_device.h>
#include <kis_selection.h>

#ifndef NDEBUG
#include <QTime>
#endif

typedef QPointer<KoUpdater> KoUpdaterPtr;

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
    KoColorTransformation* colorTransformation = createTransformation(cs, config);
    if (!colorTransformation) return;

    bool hasSelection = srcInfo.selection();
    KisHLineConstIteratorPixel srcIt = src->createHLineConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width(), srcInfo.selection());
    KisHLineIteratorPixel dstIt = dst->createHLineIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), dstInfo.selection());

    for (int row = 0; row < size.height(); ++row) {
        while (! srcIt.isDone()) {
            int srcItConseq = srcIt.nConseqHPixels();
            int dstItConseq = dstIt.nConseqHPixels();
            int conseqPixels = qMin(srcItConseq, dstItConseq);

            int pixels = 0;
            int pixelsSrc = 0;

            if (hasSelection) {
                // Get largest horizontal row of selected pixels
                const quint8* oldRawData = srcIt.oldRawData();
                while (srcIt.isSelected() && pixels < conseqPixels) {
                    ++pixels;
                    ++srcIt;
                    ++pixelsSrc;
                }
                colorTransformation->transform(oldRawData, dstIt.rawData(), pixels);

                // We apparently found a non-selected pixels, or the row
                // was done; get the stretch of non-selected pixels
                while (!srcIt.isSelected() && pixels < conseqPixels) {
                    ++ pixels;
                    ++srcIt;
                    ++pixelsSrc;
                }
            } else {
                pixels = conseqPixels;
                colorTransformation->transform(srcIt.oldRawData(), dstIt.rawData(), pixels);
            }

            // Update progress
            srcIt += (pixels - pixelsSrc);
            dstIt += pixels;
        }
        if (progressUpdater) progressUpdater->setValue(row);

        srcIt.nextRow();
        dstIt.nextRow();
    }
    delete colorTransformation;


}
