/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_sync_lod_cache_stroke_strategy.h"

#include <kis_image.h>
#include <kundo2magicstring.h>
#include "krita_utils.h"
#include "kis_layer_utils.h"
#include "kis_pointer_utils.h"
#include "KisRunnableStrokeJobUtils.h"

struct KisSyncLodCacheStrokeStrategy::Private
{
    KisImageWSP image;
};

KisSyncLodCacheStrokeStrategy::KisSyncLodCacheStrokeStrategy(KisImageWSP image, bool forgettable)
    : KisRunnableBasedStrokeStrategy(QLatin1String("SyncLodCacheStroke"), kundo2_i18n("Instant Preview")),
      m_d(new Private)
{
    m_d->image = image;

    /**
     * We shouldn't start syncing before all the updates are
     * done. Otherwise we might get artifacts!
     */
    enableJob(KisSimpleStrokeStrategy::JOB_INIT, true, KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);

    setRequestsOtherStrokesToEnd(false);
    setClearsRedoOnStart(false);
    setCanForgetAboutMe(forgettable);
}

KisSyncLodCacheStrokeStrategy::~KisSyncLodCacheStrokeStrategy()
{
}

void KisSyncLodCacheStrokeStrategy::initStrokeCallback()
{
    QVector<KisStrokeJobData *> jobs;
    createJobsData(jobs, m_d->image->root(), m_d->image->currentLevelOfDetail());
    addMutatedJobs(jobs);
}

QList<KisStrokeJobData*> KisSyncLodCacheStrokeStrategy::createJobsData(KisImageWSP /*_image*/)
{
    // all the jobs are populates in the init job
    return {};
}

void KisSyncLodCacheStrokeStrategy::createJobsData(QVector<KisStrokeJobData *> &jobs, KisNodeSP imageRoot, int levelOfDetail, KisPaintDeviceList extraDevices)
{
    using KisLayerUtils::recursiveApplyNodes;
    using KritaUtils::splitRegionIntoPatches;
    using KritaUtils::optimalPatchSize;

    using SharedData = QHash<KisPaintDeviceSP, QSharedPointer<KisPaintDevice::LodDataStruct>>;
    using SharedDataSP = QSharedPointer<SharedData>;

    SharedDataSP sharedData(new SharedData());

    KisPaintDeviceList deviceList = extraDevices;

    recursiveApplyNodes(imageRoot,
        [&deviceList](KisNodeSP node) {
             deviceList << node->getLodCapableDevices();
        });

    KritaUtils::makeContainerUnique(deviceList);


    KritaUtils::addJobBarrier(jobs, [sharedData, deviceList, levelOfDetail] () mutable {
        Q_FOREACH (KisPaintDeviceSP device, deviceList) {
            sharedData->insert(device, toQShared(device->createLodDataStruct(levelOfDetail)));
        }
    });

    KritaUtils::addJobSequential(jobs, [](){});

    Q_FOREACH (KisPaintDeviceSP device, deviceList) {
        KisRegion region = device->regionForLodSyncing();
        QVector<QRect> rects = splitRegionIntoPatches(region, optimalPatchSize());

        Q_FOREACH (const QRect &rc, rects) {
            KritaUtils::addJobConcurrent(jobs, [sharedData, device, rc] () mutable {
                KIS_ASSERT(sharedData->contains(device));

                KisPaintDevice::LodDataStruct *data = sharedData->value(device).data();
                device->updateLodDataStruct(data, rc);
            });
        }
    }

    KritaUtils::addJobSequential(jobs, [](){});

    recursiveApplyNodes(imageRoot,
        [&jobs](KisNodeSP node) {
             KritaUtils::addJobConcurrent(jobs, [node] () mutable {
                 node->syncLodCache();
             });
        });

    KritaUtils::addJobSequential(jobs, [sharedData] () mutable {
        auto it = sharedData->begin();
        auto end = sharedData->end();

        for (; it != end; ++it) {
            KisPaintDeviceSP dev = it.key();
            dev->uploadLodDataStruct(it.value().data());
        }
    });
}
