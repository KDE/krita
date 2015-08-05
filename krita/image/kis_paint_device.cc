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
#include <QIODevice>
#include <qmath.h>

#include <klocale.h>

#include <KoChannelInfo.h>
#include <KoColorProfile.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoIntegerMaths.h>
#include <KoMixColorsOp.h>


#include "kis_global.h"
#include "kis_types.h"
#include "kis_random_sub_accessor.h"
#include "kis_selection.h"
#include "kis_node.h"
#include "kis_datamanager.h"
#include "kis_paint_device_writer.h"
#include "kis_selection_component.h"
#include "kis_pixel_selection.h"
#include "kis_repeat_iterators_pixel.h"
#include "kis_fixed_paint_device.h"

#include "tiles3/kis_hline_iterator.h"
#include "tiles3/kis_vline_iterator.h"
#include "tiles3/kis_random_accessor.h"

#include "kis_default_bounds.h"

#include "kis_lod_transform.h"

#include "kis_raster_keyframe_channel.h"

#include "kis_paint_device_cache.h"
#include "kis_paint_device_data.h"
#include "kis_paint_device_frames_interface.h"


struct KisPaintDevice::Private
{
    /**
     * Used when the paint device is loading to ensure no lod/animation
     * interferes the process.
     */
    static const KisDefaultBoundsSP transitionalDefaultBounds;

public:

    class KisPaintDeviceStrategy;
    class KisPaintDeviceWrappedStrategy;

    Private(KisPaintDevice *paintDevice);
    ~Private();

    KisPaintDevice *q;
    KisNodeWSP parent;
    QScopedPointer<KisRasterKeyframeChannel> contentChannel;
    KisDefaultBoundsBaseSP defaultBounds;
    QScopedPointer<KisPaintDeviceStrategy> basicStrategy;
    QScopedPointer<KisPaintDeviceWrappedStrategy> wrappedStrategy;
    QScopedPointer<KisPaintDeviceFramesInterface> framesInterface;

    KisPaintDeviceStrategy* currentStrategy();

    void init(const KoColorSpace *cs, const quint8 *defaultPixel);
    KUndo2Command* convertColorSpace(const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags);
    bool assignProfile(const KoColorProfile * profile);

    inline const KoColorSpace* colorSpace() const { return currentData()->colorSpace(); }
    inline KisDataManagerSP dataManager() const { return currentData()->dataManager(); }

    inline qint32 x() const {return currentData()->x();}
    inline qint32 y() const {return currentData()->y();}
    inline void setX(qint32 x) { currentData()->setX(x); }
    inline void setY(qint32 y) { currentData()->setY(y); }

    inline KisPaintDeviceCache* cache() { return currentData()->cache(); }


    void cloneAllDataObjects(Private *rhs, bool copyFrames) {

        m_lodData.reset();
        m_externalFrameData.reset();

        if (!m_frames.isEmpty()) {
            m_frames.clear();
        }

        if (!copyFrames) {
            if (m_data) {
                m_data->prepareClone(rhs->currentData(), true);
            } else {
                m_data = new KisPaintDeviceData(rhs->currentData(), true);
            }
        } else {
            if (m_data && !rhs->m_data) {
                delete m_data;
                m_data = 0;
            } else if (!m_data && rhs->m_data) {
                m_data = new KisPaintDeviceData(rhs->m_data, true);
            } else if (m_data && rhs->m_data) {
                m_data->prepareClone(rhs->m_data, true);
            }

            if (!rhs->m_frames.isEmpty()) {
                FramesHash::const_iterator it = rhs->m_frames.constBegin();
                FramesHash::const_iterator end = rhs->m_frames.constEnd();

                for (; it != end; ++it) {
                    DataSP data = toQShared(new KisPaintDeviceData(it.value().data(), true));
                    m_frames.insert(it.key(), data);
                }
            }
        }
    }

    void prepareClone(KisPaintDeviceSP src)
    {
        prepareCloneImpl(src, src->m_d->currentData());
        Q_ASSERT(fastBitBltPossible(src));
    }

    bool fastBitBltPossible(KisPaintDeviceSP src)
    {
        return fastBitBltPossibleImpl(src->m_d->currentData());
    }

    int currentFrameId() const {
        KIS_ASSERT_RECOVER(contentChannel) { return -1; }
        return !defaultBounds->currentLevelOfDetail() ?
            contentChannel->frameIdAt(defaultBounds->currentTime()) :
            -1;
    }

    KisDataManagerSP frameDataManager(int frameId) const {
        Data *data = m_frames[frameId].data();
        return data->dataManager();
    }

    void invalidateFrameCache(int frameId) {
        Data *data = m_frames[frameId].data();
        return data->cache()->invalidate();
    }

private:
    typedef KisPaintDeviceData Data;
    typedef QSharedPointer<Data> DataSP;
    typedef QHash<int, DataSP> FramesHash;

    class FrameInsertionCommand : public KUndo2Command {
    public:

        FrameInsertionCommand(FramesHash *hash, DataSP data, int frameId, bool insert, KUndo2Command *parentCommand)
            : KUndo2Command(parentCommand),
              m_hash(hash),
              m_data(data),
              m_frameId(frameId),
              m_insert(insert)
        {
        }

        void redo() {
            doSwap(m_insert);
        }

        void undo() {
            doSwap(!m_insert);
        }

    private:
        void doSwap(bool insert) {
            if (insert) {
                m_hash->insert(m_frameId, m_data);
            } else {
                DataSP deletedData = m_hash->take(m_frameId);
            }
        }

    private:
        FramesHash *m_hash;
        DataSP m_data;
        int m_frameId;
        bool m_insert;
    };

public:

    int createFrame(bool copy, int copySrc, const QPoint &offset, KUndo2Command *parentCommand)
    {
        KIS_ASSERT_RECOVER(parentCommand) { return -1; }

        Data *data;

        if (m_frames.isEmpty()) {
            data = m_data;
            m_data = 0;
        } else if (copy) {
            data = new Data(m_frames[copySrc].data(), true);
        } else {
            Data *srcData = m_frames.begin().value().data();
            data = new Data(srcData, false);
        }

        data->setX(offset.x());
        data->setY(offset.y());

        int frameId = nextFreeFrameId++;

        KUndo2Command *cmd =
            new FrameInsertionCommand(&m_frames,
                                      toQShared(data),
                                      frameId, true,
                                      parentCommand);

        cmd->redo();

        return frameId;
    }

