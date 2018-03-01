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
#include <KoUpdater.h>

#include <kis_processing_information.h>
#include <kis_paint_device.h>
#include <kis_selection.h>

#ifndef NDEBUG
#include <QTime>
#endif
#include <KisSequentialIteratorProgress.h>
#include "kis_color_transformation_configuration.h"

KisColorTransformationFilter::KisColorTransformationFilter(const KoID& id, const KoID & category, const QString & entry) : KisFilter(id, category, entry)
{
    setSupportsLevelOfDetail(true);
}

KisColorTransformationFilter::~KisColorTransformationFilter()
{
}

void KisColorTransformationFilter::processImpl(KisPaintDeviceSP device,
                                               const QRect& applyRect,
                                               const KisFilterConfigurationSP config,
                                               KoUpdater* progressUpdater
                                               ) const
{
    Q_ASSERT(!device.isNull());

    const KoColorSpace * cs = device->colorSpace();
    KoColorTransformation * colorTransformation = 0;
    // Ew, casting
    KisColorTransformationConfigurationSP colorTransformationConfiguration(dynamic_cast<KisColorTransformationConfiguration*>(const_cast<KisFilterConfiguration*>(config.data())));
    if (colorTransformationConfiguration) {
        colorTransformation = colorTransformationConfiguration->colorTransformation(cs, this);
    }
    else {
        colorTransformation = createTransformation(cs, config);
    }
    if (!colorTransformation) return;

    KisSequentialIteratorProgress it(device, applyRect, progressUpdater);

    int conseq = it.nConseqPixels();
    while (it.nextPixels(conseq)) {
        conseq = it.nConseqPixels();
        colorTransformation->transform(it.oldRawData(), it.rawData(), conseq);
    }

    if (!colorTransformationConfiguration) {
        delete colorTransformation;
    }

}

KisFilterConfigurationSP  KisColorTransformationFilter::factoryConfiguration() const
{
    return new KisColorTransformationConfiguration(id(), 0);
}
