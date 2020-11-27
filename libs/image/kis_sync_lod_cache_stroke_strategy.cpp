/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_sync_lod_cache_stroke_strategy.h"

#include <kis_image.h>
#include <kundo2magicstring.h>
#include "krita_utils.h"
#include "kis_layer_utils.h"


struct KisSyncLodCacheStrokeStrategy::Private
{
    KisImageWSP image;
    QHash<KisPaintDeviceSP, KisPaintDevice::LodDataStruct*> dataObjects;

    ~Private() {
        qDeleteAll(dataObjects);
        dataObjects.clear();
    }

    class InitData : public KisStrokeJobData {
    public:
        InitData(KisPaintDeviceSP _device)
            : KisStrokeJobData(SEQUENTIAL),
              device(_device)
            {}

        KisPaintDeviceSP device;
    };

    class ProcessData : public KisStrokeJobData {
    public:
        ProcessData(KisPaintDeviceSP _device, const QRect &_rect)
            : KisStrokeJobData(CONCURRENT),
              device(_device), rect(_rect)
            {}

        KisPaintDeviceSP device;
        QRect rect;
    };

    class AdditionalProcessNode : public KisStrokeJobData {
    public:
        AdditionalProcessNode(KisNodeSP _node)
            : KisStrokeJobData(SEQUENTIAL),
              node(_node)
            {}

        KisNodeSP node;
    };
};

KisSyncLodCacheStrokeStrategy::KisSyncLodCacheStrokeStrategy(KisImageWSP image, bool forgettable)
    : KisSimpleStrokeStrategy(QLatin1String("SyncLodCacheStroke"), kundo2_i18n("Instant Preview")),
      m_d(new Private)
{
    m_d->image = image;

    /**
     * We shouldn't start syncing before all the updates are
     * done. Otherwise we might get artifacts!
     */
    enableJob(KisSimpleStrokeStrategy::JOB_INIT, true, KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);
    enableJob(KisSimpleStrokeStrategy::JOB_FINISH);
    enableJob(KisSimpleStrokeStrategy::JOB_CANCEL, true, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);

    setRequestsOtherStrokesToEnd(false);
    setClearsRedoOnStart(false);
    setCanForgetAboutMe(forgettable);
}

KisSyncLodCacheStrokeStrategy::~KisSyncLodCacheStrokeStrategy()
{
}

void KisSyncLodCacheStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    Private::InitData *initData = dynamic_cast<Private::InitData*>(data);
    Private::ProcessData *processData = dynamic_cast<Private::ProcessData*>(data);
    Private::AdditionalProcessNode *additionalProcessNode = dynamic_cast<Private::AdditionalProcessNode*>(data);

    if (initData) {
        KisPaintDeviceSP dev = initData->device;
        const int lod = dev->defaultBounds()->currentLevelOfDetail();
        m_d->dataObjects.insert(dev, dev->createLodDataStruct(lod));
    } else if (processData) {
        KisPaintDeviceSP dev = processData->device;
        KIS_ASSERT(m_d->dataObjects.contains(dev));

        KisPaintDevice::LodDataStruct *data = m_d->dataObjects.value(dev);
        dev->updateLodDataStruct(data, processData->rect);
    } else if (additionalProcessNode) {
        additionalProcessNode->node->syncLodCache();
    }
}

void KisSyncLodCacheStrokeStrategy::finishStrokeCallback()
{
    auto it = m_d->dataObjects.begin();
    auto end = m_d->dataObjects.end();

    for (; it != end; ++it) {
        KisPaintDeviceSP dev = it.key();
        dev->uploadLodDataStruct(it.value());
    }

    qDeleteAll(m_d->dataObjects);
    m_d->dataObjects.clear();
}

void KisSyncLodCacheStrokeStrategy::cancelStrokeCallback()
{
    qDeleteAll(m_d->dataObjects);
    m_d->dataObjects.clear();
}

QList<KisStrokeJobData*> KisSyncLodCacheStrokeStrategy::createJobsData(KisImageWSP _image)
{
    using KisLayerUtils::recursiveApplyNodes;
    using KritaUtils::splitRegionIntoPatches;
    using KritaUtils::optimalPatchSize;

    KisImageSP image = _image;

    KisPaintDeviceList deviceList;
    QList<KisStrokeJobData*> jobsData;

    recursiveApplyNodes(image->root(),
                        [&deviceList](KisNodeSP node) {
                            deviceList << node->getLodCapableDevices();
                        });

    KritaUtils::makeContainerUnique(deviceList);

    Q_FOREACH (KisPaintDeviceSP device, deviceList) {
        jobsData << new Private::InitData(device);
    }

    Q_FOREACH (KisPaintDeviceSP device, deviceList) {
        KisRegion region = device->regionForLodSyncing();
        QVector<QRect> rects = splitRegionIntoPatches(region, optimalPatchSize());

        Q_FOREACH (const QRect &rc, rects) {
            jobsData << new Private::ProcessData(device, rc);
        }
    }

    recursiveApplyNodes(image->root(),
                        [&jobsData](KisNodeSP node) {
                            jobsData << new Private::AdditionalProcessNode(node);
                        });

    return jobsData;
}