    void deleteFrame(int frame, KUndo2Command *parentCommand)
    {
        KIS_ASSERT_RECOVER_RETURN(m_frames.contains(frame));
        KIS_ASSERT_RECOVER_RETURN(parentCommand);

        KUndo2Command *cmd =
            new FrameInsertionCommand(&m_frames,
                                      m_frames[frame],
                                      frame, false,
                                      parentCommand);
        cmd->redo();

        if (m_frames.isEmpty()) {
            // the original m_data will be deleted by shared poiter
            // when the command will be destroyed, so just create a
            // new one for m_data
            m_data = new Data(q);
        }
    }

    QRect frameBounds(int frameId)
    {
        Data *data = m_frames[frameId].data();

        QRect extent = data->dataManager()->extent();
        extent.translate(data->x(), data->y());

        return extent;
    }

    QPoint frameOffset(int frameId) const {
        Data *data = m_frames[frameId].data();
        return QPoint(data->x(), data->y());
    }

    void setFrameOffset(int frameId, const QPoint &offset) {
        Data *data = m_frames[frameId].data();
        data->setX(offset.x());
        data->setY(offset.y());
    }

    const QList<int> frameIds() const
    {
        return m_frames.keys();
    }

    bool readFrame(QIODevice *stream, int frameId) {
        bool retval = false;
        Data *data = m_frames[frameId].data();
        retval = data->dataManager()->read(stream);
        data->cache()->invalidate();
        return retval;
    }

    bool writeFrame(KisPaintDeviceWriter &store, int frameId) {
        Data *data = m_frames[frameId].data();
        return data->dataManager()->write(store);
    }

    void setFrameDefaultPixel(const quint8 *defPixel, int frameId) {
        Data *data = m_frames[frameId].data();
        data->dataManager()->setDefaultPixel(defPixel);
    }

    const quint8* frameDefaultPixel(int frameId) const {
        Data *data = m_frames[frameId].data();
        return data->dataManager()->defaultPixel();
    }

    void fetchFrame(int frameId, KisPaintDeviceSP targetDevice);

    QRegion syncLodData(int newLod);

private:

    QRegion syncWholeDevice(Data *srcData);

    inline Data* currentNonLodData() const {
        Data *data = m_data;

        if (contentChannel) {
            if (contentChannel->keyframeCount() > 1) {
                int frameId = contentChannel->frameIdAt(defaultBounds->currentTime());
                KIS_ASSERT_RECOVER(m_frames.contains(frameId)) { return data; }
                data = m_frames[frameId].data();
            } else {
                data = m_frames.begin().value().data();
            }

            // sanity check!
            KIS_ASSERT_RECOVER_NOOP(!m_data);

        } else if (defaultBounds->externalFrameActive()) {
            if (!m_externalFrameData) {
                QMutexLocker l(&m_dataSwitchLock);
                if (!m_externalFrameData) {
                    m_externalFrameData.reset(new Data(m_data, false));
                }
            }
            data = m_externalFrameData.data();
        }

        return data;
    }

    inline Data* currentData() const {
        Data *data = m_data;

        if (defaultBounds->currentLevelOfDetail()) {
            if (!m_lodData) {
                QMutexLocker l(&m_dataSwitchLock);
                if (!m_lodData) {
                    m_lodData.reset(new Data(m_data, false));
                }
            }
            data = m_lodData.data();
        } else {
            data = currentNonLodData();
        }

        return data;
    }

    void prepareCloneImpl(KisPaintDeviceSP src, Data *srcData)
    {
        currentData()->prepareClone(srcData);

        q->setDefaultPixel(srcData->dataManager()->defaultPixel());
        q->setDefaultBounds(src->defaultBounds());
    }

    bool fastBitBltPossibleImpl(Data *srcData)
    {
        return x() == srcData->x() && y() == srcData->y() &&
            *colorSpace() == *srcData->colorSpace();
    }

    QList<Data*> allDataObjects() const {
        QList<Data*> dataObjects;

        if (m_frames.isEmpty()) {
            dataObjects << m_data;
        }
        dataObjects << m_lodData.data();
        dataObjects << m_externalFrameData.data();

        foreach (DataSP value, m_frames.values()) {
            dataObjects << value.data();
        }

        return dataObjects;
    }

    struct StrategyPolicy;
    typedef KisSequentialIteratorBase<ReadOnlyIteratorPolicy<StrategyPolicy>, StrategyPolicy> InternalSequentialConstIterator;
    typedef KisSequentialIteratorBase<WritableIteratorPolicy<StrategyPolicy>, StrategyPolicy> InternalSequentialIterator;

private:
    friend class KisPaintDeviceFramesInterface;

private:
    Data *m_data;
    mutable QScopedPointer<Data> m_lodData;
    mutable QScopedPointer<Data> m_externalFrameData;
    mutable QMutex m_dataSwitchLock;

    FramesHash m_frames;
    int nextFreeFrameId;
};

const KisDefaultBoundsSP KisPaintDevice::Private::transitionalDefaultBounds = new KisDefaultBounds();

#include "kis_paint_device_strategies.h"

KisPaintDevice::Private::Private(KisPaintDevice *paintDevice)
    : q(paintDevice),
      basicStrategy(new KisPaintDeviceStrategy(paintDevice, this)),
      m_data(new Data(paintDevice)),
      nextFreeFrameId(0)
{
}

KisPaintDevice::Private::~Private()
{
    delete m_data;
    m_frames.clear();
}

KisPaintDevice::Private::KisPaintDeviceStrategy* KisPaintDevice::Private::currentStrategy()
{
    if (!defaultBounds->wrapAroundMode()) {
        return basicStrategy.data();
    }

    QRect wrapRect = defaultBounds->bounds();
    if (!wrappedStrategy || wrappedStrategy->wrapRect() != wrapRect) {
        wrappedStrategy.reset(new KisPaintDeviceWrappedStrategy(defaultBounds->bounds(), q, this));
    }

    return wrappedStrategy.data();
}

inline int coordToLodCoord(int x, int lod) {
    return x >> lod;
}

QRegion KisPaintDevice::Private::syncLodData(int newLod)
{
    Data *srcData = currentNonLodData();

    if (!m_lodData) {
        m_lodData.reset(new Data(srcData, false));
    }

    int expectedX = coordToLodCoord(srcData->x(), newLod);
    int expectedY = coordToLodCoord(srcData->y(), newLod);

    /**
     * We compare color spaces as pure pointers, because they must be
     * exactly the same, since they come from the common source.
     */
    if (m_lodData->levelOfDetail() != newLod ||
        m_lodData->colorSpace() != srcData->colorSpace() ||
        m_lodData->x() != expectedX ||
        m_lodData->y() != expectedY) {


        m_lodData->prepareClone(srcData);

        m_lodData->setLevelOfDetail(newLod);
        m_lodData->setX(expectedX);
        m_lodData->setY(expectedY);

        // FIXME: different kind of synchronization
    }

    QRegion dirtyRegion = syncWholeDevice(srcData);
    m_lodData->cache()->invalidate();

    return dirtyRegion;
}

