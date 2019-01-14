/*
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

    KisFilterConfigurationSP factoryConfiguration() const override;
};

#endif
