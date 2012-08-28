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

#include <kis_processing_information.h>
#include <kis_paint_device.h>
#include <kis_selection.h>

#ifndef NDEBUG
#include <QTime>
#endif
#include <kis_iterator_ng.h>

typedef QPointer<KoUpdater> KoUpdaterPtr;

KisColorTransformationFilter::KisColorTransformationFilter(const KoID& id, const KoID & category, const QString & entry) : KisFilter(id, category, entry)
{
}

KisColorTransformationFilter::~KisColorTransformationFilter()
{
}

void KisColorTransformationFilter::process(KisPaintDeviceSP device, 
                                           const QRect& applyRect,
                                           const KisFilterConfiguration* config,
                                           KoUpdater* progressUpdater
                                           ) const
{
    Q_ASSERT(!device.isNull());

    if (progressUpdater) {
        progressUpdater->setRange(0, applyRect.height() * applyRect.width());
    }

    const KoColorSpace * cs = device->colorSpace();
    KoColorTransformation* colorTransformation = createTransformation(cs, config);
    if (!colorTransformation) return;

    KisRectIteratorSP it = device->createRectIteratorNG(applyRect);
    int p = 0;
    int conseq;
    do {
    
        conseq = it->nConseqPixels();

        colorTransformation->transform(it->oldRawData(), it->rawData(), conseq);

        if (progressUpdater) progressUpdater->setValue(p += conseq);

    } while(it->nextPixels(conseq));
    delete colorTransformation;

}