struct KisPaintDevice::Private::StrategyPolicy {
    StrategyPolicy(KisPaintDevice::Private::KisPaintDeviceStrategy *strategy,
                   KisDataManager *dataManager)
        : m_strategy(strategy), m_dataManager(dataManager) {}

    KisHLineConstIteratorSP createConstIterator(const QRect &rect) {
        return m_strategy->createHLineConstIteratorNG(m_dataManager, rect.x(), rect.y(), rect.width());
    }

    KisHLineIteratorSP createIterator(const QRect &rect) {
        return m_strategy->createHLineIteratorNG(m_dataManager, rect.x(), rect.y(), rect.width());
    }

    int pixelSize() const {
        return m_dataManager->pixelSize();
    }


    KisPaintDeviceStrategy *m_strategy;
    KisDataManager *m_dataManager;
};

QRegion KisPaintDevice::Private::syncWholeDevice(Data *srcData)
{
    KIS_ASSERT_RECOVER(m_lodData) { return QRegion(); }

    m_lodData->dataManager()->clear();

    int lod = m_lodData->levelOfDetail();
    int srcStepSize = 1 << lod;

    // FIXME:
    QRect rcFIXME= srcData->dataManager()->extent().translated(srcData->x(), srcData->y());
    if (!rcFIXME.isValid()) return QRegion();

    QRect srcRect = KisLodTransform::alignedRect(rcFIXME, lod);
    QRect dstRect = KisLodTransform::scaledRect(srcRect, lod);
    if (!srcRect.isValid() || !dstRect.isValid()) return QRegion();

    KIS_ASSERT_RECOVER_NOOP(srcRect.width() / srcStepSize == dstRect.width());

    const int pixelSize = srcData->dataManager()->pixelSize();

    int rowsAccumulated = 0;
    int columnsAccumulated = 0;

    KoMixColorsOp *mixOp = colorSpace()->mixColorsOp();

    QScopedArrayPointer<quint8> blendData(new quint8[srcStepSize * srcRect.width() * pixelSize]);
    quint8 *blendDataPtr = blendData.data();
    int blendDataOffset = 0;

    const int srcCellSize = srcStepSize * srcStepSize;
    const int srcCellStride = srcCellSize * pixelSize;
    const int srcStepStride = srcStepSize * pixelSize;
    const int srcColumnStride = (srcStepSize - 1) * srcStepStride;

    QScopedArrayPointer<qint16> weights(new qint16[srcCellSize]);

    {
        const qint16 averageWeight = qCeil(255.0 / srcCellSize);
        const qint16 extraWeight = averageWeight * srcCellSize - 255;
        KIS_ASSERT_RECOVER_NOOP(extraWeight == 1);

        for (int i = 0; i < srcCellSize - 1; i++) {
            weights[i] = averageWeight;
        }
        weights[srcCellSize - 1] = averageWeight - extraWeight;
    }

    InternalSequentialConstIterator srcIntIt(StrategyPolicy(currentStrategy(), srcData->dataManager().data()), srcRect);
    InternalSequentialIterator dstIntIt(StrategyPolicy(currentStrategy(), m_lodData->dataManager().data()), dstRect);

    int rowsRemaining = srcRect.height();
    while (rowsRemaining > 0) {

        int colsRemaining = srcRect.width();
        while (colsRemaining > 0) {

            memcpy(blendDataPtr, srcIntIt.rawDataConst(), pixelSize);
            blendDataPtr += pixelSize;
            columnsAccumulated++;

            if (columnsAccumulated >= srcStepSize) {
                blendDataPtr += srcColumnStride;
                columnsAccumulated = 0;
            }

            srcIntIt.nextPixel();
            colsRemaining--;
        }

        rowsAccumulated++;

        if (rowsAccumulated >= srcStepSize) {

            // blend and write the final data
            blendDataPtr = blendData.data();
            for (int i = 0; i < dstRect.width(); i++) {
                mixOp->mixColors(blendDataPtr, weights.data(), srcCellSize, dstIntIt.rawData());

                blendDataPtr += srcCellStride;
                dstIntIt.nextPixel();
            }

            // reset counters
            rowsAccumulated = 0;
            blendDataPtr = blendData.data();
            blendDataOffset = 0;
        } else {
            blendDataOffset += srcStepStride;
            blendDataPtr = blendData.data() + blendDataOffset;
        }

        rowsRemaining--;
    }

    QRegion dirtyRegion(dstRect);
    return dirtyRegion;
}

void KisPaintDevice::Private::fetchFrame(int frameId, KisPaintDeviceSP targetDevice)
{
    Data *data = m_frames[frameId].data();

    QRect extent = data->dataManager()->extent();
    extent.translate(data->x(), data->y());

    targetDevice->m_d->prepareCloneImpl(q, data);
    targetDevice->m_d->currentStrategy()->fastBitBltRough(data->dataManager(), extent);
}

KUndo2Command* KisPaintDevice::Private::convertColorSpace(const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags) {

    class DeviceChangeColorSpaceCommand : public KUndo2Command {
    public:
        DeviceChangeColorSpaceCommand(KisPaintDeviceSP device)
            : m_firstRun(true),
              m_device(device)
            {
            }

        void emitNotifications() {
            m_device->emitColorSpaceChanged();
            m_device->setDirty();
        }

        void redo() {
            KUndo2Command::redo();

            if (!m_firstRun) {
                m_firstRun = false;
                return;
            }

            emitNotifications();
        }

        void undo() {
            emitNotifications();
        }

    private:
        bool m_firstRun;
        KisPaintDeviceSP m_device;
    };


    KUndo2Command *parentCommand = new DeviceChangeColorSpaceCommand(q);

    QList<Data*> dataObjects = allDataObjects();;

    foreach (Data *data, dataObjects) {
        if (!data) continue;

        data->convertDataColorSpace(dstColorSpace, renderingIntent, conversionFlags, parentCommand);
    }

    if (!parentCommand->childCount()) {
        delete parentCommand;
        parentCommand = 0;
    } else {
        q->emitColorSpaceChanged();
    }

    return parentCommand;

}

