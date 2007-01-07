/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org
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
#ifndef KIS_PROJECTION
#define KIS_PROJECTION

#include <QObject>

#include "kis_types.h"

class QRegion;
class QRect;
class QVariant;

/**
   KisProjection is responsible for keeping the projection of the
   image's layer stack up to date. Krita's redisplay works as follows:

   * Any user action dirties a region (set of rects) on a layer.
   * The layer notifies the group layer it belongs to that it is dirty
   * This percolates up to the root layer
   * The root layer informs the projection that it has a new dirty
   region.
   * The projection aggregates the region with its dirty region.
   * The projection recomposition thread takes a rect from the dirty
   region.
   * The thread composites the rect
   * The thread emits a cross-thread signal that a certain rect has
   been recomposited
   * The canvas widget catches this signal and schedules an update
     which Qt aggregates into a paint event.


   XXX: Add a way to wait for recomposition for a particular rect to
        completed?
 */
class KisProjection : public QObject {

    Q_OBJECT

public:

    KisProjection( KisImageSP image, KisGroupLayerSP rootLayer );
    virtual ~KisProjection();

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

    /**
       Replace the current rootlayer with the specified rootlayer
    */
    void setRootLayer( KisGroupLayerSP rootLayer );

    /**
     * @return true if there is no recomposition going on or queued
     *              for the specified rect
     */
    bool upToDate(const QRect & rect);

    /**
     * @return true if there is no recomposition going on or queued
     *              for the specified region
     */
    bool upToDate(const QRegion & region);

signals:

    void sigProjectionUpdated( const QRect & );

private slots:

    void slotAddDirtyRegion( const QRegion & region );
    void slotAddDirtyRect( const QRect & rect );

    void slotTriggered( const QVariant & params );
    void slotUpdateUi( const QVariant & params );

private:

    /// Breaks up big rects in separate parts
    void scheduleRect( const QRect & rc );

private:

    class Private;
    Private * m_d;

};

#endif
