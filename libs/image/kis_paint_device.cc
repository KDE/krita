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

#include <klocalizedstring.h>

#include <KoChannelInfo.h>
#include <KoColorProfile.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoIntegerMaths.h>
#include <KoMixColorsOp.h>
#include <KoUpdater.h>

#include "kis_image.h"
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

#include "kis_transform_worker.h"
#include "kis_filter_strategy.h"
#include "krita_utils.h"


struct KisPaintDeviceSPStaticRegistrar {
    KisPaintDeviceSPStaticRegistrar() {
        qRegisterMetaType<KisPaintDeviceSP>("KisPaintDeviceSP");
    }
};
static KisPaintDeviceSPStaticRegistrar __registrar;



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

    class DeviceChangeProfileCommand;
    class DeviceChangeColorSpaceCommand;

    Private(KisPaintDevice *paintDevice);
    ~Private();

    KisPaintDevice *q;
    KisNodeWSP parent;
    QScopedPointer<KisRasterKeyframeChannel> contentChannel;
    KisDefaultBoundsBaseSP defaultBounds;
    QScopedPointer<KisPaintDeviceStrategy> basicStrategy;
    QScopedPointer<KisPaintDeviceWrappedStrategy> wrappedStrategy;
    QMutex m_wrappedStrategyMutex;

    QScopedPointer<KisPaintDeviceFramesInterface> framesInterface;
    bool isProjectionDevice;

    KisPaintDeviceStrategy* currentStrategy();

    void init(const KoColorSpace *cs, const quint8 *defaultPixel);
    void convertColorSpace(const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags, KUndo2Command *parentCommand);
    bool assignProfile(const KoColorProfile * profile, KUndo2Command *parentCommand);

    inline const KoColorSpace* colorSpace() const
    {
        return currentData()->colorSpace();
    }
    inline KisDataManagerSP dataManager() const
    {
        return currentData()->dataManager();
    }

    inline qint32 x() const
    {
        return currentData()->x();
    }
    inline qint32 y() const
    {
        return currentData()->y();
    }
    inline void setX(qint32 x)
    {
        currentData()->setX(x);
    }
    inline void setY(qint32 y)
    {
        currentData()->setY(y);
    }

    inline KisPaintDeviceCache* cache()
    {
        return currentData()->cache();
    }

    inline KisIteratorCompleteListener* cacheInvalidator() {
        return currentData()->cacheInvalidator();
    }


    void cloneAllDataObjects(Private *rhs, bool copyFrames)
    {

        m_lodData.reset();
        m_externalFrameData.reset();

        if (!m_frames.isEmpty()) {
            m_frames.clear();
        }

        if (!copyFrames) {
            if (m_data) {
                m_data->prepareClone(rhs->currentNonLodData(), true);
            } else {
                m_data = toQShared(new KisPaintDeviceData(q, rhs->currentNonLodData(), true));
            }
        } else {
            if (m_data && !rhs->m_data) {
                m_data.clear();
            } else if (!m_data && rhs->m_data) {
                m_data = toQShared(new KisPaintDeviceData(q, rhs->m_data.data(), true));
            } else if (m_data && rhs->m_data) {
                m_data->prepareClone(rhs->m_data.data(), true);
            }

            if (!rhs->m_frames.isEmpty()) {
                FramesHash::const_iterator it = rhs->m_frames.constBegin();
                FramesHash::const_iterator end = rhs->m_frames.constEnd();

                for (; it != end; ++it) {
                    DataSP data = toQShared(new KisPaintDeviceData(q, it.value().data(), true));
                    m_frames.insert(it.key(), data);
                }
            }
            m_nextFreeFrameId = rhs->m_nextFreeFrameId;
        }

        if (rhs->m_lodData) {
            m_lodData.reset(new KisPaintDeviceData(q, rhs->m_lodData.data(), true));
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

    int currentFrameId() const
    {
        KIS_ASSERT_RECOVER(contentChannel) {
            return -1;
        }
        return !defaultBounds->currentLevelOfDetail() ?
               contentChannel->frameIdAt(defaultBounds->currentTime()) :
               -1;
    }

    KisDataManagerSP frameDataManager(int frameId) const
    {
        DataSP data = m_frames[frameId];
        return data->dataManager();
    }

    void invalidateFrameCache(int frameId)
    {
        DataSP data = m_frames[frameId];
        return data->cache()->invalidate();
    }

private:
    typedef KisPaintDeviceData Data;
    typedef QSharedPointer<Data> DataSP;
    typedef QHash<int, DataSP> FramesHash;

    class FrameInsertionCommand : public KUndo2Command
    {
    public:

        FrameInsertionCommand(FramesHash *hash, DataSP data, int frameId, bool insert, KUndo2Command *parentCommand)
            : KUndo2Command(parentCommand),
              m_hash(hash),
              m_data(data),
              m_frameId(frameId),
              m_insert(insert)
        {
        }

        void redo() override
        {
            doSwap(m_insert);
        }

        void undo() override
        {
            doSwap(!m_insert);
        }

    private:
        void doSwap(bool insert)
        {
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

    int getNextFrameId() {
        int frameId = 0;
        while (m_frames.contains(frameId = m_nextFreeFrameId++));
        KIS_SAFE_ASSERT_RECOVER_NOOP(!m_frames.contains(frameId));

        return frameId;
    }

    int createFrame(bool copy, int copySrc, const QPoint &offset, KUndo2Command *parentCommand)
    {
        KIS_ASSERT_RECOVER(parentCommand) {
            return -1;
        }

        DataSP data;
        bool initialFrame = false;

        if (m_frames.isEmpty()) {
            /**
             * Here we move the contents of the paint device to the
             * new frame and clear m_data to make the "background" for
             * the areas where there is no frame at all.
             */
            data = toQShared(new Data(q, m_data.data(), true));
            m_data->dataManager()->clear();
            m_data->cache()->invalidate();
            initialFrame = true;

        } else if (copy) {
            DataSP srcData = m_frames[copySrc];
            data = toQShared(new Data(q, srcData.data(), true));
        } else {
            DataSP srcData = m_frames.begin().value();
            data = toQShared(new Data(q, srcData.data(), false));
        }

        if (!initialFrame && !copy) {
            data->setX(offset.x());
            data->setY(offset.y());
        }

        int frameId = getNextFrameId();

        KUndo2Command *cmd =
            new FrameInsertionCommand(&m_frames,
                                      data,
                                      frameId, true,
                                      parentCommand);

        cmd->redo();

        return frameId;
    }

    void deleteFrame(int frame, KUndo2Command *parentCommand)
    {
        KIS_ASSERT_RECOVER_RETURN(m_frames.contains(frame));
        KIS_ASSERT_RECOVER_RETURN(parentCommand);

        DataSP deletedData = m_frames[frame];

        KUndo2Command *cmd =
            new FrameInsertionCommand(&m_frames,
                                      deletedData,
                                      frame, false,
                                      parentCommand);
        cmd->redo();
    }

    QRect frameBounds(int frameId)
    {
        DataSP data = m_frames[frameId];

        QRect extent = data->dataManager()->extent();
        extent.translate(data->x(), data->y());

        return extent;
    }

    QPoint frameOffset(int frameId) const
    {
        DataSP data = m_frames[frameId];
        return QPoint(data->x(), data->y());
    }

    void setFrameOffset(int frameId, const QPoint &offset)
    {
        DataSP data = m_frames[frameId];
        data->setX(offset.x());
        data->setY(offset.y());
    }

    const QList<int> frameIds() const
    {
        return m_frames.keys();
    }

    bool readFrame(QIODevice *stream, int frameId)
    {
        bool retval = false;
        DataSP data = m_frames[frameId];
        retval = data->dataManager()->read(stream);
        data->cache()->invalidate();
        return retval;
    }

    bool writeFrame(KisPaintDeviceWriter &store, int frameId)
    {
        DataSP data = m_frames[frameId];
        return data->dataManager()->write(store);
    }

    void setFrameDefaultPixel(const KoColor &defPixel, int frameId)
    {
        DataSP data = m_frames[frameId];
        KoColor color(defPixel);
        color.convertTo(data->colorSpace());
        data->dataManager()->setDefaultPixel(color.data());
    }

    KoColor frameDefaultPixel(int frameId) const
    {
        DataSP data = m_frames[frameId];
        return KoColor(data->dataManager()->defaultPixel(),
                       data->colorSpace());
    }

    void fetchFrame(int frameId, KisPaintDeviceSP targetDevice);
    void uploadFrame(int srcFrameId, int dstFrameId, KisPaintDeviceSP srcDevice);
    void uploadFrame(int dstFrameId, KisPaintDeviceSP srcDevice);
    void uploadFrameData(DataSP srcData, DataSP dstData);

    struct LodDataStructImpl;
    LodDataStruct* createLodDataStruct(int lod);
    void updateLodDataStruct(LodDataStruct *dst, const QRect &srcRect);
    void uploadLodDataStruct(LodDataStruct *dst);
    QRegion regionForLodSyncing() const;

    void updateLodDataManager(KisDataManager *srcDataManager,
                              KisDataManager *dstDataManager, const QPoint &srcOffset, const QPoint &dstOffset,
                              const QRect &originalRect, int lod);

    void generateLodCloneDevice(KisPaintDeviceSP dst, const QRect &originalRect, int lod);

    void tesingFetchLodDevice(KisPaintDeviceSP targetDevice);


private:
    qint64 estimateDataSize(Data *data) const {
        const QRect &rc = data->dataManager()->extent();
        return rc.width() * rc.height() * data->colorSpace()->pixelSize();
    }

public:

    void estimateMemoryStats(qint64 &imageData, qint64 &temporaryData, qint64 &lodData) const {
        imageData = 0;
        temporaryData = 0;
        lodData = 0;

        if (m_data) {
            imageData += estimateDataSize(m_data.data());
        }

        if (m_lodData) {
            lodData += estimateDataSize(m_lodData.data());
        }

        if (m_externalFrameData) {
            temporaryData += estimateDataSize(m_externalFrameData.data());
        }

        Q_FOREACH (DataSP value, m_frames.values()) {
            imageData += estimateDataSize(value.data());
        }
    }


private:

    QRegion syncWholeDevice(Data *srcData);

    inline DataSP currentFrameData() const
    {
        DataSP data;

        const int numberOfFrames = contentChannel->keyframeCount();

        if (numberOfFrames > 1) {
            int frameId = contentChannel->frameIdAt(defaultBounds->currentTime());

            if (frameId == -1) {
                data = m_data;
            } else {
                KIS_ASSERT_RECOVER(m_frames.contains(frameId)) {
                    return m_frames.begin().value();
                }
                data = m_frames[frameId];
            }
        } else if (numberOfFrames == 1) {
            data = m_frames.begin().value();
        } else {
            data = m_data;
        }

        return data;
    }

    inline Data* currentNonLodData() const
    {
        Data *data = m_data.data();

        if (contentChannel) {
            data = currentFrameData().data();
        } else if (isProjectionDevice && defaultBounds->externalFrameActive()) {
            if (!m_externalFrameData) {
                QMutexLocker l(&m_dataSwitchLock);
                if (!m_externalFrameData) {
                    m_externalFrameData.reset(new Data(q, m_data.data(), false));
                }
            }
            data = m_externalFrameData.data();
        }

        return data;
    }

    inline void ensureLodDataPresent() const
    {
        if (!m_lodData) {
            Data *srcData = currentNonLodData();

            QMutexLocker l(&m_dataSwitchLock);
            if (!m_lodData) {
                m_lodData.reset(new Data(q, srcData, false));
            }
        }
    }

    inline Data* currentData() const
    {
        Data *data;

        if (defaultBounds->currentLevelOfDetail()) {
            ensureLodDataPresent();
            data = m_lodData.data();
        } else {
            data = currentNonLodData();
        }

        return data;
    }

    void prepareCloneImpl(KisPaintDeviceSP src, Data *srcData)
    {
        currentData()->prepareClone(srcData);

        q->setDefaultPixel(KoColor(srcData->dataManager()->defaultPixel(), colorSpace()));
        q->setDefaultBounds(src->defaultBounds());
    }

    bool fastBitBltPossibleImpl(Data *srcData)
    {
        return x() == srcData->x() && y() == srcData->y() &&
               *colorSpace() == *srcData->colorSpace();
    }

    QList<Data*> allDataObjects() const
    {
        QList<Data*> dataObjects;

        if (m_frames.isEmpty()) {
            dataObjects << m_data.data();
        }
        dataObjects << m_lodData.data();
        dataObjects << m_externalFrameData.data();

        Q_FOREACH (DataSP value, m_frames.values()) {
            dataObjects << value.data();
        }

        return dataObjects;
    }

    void transferFromData(Data *data, KisPaintDeviceSP targetDevice);

    struct Q_DECL_HIDDEN StrategyPolicy;
    typedef KisSequentialIteratorBase<ReadOnlyIteratorPolicy<StrategyPolicy>, StrategyPolicy> InternalSequentialConstIterator;
    typedef KisSequentialIteratorBase<WritableIteratorPolicy<StrategyPolicy>, StrategyPolicy> InternalSequentialIterator;

private:
    friend class KisPaintDeviceFramesInterface;

private:
    DataSP m_data;
    mutable QScopedPointer<Data> m_lodData;
    mutable QScopedPointer<Data> m_externalFrameData;
    mutable QMutex m_dataSwitchLock;

    FramesHash m_frames;
    int m_nextFreeFrameId;
};

const KisDefaultBoundsSP KisPaintDevice::Private::transitionalDefaultBounds = new KisDefaultBounds();

#include "kis_paint_device_strategies.h"

KisPaintDevice::Private::Private(KisPaintDevice *paintDevice)
    : q(paintDevice),
      basicStrategy(new KisPaintDeviceStrategy(paintDevice, this)),
      isProjectionDevice(false),
      m_data(new Data(paintDevice)),
      m_nextFreeFrameId(0)
{
}

KisPaintDevice::Private::~Private()
{
    m_frames.clear();
}

KisPaintDevice::Private::KisPaintDeviceStrategy* KisPaintDevice::Private::currentStrategy()
{
    if (!defaultBounds->wrapAroundMode()) {
        return basicStrategy.data();
    }

    const QRect wrapRect = defaultBounds->bounds();

    if (!wrappedStrategy || wrappedStrategy->wrapRect() != wrapRect) {
        QMutexLocker locker(&m_wrappedStrategyMutex);

        if (!wrappedStrategy) {
            wrappedStrategy.reset(new KisPaintDeviceWrappedStrategy(wrapRect, q, this));
        }  else if (wrappedStrategy->wrapRect() != wrapRect) {
            wrappedStrategy->setWrapRect(wrapRect);
        }
    }

    return wrappedStrategy.data();
}

struct KisPaintDevice::Private::StrategyPolicy {
    StrategyPolicy(KisPaintDevice::Private::KisPaintDeviceStrategy *strategy,
                   KisDataManager *dataManager, qint32 offsetX, qint32 offsetY)
        : m_strategy(strategy),
          m_dataManager(dataManager),
          m_offsetX(offsetX),
          m_offsetY(offsetY)
    {
    }

    KisHLineConstIteratorSP createConstIterator(const QRect &rect)
    {
        return m_strategy->createHLineConstIteratorNG(m_dataManager, rect.x(), rect.y(), rect.width(), m_offsetX, m_offsetY);
    }

    KisHLineIteratorSP createIterator(const QRect &rect)
    {
        return m_strategy->createHLineIteratorNG(m_dataManager, rect.x(), rect.y(), rect.width(), m_offsetX, m_offsetY);
    }

    int pixelSize() const
    {
        return m_dataManager->pixelSize();
    }


    KisPaintDeviceStrategy *m_strategy;
    KisDataManager *m_dataManager;
    int m_offsetX;
    int m_offsetY;
};

struct KisPaintDevice::Private::LodDataStructImpl : public KisPaintDevice::LodDataStruct {
    LodDataStructImpl(Data *_lodData) : lodData(_lodData) {}
    QScopedPointer<Data> lodData;
};

QRegion KisPaintDevice::Private::regionForLodSyncing() const
{
    Data *srcData = currentNonLodData();
    return srcData->dataManager()->region().translated(srcData->x(), srcData->y());
}

KisPaintDevice::LodDataStruct* KisPaintDevice::Private::createLodDataStruct(int newLod)
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(newLod > 0);

    Data *srcData = currentNonLodData();

    Data *lodData = new Data(q, srcData, false);
    LodDataStruct *lodStruct = new LodDataStructImpl(lodData);

    int expectedX = KisLodTransform::coordToLodCoord(srcData->x(), newLod);
    int expectedY = KisLodTransform::coordToLodCoord(srcData->y(), newLod);

    /**
     * We compare color spaces as pure pointers, because they must be
     * exactly the same, since they come from the common source.
     */
    if (lodData->levelOfDetail() != newLod ||
        lodData->colorSpace() != srcData->colorSpace() ||
        lodData->x() != expectedX ||
        lodData->y() != expectedY) {


        lodData->prepareClone(srcData);

        lodData->setLevelOfDetail(newLod);
        lodData->setX(expectedX);
        lodData->setY(expectedY);

        // FIXME: different kind of synchronization
    }

    //QRegion dirtyRegion = syncWholeDevice(srcData);
    lodData->cache()->invalidate();

    return lodStruct;
}

void KisPaintDevice::Private::updateLodDataManager(KisDataManager *srcDataManager,
                                                   KisDataManager *dstDataManager,
                                                   const QPoint &srcOffset,
                                                   const QPoint &dstOffset,
                                                   const QRect &originalRect,
                                                   int lod)
{
    if (originalRect.isEmpty()) return;

    const int srcStepSize = 1 << lod;

    KIS_ASSERT_RECOVER_RETURN(lod > 0);

    const QRect srcRect = KisLodTransform::alignedRect(originalRect, lod);
    const QRect dstRect = KisLodTransform::scaledRect(srcRect, lod);
    if (!srcRect.isValid() || !dstRect.isValid()) return;

    KIS_ASSERT_RECOVER_NOOP(srcRect.width() / srcStepSize == dstRect.width());

    const int pixelSize = srcDataManager->pixelSize();

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

    InternalSequentialConstIterator srcIntIt(StrategyPolicy(currentStrategy(), srcDataManager, srcOffset.x(), srcOffset.y()), srcRect);
    InternalSequentialIterator dstIntIt(StrategyPolicy(currentStrategy(), dstDataManager, dstOffset.x(), dstOffset.y()), dstRect);

    int rowsRemaining = srcRect.height();
    while (rowsRemaining > 0) {

        int colsRemaining = srcRect.width();
        while (colsRemaining > 0 && srcIntIt.nextPixel()) {

            memcpy(blendDataPtr, srcIntIt.rawDataConst(), pixelSize);
            blendDataPtr += pixelSize;
            columnsAccumulated++;

            if (columnsAccumulated >= srcStepSize) {
                blendDataPtr += srcColumnStride;
                columnsAccumulated = 0;
            }

            colsRemaining--;
        }

        rowsAccumulated++;

        if (rowsAccumulated >= srcStepSize) {

            // blend and write the final data
            blendDataPtr = blendData.data();

            int colsRemaining = dstRect.width();
            while (colsRemaining > 0 && dstIntIt.nextPixel()) {
                mixOp->mixColors(blendDataPtr, weights.data(), srcCellSize, dstIntIt.rawData());
                blendDataPtr += srcCellStride;

                colsRemaining--;
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
}

void KisPaintDevice::Private::updateLodDataStruct(LodDataStruct *_dst, const QRect &originalRect)
{
    LodDataStructImpl *dst = dynamic_cast<LodDataStructImpl*>(_dst);
    KIS_SAFE_ASSERT_RECOVER_RETURN(dst);

    Data *lodData = dst->lodData.data();
    Data *srcData = currentNonLodData();

    const int lod = lodData->levelOfDetail();

    updateLodDataManager(srcData->dataManager().data(), lodData->dataManager().data(),
                         QPoint(srcData->x(), srcData->y()),
                         QPoint(lodData->x(), lodData->y()),
                         originalRect, lod);
}

void KisPaintDevice::Private::generateLodCloneDevice(KisPaintDeviceSP dst, const QRect &originalRect, int lod)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(fastBitBltPossible(dst));

    Data *srcData = currentNonLodData();
    updateLodDataManager(srcData->dataManager().data(), dst->dataManager().data(),
                         QPoint(srcData->x(), srcData->y()),
                         QPoint(dst->x(), dst->y()),
                         originalRect, lod);
}

void KisPaintDevice::Private::uploadLodDataStruct(LodDataStruct *_dst)
{
    LodDataStructImpl *dst = dynamic_cast<LodDataStructImpl*>(_dst);
    KIS_SAFE_ASSERT_RECOVER_RETURN(dst);

    KIS_SAFE_ASSERT_RECOVER_RETURN(
        dst->lodData->levelOfDetail() == defaultBounds->currentLevelOfDetail());

    ensureLodDataPresent();

    m_lodData->prepareClone(dst->lodData.data());
    m_lodData->dataManager()->bitBltRough(dst->lodData->dataManager(), dst->lodData->dataManager()->extent());
}

void KisPaintDevice::Private::transferFromData(Data *data, KisPaintDeviceSP targetDevice)
{
    QRect extent = data->dataManager()->extent();
    extent.translate(data->x(), data->y());

    targetDevice->m_d->prepareCloneImpl(q, data);
    targetDevice->m_d->currentStrategy()->fastBitBltRough(data->dataManager(), extent);
}

void KisPaintDevice::Private::fetchFrame(int frameId, KisPaintDeviceSP targetDevice)
{
    DataSP data = m_frames[frameId];
    transferFromData(data.data(), targetDevice);
}

void KisPaintDevice::Private::uploadFrame(int srcFrameId, int dstFrameId, KisPaintDeviceSP srcDevice)
{
    DataSP dstData = m_frames[dstFrameId];
    KIS_ASSERT_RECOVER_RETURN(dstData);

    DataSP srcData = srcDevice->m_d->m_frames[srcFrameId];
    KIS_ASSERT_RECOVER_RETURN(srcData);

    uploadFrameData(srcData, dstData);
}

void KisPaintDevice::Private::uploadFrame(int dstFrameId, KisPaintDeviceSP srcDevice)
{
    DataSP dstData = m_frames[dstFrameId];
    KIS_ASSERT_RECOVER_RETURN(dstData);

    DataSP srcData = srcDevice->m_d->m_data;
    KIS_ASSERT_RECOVER_RETURN(srcData);

    uploadFrameData(srcData, dstData);
}

void KisPaintDevice::Private::uploadFrameData(DataSP srcData, DataSP dstData)
{
    if (srcData->colorSpace() != dstData->colorSpace() &&
        *srcData->colorSpace() != *dstData->colorSpace()) {

        KUndo2Command tempCommand;

        srcData = toQShared(new Data(q, srcData.data(), true));
        srcData->convertDataColorSpace(dstData->colorSpace(),
                                       KoColorConversionTransformation::internalRenderingIntent(),
                                       KoColorConversionTransformation::internalConversionFlags(),
                                       &tempCommand);
    }

    dstData->dataManager()->clear();
    dstData->cache()->invalidate();

    const QRect rect = srcData->dataManager()->extent();
    dstData->dataManager()->bitBltRough(srcData->dataManager(), rect);
    dstData->setX(srcData->x());
    dstData->setY(srcData->y());
}

void KisPaintDevice::Private::tesingFetchLodDevice(KisPaintDeviceSP targetDevice)
{
    Data *data = m_lodData.data();
    Q_ASSERT(data);

    transferFromData(data, targetDevice);
}

class KisPaintDevice::Private::DeviceChangeProfileCommand : public KUndo2Command
{
public:
    DeviceChangeProfileCommand(KisPaintDeviceSP device, KUndo2Command *parent = 0)
        : KUndo2Command(parent),
          m_firstRun(true),
          m_device(device)
    {
    }

    virtual void emitNotifications()
    {
        m_device->emitProfileChanged();
    }

    void redo() override
    {
        KUndo2Command::redo();

        if (!m_firstRun) {
            m_firstRun = false;
            return;
        }

        emitNotifications();
    }

    void undo() override
    {
        KUndo2Command::undo();
        emitNotifications();
    }

protected:
    KisPaintDeviceSP m_device;

private:
    bool m_firstRun;
};

class KisPaintDevice::Private::DeviceChangeColorSpaceCommand : public DeviceChangeProfileCommand
{
public:
    DeviceChangeColorSpaceCommand(KisPaintDeviceSP device, KUndo2Command *parent = 0)
        : DeviceChangeProfileCommand(device, parent)
    {
    }

    void emitNotifications() override
    {
        m_device->emitColorSpaceChanged();
    }
};

void KisPaintDevice::Private::convertColorSpace(const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags, KUndo2Command *parentCommand)
{
    QList<Data*> dataObjects = allDataObjects();
    if (dataObjects.isEmpty()) return;

    KUndo2Command *mainCommand =
        parentCommand ? new DeviceChangeColorSpaceCommand(q, parentCommand) : 0;

    Q_FOREACH (Data *data, dataObjects) {
        if (!data) continue;

        data->convertDataColorSpace(dstColorSpace, renderingIntent, conversionFlags, mainCommand);
    }

    q->emitColorSpaceChanged();
}

bool KisPaintDevice::Private::assignProfile(const KoColorProfile * profile, KUndo2Command *parentCommand)
{
    if (!profile) return false;

    const KoColorSpace *dstColorSpace =
        KoColorSpaceRegistry::instance()->colorSpace(colorSpace()->colorModelId().id(), colorSpace()->colorDepthId().id(), profile);
    if (!dstColorSpace) return false;

    KUndo2Command *mainCommand =
        parentCommand ? new DeviceChangeColorSpaceCommand(q, parentCommand) : 0;


    QList<Data*> dataObjects = allDataObjects();
    Q_FOREACH (Data *data, dataObjects) {
        if (!data) continue;
        data->assignColorSpace(dstColorSpace, mainCommand);
    }
    q->emitProfileChanged();

    // no undo information is provided here
    return true;
}

void KisPaintDevice::Private::init(const KoColorSpace *cs, const quint8 *defaultPixel)
{
    QList<Data*> dataObjects = allDataObjects();
    Q_FOREACH (Data *data, dataObjects) {
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

KisPaintDevice::KisPaintDevice(const KisPaintDevice& rhs, KritaUtils::DeviceCopyMode copyMode, KisNode *newParentNode)
    : QObject()
    , KisShared()
    , m_d(new Private(this))
{
    if (this != &rhs) {
        makeFullCopyFrom(rhs, copyMode, newParentNode);
    }
}

void KisPaintDevice::makeFullCopyFrom(const KisPaintDevice &rhs, KritaUtils::DeviceCopyMode copyMode, KisNode *newParentNode)
{
    // temporary def. bounds object for the initialization phase only
    m_d->defaultBounds = m_d->transitionalDefaultBounds;

    // copy data objects with or without frames
    m_d->cloneAllDataObjects(rhs.m_d, copyMode == KritaUtils::CopyAllFrames);

    if (copyMode == KritaUtils::CopyAllFrames && rhs.m_d->framesInterface) {
        KIS_ASSERT_RECOVER_RETURN(rhs.m_d->framesInterface);
        KIS_ASSERT_RECOVER_RETURN(rhs.m_d->contentChannel);
        m_d->framesInterface.reset(new KisPaintDeviceFramesInterface(this));
        m_d->contentChannel.reset(new KisRasterKeyframeChannel(*rhs.m_d->contentChannel.data(), newParentNode, this));
    }

    setDefaultBounds(rhs.m_d->defaultBounds);
    setParentNode(newParentNode);
}

KisPaintDevice::~KisPaintDevice()
{
    delete m_d;
}

void KisPaintDevice::setProjectionDevice(bool value)
{
    m_d->isProjectionDevice = value;
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

int KisPaintDevice::sequenceNumber() const
{
    return m_d->cache()->sequenceNumber();
}

void KisPaintDevice::estimateMemoryStats(qint64 &imageData, qint64 &temporaryData, qint64 &lodData) const
{
    m_d->estimateMemoryStats(imageData, temporaryData, lodData);
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

void KisPaintDevice::moveTo(const QPoint &pt)
{
    m_d->currentStrategy()->move(pt);
    m_d->cache()->invalidate();
}

QPoint KisPaintDevice::offset() const
{
    return QPoint(x(), y());
}

void KisPaintDevice::moveTo(qint32 x, qint32 y)
{
    moveTo(QPoint(x, y));
}

void KisPaintDevice::setX(qint32 x)
{
    moveTo(QPoint(x, m_d->y()));
}

void KisPaintDevice::setY(qint32 y)
{
    moveTo(QPoint(m_d->x(), y));
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

QRect KisPaintDevice::exactBoundsAmortized() const
{
    return m_d->cache()->exactBoundsAmortized();
}

namespace Impl
{

struct CheckFullyTransparent {
    CheckFullyTransparent(const KoColorSpace *colorSpace)
        : m_colorSpace(colorSpace)
    {
    }

    bool isPixelEmpty(const quint8 *pixelData)
    {
        return m_colorSpace->opacityU8(pixelData) == OPACITY_TRANSPARENT_U8;
    }

private:
    const KoColorSpace *m_colorSpace;
};

struct CheckNonDefault {
    CheckNonDefault(int pixelSize, const quint8 *defaultPixel)
        : m_pixelSize(pixelSize),
          m_defaultPixel(defaultPixel)
    {
    }

    bool isPixelEmpty(const quint8 *pixelData)
    {
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

    // the passed extent might have weird invalid structure that
    // can overflow integer precision when calling startRect.right()
    if (!startRect.isValid()) return QRect();

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
            for (qint32 y2 = y + h - 1; y2 >= y || found; --y2) {
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

    quint8 defaultOpacity = defaultPixel().opacityU8();
    if (defaultOpacity != OPACITY_TRANSPARENT_U8) {
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
        const KoColor defaultPixel = this->defaultPixel();
        Impl::CheckNonDefault compareOp(pixelSize(), defaultPixel.data());
        endRect = Impl::calculateExactBoundsImpl(this, startRect, endRect, compareOp);
    } else {
        Impl::CheckFullyTransparent compareOp(m_d->colorSpace());
        endRect = Impl::calculateExactBoundsImpl(this, startRect, endRect, compareOp);
    }

    return endRect;
}

QRegion KisPaintDevice::regionExact() const
{
    QRegion resultRegion;
    QVector<QRect> rects = region().rects();

    const KoColor defaultPixel = this->defaultPixel();
    Impl::CheckNonDefault compareOp(pixelSize(), defaultPixel.data());

    Q_FOREACH (const QRect &rc1, rects) {
        const int patchSize = 64;
        QVector<QRect> smallerRects = KritaUtils::splitRectIntoPatches(rc1, QSize(patchSize, patchSize));
        Q_FOREACH (const QRect &rc2, smallerRects) {

            const QRect result =
                Impl::calculateExactBoundsImpl(this, rc2, QRect(), compareOp);

            if (!result.isEmpty()) {
                resultRegion += result;
            }
        }
    }
    return resultRegion;
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

void KisPaintDevice::setDefaultPixel(const KoColor &defPixel)
{
    KoColor color(defPixel);
    color.convertTo(colorSpace());

    m_d->dataManager()->setDefaultPixel(color.data());
    m_d->cache()->invalidate();
}

KoColor KisPaintDevice::defaultPixel() const
{
    return KoColor(m_d->dataManager()->defaultPixel(), colorSpace());
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
    KIS_ASSERT_RECOVER_RETURN(*color.colorSpace() == *colorSpace());
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

void KisPaintDevice::convertTo(const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags, KUndo2Command *parentCommand)
{
    m_d->convertColorSpace(dstColorSpace, renderingIntent, conversionFlags, parentCommand);
}

bool KisPaintDevice::setProfile(const KoColorProfile * profile, KUndo2Command *parentCommand)
{
    return m_d->assignProfile(profile, parentCommand);
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
        writeBytes(image.constBits(), offsetX, offsetY, image.width(), image.height());
    } else {
        try {
            quint8 * dstData = new quint8[image.width() * image.height() * pixelSize()];
            KoColorSpaceRegistry::instance()
            ->colorSpace(RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), profile)
            ->convertPixelsTo(image.constBits(), dstData, colorSpace(), image.width() * image.height(),
                              KoColorConversionTransformation::internalRenderingIntent(),
                              KoColorConversionTransformation::internalConversionFlags());

            writeBytes(dstData, offsetX, offsetY, image.width(), image.height());
            delete[] dstData;
        } catch (const std::bad_alloc&) {
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

QImage KisPaintDevice::convertToQImage(const KoColorProfile *dstProfile, qint32 x1, qint32 y1, qint32 w, qint32 h, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags) const
{

    if (w < 0)
        return QImage();

    if (h < 0)
        return QImage();

    quint8 *data = 0;
    try {
        data = new quint8 [w * h * pixelSize()];
    } catch (const std::bad_alloc&) {
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

inline bool moveBy(KisSequentialConstIterator& iter, int numPixels)
{
    int pos = 0;
    while (pos < numPixels) {
        int step = std::min(iter.nConseqPixels(), numPixels - pos);
        if (!iter.nextPixels(step))
            return false;
        pos += step;
    }
    return true;
}

static KisPaintDeviceSP createThumbnailDeviceInternal(const KisPaintDevice* srcDev, qint32 srcX0, qint32 srcY0, qint32 srcWidth, qint32 srcHeight, qint32 w, qint32 h, QRect outputRect)
{
    KisPaintDeviceSP thumbnail = new KisPaintDevice(srcDev->colorSpace());
    qint32 pixelSize = srcDev->pixelSize();

    KisRandomConstAccessorSP srcIter = srcDev->createRandomConstAccessorNG(0, 0);
    KisRandomAccessorSP dstIter = thumbnail->createRandomAccessorNG(0, 0);

    for (qint32 y = outputRect.y(); y < outputRect.y() + outputRect.height(); ++y) {
        qint32 iY = srcY0 + (y * srcHeight) / h;
        for (qint32 x = outputRect.x(); x < outputRect.x() + outputRect.width(); ++x) {
            qint32 iX = srcX0 + (x * srcWidth) / w;
            srcIter->moveTo(iX, iY);
            dstIter->moveTo(x,  y);
            memcpy(dstIter->rawData(), srcIter->rawDataConst(), pixelSize);
        }
    }
    return thumbnail;
}

QSize fixThumbnailSize(QSize size)
{
    if (!size.width() && size.height()) {
        size.setWidth(1);
    }

    if (size.width() && !size.height()) {
        size.setHeight(1);
    }

    return size;
}

KisPaintDeviceSP KisPaintDevice::createThumbnailDevice(qint32 w, qint32 h, QRect rect, QRect outputRect) const
{
    QSize thumbnailSize(w, h);

    QRect imageRect = rect.isValid() ? rect : extent();

    if ((thumbnailSize.width() > imageRect.width()) || (thumbnailSize.height() > imageRect.height())) {
        thumbnailSize.scale(imageRect.size(), Qt::KeepAspectRatio);
    }

    thumbnailSize = fixThumbnailSize(thumbnailSize);

    //can't create thumbnail for an empty device, e.g. layer thumbnail for empty image
    if (imageRect.isEmpty() || thumbnailSize.isEmpty()) {
        return new KisPaintDevice(colorSpace());
    }

    int srcWidth, srcHeight;
    int srcX0, srcY0;
    imageRect.getRect(&srcX0, &srcY0, &srcWidth, &srcHeight);

    if (!outputRect.isValid()) {
        outputRect = QRect(0, 0, w, h);
    }

    KisPaintDeviceSP thumbnail = createThumbnailDeviceInternal(this, imageRect.x(), imageRect.y(), imageRect.width(), imageRect.height(),
                                 thumbnailSize.width(), thumbnailSize.height(), outputRect);

    return thumbnail;
}

KisPaintDeviceSP KisPaintDevice::createThumbnailDeviceOversampled(qint32 w, qint32 h, qreal oversample, QRect rect,  QRect outputTileRect) const
{
    QSize thumbnailSize(w, h);
    qreal oversampleAdjusted = qMax(oversample, 1.);
    QSize thumbnailOversampledSize = oversampleAdjusted * thumbnailSize;

    QRect outputRect;
    QRect imageRect = rect.isValid() ? rect : extent();

    qint32 hstart = thumbnailOversampledSize.height();

    if ((thumbnailOversampledSize.width() > imageRect.width()) || (thumbnailOversampledSize.height() > imageRect.height())) {
        thumbnailOversampledSize.scale(imageRect.size(), Qt::KeepAspectRatio);
    }

    thumbnailOversampledSize = fixThumbnailSize(thumbnailOversampledSize);

    //can't create thumbnail for an empty device, e.g. layer thumbnail for empty image
    if (imageRect.isEmpty() || thumbnailSize.isEmpty() || thumbnailOversampledSize.isEmpty()) {
        return new KisPaintDevice(colorSpace());
    }

    oversampleAdjusted *= (hstart > 0) ? ((qreal)thumbnailOversampledSize.height() / hstart) : 1.; //readjusting oversample ratio, given that we had to adjust thumbnail size

    outputRect = QRect(0, 0, thumbnailOversampledSize.width(), thumbnailOversampledSize.height());

    if (outputTileRect.isValid()) {
        //compensating output rectangle for oversampling
        outputTileRect = QRect(oversampleAdjusted * outputTileRect.topLeft(), oversampleAdjusted * outputTileRect.bottomRight());
        outputRect = outputRect.intersected(outputTileRect);
    }

    KisPaintDeviceSP thumbnail = createThumbnailDeviceInternal(this, imageRect.x(), imageRect.y(), imageRect.width(), imageRect.height(),
                                 thumbnailOversampledSize.width(), thumbnailOversampledSize.height(), outputRect);

    if (oversample != 1. && oversampleAdjusted != 1.) {
        KoDummyUpdater updater;
        KisTransformWorker worker(thumbnail, 1 / oversampleAdjusted, 1 / oversampleAdjusted, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                  &updater, KisFilterStrategyRegistry::instance()->value("Bilinear"));
        worker.run();
    }
    return thumbnail;
}

QImage KisPaintDevice::createThumbnail(qint32 w, qint32 h, QRect rect, qreal oversample, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    QSize size = fixThumbnailSize(QSize(w, h));

    KisPaintDeviceSP dev = createThumbnailDeviceOversampled(size.width(), size.height(), oversample, rect);
    QImage thumbnail = dev->convertToQImage(KoColorSpaceRegistry::instance()->rgb8()->profile(), 0, 0, w, h, renderingIntent, conversionFlags);
    return thumbnail;
}

QImage KisPaintDevice::createThumbnail(qint32 w, qint32 h, qreal oversample, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    QSize size = fixThumbnailSize(QSize(w, h));

    return m_d->cache()->createThumbnail(size.width(), size.height(), oversample, renderingIntent, conversionFlags);
}

KisHLineIteratorSP KisPaintDevice::createHLineIteratorNG(qint32 x, qint32 y, qint32 w)
{
    m_d->cache()->invalidate();
    return m_d->currentStrategy()->createHLineIteratorNG(m_d->dataManager().data(), x, y, w, m_d->x(), m_d->y());
}

KisHLineConstIteratorSP KisPaintDevice::createHLineConstIteratorNG(qint32 x, qint32 y, qint32 w) const
{
    return m_d->currentStrategy()->createHLineConstIteratorNG(m_d->dataManager().data(), x, y, w, m_d->x(), m_d->y());
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
    return new KisRepeatHLineConstIteratorNG(m_d->dataManager().data(), x, y, w, m_d->x(), m_d->y(), _dataWidth, m_d->cacheInvalidator());
}

KisRepeatVLineConstIteratorSP KisPaintDevice::createRepeatVLineConstIterator(qint32 x, qint32 y, qint32 h, const QRect& _dataWidth) const
{
    return new KisRepeatVLineConstIteratorNG(m_d->dataManager().data(), x, y, h, m_d->x(), m_d->y(), _dataWidth, m_d->cacheInvalidator());
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
    const QRect r = selection->selectedExactRect();

    if (r.isValid()) {

        {
            KisHLineIteratorSP devIt = createHLineIteratorNG(r.x(), r.y(), r.width());
            KisHLineConstIteratorSP selectionIt = selection->projection()->createHLineConstIteratorNG(r.x(), r.y(), r.width());

            const KoColor defaultPixel = this->defaultPixel();
            bool transparentDefault = (defaultPixel.opacityU8() == OPACITY_TRANSPARENT_U8);
            for (qint32 y = 0; y < r.height(); y++) {

                do {
                    // XXX: Optimize by using stretches
                    colorSpace->applyInverseAlphaU8Mask(devIt->rawData(), selectionIt->rawDataConst(), 1);
                    if (transparentDefault && colorSpace->opacityU8(devIt->rawData()) == OPACITY_TRANSPARENT_U8) {
                        memcpy(devIt->rawData(), defaultPixel.data(), colorSpace->pixelSize());
                    }
                } while (devIt->nextPixel() && selectionIt->nextPixel());
                devIt->nextRow();
                selectionIt->nextRow();
            }
        }

        // purge() must be executed **after** all iterators have been destroyed!
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

KisRasterKeyframeChannel *KisPaintDevice::createKeyframeChannel(const KoID &id)
{
    Q_ASSERT(!m_d->framesInterface);
    m_d->framesInterface.reset(new KisPaintDeviceFramesInterface(this));

    Q_ASSERT(!m_d->contentChannel);
    m_d->contentChannel.reset(new KisRasterKeyframeChannel(id, this, m_d->defaultBounds));

    // Raster channels always have at least one frame (representing a static image)
    KUndo2Command tempParentCommand;
    m_d->contentChannel->addKeyframe(0, &tempParentCommand);

    return m_d->contentChannel.data();
}

KisRasterKeyframeChannel* KisPaintDevice::keyframeChannel() const
{
    if (m_d->contentChannel) {
        return m_d->contentChannel.data();
    }
    return 0;
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
                     KoColorConversionTransformation::internalRenderingIntent(),
                     KoColorConversionTransformation::internalConversionFlags());
    return clone;
}

KisPaintDeviceSP KisPaintDevice::createCompositionSourceDevice(KisPaintDeviceSP cloneSource, const QRect roughRect) const
{
    KisPaintDeviceSP clone = new KisPaintDevice(colorSpace());
    clone->setDefaultBounds(defaultBounds());
    clone->makeCloneFromRough(cloneSource, roughRect);
    clone->convertTo(compositionSourceColorSpace(),
                     KoColorConversionTransformation::internalRenderingIntent(),
                     KoColorConversionTransformation::internalConversionFlags());
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
    std::sort(channels.begin(), channels.end());

    Q_FOREACH (KoChannelInfo * channelInfo, channels) {
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

KisPaintDevice::LodDataStruct::~LodDataStruct()
{
}

QRegion KisPaintDevice::regionForLodSyncing() const
{
    return m_d->regionForLodSyncing();
}

KisPaintDevice::LodDataStruct* KisPaintDevice::createLodDataStruct(int lod)
{
    return m_d->createLodDataStruct(lod);
}

void KisPaintDevice::updateLodDataStruct(LodDataStruct *dst, const QRect &srcRect)
{
    m_d->updateLodDataStruct(dst, srcRect);
}

void KisPaintDevice::uploadLodDataStruct(LodDataStruct *dst)
{
    m_d->uploadLodDataStruct(dst);
}

void KisPaintDevice::generateLodCloneDevice(KisPaintDeviceSP dst, const QRect &originalRect, int lod)
{
    m_d->generateLodCloneDevice(dst, originalRect, lod);
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

void KisPaintDeviceFramesInterface::uploadFrame(int srcFrameId, int dstFrameId, KisPaintDeviceSP srcDevice)
{
    q->m_d->uploadFrame(srcFrameId, dstFrameId, srcDevice);
}

void KisPaintDeviceFramesInterface::uploadFrame(int dstFrameId, KisPaintDeviceSP srcDevice)
{
    q->m_d->uploadFrame(dstFrameId, srcDevice);
}

QRect KisPaintDeviceFramesInterface::frameBounds(int frameId)
{
    return q->m_d->frameBounds(frameId);
}

QPoint KisPaintDeviceFramesInterface::frameOffset(int frameId) const
{
    return q->m_d->frameOffset(frameId);
}

void KisPaintDeviceFramesInterface::setFrameDefaultPixel(const KoColor &defPixel, int frameId)
{
    KIS_ASSERT_RECOVER_RETURN(frameId >= 0);
    q->m_d->setFrameDefaultPixel(defPixel, frameId);
}

KoColor KisPaintDeviceFramesInterface::frameDefaultPixel(int frameId) const
{
    KIS_ASSERT_RECOVER(frameId >= 0) {
        return KoColor(Qt::red, q->m_d->colorSpace());
    }
    return q->m_d->frameDefaultPixel(frameId);
}

bool KisPaintDeviceFramesInterface::writeFrame(KisPaintDeviceWriter &store, int frameId)
{
    KIS_ASSERT_RECOVER(frameId >= 0) {
        return false;
    }
    return q->m_d->writeFrame(store, frameId);
}

bool KisPaintDeviceFramesInterface::readFrame(QIODevice *stream, int frameId)
{
    KIS_ASSERT_RECOVER(frameId >= 0) {
        return false;
    }
    return q->m_d->readFrame(stream, frameId);
}

int KisPaintDeviceFramesInterface::currentFrameId() const
{
    return q->m_d->currentFrameId();
}

KisDataManagerSP KisPaintDeviceFramesInterface::frameDataManager(int frameId) const
{
    KIS_ASSERT_RECOVER(frameId >= 0) {
        return q->m_d->dataManager();
    }
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

    objects.m_data = q->m_d->m_data.data();
    objects.m_lodData = q->m_d->m_lodData.data();
    objects.m_externalFrameData = q->m_d->m_externalFrameData.data();

    typedef KisPaintDevice::Private::FramesHash FramesHash;

    FramesHash::const_iterator it = q->m_d->m_frames.constBegin();
    FramesHash::const_iterator end = q->m_d->m_frames.constEnd();

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

void KisPaintDevice::tesingFetchLodDevice(KisPaintDeviceSP targetDevice)
{
    m_d->tesingFetchLodDevice(targetDevice);
}
