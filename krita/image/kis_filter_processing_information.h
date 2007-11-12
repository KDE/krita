/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_FILTER_PROCESSING_INFORMATION_H_
#define _KIS_FILTER_PROCESSING_INFORMATION_H_

#include "kis_types.h"

#include "krita_export.h"


/**
 * This class is used in KisFilter to contain information needed to apply a filter
 * on a paint device.
 * This one have only a const paint device and holds information about the source.
 */
class KRITAIMAGE_EXPORT KisFilterConstantProcessingInformation {
    public:
        KisFilterConstantProcessingInformation(const KisPaintDeviceSP device, const QPoint& topLeft, const KisSelectionSP selection = 0);
        /**
         * @return the paint device
         */
        const KisPaintDeviceSP paintDevice() const;
        const KisSelectionSP selection() const;
        /**
         * @return the top left pixel that need to process
         */
        const QPoint& topLeft() const;
    private:
        struct Private;
        Private* const d;
};

/**
 * This class is used in KisFilter to contain information needed to apply a filter
 * on a paint device.
 * This one can have a non const paint device and holds information about the destination.
 */
class KRITAIMAGE_EXPORT KisFilterProcessingInformation : public KisFilterConstantProcessingInformation {
    public:
        KisFilterProcessingInformation(KisPaintDeviceSP device, const QPoint& topLeft, const KisSelectionSP selection = 0);
        /**
         * @return the paint device
         */
        KisPaintDeviceSP paintDevice();
    private:
        struct Private;
        Private* const d;
};

#endif
