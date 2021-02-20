/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_DATAMANAGER_H_
#define KIS_DATAMANAGER_H_

#include <QtGlobal>

class QRect;
class KisPaintDeviceWriter;
class QIODevice;

#include <tiles3/kis_tiled_data_manager.h>
#include <tiles3/kis_memento.h>

#define ACTUAL_DATAMGR KisTiledDataManager

/**
 * KisDataManager defines the interface that modules responsible for
 * storing and retrieving data must inmplement. Data modules, like
 * the tile manager, are responsible for:
 *
 * * Storing undo/redo data
 * * Offering ordered and unordered iterators over rects of pixels
 * * (eventually) efficiently loading and saving data in a format
 * that may allow deferred loading.
 *
 * A datamanager knows nothing about the type of pixel data except
 * how many quint8's a single pixel takes.
 */
class KisDataManager : public ACTUAL_DATAMGR
{

public:

    /**
     * Create a new datamanger where every pixel will take pixelSize bytes and will be initialized
     * by default with defPixel. The value of defPixel is copied, the caller still owns the pointer.
     *
     * Note that if pixelSize > size of the defPixel array, we will happily read beyond the
     * defPixel array.
     */
KisDataManager(quint32 pixelSize, const quint8 *defPixel) : ACTUAL_DATAMGR(pixelSize, defPixel) {}
    KisDataManager(const KisDataManager& dm) : ACTUAL_DATAMGR(dm) { }

    ~KisDataManager() override {
    }

public:
    /**
     * Sets the default pixel. New data will be initialised with this pixel. The pixel is copied: the
     * caller still owns the pointer.
     */
    inline void setDefaultPixel(const quint8 *defPixel) {
        return ACTUAL_DATAMGR::setDefaultPixel(defPixel);
    }

    /**
     * Get a pointer to the default pixel.
     */
    inline const quint8 *defaultPixel() const {
        return ACTUAL_DATAMGR::defaultPixel();
    }

    /**
     * Reguests a memento from the data manager. There is only one memento active
     * at any given moment for a given paint device and all and any
     * write actions on the datamanger builds undo data into this memento
     * necessary to rollback the transaction.
     */
    inline KisMementoSP getMemento() {
        return ACTUAL_DATAMGR::getMemento();
    }

    /**
     * Restores the image data to the state at the time of the getMemento() call.
     *
     * Note that rollback should be performed with mementos in the reverse order of
     * their creation, as mementos only store incremental changes
     */
    inline void rollback(KisMementoSP memento) {
        ACTUAL_DATAMGR::rollback(memento);
    }

    /**
     * Restores the image data to the state at the time of the rollback call of the memento.
     *
     * Note that rollforward must only be called when an rollback have previously been performed, and
     * no intermittent actions have been performed (though it's ok to rollback other mementos and
     * roll them forward again)
     */
    inline void rollforward(KisMementoSP memento) {
        ACTUAL_DATAMGR::rollforward(memento);
    }

    /**
     * @returns true if there is a memento active. This means that
     * iterators can rely on the oldData() function.
     */
    inline bool hasCurrentMemento() const {
        return ACTUAL_DATAMGR::hasCurrentMemento();
    }

public:

    /**
     * Reads and writes the tiles
     *
     */
    inline bool write(KisPaintDeviceWriter &writer) {
        return ACTUAL_DATAMGR::write(writer);
    }

    inline bool read(QIODevice *io) {
        return ACTUAL_DATAMGR::read(io);
    }

    inline void purge(const QRect& area) {
        ACTUAL_DATAMGR::purge(area);
    }

    /**
     * The tiles may be not allocated directly from the glibc, but
     * instead can be allocated in bigger blobs. After you freed quite
     * a lot of data and are sure you won't need it anymore, you can
     * release these pools to save the memory.
     */
    static inline void releaseInternalPools() {
        ACTUAL_DATAMGR::releaseInternalPools();
    }

public:

    /**
     * Returns the number of bytes a pixel takes
     */
    inline quint32 pixelSize() const {
        return ACTUAL_DATAMGR::pixelSize();
    }

