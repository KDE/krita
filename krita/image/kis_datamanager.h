/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_DATAMANAGER_H_
#define KIS_DATAMANAGER_H_

#include <qglobal.h>
#include <QRect>

class KoStore;


// Change the following two lines to switch (at compiletime) to another datamanager
#include "tiles/kis_tileddatamanager.h"
#define ACTUAL_DATAMGR KisTiledDataManager

/**
 * KisDataManager defines the interface that modules responsible for
 * storing and retrieving data must inmplement. Data modules, like
 * the tile manager, are responsible for:
 *
 * * Storing undo/redo data
 * * Offering ordererd and unordered iterators over rects of pixels
 * * (eventually) efficiently loading and saving data in a format
 * that may allow deferred loading.
 *
 * A datamanager knows nothing about the type of pixel data except
 * how many quint8's a single pixel takes.
 */
class KisDataManager : public ACTUAL_DATAMGR {

public:
    KisDataManager(quint32 pixelSize, const quint8 *defPixel) : ACTUAL_DATAMGR(pixelSize, defPixel) {}
    KisDataManager(const KisDataManager& dm) : ACTUAL_DATAMGR(dm) { }

public:
    /**
     * Sets the default pixel. Note that this might change every occurrance, and it might not, but new data
     * well be initialised with this pixel
     */
    inline void setDefaultPixel(const quint8 *defPixel) { return ACTUAL_DATAMGR::setDefaultPixel(defPixel); }

    /**
     * Gets the default pixel.
     */
     inline const quint8 *defaultPixel() const { return ACTUAL_DATAMGR::defaultPixel(); }

    /**
     * Reguests a memento from the data manager. There is only one memento active
     * at any given moment for a given paint device and all and any
     * write actions on the datamanger builds undo data into this memento
     * necessary to rollback the transaction.
     */
    inline KisMementoSP getMemento() { return ACTUAL_DATAMGR::getMemento(); }

    /**
     * Restores the image data to the state at the time of the getMemento() call.
     *
     * Note that rollback should be performed with mementos in the reverse order of
     * their creation, as mementos only store incremental changes
     */
    inline void rollback(KisMementoSP memento) { ACTUAL_DATAMGR::rollback(memento); }

    /**
     * Restores the image data to the state at the time of the rollback call of the memento.
     *
     * Note that rollforward must only be called when an rollback have previously been performed, and
     * no intermittent actions have been performed (though it's ok to rollback other mementos and
     * roll them forward again)
     */
    inline void rollforward(KisMementoSP memento) { ACTUAL_DATAMGR::rollforward(memento); }

public:
    /**
     * Reads and writes the tiles from/onto a KoStore (wich is simply a file within a zip file)
     *
     */
    inline bool write(KoStore *store) { return ACTUAL_DATAMGR::write(store); }
    inline bool read(KoStore *store) { return ACTUAL_DATAMGR::read(store); }

public:

    /**
     * Returns the number of bytes a pixel takes
     */
    inline quint32 pixelSize() { return ACTUAL_DATAMGR::pixelSize(); }

    /**
     * Return the extent of the data in x,y,w,h.
     */
    inline void extent(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const
                         { return ACTUAL_DATAMGR::extent(x, y, w, h); }

     QRect extent() const { return ACTUAL_DATAMGR::extent(); }


public:

    /**
      * Crop or extend the data to x, y, w, h.
      */
    inline void setExtent(qint32 x, qint32 y, qint32 w, qint32 h)
                         { return ACTUAL_DATAMGR::setExtent(x, y, w, h); }

    inline void setExtent(const QRect & rect) { setExtent(rect.x(), rect.y(), rect.width(), rect.height()); }

public:

    /**
     * Clear the specified rect to the specified value.
     */
    inline void clear(qint32 x, qint32 y,
           qint32 w, qint32 h,
           quint8 def) { ACTUAL_DATAMGR::clear(x, y, w, h, def); }

    /**
     * Clear the specified rect to the specified pixel value.
     */
    inline void clear(qint32 x, qint32 y,
           qint32 w, qint32 h,
           const quint8 * def) { ACTUAL_DATAMGR::clear(x, y, w, h, def); }

    /**
     * Clear all back to default values.
     */
    inline void clear() { ACTUAL_DATAMGR::clear(); }


public:

    /**
     * Copy the specified rect from the specified data into this
     * data.
     */
    inline void paste(KisDataManagerSP data,  qint32 sx, qint32 sy, qint32 dx, qint32 dy,
           qint32 w, qint32 h) { ACTUAL_DATAMGR::paste(data, sx, sy, dx, dy, w, h); }

public:
    /**
     * Get a read-only pointer to the specified pixel.
     */
      inline const quint8* pixel(qint32 x, qint32 y)
          { return ACTUAL_DATAMGR::pixel(x, y); }

    /**
     * Get a read-write pointer to the specified pixel.
     */
      inline quint8* writablePixel(qint32 x, qint32 y)
          { return ACTUAL_DATAMGR::writablePixel(x, y); }

    /**
     * Write the specified data to x, y. There is no checking on pixelSize!
     */
    inline void setPixel(qint32 x, qint32 y, const quint8 * data)
        { ACTUAL_DATAMGR::setPixel(x, y, data);}


     /**
      * Copy the bytes in the specified rect to a chunk of memory.
      * The pixelSize in bytes is w * h * pixelSize. XXX: Better
      * use QValueVector?
      */
     inline void readBytes(quint8 * data,
               qint32 x, qint32 y,
               qint32 w, qint32 h)
        { ACTUAL_DATAMGR::readBytes(data, x, y, w, h);}

     /**
     * Copy the bytes to the specified rect. w * h * pixelSize bytes will be read, whether
     * the caller prepared them or not. XXX: Better use QValueVector?
      */
     inline void writeBytes(const quint8 * data,
             qint32 x, qint32 y,
             qint32 w, qint32 h)
        {ACTUAL_DATAMGR::writeBytes( data, x, y, w, h); }

    // Get the number of contiguous columns starting at x, valid for all values
    // of y between minY and maxY.
    inline qint32 numContiguousColumns(qint32 x, qint32 minY, qint32 maxY)
        { return ACTUAL_DATAMGR::numContiguousColumns(x, minY, maxY); }


    // Get the number of contiguous rows starting at y, valid for all values
    // of x between minX and maxX.
    inline qint32 numContiguousRows(qint32 y, qint32 minX, qint32 maxX)
        { return ACTUAL_DATAMGR::numContiguousRows(y, minX, maxX); }


    // Get the row stride at pixel (x, y). This is the number of bytes to add to a
    // pointer to pixel (x, y) to access (x, y + 1).
    inline qint32 rowStride(qint32 x, qint32 y)
        { return ACTUAL_DATAMGR::rowStride(x, y); }

protected:
    friend class KisRectIterator;
    friend class KisHLineIterator;
    friend class KisVLineIterator;
};


#endif // KIS_DATAMANAGER_H_

