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
#ifndef KIS_BOTTOM_UP_UPDATESTRATEGY_H
#define KIS_BOTTOM_UP_UPDATESTRATEGY_H

#include <QObject>

#include "krita_export.h"

#include "kis_projection_update_strategy.h"
#include "kis_types.h"

//#define USE_PAINTERPATH
class QRect;
class QRegion;

/**
 * The bottom-up projection update strategy filters the dirty marking 
 * down to the root node, and starts a projection thread from there,
 * marking nodes clean as it goes
 */
class KRITAIMAGE_EXPORT KisBottomUpUpdateStrategy : public QObject, public KisProjectionUpdateStrategy
{
    Q_OBJECT

public:

    KisBottomUpUpdateStrategy( KisNodeSP node );

    ~KisBottomUpUpdateStrategy();

public:

    void setDirty( const QRect & rc );

    virtual void setImage(KisImageSP image);
    
    /**
       Lock the projection: we will add new rects to the dirty region,
       but not composite until unlocked
    */
    void lock();

    /**
       Unlock the projection. We will iterate through the accumulated
       dirty region and emit projectionUpdated signals
    */
    void unlock();
    
public: // Extensions for this strategy
    /**
     * @return true if any part of this layer has been marked dirty
     */
    bool isDirty() const;

    /**
     *  @return true if the given rect overlaps with the dirty region
     *  of this node
     */
    bool isDirty( const QRect & rect ) const;

    /**
     * Mark the specified area as clean
     */
    void setClean( const QRect & rc );

    /**
     * Mark the whole layer as clean
     */
    void setClean();

    /**
     * @return the region from the given rect that is dirty.
     */
    QRegion dirtyRegion( const QRect & rc );

signals:

    void rectDirtied( const QRect & rc );

protected:

    virtual KisPaintDeviceSP updateGroupLayerProjection( const QRect & rc, KisPaintDeviceSP projection );

private:

    class Private;
    Private * const m_d;

};

#endif
