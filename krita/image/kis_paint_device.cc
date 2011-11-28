/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#include "kis_paint_device.h"

#include <QRect>
#include <QTransform>
#include <QImage>
#include <QList>
#include <QHash>

#include <klocale.h>

#include <KoColorProfile.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoIntegerMaths.h>
#include <KoStore.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_iterator.h"
#include "kis_iterators_pixel.h"
#include "kis_iterator_pixel_trait.h"
#include "kis_random_accessor.h"
#include "kis_random_sub_accessor.h"
#include "kis_selection.h"
#include "kis_node.h"
#include "commands/kis_paintdevice_convert_type_command.h"
#include "kis_datamanager.h"
#include "kis_paint_device.h"

#include "kis_selection_component.h"
#include "kis_pixel_selection.h"
#include "kis_repeat_iterators_pixel.h"
#include <KoColorModelStandardIds.h>

#include "tiles3/kis_hline_iterator.h"
#include "tiles3/kis_vline_iterator.h"
#include "tiles3/kis_rect_iterator.h"
#include "tiles3/kis_random_accessor.h"

#include "kis_default_bounds.h"

class CacheData : public KisDataManager::AbstractCache
{
public:
    bool m_thumbnailsValid;
    QMap<int, QMap<int, QImage> > m_thumbnails;

    bool m_exactBoundsValid;
    QRect m_exactBounds;

    bool m_regionValid;
    QRegion m_region;
};

class PaintDeviceCache
{
public:
    PaintDeviceCache(KisPaintDevice *paintDevice) {
        m_paintDevice = paintDevice;
    }


    void setupCache() {
        CacheData *data = static_cast<CacheData *>(m_paintDevice->m_datamanager->cache());

        if(!data) {
            data = new CacheData();
            m_paintDevice->m_datamanager->setCache(data);
        }

        m_data = data;
        invalidate();
    }

    void invalidate() {
        m_data->m_thumbnailsValid = false;
        m_data->m_exactBoundsValid = false;
        m_data->m_regionValid = false;
    }

    QRect exactBounds() {
        if (m_data->m_exactBoundsValid) {
            return m_data->m_exactBounds;
        }
        m_data->m_exactBounds = m_paintDevice->calculateExactBounds();
        m_data->m_exactBoundsValid = true;
        return m_data->m_exactBounds;
    }

    QRegion region() {
        if (m_data->m_regionValid) {
            return m_data->m_region;
        }
        m_data->m_region = m_paintDevice->dataManager()->region();
        m_data->m_regionValid = true;
        return m_data->m_region;

    }

    QImage createThumbnail(qint32 w, qint32 h) {
        QImage thumbnail;

        if(m_data->m_thumbnailsValid) {
            thumbnail = findThumbnail(w, h);
        }
        else {
            m_data->m_thumbnails.clear();
            m_data->m_thumbnailsValid = true;
        }

        if(thumbnail.isNull()) {
            thumbnail = m_paintDevice->createThumbnail(w, h, 0, QRect());
            cacheThumbnail(w, h, thumbnail);
        }

        Q_ASSERT(!thumbnail.isNull());
        return thumbnail;
    }

private:
    inline QImage findThumbnail(qint32 w, qint32 h) {
        QImage resultImage;
        if (m_data->m_thumbnails.contains(w) && m_data->m_thumbnails[w].contains(h)) {
            resultImage = m_data->m_thumbnails[w][h];
        }
        return resultImage;
    }

    inline void cacheThumbnail(qint32 w, qint32 h, QImage image) {
        m_data->m_thumbnails[w][h] = image;
    }

private:
    KisPaintDevice *m_paintDevice;
    CacheData *m_data;
};


struct KisPaintDevice::Private
{

public:

    Private(KisPaintDevice *paintDevice)
        : cache(paintDevice),
          x(0),
          y(0)
    {
    }

    ~Private()
    {
        delete defaultBounds;
    }

    KisNodeWSP parent;
    KisDefaultBounds *defaultBounds;
    PaintDeviceCache cache;
    qint32 x;
    qint32 y;
    KoColorSpace* colorSpace;

};

KisPaintDevice::KisPaintDevice(const KoColorSpace * colorSpace, const QString& name)
    : QObject(0)
    , m_d(new Private(this))
{
    init(0, colorSpace, new KisDefaultBounds(), 0, name);
}

KisPaintDevice::KisPaintDevice(KisNodeWSP parent, const KoColorSpace * colorSpace, KisDefaultBounds * defaultBounds, const QString& name)
    : QObject(0)
    , m_d(new Private(this))
{
    if (!defaultBounds) {
        defaultBounds = new KisDefaultBounds();
    }
    init(0, colorSpace, defaultBounds, parent, name);
}

