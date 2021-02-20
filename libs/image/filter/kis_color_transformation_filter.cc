/*
 * SPDX-FileCopyrightText: 2004, 2009 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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

KisFilterConfigurationSP  KisColorTransformationFilter::factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    return new KisColorTransformationConfiguration(id(), 0, resourcesInterface);
}
