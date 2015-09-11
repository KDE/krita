/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_memory_statistics_server.h"

#include <kglobal.h>

#include "kis_image.h"
#include "kis_image_config.h"
#include "kis_signal_compressor.h"

#include "tiles3/kis_tile_data_store.h"


struct Q_DECL_HIDDEN KisMemoryStatisticsServer::Private
{
    Private()
        : updateCompressor(1000 /* ms */, KisSignalCompressor::POSTPONE)
    {
    }

    KisSignalCompressor updateCompressor;
};


KisMemoryStatisticsServer::KisMemoryStatisticsServer()
    : m_d(new Private)
{
    connect(&m_d->updateCompressor, SIGNAL(timeout()), SIGNAL(sigUpdateMemoryStatistics()));
}

KisMemoryStatisticsServer::~KisMemoryStatisticsServer()
{
}

KisMemoryStatisticsServer* KisMemoryStatisticsServer::instance()
{
    K_GLOBAL_STATIC(KisMemoryStatisticsServer, s_instance);
    return s_instance;
}

inline void addDevice(KisPaintDeviceSP dev,
                      QSet<KisPaintDevice*> &devices,
                      qint64 &memBound)
{
    if (dev && !devices.contains(dev.data())) {
        devices.insert(dev.data());
        memBound +=
            dev->extent().width() *
            dev->extent().height() *
            dev->pixelSize();
    }
}

qint64 calculateNodeMemoryHiBoundStep(KisNodeSP node,
                                      QSet<KisPaintDevice*> &devices)
{
    qint64 memBound = 0;

    addDevice(node->paintDevice(), devices, memBound);
    addDevice(node->original(), devices, memBound);
    addDevice(node->projection(), devices, memBound);

    node = node->firstChild();
    while (node) {
        memBound += calculateNodeMemoryHiBoundStep(node, devices);
        node = node->nextSibling();
    }

    return memBound;
}

qint64 calculateNodeMemoryHiBound(KisNodeSP node)
{
    QSet<KisPaintDevice*> devices;
    return calculateNodeMemoryHiBoundStep(node, devices);
}


KisMemoryStatisticsServer::Statistics
KisMemoryStatisticsServer::fetchMemoryStatistics(KisImageSP image) const
{
    KisTileDataStore::MemoryStatistics tileStats =
        KisTileDataStore::instance()->memoryStatistics();

    Statistics stats;
    if (image) {
        stats.imageSize = calculateNodeMemoryHiBound(image->root());
    }
    stats.totalMemorySize = tileStats.totalMemorySize;
    stats.realMemorySize = tileStats.realMemorySize;
    stats.historicalMemorySize = tileStats.historicalMemorySize;
    stats.poolSize = tileStats.poolSize;

    stats.swapSize = tileStats.swapSize;

    KisImageConfig cfg;

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