KisPaintDevice::KisPaintDevice(KisDataManagerSP explicitDataManager,
                               KisPaintDeviceSP src,
                               const QString& name)
    : QObject(0)
    , m_d(new Private(this))
{
    init(explicitDataManager, src->colorSpace(), src->defaultBounds(), 0, name);
    m_d->x = src->x();
    m_d->y = src->y();
}

void KisPaintDevice::init(KisDataManagerSP explicitDataManager,
                          const KoColorSpace *colorSpace,
                          KisDefaultBounds * defaultBounds,
                          KisNodeWSP parent, const QString& name)
{
    Q_ASSERT(colorSpace);
    setObjectName(name);

    m_d->colorSpace = KoColorSpaceRegistry::instance()->grabColorSpace(colorSpace);
    Q_ASSERT(m_d->colorSpace);

    if(explicitDataManager) {
        m_datamanager = explicitDataManager;
    }
    else {
        const qint32 pixelSize = colorSpace->pixelSize();

        quint8* defaultPixel = new quint8[colorSpace->pixelSize()];
        colorSpace->fromQColor(Qt::transparent, defaultPixel);

        m_datamanager = new KisDataManager(pixelSize, defaultPixel);
        delete[] defaultPixel;

        Q_CHECK_PTR(m_datamanager);
    }
    m_d->cache.setupCache();

    setDefaultBounds(defaultBounds);
    setParentNode(parent);
}

KisPaintDevice::KisPaintDevice(const KisPaintDevice& rhs)
    : QObject()
    , KisShared()
    , m_d(new Private(this))
{
    if (this != &rhs) {
        m_d->colorSpace = KoColorSpaceRegistry::instance()->grabColorSpace(rhs.m_d->colorSpace);
        Q_ASSERT(m_d->colorSpace);

        m_d->x = rhs.m_d->x;
        m_d->y = rhs.m_d->y;

        Q_ASSERT(rhs.m_datamanager);
        m_datamanager = new KisDataManager(*rhs.m_datamanager);
        Q_CHECK_PTR(m_datamanager);
        m_d->cache.setupCache();

        setDefaultBounds(rhs.defaultBounds());
        setParentNode(0);
    }
}

KisPaintDevice::~KisPaintDevice()
{
    KoColorSpaceRegistry::instance()->releaseColorSpace(m_d->colorSpace);
    delete m_d;
}

void KisPaintDevice::prepareClone(KisPaintDeviceSP src)
{
    clear();
    m_d->x = src->x();
    m_d->y = src->y();

    if(!(*colorSpace() == *src->colorSpace())) {
        if (m_d->colorSpace->pixelSize() != src->colorSpace()->pixelSize()) {
            m_datamanager = 0;
            m_datamanager = new KisDataManager(src->pixelSize(), src->defaultPixel());
            m_d->cache.setupCache();
        }
        else {
            setDefaultPixel(src->defaultPixel());
        }

        KoColorSpaceRegistry::instance()->releaseColorSpace(m_d->colorSpace);
        m_d->colorSpace = KoColorSpaceRegistry::instance()->grabColorSpace(src->colorSpace());
    }
    setDefaultBounds(src->defaultBounds());
    setParentNode(0);

    Q_ASSERT(fastBitBltPossible(src));
}

void KisPaintDevice::makeCloneFrom(KisPaintDeviceSP src, const QRect &rect)
{
    prepareClone(src);
    fastBitBlt(src, rect);
}

void KisPaintDevice::makeCloneFromRough(KisPaintDeviceSP src, const QRect &minimalRect)
{
    prepareClone(src);
    fastBitBltRough(src, minimalRect);
}

void KisPaintDevice::setDirty()
{
    m_d->cache.invalidate();
    if (m_d->parent.isValid())
        m_d->parent->setDirty();
}

void KisPaintDevice::setDirty(const QRect & rc)
{
    m_d->cache.invalidate();
    if (m_d->parent.isValid())
        m_d->parent->setDirty(rc);
}

void KisPaintDevice::setDirty(const QRegion & region)
{
    m_d->cache.invalidate();
    if (m_d->parent.isValid())
        m_d->parent->setDirty(region);
}

void KisPaintDevice::setDirty(const QVector<QRect> rects)
{
    m_d->cache.invalidate();
    if (m_d->parent.isValid())
        m_d->parent->setDirty(rects);
}

void KisPaintDevice::setParentNode(KisNodeWSP parent)
{
    m_d->parent = parent;
}

void KisPaintDevice::setDefaultBounds(KisDefaultBounds * defaultBounds)
{
    delete m_d->defaultBounds;
    m_d->defaultBounds = defaultBounds;
    m_d->cache.invalidate();
}

KisDefaultBounds * KisPaintDevice::defaultBounds() const
{
    return m_d->defaultBounds;
}

void KisPaintDevice::move(qint32 x, qint32 y)
{
    QRect dirtyRegion = extent();

    m_d->x = x;
    m_d->y = y;

    dirtyRegion |= extent();

    setDirty(dirtyRegion);

}

