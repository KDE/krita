/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_PAINT_DEVICE_STRATEGIES_H
#define __KIS_PAINT_DEVICE_STRATEGIES_H

#include "kis_wrapped_rect.h"
#include "kis_wrapped_hline_iterator.h"
#include "kis_wrapped_vline_iterator.h"
#include "kis_wrapped_random_accessor.h"


class KisPaintDevice::Private::KisPaintDeviceStrategy
{
public:
    KisPaintDeviceStrategy(KisPaintDevice *device,
                           KisPaintDevice::Private *d)
        : m_device(device), m_d(d)
    {
    }

    virtual ~KisPaintDeviceStrategy() {
    }

    virtual void move(const QPoint& pt) {
        m_d->setX(pt.x());
        m_d->setY(pt.y());
        m_d->cache()->invalidate();
    }

    virtual QRect extent() const {
        QRect extent;

        qint32 x, y, w, h;
        m_d->dataManager()->extent(x, y, w, h);
        x += m_d->x();
        y += m_d->y();
        extent = QRect(x, y, w, h);

        quint8 defaultOpacity =
            m_device->colorSpace()->opacityU8(m_device->defaultPixel());

        if (defaultOpacity != OPACITY_TRANSPARENT_U8)
            extent |= m_d->defaultBounds->bounds();

        return extent;
    }

    virtual QRegion region() const {
        return m_d->cache()->region().translated(m_d->x(), m_d->y());
    }

    virtual void crop(const QRect &rect) {
        m_d->dataManager()->setExtent(rect.translated(-m_d->x(), -m_d->y()));
        m_d->cache()->invalidate();
    }

    virtual void clear(const QRect & rc) {
        KisDataManagerSP dm = m_d->dataManager();

        dm->clear(rc.x() - m_d->x(), rc.y() - m_d->y(),
                  rc.width(), rc.height(),
                  dm->defaultPixel());
        m_d->cache()->invalidate();
    }

    virtual void fill(const QRect &rc, const quint8 *fillPixel) {
        m_d->dataManager()->clear(rc.x() - m_d->x(),
                                  rc.y() - m_d->y(),
                                  rc.width(),
                                  rc.height(),
                                  fillPixel);
        m_d->cache()->invalidate();
    }


    virtual KisHLineIteratorSP createHLineIteratorNG(KisDataManager *dataManager, qint32 x, qint32 y, qint32 w, qint32 offsetX, qint32 offsetY) {
        return new KisHLineIterator2(dataManager, x, y, w, offsetX, offsetY, true);
    }

    virtual KisHLineConstIteratorSP createHLineConstIteratorNG(KisDataManager *dataManager, qint32 x, qint32 y, qint32 w, qint32 offsetX, qint32 offsetY) const {
        return new KisHLineIterator2(dataManager, x, y, w, offsetX, offsetY, false);
    }


    virtual KisVLineIteratorSP createVLineIteratorNG(qint32 x, qint32 y, qint32 w) {
        m_d->cache()->invalidate();
        return new KisVLineIterator2(m_d->dataManager().data(), x, y, w, m_d->x(), m_d->y(), true);
    }

    virtual KisVLineConstIteratorSP createVLineConstIteratorNG(qint32 x, qint32 y, qint32 w) const {
        return new KisVLineIterator2(m_d->dataManager().data(), x, y, w, m_d->x(), m_d->y(), false);
    }

    virtual KisRandomAccessorSP createRandomAccessorNG(qint32 x, qint32 y) {
        m_d->cache()->invalidate();
        return new KisRandomAccessor2(m_d->dataManager().data(), x, y, m_d->x(), m_d->y(), true);
    }

    virtual KisRandomConstAccessorSP createRandomConstAccessorNG(qint32 x, qint32 y) const {
        return new KisRandomAccessor2(m_d->dataManager().data(), x, y, m_d->x(), m_d->y(), false);
    }

    virtual void fastBitBlt(KisPaintDeviceSP src, const QRect &rect) {
        Q_ASSERT(m_device->fastBitBltPossible(src));
        fastBitBltImpl(src->dataManager(), rect);
    }

    virtual void fastBitBltOldData(KisPaintDeviceSP src, const QRect &rect) {
        Q_ASSERT(m_device->fastBitBltPossible(src));

        m_d->dataManager()->bitBltOldData(src->dataManager(), rect.translated(-m_d->x(), -m_d->y()));
        m_d->cache()->invalidate();
    }

