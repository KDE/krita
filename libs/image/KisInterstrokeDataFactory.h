/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISINTERSTROKEDATAFACTORY_H
#define KISINTERSTROKEDATAFACTORY_H

#include "kritaimage_export.h"
#include "kis_types.h"

class KisInterstrokeData;

/**
 * A factory class for creation KisInterstrokeData by paint tools
 *
 * The main purpose for the factory is to check if the currently
 * attached interstroke data is compatible with the current tool
 * and replace it with the new one if needed.
 *
 * \see KisInterstrokeData
 */
class KRITAIMAGE_EXPORT KisInterstrokeDataFactory
{
public:
    virtual ~KisInterstrokeDataFactory();

    virtual bool isCompatible(KisInterstrokeData *data) = 0;
    virtual KisInterstrokeData* create(KisPaintDeviceSP device) = 0;
};

#endif // KISINTERSTROKEDATAFACTORY_H
