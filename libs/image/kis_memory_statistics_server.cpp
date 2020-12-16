/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_memory_statistics_server.h"

#include <QGlobalStatic>
#include <QApplication>

#include "kis_image.h"
#include "kis_image_config.h"
#include "kis_signal_compressor.h"

#include "tiles3/kis_tile_data_store.h"

Q_GLOBAL_STATIC(KisMemoryStatisticsServer, s_instance)

struct Q_DECL_HIDDEN KisMemoryStatisticsServer::Private
{
    Private(KisMemoryStatisticsServer *q)
        : updateCompressor(1000 /* ms */, KisSignalCompressor::POSTPONE, q)
    {
    }

    KisSignalCompressor updateCompressor;
};


KisMemoryStatisticsServer::KisMemoryStatisticsServer()
    : m_d(new Private(this))
{
    /**
     * The first instance() call may happen from non-gui thread,
     * so we should ensure the signals and timers are running in the
     * correct (GUI) thread.
     */
    moveToThread(qApp->thread());
    connect(&m_d->updateCompressor, SIGNAL(timeout()), SIGNAL(sigUpdateMemoryStatistics()));
}

KisMemoryStatisticsServer::~KisMemoryStatisticsServer()
{
}

KisMemoryStatisticsServer* KisMemoryStatisticsServer::instance()
{
    return s_instance;
}

inline void addDevice(KisPaintDeviceSP dev,
                      bool isProjection,
                      QSet<KisPaintDevice*> &devices,
                      qint64 &memBound,
                      qint64 &layersSize,
                      qint64 &projectionsSize,
                      qint64 &lodSize)
{
    if (dev && !devices.contains(dev.data())) {
        devices.insert(dev.data());

        qint64 imageData = 0;
        qint64 temporaryData = 0;
        qint64 lodData = 0;

        dev->estimateMemoryStats(imageData, temporaryData, lodData);
        memBound += imageData + temporaryData + lodData;

        KIS_SAFE_ASSERT_RECOVER_NOOP(!temporaryData || isProjection);

        if (!isProjection) {
            layersSize += imageData + temporaryData;
        } else {
            projectionsSize += imageData + temporaryData;
        }

        lodSize += lodData;
    }
}

qint64 calculateNodeMemoryHiBoundStep(KisNodeSP node,
                                      QSet<KisPaintDevice*> &devices,
                                      qint64 &layersSize,
                                      qint64 &projectionsSize,
                                      qint64 &lodSize)
{
    qint64 memBound = 0;

    const bool originalIsProjection =
            node->inherits("KisGroupLayer") ||
            node->inherits("KisAdjustmentLayer");


    addDevice(node->paintDevice(), false, devices, memBound, layersSize, projectionsSize, lodSize);
    addDevice(node->original(), originalIsProjection, devices, memBound, layersSize, projectionsSize, lodSize);
    addDevice(node->projection(), true, devices, memBound, layersSize, projectionsSize, lodSize);

    node = node->firstChild();
    while (node) {
        memBound += calculateNodeMemoryHiBoundStep(node, devices,
                                                   layersSize, projectionsSize, lodSize);
        node = node->nextSibling();
    }

    return memBound;
}

qint64 calculateNodeMemoryHiBound(KisNodeSP node,
                                  qint64 &layersSize,
                                  qint64 &projectionsSize,
                                  qint64 &lodSize)
{
    layersSize = 0;
    projectionsSize = 0;
    lodSize = 0;

    QSet<KisPaintDevice*> devices;
    return calculateNodeMemoryHiBoundStep(node,
                                          devices,
                                          layersSize,
                                          projectionsSize,
                                          lodSize);
}


KisMemoryStatisticsServer::Statistics
KisMemoryStatisticsServer::fetchMemoryStatistics(KisImageSP image) const
{
    KisTileDataStore::MemoryStatistics tileStats =
        KisTileDataStore::instance()->memoryStatistics();

    Statistics stats;
    if (image) {
        stats.imageSize =
            calculateNodeMemoryHiBound(image->root(),
                                       stats.layersSize,
                                       stats.projectionsSize,
                                       stats.lodSize);
    }
    stats.totalMemorySize = tileStats.totalMemorySize;
    stats.realMemorySize = tileStats.realMemorySize;
    stats.historicalMemorySize = tileStats.historicalMemorySize;
    stats.poolSize = tileStats.poolSize;

    stats.swapSize = tileStats.swapSize;

    KisImageConfig cfg(true);

    stats.tilesHardLimit = cfg.tilesHardLimit() * MiB;
    stats.tilesSoftLimit = cfg.tilesSoftLimit() * MiB;
    stats.tilesPoolLimit = cfg.poolLimit() * MiB;
    stats.totalMemoryLimit = stats.tilesHardLimit + stats.tilesPoolLimit;

    return stats;
}

void KisMemoryStatisticsServer::notifyImageChanged()
{
    m_d->updateCompressor.start();
}