    virtual void fastBitBltRough(KisPaintDeviceSP src, const QRect &rect) {
        Q_ASSERT(m_device->fastBitBltPossible(src));
        fastBitBltRoughImpl(src->dataManager(), rect);
    }

    virtual void fastBitBltRough(KisDataManagerSP srcDataManager, const QRect &rect) {
        fastBitBltRoughImpl(srcDataManager, rect);
    }

    virtual void fastBitBltRoughOldData(KisPaintDeviceSP src, const QRect &rect) {
        Q_ASSERT(m_device->fastBitBltPossible(src));

        m_d->dataManager()->bitBltRoughOldData(src->dataManager(), rect.translated(-m_d->x(), -m_d->y()));
        m_d->cache()->invalidate();
    }

    virtual void readBytes(quint8 *data, const QRect &rect) const {
        readBytesImpl(data, rect, -1);
    }

    virtual void writeBytes(const quint8 * data, const QRect &rect) {
        writeBytesImpl(data, rect, -1);
    }

    virtual QVector<quint8*> readPlanarBytes(qint32 x, qint32 y, qint32 w, qint32 h) const {
        return m_d->dataManager()->readPlanarBytes(m_device->channelSizes(), x, y, w, h);
    }

    virtual void writePlanarBytes(QVector<quint8*> planes, qint32 x, qint32 y, qint32 w, qint32 h) {
        m_d->dataManager()->writePlanarBytes(planes, m_device->channelSizes(), x, y, w, h);
        m_d->cache()->invalidate();
    }
protected:
    virtual void readBytesImpl(quint8 *data, const QRect &rect, int dataRowStride) const {
        m_d->dataManager()->readBytes(data,
                                      rect.x() - m_d->x(),
                                      rect.y() - m_d->y(),
                                      rect.width(),
                                      rect.height(),
                                      dataRowStride);
    }

    virtual void writeBytesImpl(const quint8 * data, const QRect &rect, int dataRowStride) {
        m_d->dataManager()->writeBytes(data,
                                       rect.x() - m_d->x(),
                                       rect.y() - m_d->y(),
                                       rect.width(),
                                       rect.height(),
                                       dataRowStride);
        m_d->cache()->invalidate();
    }

    virtual void fastBitBltImpl(KisDataManagerSP srcDataManager, const QRect &rect)
    {
        m_d->dataManager()->bitBlt(srcDataManager, rect.translated(-m_d->x(), -m_d->y()));
        m_d->cache()->invalidate();
    }

    virtual void fastBitBltRoughImpl(KisDataManagerSP srcDataManager, const QRect &rect)
    {
        m_d->dataManager()->bitBltRough(srcDataManager, rect.translated(-m_d->x(), -m_d->y()));
        m_d->cache()->invalidate();
    }

protected:
    KisPaintDevice *m_device;
    KisPaintDevice::Private * const m_d;
};


class KisPaintDevice::Private::KisPaintDeviceWrappedStrategy : public KisPaintDeviceStrategy
{
public:
    KisPaintDeviceWrappedStrategy(const QRect &wrapRect,
                                  KisPaintDevice *device,
                                  KisPaintDevice::Private *d)
        : KisPaintDeviceStrategy(device, d),
          m_wrapRect(wrapRect)
    {
    }

    const QRect wrapRect() const {
        return m_wrapRect;
    }

    void move(const QPoint& pt) {
        QPoint offset (pt.x() - m_device->x(), pt.y() - m_device->y());

        QRect exactBoundsBeforeMove = m_device->exactBounds();
        KisPaintDeviceStrategy::move(pt);

        QRegion borderRegion(exactBoundsBeforeMove.translated(offset.x(), offset.y()));
        borderRegion -= m_wrapRect;

        const int pixelSize = m_device->pixelSize();

        Q_FOREACH (const QRect &rc, borderRegion.rects()) {
            KisRandomConstAccessorSP srcIt = KisPaintDeviceStrategy::createRandomConstAccessorNG(rc.x(), rc.y());
            KisRandomAccessorSP dstIt = createRandomAccessorNG(rc.x(), rc.y());

            int rows = 1;
            int columns = 1;

            for (int y = rc.y(); y <= rc.bottom(); y += rows) {
                int rows = qMin(srcIt->numContiguousRows(y), dstIt->numContiguousRows(y));
                rows = qMin(rows, rc.bottom() - y + 1);

                for (int x = rc.x(); x <= rc.right(); x += columns) {
                    int columns = qMin(srcIt->numContiguousColumns(x), dstIt->numContiguousColumns(x));
                    columns = qMin(columns, rc.right() - x + 1);

                    srcIt->moveTo(x, y);
                    dstIt->moveTo(x, y);

                    int srcRowStride = srcIt->rowStride(x, y);
                    int dstRowStride = dstIt->rowStride(x, y);
                    const quint8 *srcPtr = srcIt->rawDataConst();
                    quint8 *dstPtr = dstIt->rawData();

                    for (int i = 0; i < rows; i++) {
                        memcpy(dstPtr, srcPtr, pixelSize * columns);
                        srcPtr += srcRowStride;
                        dstPtr += dstRowStride;
                    }
                }
            }
        }
    }