    /**
     * Return the extent of the data in x,y,w,h.
     */
    inline void extent(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const {
        return ACTUAL_DATAMGR::extent(x, y, w, h);
    }

    QRect extent() const {
        return ACTUAL_DATAMGR::extent();
    }

    KisRegion region() const {
        return ACTUAL_DATAMGR::region();
    }

public:

    /**
      * Crop or extend the data to x, y, w, h.
      */
    inline void setExtent(qint32 x, qint32 y, qint32 w, qint32 h) {
        return ACTUAL_DATAMGR::setExtent(x, y, w, h);
    }

    inline void setExtent(const QRect & rect) {
        setExtent(rect.x(), rect.y(), rect.width(), rect.height());
    }

public:

    /**
     * Clear the specified rect to the specified value.
     */
    inline void clear(qint32 x, qint32 y,
                      qint32 w, qint32 h,
                      quint8 def) {
        ACTUAL_DATAMGR::clear(x, y, w, h, def);
    }

    /**
     * Clear the specified rect to the specified pixel value.
     */
    inline void clear(qint32 x, qint32 y,
                      qint32 w, qint32 h,
                      const quint8 * def) {
        ACTUAL_DATAMGR::clear(x, y, w, h, def);
    }


    /**
     * Clear all back to default values.
     */
    inline void clear() {
        ACTUAL_DATAMGR::clear();
    }

public:

    /**
     * Clones rect from another datamanager. The cloned area will be
     * shared between both datamanagers as much as possible using
     * copy-on-write. Parts of the rect that cannot be shared
     * (cross tiles) are deep-copied,
     */
    inline void bitBlt(KisTiledDataManagerSP srcDM, const QRect &rect) {
        ACTUAL_DATAMGR::bitBlt(const_cast<KisTiledDataManager*>(srcDM.data()), rect);
    }

    /**
     * The same as \ref bitBlt() but reads old data
     */
    inline void bitBltOldData(KisTiledDataManagerSP srcDM, const QRect &rect) {
        ACTUAL_DATAMGR::bitBltOldData(const_cast<KisTiledDataManager*>(srcDM.data()), rect);
    }

    /**
     * Clones rect from another datamanager in a rough and fast way.
     * All the tiles touched by rect will be shared, between both
     * devices, that means it will copy a bigger area than was
     * requested. This method is supposed to be used for bitBlt'ing
     * into temporary paint devices.
     */
    inline void bitBltRough(KisTiledDataManagerSP srcDM, const QRect &rect) {
        ACTUAL_DATAMGR::bitBltRough(const_cast<KisTiledDataManager*>(srcDM.data()), rect);
    }

    /**
     * The same as \ref bitBltRough() but reads old data
     */
    inline void bitBltRoughOldData(KisTiledDataManagerSP srcDM, const QRect &rect) {
        ACTUAL_DATAMGR::bitBltRoughOldData(const_cast<KisTiledDataManager*>(srcDM.data()), rect);
    }

public:

    /**
     * Write the specified data to x, y. There is no checking on pixelSize!
     */
    inline void setPixel(qint32 x, qint32 y, const quint8 * data) {
        ACTUAL_DATAMGR::setPixel(x, y, data);
    }


    /**
     * Copy the bytes in the specified rect to a chunk of memory.
     * The pixelSize in bytes is w * h * pixelSize
     */
    inline void readBytes(quint8 * data,
                          qint32 x, qint32 y,
                          qint32 w, qint32 h,
                          qint32 dataRowStride = -1) const {
        ACTUAL_DATAMGR::readBytes(data, x, y, w, h, dataRowStride);
    }

    /**
     * Copy the bytes to the specified rect. w * h * pixelSize bytes
     * will be read, whether the caller prepared them or not.
     */
    inline void writeBytes(const quint8 * data,
                           qint32 x, qint32 y,
                           qint32 w, qint32 h,
                           qint32 dataRowStride = -1) {
        ACTUAL_DATAMGR::writeBytes(data, x, y, w, h, dataRowStride);
    }


    /**
     * Copy the bytes in the paint device into a vector of arrays of bytes,
     * where the number of arrays is the number of channels in the
     * paint device. If the specified area is larger than the paint
     * device's extent, the default pixel will be read.
     *
     * @param channelsizes a vector with for every channel its size in bytes
     * @param x x coordinate of the top left corner
     * @param y y coordinate of the top left corner
     * @param w width
     * @param h height
     */
    QVector<quint8*> readPlanarBytes(QVector<qint32> channelsizes, qint32 x, qint32 y, qint32 w, qint32 h) const {
        return ACTUAL_DATAMGR::readPlanarBytes(channelsizes, x, y, w, h);
    }

    /**
     * Write the data in the separate arrays to the channels. If there
     * are less vectors than channels, the remaining channels will not
     * be copied. If any of the arrays points to 0, the channel in
     * that location will not be touched. If the specified area is
     * larger than the paint device, the paint device will be
     * extended. There are no guards: if the area covers more pixels
     * than there are bytes in the arrays, krita will happily fill
     * your paint device with areas of memory you never wanted to be
     * read. Krita may also crash.
     *
     * @param planes a vector with a byte array for every plane
     * @param channelsizes a vector with for every channel its size in
     * bytes
     * @param x x coordinate of the top left corner
     * @param y y coordinate of the top left corner
     * @param w width
     * @param h height
     *
     * XXX: what about undo?
     */
    void writePlanarBytes(QVector<quint8*> planes, QVector<qint32> channelsizes,  qint32 x, qint32 y, qint32 w, qint32 h) {
        ACTUAL_DATAMGR::writePlanarBytes(planes, channelsizes, x, y, w, h);
    }


    /**
     * Get the number of contiguous columns starting at x, valid for all values
     * of y between minY and maxY.
     */
    inline qint32 numContiguousColumns(qint32 x, qint32 minY, qint32 maxY) const {
        return ACTUAL_DATAMGR::numContiguousColumns(x, minY, maxY);
    }


    /**
     * Get the number of contiguous rows starting at y, valid for all
     * values of x between minX and maxX.
     */
    inline qint32 numContiguousRows(qint32 y, qint32 minX, qint32 maxX) const {
        return ACTUAL_DATAMGR::numContiguousRows(y, minX, maxX);
    }


    /**
     * Get the row stride at pixel (x, y). This is the number of bytes
     * to add to a pointer to pixel (x, y) to access (x, y + 1).
     */
    inline qint32 rowStride(qint32 x, qint32 y) const {
        return ACTUAL_DATAMGR::rowStride(x, y);
    }

protected:
    friend class KisRectIterator;
    friend class KisHLineIterator;
    friend class KisVLineIterator;
};


#endif // KIS_DATAMANAGER_H_

