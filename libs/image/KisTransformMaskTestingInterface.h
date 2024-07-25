/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISTRANSFORMMASKTESTINGINTERFACE_H
#define KISTRANSFORMMASKTESTINGINTERFACE_H

#include <QMutex>
#include "kritaimage_export.h"

class KRITAIMAGE_EXPORT KisTransformMaskTestingInterface
{
public:
    virtual ~KisTransformMaskTestingInterface();

    virtual void notifyForceUpdateTimedNode() = 0;
    virtual void notifyThreadSafeForceStaticImageUpdate() = 0;
    virtual void notifySlotDelayedStaticUpdate() = 0;
    virtual void notifyDecorateRectTriggeredStaticImageUpdate() = 0;

    virtual void notifyRecalculateStaticImage() = 0;
};

#endif // KISTRANSFORMMASKTESTINGINTERFACE_H
