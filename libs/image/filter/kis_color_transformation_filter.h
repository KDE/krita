/*
 * SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KIS_COLOR_TRANSFORMATION_FILTER_H_
#define _KIS_COLOR_TRANSFORMATION_FILTER_H_

#include "kis_filter.h"
#include "kritaimage_export.h"

/**
 * This is a base class for filters that implement a filter for
 * KoColorTransformationFactory based filters.
 */
class KRITAIMAGE_EXPORT KisColorTransformationFilter : public KisFilter
{
public:
    KisColorTransformationFilter(const KoID& id, const KoID & category, const QString & entry);
    ~KisColorTransformationFilter() override;
    void processImpl(KisPaintDeviceSP device,
                             const QRect& applyRect,
                             const KisFilterConfigurationSP config,
                             KoUpdater* progressUpdater
                             ) const override;
    /**
     * Create the color transformation that will be applied on the device.
     */
    virtual KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const = 0;

    KisFilterConfigurationSP factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
};

#endif