void KisPaintDevice::move(const QPoint& pt)
{
    move(pt.x(), pt.y());
}

void KisPaintDevice::extent(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const
{
    QRect rc = extent();
    x = rc.x();
    y = rc.y();
    w = rc.width();
    h = rc.height();
}

QRect KisPaintDevice::extent() const
{
    QRect extent;

    qint32 x, y, w, h;
    m_datamanager->extent(x, y, w, h);
    x += m_d->x;
    y += m_d->y;
    extent = QRect(x, y, w, h);

    quint8 defaultOpacity = colorSpace()->opacityU8(defaultPixel());
    if (defaultOpacity != OPACITY_TRANSPARENT_U8)
        extent |= m_d->defaultBounds->bounds();

    return extent;
}

QRegion KisPaintDevice::region() const
{
    return m_d->cache.region();
}

void KisPaintDevice::exactBounds(qint32 &x, qint32 &y, qint32 &w, qint32 &h) const
{
    QRect rc = exactBounds();
    x = rc.x();
    y = rc.y();
    w = rc.width();
    h = rc.height();
}

QRect KisPaintDevice::exactBounds() const
{
    return m_d->cache.exactBounds();
}

QRect KisPaintDevice::calculateExactBounds() const
{
    quint8 defaultOpacity = colorSpace()->opacityU8(defaultPixel());
    if(defaultOpacity != OPACITY_TRANSPARENT_U8) {
        /**
         * We will not calculate exact bounds for the device,
         * that is knows to be at least not smaller than image.
         * It isn't worth it.
         */

        return extent();
    }

    // Solution n°2
    qint32  x, y, w, h, boundX2, boundY2, boundW2, boundH2;
    QRect rc = extent();

    x = boundX2 = rc.x();
    y = boundY2 = rc.y();
    w = boundW2 = rc.width();
    h = boundH2 = rc.height();

    // XXX: a small optimization is possible by using H/V line iterators in the first
    //      and third cases, at the cost of making the code a bit more complex

    const qint32 pixelSize = this->pixelSize();
    const quint8* defaultPixel = m_datamanager->defaultPixel();

    KisRandomConstAccessorSP accessor = createRandomConstAccessorNG(x, y);

    bool found = false;
    {
        for (qint32 y2 = y; y2 < y + h ; ++y2) {
            for (qint32 x2 = x; x2 < x + w || found; ++ x2) {
                accessor->moveTo(x2, y2);
                if (memcmp(accessor->oldRawData(), defaultPixel, pixelSize) != 0) {
                    boundY2 = y2;
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
    }

    found = false;

    for (qint32 y2 = y + h - 1; y2 >= y ; --y2) {
        for (qint32 x2 = x + w - 1; x2 >= x || found; --x2) {
            accessor->moveTo(x2, y2);
            if (memcmp(accessor->oldRawData(), defaultPixel, pixelSize) != 0) {
                boundH2 = y2 - boundY2 + 1;
                found = true;
                break;
            }
        }
        if (found) break;
    }
    found = false;

    {
        for (qint32 x2 = x; x2 < x + w ; ++x2) {
            for (qint32 y2 = y; y2 < y + h || found; ++y2) {
                accessor->moveTo(x2, y2);
                if (memcmp(accessor->oldRawData(), defaultPixel, pixelSize) != 0) {
                    boundX2 = x2;
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
    }

    found = false;

    // Look for right edge )
    {

        for (qint32 x2 = x + w - 1; x2 >= x; --x2) {
            for (qint32 y2 = y + h -1; y2 >= y || found; --y2) {
                accessor->moveTo(x2, y2);
                if (memcmp(accessor->oldRawData(), defaultPixel, pixelSize) != 0) {
                    boundW2 = x2 - boundX2 + 1;
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
    }

    return QRect(boundX2, boundY2, boundW2, boundH2);
}


void KisPaintDevice::crop(qint32 x, qint32 y, qint32 w, qint32 h)
{
    crop(QRect(x, y, w, h));
}


void KisPaintDevice::crop(const QRect &rect)
{
    m_datamanager->setExtent(rect.translated(-m_d->x, -m_d->y));
    m_d->cache.invalidate();
}

void KisPaintDevice::setDefaultPixel(const quint8 *defPixel)
{
    m_datamanager->setDefaultPixel(defPixel);
    m_d->cache.invalidate();
}

const quint8 *KisPaintDevice::defaultPixel() const
{
    return m_datamanager->defaultPixel();
}

void KisPaintDevice::clear()
{
    m_datamanager->clear();
    m_d->cache.invalidate();
}

void KisPaintDevice::clear(const QRect & rc)
{
    m_datamanager->clear(rc.x() - m_d->x, rc.y() - m_d->y,
                         rc.width(), rc.height(),
                         m_datamanager->defaultPixel());
    m_d->cache.invalidate();
}

void KisPaintDevice::fill(const QRect & rc, const KoColor &color)
{
    m_datamanager->clear(rc.x() - m_d->x, rc.y() - m_d->y,
                         rc.width(), rc.height(), color.data());
    m_d->cache.invalidate();
}

void KisPaintDevice::fill(qint32 x, qint32 y, qint32 w, qint32 h, const quint8 *fillPixel)
{
    m_datamanager->clear(x - m_d->x, y - m_d->y, w, h, fillPixel);
    m_d->cache.invalidate();
}

bool KisPaintDevice::write(KoStore *store)
{
    return m_datamanager->write(store);
}

bool KisPaintDevice::read(KoStore *store)
{
    bool retval = m_datamanager->read(store);
    m_d->cache.invalidate();
    return retval;
}

KUndo2Command* KisPaintDevice::convertTo(const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent)
{
    m_d->cache.invalidate();
    dbgImage << this << colorSpace()->id() << dstColorSpace->id() << renderingIntent;
    if (*colorSpace() == *dstColorSpace) {
        return 0;
    }

    KisPaintDevice dst(dstColorSpace);
    dst.setX(x());
    dst.setY(y());

    qint32 x, y, w, h;
    QRect rc = exactBounds();
    x = rc.x();
    y = rc.y();
    w = rc.width();
    h = rc.height();


    for (qint32 row = y; row < y + h; ++row) {

        qint32 column = x;
        qint32 columnsRemaining = w;

        while (columnsRemaining > 0) {

            qint32 numContiguousDstColumns = dst.numContiguousColumns(column, row, row);
            qint32 numContiguousSrcColumns = numContiguousColumns(column, row, row);

            qint32 columns = qMin(numContiguousDstColumns, numContiguousSrcColumns);
            columns = qMin(columns, columnsRemaining);

            KisHLineConstIteratorPixel srcIt = createHLineConstIterator(column, row, columns);
            KisHLineIteratorPixel dstIt = dst.createHLineIterator(column, row, columns);

            const quint8 *srcData = srcIt.rawData();
            quint8 *dstData = dstIt.rawData();

            m_d->colorSpace->convertPixelsTo(srcData, dstData, dstColorSpace, columns, renderingIntent);

            column += columns;
            columnsRemaining -= columns;
        }
    }

    KisDataManagerSP oldData = m_datamanager;
    KoColorSpace *oldColorSpace = m_d->colorSpace;

    KisPaintDeviceConvertTypeCommand* cmd = new KisPaintDeviceConvertTypeCommand(this,
                                                                                 oldData,
                                                                                 oldColorSpace,
                                                                                 dst.m_datamanager,
                                                                                 dstColorSpace);

    setDataManager(dst.m_datamanager, dstColorSpace);

    return cmd;
}

void KisPaintDevice::setProfile(const KoColorProfile * profile)
{
    if (profile == 0) return;
    m_d->cache.invalidate();
    const KoColorSpace * dstSpace =
            KoColorSpaceRegistry::instance()->colorSpace(colorSpace()->colorModelId().id(), colorSpace()->colorDepthId().id(), profile);
    if (dstSpace) {
        KoColorSpaceRegistry::instance()->releaseColorSpace(m_d->colorSpace);
        m_d->colorSpace = KoColorSpaceRegistry::instance()->grabColorSpace(dstSpace);
    }
    emit profileChanged(profile);
}

void KisPaintDevice::setDataManager(KisDataManagerSP data, const KoColorSpace * colorSpace)
{
    m_datamanager = data;
    m_d->cache.setupCache();

    if(colorSpace) {
        KoColorSpaceRegistry::instance()->releaseColorSpace(m_d->colorSpace);
        m_d->colorSpace = KoColorSpaceRegistry::instance()->grabColorSpace(colorSpace);
        emit colorSpaceChanged(colorSpace);
    }
}

void KisPaintDevice::convertFromQImage(const QImage& _image, const KoColorProfile *profile,
                                       qint32 offsetX, qint32 offsetY)
{
    QImage image = _image;

    if (image.format() != QImage::Format_ARGB32) {
        image = image.convertToFormat(QImage::Format_ARGB32);
    }
    // Don't convert if not no profile is given and both paint dev and qimage are rgba.
    if (!profile && colorSpace()->id() == "RGBA") {
        writeBytes(image.bits(), offsetX, offsetY, image.width(), image.height());
    } else {
        quint8 * dstData = new quint8[image.width() * image.height() * pixelSize()];
        KoColorSpaceRegistry::instance()
                ->colorSpace(RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), profile)
                ->convertPixelsTo(image.bits(), dstData, colorSpace(), image.width() * image.height());

        writeBytes(dstData, offsetX, offsetY, image.width(), image.height());
        delete[] dstData;
    }
    m_d->cache.invalidate();
}

QImage KisPaintDevice::convertToQImage(const KoColorProfile *dstProfile) const
{
    qint32 x1;
    qint32 y1;
    qint32 w;
    qint32 h;

    x1 = - x();
    y1 = - y();

    QRect rc = exactBounds();
    x1 = rc.x();
    y1 = rc.y();
    w = rc.width();
    h = rc.height();

    return convertToQImage(dstProfile, x1, y1, w, h);
}

QImage KisPaintDevice::convertToQImage(const KoColorProfile *  dstProfile, qint32 x1, qint32 y1, qint32 w, qint32 h) const
{

    if (w < 0)
        return QImage();

    if (h < 0)
        return QImage();

    quint8 * data;
    try {
        data = new quint8 [w * h * pixelSize()];
    } catch (std::bad_alloc) {
        warnKrita << "KisPaintDevice::convertToQImage std::bad_alloc for " << w << " * " << h << " * " << pixelSize();
        //delete[] data; // data is not allocated, so don't free it
        return QImage();
    }
    Q_CHECK_PTR(data);

    // XXX: Is this really faster than converting line by line and building the QImage directly?
    //      This copies potentially a lot of data.
    readBytes(data, x1, y1, w, h);
    QImage image = colorSpace()->convertToQImage(data, w, h, dstProfile,
                                                 KoColorConversionTransformation::IntentPerceptual);
    delete[] data;

    return image;
}

KisPaintDeviceSP KisPaintDevice::createThumbnailDevice(qint32 w, qint32 h, const KisSelection * selection, QRect rect) const
{
    KisPaintDeviceSP thumbnail = new KisPaintDevice(colorSpace());

    int srcWidth, srcHeight;
    int srcX0, srcY0;
    QRect e;
    if (!rect.isValid())
        e = extent();
    else
        e = rect.translated(-m_d->x, -m_d->y);
    e.getRect(&srcX0, &srcY0, &srcWidth, &srcHeight);


    if (w > srcWidth) {
        w = srcWidth;
        h = qint32(double(srcWidth) / w * h);
    }
    if (h > srcHeight) {
        h = srcHeight;
        w = qint32(double(srcHeight) / h * w);
    }

    if (srcWidth > srcHeight)
        h = qint32(double(srcHeight) / srcWidth * w);
    else if (srcHeight > srcWidth)
        w = qint32(double(srcWidth) / srcHeight * h);

    const qint32 pixelSize = this->pixelSize();

    KisRandomConstAccessorPixel iter = createRandomConstAccessor(0, 0, selection);
    KisRandomAccessorPixel dstIter = thumbnail->createRandomAccessor(0, 0);

    for (qint32 y = 0; y < h; ++y) {
        qint32 iY = srcY0 + (y * srcHeight) / h;
        for (qint32 x = 0; x < w; ++x) {
            qint32 iX = srcX0 + (x * srcWidth) / w;
            iter.moveTo(iX, iY);
            dstIter.moveTo(x,  y);
            memcpy(dstIter.rawData(), iter.rawData(), pixelSize);
        }
    }

    return thumbnail;

}

QImage KisPaintDevice::createThumbnail(qint32 w, qint32 h, const KisSelection *selection, QRect rect)
{
    KisPaintDeviceSP dev = createThumbnailDevice(w, h, selection, rect);
    QImage thumbnail = dev->convertToQImage(KoColorSpaceRegistry::instance()->rgb8()->profile());
    return thumbnail;
}

QImage KisPaintDevice::createThumbnail(qint32 w, qint32 h)
{
    return m_d->cache.createThumbnail(w, h);
}

KisRectIteratorPixel KisPaintDevice::createRectIterator(qint32 left, qint32 top, qint32 w, qint32 h, const KisSelection * selection)
{
    m_d->cache.invalidate();

    KisDataManager* selectionDm = 0;

    if (selection)
        selectionDm = selection->projection()->dataManager().data();

    return KisRectIteratorPixel(m_datamanager.data(), selectionDm, left, top, w, h, m_d->x, m_d->y);
}

KisRectConstIteratorPixel KisPaintDevice::createRectConstIterator(qint32 left, qint32 top, qint32 w, qint32 h, const KisSelection * selection) const
{
    KisDataManager* dm = const_cast< KisDataManager*>(m_datamanager.data()); // TODO: don't do this
    KisDataManager* selectionDm = 0;

    if (selection)
        selectionDm = const_cast< KisDataManager*>(selection->projection()->dataManager().data());

    return KisRectConstIteratorPixel(dm, selectionDm, left, top, w, h, m_d->x, m_d->y);
}

KisHLineIteratorPixel  KisPaintDevice::createHLineIterator(qint32 x, qint32 y, qint32 w, const KisSelection * selection)
{
    m_d->cache.invalidate();
    KisDataManager* selectionDm = 0;

    if (selection)
        selectionDm = selection->projection()->dataManager().data();

    return KisHLineIteratorPixel(m_datamanager.data(), selectionDm, x, y, w, m_d->x, m_d->y);
}


KisHLineIteratorSP KisPaintDevice::createHLineIteratorNG(qint32 x, qint32 y, qint32 w)
{
    m_d->cache.invalidate();
    return new KisHLineIterator2(m_datamanager.data(), x, y, w, m_d->x, m_d->y, true);
}

KisHLineConstIteratorSP KisPaintDevice::createHLineConstIteratorNG(qint32 x, qint32 y, qint32 w) const
{
    KisDataManager* dm = const_cast< KisDataManager*>(m_datamanager.data()); // TODO: don't do this
    return new KisHLineIterator2(dm, x, y, w, m_d->x, m_d->y, false);
}

KisVLineIteratorSP KisPaintDevice::createVLineIteratorNG(qint32 x, qint32 y, qint32 w)
{
    m_d->cache.invalidate();
    return new KisVLineIterator2(m_datamanager.data(), x, y, w, m_d->x, m_d->y, true);
}

KisVLineConstIteratorSP KisPaintDevice::createVLineConstIteratorNG(qint32 x, qint32 y, qint32 w) const
{
    KisDataManager* dm = const_cast< KisDataManager*>(m_datamanager.data()); // TODO: don't do this
    return new KisVLineIterator2(dm, x, y, w, m_d->x, m_d->y, false);
}

KisRectIteratorSP KisPaintDevice::createRectIteratorNG(qint32 x, qint32 y, qint32 w, qint32 h)
{
    m_d->cache.invalidate();
    return new KisRectIterator2(m_datamanager.data(), x, y, w, h, m_d->x, m_d->y, true);
}

KisRectIteratorSP KisPaintDevice::createRectIteratorNG(const QRect& r)
{
    return createRectIteratorNG(r.x(), r.y(), r.width(), r.height());
}

KisRectConstIteratorSP KisPaintDevice::createRectConstIteratorNG(qint32 x, qint32 y, qint32 w, qint32 h) const
{
    KisDataManager* dm = const_cast< KisDataManager*>(m_datamanager.data()); // TODO: don't do this
    return new KisRectIterator2(dm, x, y, w, h, m_d->x, m_d->y, false);
}

KisRectConstIteratorSP KisPaintDevice::createRectConstIteratorNG(const QRect& r) const
{
    return createRectConstIteratorNG(r.x(), r.y(), r.width(), r.height());
}

KisHLineConstIteratorPixel  KisPaintDevice::createHLineConstIterator(qint32 x, qint32 y, qint32 w, const KisSelection * selection) const
{
    KisDataManager* dm = const_cast< KisDataManager*>(m_datamanager.data()); // TODO: don't do this
    KisDataManager* selectionDm = 0;

    if (selection)
        selectionDm = const_cast< KisDataManager*>(selection->projection()->dataManager().data());

    return KisHLineConstIteratorPixel(dm, selectionDm, x, y, w, m_d->x, m_d->y);
}

KisRepeatHLineConstIteratorPixel KisPaintDevice::createRepeatHLineConstIterator(qint32 x, qint32 y, qint32 w, const QRect& _dataWidth, const KisSelection * selection) const
{
    KisDataManager* dm = const_cast< KisDataManager*>(m_datamanager.data()); // TODO: don't do this
    KisDataManager* selectionDm = 0;

    if (selection)
        selectionDm = const_cast< KisDataManager*>(selection->projection()->dataManager().data());

    return KisRepeatHLineConstIteratorPixel(dm, selectionDm, x, y, w, m_d->x, m_d->y, _dataWidth);
}

KisRepeatVLineConstIteratorPixel KisPaintDevice::createRepeatVLineConstIterator(qint32 x, qint32 y, qint32 h, const QRect& _dataWidth, const KisSelection * selection) const
{
    KisDataManager* dm = const_cast< KisDataManager*>(m_datamanager.data()); // TODO: don't do this
    KisDataManager* selectionDm = 0;

    if (selection)
        selectionDm = const_cast< KisDataManager*>(selection->projection()->dataManager().data());

    return KisRepeatVLineConstIteratorPixel(dm, selectionDm, x, y, h, m_d->x, m_d->y, _dataWidth);
}

KisVLineIteratorPixel  KisPaintDevice::createVLineIterator(qint32 x, qint32 y, qint32 h, const KisSelection * selection)
{
    m_d->cache.invalidate();
    KisDataManager* selectionDm = 0;

    if (selection)
        selectionDm = selection->projection()->dataManager().data();

    return KisVLineIteratorPixel(m_datamanager.data(), selectionDm, x, y, h, m_d->x, m_d->y);
}

KisVLineConstIteratorPixel  KisPaintDevice::createVLineConstIterator(qint32 x, qint32 y, qint32 h, const KisSelection * selection) const
{
    KisDataManager* dm = const_cast< KisDataManager*>(m_datamanager.data()); // TODO: don't do this
    KisDataManager* selectionDm = 0;

    if (selection)
        selectionDm = const_cast< KisDataManager*>(selection->projection()->dataManager().data());

    return KisVLineConstIteratorPixel(dm, selectionDm, x, y, h, m_d->x, m_d->y);
}

KisRandomAccessorPixel KisPaintDevice::createRandomAccessor(qint32 x, qint32 y, const KisSelection * selection)
{
    m_d->cache.invalidate();
    KisDataManager* selectionDm = 0;

    if (selection)
        selectionDm = selection->projection()->dataManager().data();

    return KisRandomAccessorPixel(m_datamanager.data(), selectionDm, x, y, m_d->x, m_d->y);
}

KisRandomConstAccessorPixel KisPaintDevice::createRandomConstAccessor(qint32 x, qint32 y, const KisSelection * selection) const
{
    KisDataManager* dm = const_cast< KisDataManager*>(m_datamanager.data()); // TODO: don't do this
    KisDataManager* selectionDm = 0;

    if (selection)
        selectionDm = const_cast< KisDataManager*>(selection->projection()->dataManager().data());

    return KisRandomConstAccessorPixel(dm, selectionDm, x, y, m_d->x, m_d->y);
}

KisRandomAccessorSP KisPaintDevice::createRandomAccessorNG(qint32 x, qint32 y)
{
    m_d->cache.invalidate();
    return new KisRandomAccessor2(m_datamanager.data(), x, y, m_d->x, m_d->y, true);
}

KisRandomConstAccessorSP KisPaintDevice::createRandomConstAccessorNG(qint32 x, qint32 y) const
{
    KisDataManager* dm = const_cast< KisDataManager*>(m_datamanager.data()); // TODO: don't do this

    return new KisRandomAccessor2(dm, x, y, m_d->x, m_d->y, false);
}

KisRandomSubAccessorPixel KisPaintDevice::createRandomSubAccessor(const KisSelection * selection) const
{
    m_d->cache.invalidate();
    // XXX: shouldn't we use the selection here?
    Q_UNUSED(selection);
    KisPaintDevice* pd = const_cast<KisPaintDevice*>(this);
    return KisRandomSubAccessorPixel(pd);
}

void KisPaintDevice::clearSelection(KisSelectionSP selection)
{
    QRect r = selection->selectedExactRect();

    if (r.isValid()) {

        KisHLineIterator devIt = createHLineIterator(r.x(), r.y(), r.width());
        KisHLineConstIterator selectionIt = selection->projection()->createHLineIterator(r.x(), r.y(), r.width());

        const quint8* defaultPixel_ = defaultPixel();
        bool transparentDefault = (m_d->colorSpace->opacityU8(defaultPixel_) == OPACITY_TRANSPARENT_U8);
        for (qint32 y = 0; y < r.height(); y++) {

            while (!devIt.isDone()) {
                // XXX: Optimize by using stretches

                m_d->colorSpace->applyInverseAlphaU8Mask(devIt.rawData(), selectionIt.rawData(), 1);
                if (transparentDefault && m_d->colorSpace->opacityU8(devIt.rawData()) == OPACITY_TRANSPARENT_U8) {
                    memcpy(devIt.rawData(), defaultPixel_, m_d->colorSpace->pixelSize());
                }
                ++devIt;
                ++selectionIt;
            }
            devIt.nextRow();
            selectionIt.nextRow();
        }
        m_datamanager->purge(r.translated(-m_d->x, -m_d->y));
        setDirty(r);
    }
}

bool KisPaintDevice::pixel(qint32 x, qint32 y, QColor *c) const
{
    KisHLineConstIteratorPixel iter = createHLineConstIterator(x, y, 1);

    const quint8 *pix = iter.rawData();

    if (!pix) return false;

    colorSpace()->toQColor(pix, c);

    return true;
}


bool KisPaintDevice::pixel(qint32 x, qint32 y, KoColor * kc) const
{
    KisHLineConstIteratorPixel iter = createHLineConstIterator(x, y, 1);

    const quint8 *pix = iter.rawData();

    if (!pix) return false;

    kc->setColor(pix, m_d->colorSpace);

    return true;
}

bool KisPaintDevice::setPixel(qint32 x, qint32 y, const QColor& c)
{
    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1);

    colorSpace()->fromQColor(c, iter.rawData());
    m_d->cache.invalidate();
    return true;
}

bool KisPaintDevice::setPixel(qint32 x, qint32 y, const KoColor& kc)
{
    const quint8 * pix;
    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1);
    if (kc.colorSpace() != m_d->colorSpace) {
        KoColor kc2(kc, m_d->colorSpace);
        pix = kc2.data();
        memcpy(iter.rawData(), pix, m_d->colorSpace->pixelSize());
    } else {
        pix = kc.data();
        memcpy(iter.rawData(), pix, m_d->colorSpace->pixelSize());
    }
    m_d->cache.invalidate();
    return true;
}


qint32 KisPaintDevice::numContiguousColumns(qint32 x, qint32 minY, qint32 maxY) const
{
    return m_datamanager->numContiguousColumns(x - m_d->x, minY - m_d->y, maxY - m_d->y);
}

qint32 KisPaintDevice::numContiguousRows(qint32 y, qint32 minX, qint32 maxX) const
{
    return m_datamanager->numContiguousRows(y - m_d->y, minX - m_d->x, maxX - m_d->x);
}

qint32 KisPaintDevice::rowStride(qint32 x, qint32 y) const
{
    return m_datamanager->rowStride(x - m_d->x, y - m_d->y);
}

void KisPaintDevice::setX(qint32 x)
{
    m_d->x = x;
    m_d->cache.invalidate();
}

void KisPaintDevice::setY(qint32 y)
{
    m_d->y = y;
    m_d->cache.invalidate();
}

bool KisPaintDevice::fastBitBltPossible(KisPaintDeviceSP src)
{
    return m_d->x == src->x() && m_d->y == src->y() &&
            *colorSpace() == *src->colorSpace();
}

void KisPaintDevice::fastBitBlt(KisPaintDeviceSP src, const QRect &rect)
{
    Q_ASSERT(fastBitBltPossible(src));

    m_datamanager->bitBlt(src->dataManager(), rect.translated(-m_d->x, -m_d->y));
    m_d->cache.invalidate();
}

void KisPaintDevice::fastBitBltRough(KisPaintDeviceSP src, const QRect &rect)
{
    Q_ASSERT(fastBitBltPossible(src));

    m_datamanager->bitBltRough(src->dataManager(), rect.translated(-m_d->x, -m_d->y));
    m_d->cache.invalidate();
}

void KisPaintDevice::readBytes(quint8 * data, qint32 x, qint32 y, qint32 w, qint32 h) const
{
    m_datamanager->readBytes(data, x - m_d->x, y - m_d->y, w, h);
}

void KisPaintDevice::readBytes(quint8 * data, const QRect &rect)
{
    readBytes(data, rect.x(), rect.y(), rect.width(), rect.height());
}

void KisPaintDevice::writeBytes(const quint8 * data, qint32 x, qint32 y, qint32 w, qint32 h)
{
    m_datamanager->writeBytes(data, x - m_d->x, y - m_d->y, w, h);
    m_d->cache.invalidate();
}

void KisPaintDevice::writeBytes(const quint8 * data, const QRect &rect)
{
    writeBytes(data, rect.x(), rect.y(), rect.width(), rect.height());
}

QVector<quint8*> KisPaintDevice::readPlanarBytes(qint32 x, qint32 y, qint32 w, qint32 h)
{
    return m_datamanager->readPlanarBytes(channelSizes(), x, y, w, h);
}

void KisPaintDevice::writePlanarBytes(QVector<quint8*> planes, qint32 x, qint32 y, qint32 w, qint32 h)
{
    m_datamanager->writePlanarBytes(planes, channelSizes(), x, y, w, h);
    m_d->cache.invalidate();
}


KisDataManagerSP KisPaintDevice::dataManager() const
{
    return m_datamanager;
}


quint32 KisPaintDevice::pixelSize() const
{
    quint32 _pixelSize = m_d->colorSpace->pixelSize();
    Q_ASSERT(_pixelSize > 0);
    return _pixelSize;
}

quint32 KisPaintDevice::channelCount() const
{
    quint32 _channelCount = m_d->colorSpace->channelCount();
    Q_ASSERT(_channelCount > 0);
    return _channelCount;
}

KoColorSpace * KisPaintDevice::colorSpace()
{
    Q_ASSERT(m_d->colorSpace != 0);
    return m_d->colorSpace;
}

const KoColorSpace * KisPaintDevice::colorSpace() const
{
    Q_ASSERT(m_d->colorSpace != 0);
    return m_d->colorSpace;
}


qint32 KisPaintDevice::x() const
{
    return m_d->x;
}

qint32 KisPaintDevice::y() const
{
    return m_d->y;
}

QVector<qint32> KisPaintDevice::channelSizes()
{
    QVector<qint32> sizes;
    QList<KoChannelInfo*> channels = colorSpace()->channels();
    qSort(channels);

    foreach(KoChannelInfo * channelInfo, channels) {
        sizes.append(channelInfo->size());
    }
    return sizes;
}

#include "kis_paint_device.moc"
