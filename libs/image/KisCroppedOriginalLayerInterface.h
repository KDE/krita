/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCROPPEDORIGINALLAYERINTERFACE_H
#define KISCROPPEDORIGINALLAYERINTERFACE_H

#include "kritaimage_export.h"

class KRITAIMAGE_EXPORT KisCroppedOriginalLayerInterface
{
public:
    virtual ~KisCroppedOriginalLayerInterface();


    /**
     * Force regeneration of the hidden part of original() device
     * (the one outside image bounds). After regeneration is completed,
     * the layer will emit dirty signals itself, so no manual forced
     * update is needed.
     */
    virtual void forceUpdateHiddenAreaOnOriginal() = 0;
};

#endif // KISCROPPEDORIGINALLAYERINTERFACE_H
