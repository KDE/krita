/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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
#ifndef KIS_PROJECTION_UPDATE_STRATEGY_H
#define KIS_PROJECTION_UPDATE_STRATEGY_H

#include "kis_types.h"

class QRect;
class QRegion;


/**
 * The projection update strategy is a base
 * interface for classes that implement projection update
 * strategies, such as the TopDown or BottomUp projection
 * update strategy.
 */
class KisProjectionUpdateStrategy
{

public:

    KisProjectionUpdateStrategy() {
    }

    virtual ~KisProjectionUpdateStrategy() {
    }

    /**
     * Nodes call this to inform the update strategy that a particular
     * area is dirty. The update strategy is responsible for the rest.
     * @param rc the dirty rect
     * @param filthyNode the node that set dirty
     */
    virtual void setDirty(const QRect & rc) = 0;


    /**
     * If a KisImage is set on the update strategy, the image will
     * be notified if the update is done.
     */
    virtual void setImage(KisImageWSP image) = 0;

    /**
       Lock the projection: we will add new rects to the dirty region,
       but not composite until unlocked
    */
    virtual void lock() = 0;

    /**
       Unlock the projection. We will iterate through the accumulated
       dirty region and emit projectionUpdated signals
    */
    virtual void unlock() = 0;

protected:

    friend class KisGroupLayer;

    /**
     * The way group layers are updated is very much dependent on the update strategy
     * because group layers have to take into account that adjustment layers can
     * be used to cache part of a composition stack.
     */
    virtual KisPaintDeviceSP updateGroupLayerProjection(const QRect & rc, KisPaintDeviceSP projection) = 0;
};


#endif