bool KisPaintDevice::Private::assignProfile(const KoColorProfile * profile)
{
    if (!profile) return false;

    const KoColorSpace *dstColorSpace =
        KoColorSpaceRegistry::instance()->colorSpace(colorSpace()->colorModelId().id(), colorSpace()->colorDepthId().id(), profile);
    if (!dstColorSpace) return false;

    QList<Data*> dataObjects = allDataObjects();;
    foreach (Data *data, dataObjects) {
        if (!data) continue;
        data->assignColorSpace(dstColorSpace);
    }
    q->emitProfileChanged();

    // no undo information is provided here
    return true;
}

void KisPaintDevice::Private::init(const KoColorSpace *cs, const quint8 *defaultPixel)
{
    QList<Data*> dataObjects = allDataObjects();;
    foreach (Data *data, dataObjects) {
        if (!data) continue;

        KisDataManagerSP dataManager = new KisDataManager(cs->pixelSize(), defaultPixel);
        data->init(cs, dataManager);
    }
}

KisPaintDevice::KisPaintDevice(const KoColorSpace * colorSpace, const QString& name)
    : QObject(0)
    , m_d(new Private(this))
{
    init(colorSpace, new KisDefaultBounds(), 0, name);
}

KisPaintDevice::KisPaintDevice(KisNodeWSP parent, const KoColorSpace * colorSpace, KisDefaultBoundsBaseSP defaultBounds, const QString& name)
    : QObject(0)
    , m_d(new Private(this))
{
    init(colorSpace, defaultBounds, parent, name);
}

void KisPaintDevice::init(const KoColorSpace *colorSpace,
                          KisDefaultBoundsBaseSP defaultBounds,
                          KisNodeWSP parent, const QString& name)
{
    Q_ASSERT(colorSpace);
    setObjectName(name);

    // temporary def. bounds object for the initialization phase only
    m_d->defaultBounds = m_d->transitionalDefaultBounds;

    if (!defaultBounds) {
        // Reuse transitionalDefaultBounds here. Change if you change
        // semantics of transitionalDefaultBounds
        defaultBounds = m_d->transitionalDefaultBounds;
    }

    QScopedArrayPointer<quint8> defaultPixel(new quint8[colorSpace->pixelSize()]);
    colorSpace->fromQColor(Qt::transparent, defaultPixel.data());
    m_d->init(colorSpace, defaultPixel.data());

    Q_ASSERT(m_d->colorSpace());

    setDefaultBounds(defaultBounds);
    setParentNode(parent);
}

KisPaintDevice::KisPaintDevice(const KisPaintDevice& rhs, bool copyFrames)
    : QObject()
    , KisShared()
    , m_d(new Private(this))
{
    if (this != &rhs) {
        // temporary def. bounds object for the initialization phase only
        m_d->defaultBounds = m_d->transitionalDefaultBounds;

        // copy data objects with or without frames
        m_d->cloneAllDataObjects(rhs.m_d, copyFrames);

        setDefaultBounds(rhs.m_d->defaultBounds);
        setParentNode(0);
    }
}

KisPaintDevice::~KisPaintDevice()
{
    delete m_d;
}

void KisPaintDevice::prepareClone(KisPaintDeviceSP src)
{
    m_d->prepareClone(src);
    Q_ASSERT(fastBitBltPossible(src));
}

void KisPaintDevice::makeCloneFrom(KisPaintDeviceSP src, const QRect &rect)
{
    prepareClone(src);

     // we guarantee that *this is totally empty, so copy pixels that
    // are areally present on the source image only
    const QRect optimizedRect = rect & src->extent();

    fastBitBlt(src, optimizedRect);
}

void KisPaintDevice::makeCloneFromRough(KisPaintDeviceSP src, const QRect &minimalRect)
{
    prepareClone(src);

    // we guarantee that *this is totally empty, so copy pixels that
    // are areally present on the source image only
    const QRect optimizedRect = minimalRect & src->extent();

    fastBitBltRough(src, optimizedRect);
}

void KisPaintDevice::setDirty()
{
    m_d->cache()->invalidate();
    if (m_d->parent.isValid())
        m_d->parent->setDirty();
}

void KisPaintDevice::setDirty(const QRect & rc)
{
    m_d->cache()->invalidate();
    if (m_d->parent.isValid())
        m_d->parent->setDirty(rc);
}

void KisPaintDevice::setDirty(const QRegion & region)
{
    m_d->cache()->invalidate();
    if (m_d->parent.isValid())
        m_d->parent->setDirty(region);
}

void KisPaintDevice::setDirty(const QVector<QRect> rects)
{
    m_d->cache()->invalidate();
    if (m_d->parent.isValid())
        m_d->parent->setDirty(rects);
}

void KisPaintDevice::requestTimeSwitch(int time)
{
    if (m_d->parent.isValid()) {
        m_d->parent->requestTimeSwitch(time);
    }
}

void KisPaintDevice::setParentNode(KisNodeWSP parent)
{
    m_d->parent = parent;
}

// for testing purposes only
KisNodeWSP KisPaintDevice::parentNode() const
{
    return m_d->parent;
}

void KisPaintDevice::setDefaultBounds(KisDefaultBoundsBaseSP defaultBounds)
{
    m_d->defaultBounds = defaultBounds;
    m_d->cache()->invalidate();
}

KisDefaultBoundsBaseSP KisPaintDevice::defaultBounds() const
{
    return m_d->defaultBounds;
}

void KisPaintDevice::move(const QPoint &pt)
{
    m_d->currentStrategy()->move(pt);
}

void KisPaintDevice::move(qint32 x, qint32 y)
{
    move(QPoint(x, y));
}

void KisPaintDevice::setX(qint32 x)
{
    move(QPoint(x, m_d->y()));
}

void KisPaintDevice::setY(qint32 y)
{
    move(QPoint(m_d->x(), y));
}

qint32 KisPaintDevice::x() const
{
    return m_d->x();
}

qint32 KisPaintDevice::y() const
{
    return m_d->y();
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
    return m_d->currentStrategy()->extent();
}

QRegion KisPaintDevice::region() const
{
    return m_d->currentStrategy()->region();
}

QRect KisPaintDevice::nonDefaultPixelArea() const
{
    return m_d->cache()->nonDefaultPixelArea();
}

QRect KisPaintDevice::exactBounds() const
{
    return m_d->cache()->exactBounds();
}

namespace Impl {

struct CheckFullyTransparent
{
    CheckFullyTransparent(const KoColorSpace *colorSpace)
        : m_colorSpace(colorSpace)
    {
    }