    QRect extent() const {
        return KisPaintDeviceStrategy::extent() & m_wrapRect;
    }

    QRegion region() const {
        return KisPaintDeviceStrategy::region() & m_wrapRect;
    }

    void crop(const QRect &rect) {
        KisPaintDeviceStrategy::crop(rect & m_wrapRect);
    }

    void clear(const QRect &rect) {
        KisWrappedRect splitRect(rect, m_wrapRect);
        Q_FOREACH (const QRect &rc, splitRect) {
            KisPaintDeviceStrategy::clear(rc);
        }
    }

    void fill(const QRect &rect, const quint8 *fillPixel) {
        KisWrappedRect splitRect(rect, m_wrapRect);
        Q_FOREACH (const QRect &rc, splitRect) {
            KisPaintDeviceStrategy::fill(rc, fillPixel);
        }
    }

    virtual KisHLineIteratorSP createHLineIteratorNG(KisDataManager *dataManager, qint32 x, qint32 y, qint32 w, qint32 offsetX, qint32 offsetY) {
        KisWrappedRect splitRect(QRect(x, y, w, m_wrapRect.height()), m_wrapRect);
        if (!splitRect.isSplit()) {
            return KisPaintDeviceStrategy::createHLineIteratorNG(dataManager, x, y, w, offsetX, offsetY);
        }
        return new KisWrappedHLineIterator(dataManager, splitRect, offsetX, offsetY, true);
    }

    virtual KisHLineConstIteratorSP createHLineConstIteratorNG(KisDataManager *dataManager, qint32 x, qint32 y, qint32 w, qint32 offsetX, qint32 offsetY) const {
        KisWrappedRect splitRect(QRect(x, y, w, m_wrapRect.height()), m_wrapRect);
        if (!splitRect.isSplit()) {
            return KisPaintDeviceStrategy::createHLineConstIteratorNG(dataManager, x, y, w, offsetX, offsetY);
        }
        return new KisWrappedHLineIterator(dataManager, splitRect, offsetX, offsetY, false);
    }

    virtual KisVLineIteratorSP createVLineIteratorNG(qint32 x, qint32 y, qint32 h) {
        m_d->cache()->invalidate();

        KisWrappedRect splitRect(QRect(x, y, m_wrapRect.width(), h), m_wrapRect);
        if (!splitRect.isSplit()) {
            return KisPaintDeviceStrategy::createVLineIteratorNG(x, y, h);
        }
        return new KisWrappedVLineIterator(m_d->dataManager().data(), splitRect, m_d->x(), m_d->y(), true);
    }

    virtual KisVLineConstIteratorSP createVLineConstIteratorNG(qint32 x, qint32 y, qint32 h) const {
        KisWrappedRect splitRect(QRect(x, y, m_wrapRect.width(), h), m_wrapRect);
        if (!splitRect.isSplit()) {
            return KisPaintDeviceStrategy::createVLineConstIteratorNG(x, y, h);
        }
        return new KisWrappedVLineIterator(m_d->dataManager().data(), splitRect, m_d->x(), m_d->y(), false);
    }

    virtual KisRandomAccessorSP createRandomAccessorNG(qint32 x, qint32 y) {
        m_d->cache()->invalidate();
        return new KisWrappedRandomAccessor(m_d->dataManager().data(), x, y, m_d->x(), m_d->y(), true, m_wrapRect);
    }

    virtual KisRandomConstAccessorSP createRandomConstAccessorNG(qint32 x, qint32 y) const {
        return new KisWrappedRandomAccessor(m_d->dataManager().data(), x, y, m_d->x(), m_d->y(), false, m_wrapRect);
    }

