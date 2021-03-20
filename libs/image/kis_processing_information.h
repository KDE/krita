/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_PROCESSING_INFORMATION_H_
#define _KIS_PROCESSING_INFORMATION_H_

#include "kis_types.h"

#include "kritaimage_export.h"


/**
 * This class is used in KisFilter to contain information needed to apply a filter
 * on a paint device.
 * This one have only a const paint device and holds information about the source.
 */
class KRITAIMAGE_EXPORT KisConstProcessingInformation
{
public:
    KisConstProcessingInformation(const KisPaintDeviceSP device, const QPoint& topLeft, const KisSelectionSP selection);
    KisConstProcessingInformation(const KisConstProcessingInformation& _rhs);
    KisConstProcessingInformation& operator=(const KisConstProcessingInformation& _rhs);
    ~KisConstProcessingInformation();
    /**
     * @return the paint device
     */
    const KisPaintDeviceSP paintDevice() const;

    /**
     * @return the active selection
     */
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
class KRITAIMAGE_EXPORT KisProcessingInformation : public KisConstProcessingInformation
{
public:
    KisProcessingInformation(KisPaintDeviceSP device, const QPoint& topLeft, const KisSelectionSP selection);
    KisProcessingInformation(const KisProcessingInformation& _rhs);
    KisProcessingInformation& operator=(const KisProcessingInformation& _rhs);
    ~KisProcessingInformation();
    /**
     * @return the paint device
     */
    KisPaintDeviceSP paintDevice();
private:
    struct Private;
    Private* const d;
};

#endif