    bool isPixelEmpty(const quint8 *pixelData) {
        return m_colorSpace->opacityU8(pixelData) == OPACITY_TRANSPARENT_U8;
    }

private:
    const KoColorSpace *m_colorSpace;
};

struct CheckNonDefault
{
    CheckNonDefault(int pixelSize, const quint8 *defaultPixel)
        : m_pixelSize(pixelSize),
          m_defaultPixel(defaultPixel)
    {
    }

    bool isPixelEmpty(const quint8 *pixelData) {
        return memcmp(m_defaultPixel, pixelData, m_pixelSize) == 0;
    }

private:
    int m_pixelSize;
    const quint8 *m_defaultPixel;
};

template <class ComparePixelOp>
QRect calculateExactBoundsImpl(const KisPaintDevice *device, const QRect &startRect, const QRect &endRect, ComparePixelOp compareOp)
{
    if (startRect == endRect) return startRect;

    // Solution n°2
    int  x, y, w, h;
    int boundLeft, boundTop, boundRight, boundBottom;
    int endDirN, endDirE, endDirS, endDirW;

    startRect.getRect(&x, &y, &w, &h);

    if (endRect.isEmpty()) {
        endDirS = startRect.bottom();
        endDirN = startRect.top();
        endDirE = startRect.right();
        endDirW = startRect.left();
        startRect.getCoords(&boundLeft, &boundTop, &boundRight, &boundBottom);
    } else {
        endDirS = endRect.top() - 1;
        endDirN = endRect.bottom() + 1;
        endDirE = endRect.left() - 1;
        endDirW = endRect.right() + 1;
        endRect.getCoords(&boundLeft, &boundTop, &boundRight, &boundBottom);
    }

    // XXX: a small optimization is possible by using H/V line iterators in the first
    //      and third cases, at the cost of making the code a bit more complex

    KisRandomConstAccessorSP accessor = device->createRandomConstAccessorNG(x, y);

    bool found = false;
    {
        for (qint32 y2 = y; y2 <= endDirS; ++y2) {
            for (qint32 x2 = x; x2 < x + w || found; ++ x2) {
                accessor->moveTo(x2, y2);
                if (!compareOp.isPixelEmpty(accessor->rawDataConst())) {
                    boundTop = y2;
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
    }

    /**
     * If the first pass hasn't found any opaque pixel, there is no
     * reason to check that 3 more times. They will not appear in the
     * meantime. Just return an empty bounding rect.
     */
    if (!found && endRect.isEmpty()) {
        return QRect();
    }

    found = false;

    for (qint32 y2 = y + h - 1; y2 >= endDirN ; --y2) {
        for (qint32 x2 = x + w - 1; x2 >= x || found; --x2) {
            accessor->moveTo(x2, y2);
            if (!compareOp.isPixelEmpty(accessor->rawDataConst())) {
                boundBottom = y2;
                found = true;
                break;
            }
        }
        if (found) break;
    }
    found = false;

    {
        for (qint32 x2 = x; x2 <= endDirE ; ++x2) {
            for (qint32 y2 = y; y2 < y + h || found; ++y2) {
                accessor->moveTo(x2, y2);
                if (!compareOp.isPixelEmpty(accessor->rawDataConst())) {
                    boundLeft = x2;
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

        for (qint32 x2 = x + w - 1; x2 >= endDirW; --x2) {
            for (qint32 y2 = y + h -1; y2 >= y || found; --y2) {
                accessor->moveTo(x2, y2);
                if (!compareOp.isPixelEmpty(accessor->rawDataConst())) {
                    boundRight = x2;
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
    }

    return QRect(boundLeft, boundTop,
                 boundRight - boundLeft + 1,
                 boundBottom - boundTop + 1);
}

}

QRect KisPaintDevice::calculateExactBounds(bool nonDefaultOnly) const
{
    QRect startRect = extent();
    QRect endRect;

    quint8 defaultOpacity = m_d->colorSpace()->opacityU8(defaultPixel());
    if(defaultOpacity != OPACITY_TRANSPARENT_U8) {
        if (!nonDefaultOnly) {
            /**
             * We will calculate exact bounds only outside of the
             * image bounds, and that'll be nondefault area only.
             */

            endRect = defaultBounds()->bounds();
            nonDefaultOnly = true;

        } else {
            startRect = region().boundingRect();
        }
    }

    if (nonDefaultOnly) {
        Impl::CheckNonDefault compareOp(pixelSize(), defaultPixel());
        endRect = Impl::calculateExactBoundsImpl(this, startRect, endRect, compareOp);
    } else {
        Impl::CheckFullyTransparent compareOp(m_d->colorSpace());
        endRect = Impl::calculateExactBoundsImpl(this, startRect, endRect, compareOp);
    }

    return endRect;
}


void KisPaintDevice::crop(qint32 x, qint32 y, qint32 w, qint32 h)
{
    crop(QRect(x, y, w, h));
}


void KisPaintDevice::crop(const QRect &rect)
{
    m_d->currentStrategy()->crop(rect);
}

void KisPaintDevice::purgeDefaultPixels()
{
    KisDataManagerSP dm = m_d->dataManager();
    dm->purge(dm->extent());
}

void KisPaintDevice::setDefaultPixel(const quint8 *defPixel)
{
    m_d->dataManager()->setDefaultPixel(defPixel);
    m_d->cache()->invalidate();
}

const quint8 *KisPaintDevice::defaultPixel() const
{
    return m_d->dataManager()->defaultPixel();
}

void KisPaintDevice::clear()
{
    m_d->dataManager()->clear();
    m_d->cache()->invalidate();
}

void KisPaintDevice::clear(const QRect & rc)
{
    m_d->currentStrategy()->clear(rc);
}

void KisPaintDevice::fill(const QRect & rc, const KoColor &color)
{
    Q_ASSERT(*color.colorSpace() == *colorSpace());
    m_d->currentStrategy()->fill(rc, color.data());
}

void KisPaintDevice::fill(qint32 x, qint32 y, qint32 w, qint32 h, const quint8 *fillPixel)
{
    m_d->currentStrategy()->fill(QRect(x, y, w, h), fillPixel);
}


bool KisPaintDevice::write(KisPaintDeviceWriter &store)
{
    return m_d->dataManager()->write(store);
}

bool KisPaintDevice::read(QIODevice *stream)
{
    bool retval;

    retval = m_d->dataManager()->read(stream);
    m_d->cache()->invalidate();

    return retval;
}

void KisPaintDevice::emitColorSpaceChanged()
{
    emit colorSpaceChanged(m_d->colorSpace());
}

void KisPaintDevice::emitProfileChanged()
{
    emit profileChanged(m_d->colorSpace()->profile());
}

KUndo2Command* KisPaintDevice::convertTo(const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    KUndo2Command *command = m_d->convertColorSpace(dstColorSpace, renderingIntent, conversionFlags);
    return command;
}

bool KisPaintDevice::setProfile(const KoColorProfile * profile)
{
    return m_d->assignProfile(profile);
}

KisDataManagerSP KisPaintDevice::dataManager() const
{
    return m_d->dataManager();
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
#if QT_VERSION >= 0x040700
        writeBytes(image.constBits(), offsetX, offsetY, image.width(), image.height());
#else
        writeBytes(image.bits(), offsetX, offsetY, image.width(), image.height());
#endif
    } else {
        try {
            quint8 * dstData = new quint8[image.width() * image.height() * pixelSize()];
            KoColorSpaceRegistry::instance()
                    ->colorSpace(RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), profile)
#if QT_VERSION >= 0x040700
                    ->convertPixelsTo(image.constBits(), dstData, colorSpace(), image.width() * image.height(),
#else
                    ->convertPixelsTo(image.bits(), dstData, colorSpace(), image.width() * image.height(),
#endif
                                      KoColorConversionTransformation::InternalRenderingIntent,
                                      KoColorConversionTransformation::InternalConversionFlags);

            writeBytes(dstData, offsetX, offsetY, image.width(), image.height());
            delete[] dstData;
        } catch (std::bad_alloc) {
            warnKrita << "KisPaintDevice::convertFromQImage: Could not allocate" << image.width() * image.height() * pixelSize() << "bytes";
            return;
        }
    }
    m_d->cache()->invalidate();
}

QImage KisPaintDevice::convertToQImage(const KoColorProfile *dstProfile, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags) const
{
    qint32 x1;
    qint32 y1;
    qint32 w;
    qint32 h;

    QRect rc = exactBounds();
    x1 = rc.x();
    y1 = rc.y();
    w = rc.width();
    h = rc.height();

    return convertToQImage(dstProfile, x1, y1, w, h, renderingIntent, conversionFlags);
}

QImage KisPaintDevice::convertToQImage(const KoColorProfile *dstProfile,
                                       const QRect &rc,
                                       KoColorConversionTransformation::Intent renderingIntent,
                                       KoColorConversionTransformation::ConversionFlags conversionFlags) const
{
    return convertToQImage(dstProfile,
                           rc.x(), rc.y(), rc.width(), rc.height(),
                           renderingIntent, conversionFlags);
}

QImage KisPaintDevice::convertToQImage(const KoColorProfile *  dstProfile, qint32 x1, qint32 y1, qint32 w, qint32 h, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags) const
{

    if (w < 0)
        return QImage();

    if (h < 0)
        return QImage();

    quint8 *data = 0;
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
    QImage image = colorSpace()->convertToQImage(data, w, h, dstProfile, renderingIntent, conversionFlags);
    delete[] data;

    return image;
}

KisPaintDeviceSP KisPaintDevice::createThumbnailDevice(qint32 w, qint32 h, QRect rect) const
{
    KisPaintDeviceSP thumbnail = new KisPaintDevice(colorSpace());

    int srcWidth, srcHeight;
    int srcX0, srcY0;
    QRect e = rect.isValid() ? rect : extent();
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

    KisRandomConstAccessorSP iter = createRandomConstAccessorNG(0, 0);
    KisRandomAccessorSP dstIter = thumbnail->createRandomAccessorNG(0, 0);

    for (qint32 y = 0; y < h; ++y) {
        qint32 iY = srcY0 + (y * srcHeight) / h;
        for (qint32 x = 0; x < w; ++x) {
            qint32 iX = srcX0 + (x * srcWidth) / w;
            iter->moveTo(iX, iY);
            dstIter->moveTo(x,  y);
            memcpy(dstIter->rawData(), iter->rawDataConst(), pixelSize);
        }
    }
    return thumbnail;

}

QImage KisPaintDevice::createThumbnail(qint32 w, qint32 h, QRect rect, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    KisPaintDeviceSP dev = createThumbnailDevice(w, h, rect);
    QImage thumbnail = dev->convertToQImage(KoColorSpaceRegistry::instance()->rgb8()->profile(), 0, 0, w, h, renderingIntent, conversionFlags);
    return thumbnail;
}

QImage KisPaintDevice::createThumbnail(qint32 w, qint32 h, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    return m_d->cache()->createThumbnail(w, h, renderingIntent, conversionFlags);
}

KisHLineIteratorSP KisPaintDevice::createHLineIteratorNG(qint32 x, qint32 y, qint32 w)
{
    m_d->cache()->invalidate();
    return m_d->currentStrategy()->createHLineIteratorNG(m_d->dataManager().data(), x, y, w);
}

KisHLineConstIteratorSP KisPaintDevice::createHLineConstIteratorNG(qint32 x, qint32 y, qint32 w) const
{
    return m_d->currentStrategy()->createHLineConstIteratorNG(m_d->dataManager().data(), x, y, w);
}

KisVLineIteratorSP KisPaintDevice::createVLineIteratorNG(qint32 x, qint32 y, qint32 w)
{
    m_d->cache()->invalidate();
    return m_d->currentStrategy()->createVLineIteratorNG(x, y, w);
}

KisVLineConstIteratorSP KisPaintDevice::createVLineConstIteratorNG(qint32 x, qint32 y, qint32 w) const
{
    return m_d->currentStrategy()->createVLineConstIteratorNG(x, y, w);
}

KisRepeatHLineConstIteratorSP KisPaintDevice::createRepeatHLineConstIterator(qint32 x, qint32 y, qint32 w, const QRect& _dataWidth) const
{
    return new KisRepeatHLineConstIteratorNG(m_d->dataManager().data(), x, y, w, m_d->x(), m_d->y(), _dataWidth);
}

KisRepeatVLineConstIteratorSP KisPaintDevice::createRepeatVLineConstIterator(qint32 x, qint32 y, qint32 h, const QRect& _dataWidth) const
{
    return new KisRepeatVLineConstIteratorNG(m_d->dataManager().data(), x, y, h, m_d->x(), m_d->y(), _dataWidth);
}

KisRandomAccessorSP KisPaintDevice::createRandomAccessorNG(qint32 x, qint32 y)
{
    m_d->cache()->invalidate();
    return m_d->currentStrategy()->createRandomAccessorNG(x, y);
}

KisRandomConstAccessorSP KisPaintDevice::createRandomConstAccessorNG(qint32 x, qint32 y) const
{
    return m_d->currentStrategy()->createRandomConstAccessorNG(x, y);
}

KisRandomSubAccessorSP KisPaintDevice::createRandomSubAccessor() const
{
    KisPaintDevice* pd = const_cast<KisPaintDevice*>(this);
    return new KisRandomSubAccessor(pd);
}

void KisPaintDevice::clearSelection(KisSelectionSP selection)
{
    const KoColorSpace *colorSpace = m_d->colorSpace();
    QRect r = selection->selectedExactRect() & m_d->defaultBounds->bounds();

    if (r.isValid()) {

        KisHLineIteratorSP devIt = createHLineIteratorNG(r.x(), r.y(), r.width());
        KisHLineConstIteratorSP selectionIt = selection->projection()->createHLineConstIteratorNG(r.x(), r.y(), r.width());

        const quint8* defaultPixel_ = defaultPixel();
        bool transparentDefault = (colorSpace->opacityU8(defaultPixel_) == OPACITY_TRANSPARENT_U8);
        for (qint32 y = 0; y < r.height(); y++) {

            do {
                // XXX: Optimize by using stretches
                colorSpace->applyInverseAlphaU8Mask(devIt->rawData(), selectionIt->rawDataConst(), 1);
                if (transparentDefault && colorSpace->opacityU8(devIt->rawData()) == OPACITY_TRANSPARENT_U8) {
                    memcpy(devIt->rawData(), defaultPixel_, colorSpace->pixelSize());
                }
            } while (devIt->nextPixel() && selectionIt->nextPixel());
            devIt->nextRow();
            selectionIt->nextRow();
        }
        m_d->dataManager()->purge(r.translated(-m_d->x(), -m_d->y()));
        setDirty(r);
    }
}

bool KisPaintDevice::pixel(qint32 x, qint32 y, QColor *c) const
{
    KisHLineConstIteratorSP iter = createHLineConstIteratorNG(x, y, 1);

    const quint8 *pix = iter->rawDataConst();

    if (!pix) return false;

    colorSpace()->toQColor(pix, c);

    return true;
}


bool KisPaintDevice::pixel(qint32 x, qint32 y, KoColor * kc) const
{
    KisHLineConstIteratorSP iter = createHLineConstIteratorNG(x, y, 1);

    const quint8 *pix = iter->rawDataConst();

    if (!pix) return false;

    kc->setColor(pix, m_d->colorSpace());

    return true;
}

bool KisPaintDevice::setPixel(qint32 x, qint32 y, const QColor& c)
{
    KisHLineIteratorSP iter = createHLineIteratorNG(x, y, 1);

    colorSpace()->fromQColor(c, iter->rawData());
    m_d->cache()->invalidate();
    return true;
}

bool KisPaintDevice::setPixel(qint32 x, qint32 y, const KoColor& kc)
{
    const quint8 * pix;
    KisHLineIteratorSP iter = createHLineIteratorNG(x, y, 1);
    if (kc.colorSpace() != m_d->colorSpace()) {
        KoColor kc2(kc, m_d->colorSpace());
        pix = kc2.data();
        memcpy(iter->rawData(), pix, m_d->colorSpace()->pixelSize());
    } else {
        pix = kc.data();
        memcpy(iter->rawData(), pix, m_d->colorSpace()->pixelSize());
    }
    m_d->cache()->invalidate();
    return true;
}

bool KisPaintDevice::fastBitBltPossible(KisPaintDeviceSP src)
{
    return m_d->fastBitBltPossible(src);
}

void KisPaintDevice::fastBitBlt(KisPaintDeviceSP src, const QRect &rect)
{
    m_d->currentStrategy()->fastBitBlt(src, rect);
}

void KisPaintDevice::fastBitBltOldData(KisPaintDeviceSP src, const QRect &rect)
{
    m_d->currentStrategy()->fastBitBltOldData(src, rect);
}

void KisPaintDevice::fastBitBltRough(KisPaintDeviceSP src, const QRect &rect)
{
    m_d->currentStrategy()->fastBitBltRough(src, rect);
}

void KisPaintDevice::fastBitBltRoughOldData(KisPaintDeviceSP src, const QRect &rect)
{
    m_d->currentStrategy()->fastBitBltRoughOldData(src, rect);
}

void KisPaintDevice::readBytes(quint8 * data, qint32 x, qint32 y, qint32 w, qint32 h) const
{
    readBytes(data, QRect(x, y, w, h));
}

void KisPaintDevice::readBytes(quint8 *data, const QRect &rect) const
{
    m_d->currentStrategy()->readBytes(data, rect);
}

void KisPaintDevice::writeBytes(const quint8 *data, qint32 x, qint32 y, qint32 w, qint32 h)
{
    writeBytes(data, QRect(x, y, w, h));
}

void KisPaintDevice::writeBytes(const quint8 *data, const QRect &rect)
{
    m_d->currentStrategy()->writeBytes(data, rect);
}

QVector<quint8*> KisPaintDevice::readPlanarBytes(qint32 x, qint32 y, qint32 w, qint32 h) const
{
    return m_d->currentStrategy()->readPlanarBytes(x, y, w, h);
}

void KisPaintDevice::writePlanarBytes(QVector<quint8*> planes, qint32 x, qint32 y, qint32 w, qint32 h)
{
    m_d->currentStrategy()->writePlanarBytes(planes, x, y, w, h);
}


quint32 KisPaintDevice::pixelSize() const
{
    quint32 _pixelSize = m_d->colorSpace()->pixelSize();
    Q_ASSERT(_pixelSize > 0);
    return _pixelSize;
}

quint32 KisPaintDevice::channelCount() const
{
    quint32 _channelCount = m_d->colorSpace()->channelCount();
    Q_ASSERT(_channelCount > 0);
    return _channelCount;
}

KisRasterKeyframeChannel *KisPaintDevice::createKeyframeChannel(const KoID &id, const KisNodeWSP node)
{
    Q_ASSERT(!m_d->framesInterface);
    m_d->framesInterface.reset(new KisPaintDeviceFramesInterface(this));

    Q_ASSERT(!m_d->contentChannel);
    m_d->contentChannel.reset(new KisRasterKeyframeChannel(id, node, this));

    // Raster channels always have at least one frame (representing a static image)
    KUndo2Command tempParentCommand;
    m_d->contentChannel->addKeyframe(0, &tempParentCommand);

    return m_d->contentChannel.data();
}

KisRasterKeyframeChannel* KisPaintDevice::keyframeChannel() const
{
    Q_ASSERT(m_d->contentChannel);
    return m_d->contentChannel.data();
}

const KoColorSpace* KisPaintDevice::colorSpace() const
{
    Q_ASSERT(m_d->colorSpace() != 0);
    return m_d->colorSpace();
}

KisPaintDeviceSP KisPaintDevice::createCompositionSourceDevice() const
{
    KisPaintDeviceSP device = new KisPaintDevice(compositionSourceColorSpace());
    device->setDefaultBounds(defaultBounds());
    return device;
}

KisPaintDeviceSP KisPaintDevice::createCompositionSourceDevice(KisPaintDeviceSP cloneSource) const
{
    KisPaintDeviceSP clone = new KisPaintDevice(*cloneSource);
    clone->setDefaultBounds(defaultBounds());
    clone->convertTo(compositionSourceColorSpace(),
                     KoColorConversionTransformation::InternalRenderingIntent,
                     KoColorConversionTransformation::InternalConversionFlags);
    return clone;
}

KisPaintDeviceSP KisPaintDevice::createCompositionSourceDevice(KisPaintDeviceSP cloneSource, const QRect roughRect) const
{
    KisPaintDeviceSP clone = new KisPaintDevice(colorSpace());
    clone->setDefaultBounds(defaultBounds());
    clone->makeCloneFromRough(cloneSource, roughRect);
    clone->convertTo(compositionSourceColorSpace(),
                     KoColorConversionTransformation::InternalRenderingIntent,
                     KoColorConversionTransformation::InternalConversionFlags);
    return clone;
}

KisFixedPaintDeviceSP KisPaintDevice::createCompositionSourceDeviceFixed() const
{
    return new KisFixedPaintDevice(compositionSourceColorSpace());
}

const KoColorSpace* KisPaintDevice::compositionSourceColorSpace() const
{
    return colorSpace();
}

QVector<qint32> KisPaintDevice::channelSizes() const
{
    QVector<qint32> sizes;
    QList<KoChannelInfo*> channels = colorSpace()->channels();
    qSort(channels);

    foreach(KoChannelInfo * channelInfo, channels) {
        sizes.append(channelInfo->size());
    }
    return sizes;
}

KisPaintDevice::MemoryReleaseObject::~MemoryReleaseObject()
{
    KisDataManager::releaseInternalPools();
}

KisPaintDevice::MemoryReleaseObject* KisPaintDevice::createMemoryReleaseObject()
{
    return new MemoryReleaseObject();
}

QRegion KisPaintDevice::syncLodCache(int levelOfDetail)
{
    QRegion dirtyRegion;

    if (levelOfDetail) {
        dirtyRegion = m_d->syncLodData(levelOfDetail);
    }

    return dirtyRegion;
}

KisPaintDeviceFramesInterface* KisPaintDevice::framesInterface()
{
    return m_d->framesInterface.data();
}

/******************************************************************/
/*               KisPaintDeviceFramesInterface                    */
/******************************************************************/

KisPaintDeviceFramesInterface::KisPaintDeviceFramesInterface(KisPaintDevice *parentDevice)
    : q(parentDevice)
{
}

QList<int> KisPaintDeviceFramesInterface::frames()
{
    return q->m_d->frameIds();
}

int KisPaintDeviceFramesInterface::createFrame(bool copy, int copySrc, const QPoint &offset, KUndo2Command *parentCommand)
{
    return q->m_d->createFrame(copy, copySrc, offset, parentCommand);
}

void KisPaintDeviceFramesInterface::deleteFrame(int frame, KUndo2Command *parentCommand)
{
    return q->m_d->deleteFrame(frame, parentCommand);
}

void KisPaintDeviceFramesInterface::fetchFrame(int frameId, KisPaintDeviceSP targetDevice)
{
    q->m_d->fetchFrame(frameId, targetDevice);
}

QRect KisPaintDeviceFramesInterface::frameBounds(int frameId)
{
    return q->m_d->frameBounds(frameId);
}

QPoint KisPaintDeviceFramesInterface::frameOffset(int frameId) const
{
    return q->m_d->frameOffset(frameId);
}

void KisPaintDeviceFramesInterface::setFrameDefaultPixel(const quint8 *defPixel, int frameId)
{
    KIS_ASSERT_RECOVER_RETURN(frameId >= 0);
    q->m_d->setFrameDefaultPixel(defPixel, frameId);
}

const quint8* KisPaintDeviceFramesInterface::frameDefaultPixel(int frameId) const
{
    KIS_ASSERT_RECOVER(frameId >= 0) { return (quint8*)"deadbeef"; }
    return q->m_d->frameDefaultPixel(frameId);
}

bool KisPaintDeviceFramesInterface::writeFrame(KisPaintDeviceWriter &store, int frameId)
{
    KIS_ASSERT_RECOVER(frameId >= 0) { return false; }
    return q->m_d->writeFrame(store, frameId);
}

bool KisPaintDeviceFramesInterface::readFrame(QIODevice *stream, int frameId)
{
    KIS_ASSERT_RECOVER(frameId >= 0) { return false; }
    return q->m_d->readFrame(stream, frameId);
}

int KisPaintDeviceFramesInterface::currentFrameId() const
{
    return q->m_d->currentFrameId();
}

KisDataManagerSP KisPaintDeviceFramesInterface::frameDataManager(int frameId) const
{
    KIS_ASSERT_RECOVER(frameId >= 0) { return q->m_d->dataManager(); }
    return q->m_d->frameDataManager(frameId);
}

void KisPaintDeviceFramesInterface::invalidateFrameCache(int frameId)
{
    KIS_ASSERT_RECOVER_RETURN(frameId >= 0);

    return q->m_d->invalidateFrameCache(frameId);
}

void KisPaintDeviceFramesInterface::setFrameOffset(int frameId, const QPoint &offset)
{
    KIS_ASSERT_RECOVER_RETURN(frameId >= 0);
    return q->m_d->setFrameOffset(frameId, offset);
}

KisPaintDeviceFramesInterface::TestingDataObjects
KisPaintDeviceFramesInterface::testingGetDataObjects() const
{
    TestingDataObjects objects;

    objects.m_data = q->m_d->m_data;
    objects.m_lodData = q->m_d->m_lodData.data();
    objects.m_externalFrameData = q->m_d->m_externalFrameData.data();

    typedef KisPaintDevice::Private::FramesHash FramesHash;

    FramesHash::iterator it = q->m_d->m_frames.begin();
    FramesHash::iterator end = q->m_d->m_frames.end();

    for (; it != end; ++it) {
        objects.m_frames.insert(it.key(), it.value().data());
    }

    objects.m_currentData = q->m_d->currentData();

    return objects;
}

QList<KisPaintDeviceData*> KisPaintDeviceFramesInterface::testingGetDataObjectsList() const
{
    return q->m_d->allDataObjects();
}

#include "kis_paint_device.moc"