    void fastBitBltImpl(KisDataManagerSP srcDataManager, const QRect &rect) {
        KisWrappedRect splitRect(rect, m_wrapRect);
        Q_FOREACH (const QRect &rc, splitRect) {
            KisPaintDeviceStrategy::fastBitBltImpl(srcDataManager, rc);
        }
    }

    void fastBitBltOldData(KisPaintDeviceSP src, const QRect &rect) {
        KisWrappedRect splitRect(rect, m_wrapRect);
        Q_FOREACH (const QRect &rc, splitRect) {
            KisPaintDeviceStrategy::fastBitBltOldData(src, rc);
        }
    }

    virtual void fastBitBltRoughImpl(KisDataManagerSP srcDataManager, const QRect &rect)
    {
        // no rough version in wrapped mode
        fastBitBltImpl(srcDataManager, rect);
    }

    void fastBitBltRoughOldData(KisPaintDeviceSP src, const QRect &rect) {
        // no rough version in wrapped mode
        fastBitBltOldData(src, rect);
    }

    void readBytes(quint8 *data, const QRect &rect) const {
        KisWrappedRect splitRect(rect, m_wrapRect);

        if (!splitRect.isSplit()) {
            KisPaintDeviceStrategy::readBytes(data, rect);
        } else {
            const int pixelSize = m_device->pixelSize();

            int leftWidth = splitRect[KisWrappedRect::TOPLEFT].width();
            int rightWidth = splitRect[KisWrappedRect::TOPRIGHT].width();

            int totalHeight = rect.height();
            int totalWidth = rect.width();
            int dataRowStride = totalWidth * pixelSize;

            int bufOffset = 0;
            int row = 0;
            while (row < totalHeight) {
                int leftIndex = KisWrappedRect::TOPLEFT + bufOffset;
                int rightIndex = KisWrappedRect::TOPRIGHT + bufOffset;

                QPoint leftRectOrigin = splitRect[leftIndex].topLeft();
                QPoint rightRectOrigin = splitRect[rightIndex].topLeft();

                int height = qMin(splitRect[leftIndex].height(), totalHeight - row);

                int col = 0;
                while (col < totalWidth) {
                    int width;
                    quint8 *dataPtr;

                    width = qMin(leftWidth, totalWidth - col);
                    dataPtr = data + pixelSize * (col + row * totalWidth);
                    readBytesImpl(dataPtr, QRect(leftRectOrigin, QSize(width, height)), dataRowStride);
                    col += width;

                    if (col >= totalWidth) break;

                    width = qMin(rightWidth, totalWidth - col);
                    dataPtr = data + pixelSize * (col + row * totalWidth);
                    readBytesImpl(dataPtr, QRect(rightRectOrigin, QSize(width, height)), dataRowStride);
                    col += width;
                }

                row += height;
                bufOffset = (bufOffset + 2) % 4;
            }
        }
    }

    void writeBytes(const quint8 *data, const QRect &rect) {
        KisWrappedRect splitRect(rect, m_wrapRect);

        if (!splitRect.isSplit()) {
            KisPaintDeviceStrategy::writeBytes(data, rect);
        } else {
            const int pixelSize = m_device->pixelSize();

            int totalWidth = rect.width();
            int dataRowStride = totalWidth * pixelSize;

            QRect rc;
            QPoint origin;
            const quint8 *dataPtr;

            origin.rx() = 0;
            origin.ry() = 0;
            rc = splitRect.topLeft();
            dataPtr = data + pixelSize * (origin.x() + totalWidth * origin.y());
            writeBytesImpl(dataPtr, rc, dataRowStride);

            origin.rx() = splitRect.topLeft().width();
            origin.ry() = 0;
            rc = splitRect.topRight();
            dataPtr = data + pixelSize * (origin.x() + totalWidth * origin.y());
            writeBytesImpl(dataPtr, rc, dataRowStride);

            origin.rx() = 0;
            origin.ry() = splitRect.topLeft().height();
            rc = splitRect.bottomLeft();
            dataPtr = data + pixelSize * (origin.x() + totalWidth * origin.y());
            writeBytesImpl(dataPtr, rc, dataRowStride);

            origin.rx() = splitRect.topLeft().width();
            origin.ry() = splitRect.topLeft().height();
            rc = splitRect.bottomRight();
            dataPtr = data + pixelSize * (origin.x() + totalWidth * origin.y());
            writeBytesImpl(dataPtr, rc, dataRowStride);
        }
    }

private:
    QRect m_wrapRect;
};



#endif /* __KIS_PAINT_DEVICE_STRATEGIES_H */
