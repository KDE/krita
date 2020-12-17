/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_low_memory_benchmark.h"

#include <QTest>

#include "kis_benchmark_values.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>
#include "kis_paint_device.h"
#include "kis_painter.h"

#include <brushengine/kis_paint_information.h>
#include <brushengine/kis_paintop_registry.h>
#include <brushengine/kis_paintop_preset.h>

#include "KisGlobalResourcesInterface.h"

#include "tiles3/kis_tile_data_store.h"
#include "kis_surrogate_undo_adapter.h"
#include "kis_image_config.h"
#define LOAD_PRESET_OR_RETURN(preset, fileName)                         \
    if(!preset->load(KisGlobalResourcesInterface::instance())) { dbgKrita << "Preset" << fileName << "was NOT loaded properly. Done."; return; } \
    else dbgKrita << "Loaded preset:" << fileName

#define HUGE_IMAGE_SIZE 8000

/**
 * This benchmark runs a series of huge strokes on a canvas with a
 * particular configuration of the swapper/pooler and history
 * management. After the test is done you can visualize the results
 * with the GNU Octave. Please use kis_low_memory_show_report.m file
 * for that.
 */
void KisLowMemoryBenchmark::benchmarkWideArea(const QString presetFileName,
                                              const QRectF &rect, qreal vstep,
                                              int numCycles,
                                              bool createTransaction,
                                              int hardLimitMiB,
                                              int softLimitMiB,
                                              int poolLimitMiB,
                                              int index)
{
    KisPaintOpPresetSP preset(new KisPaintOpPreset(QString(FILES_DATA_DIR) + '/' + presetFileName));
    LOAD_PRESET_OR_RETURN(preset, presetFileName);


    /**
     * Initialize image and painter
     */
    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, HUGE_IMAGE_SIZE, HUGE_IMAGE_SIZE, colorSpace, "stroke sample image");
    KisLayerSP layer = new KisPaintLayer(image, "temporary for stroke sample", OPACITY_OPAQUE_U8, colorSpace);
    KisLayerSP layerExtra = new KisPaintLayer(image, "temporary for threading", OPACITY_OPAQUE_U8, colorSpace);

    image->addNode(layer, image->root());
    image->addNode(layerExtra, image->root());

    KisPainter *painter = new KisPainter(layer->paintDevice());

    painter->setPaintColor(KoColor(Qt::black, colorSpace));
    painter->setPaintOpPreset(preset, layer, image);

    /**
     * A simple adapter that will store all the transactions for us
     */
    KisSurrogateUndoAdapter undoAdapter;

    /**
     * Reset configuration to the desired settings
     */
    KisImageConfig config(false);
    qreal oldHardLimit = config.memoryHardLimitPercent();
    qreal oldSoftLimit = config.memorySoftLimitPercent();
    qreal oldPoolLimit = config.memoryPoolLimitPercent();
    const qreal _MiB = 100.0 / KisImageConfig::totalRAM();

    config.setMemoryHardLimitPercent(hardLimitMiB * _MiB);
    config.setMemorySoftLimitPercent(softLimitMiB * _MiB);
    config.setMemoryPoolLimitPercent(poolLimitMiB * _MiB);

    KisTileDataStore::instance()->testingRereadConfig();

    /**
     * Create an empty the log file
     */
    QString fileName;
    fileName = QString("log_%1_%2_%3_%4_%5.txt")
        .arg(createTransaction)
        .arg(hardLimitMiB)
        .arg(softLimitMiB)
        .arg(poolLimitMiB)
        .arg(index);

    QFile logFile(fileName);
    logFile.open(QFile::WriteOnly | QFile::Truncate);
    QTextStream logStream(&logFile);
    logStream.setFieldWidth(10);
    logStream.setFieldAlignment(QTextStream::AlignRight);

    /**
     * Start painting on the image
     */

    QElapsedTimer cycleTime;
    QElapsedTimer lineTime;
    cycleTime.start();
    lineTime.start();

    qreal rectBottom = rect.y() + rect.height();

    for (int i = 0; i < numCycles; i++) {
        cycleTime.restart();

        QLineF line(rect.topLeft(), rect.topLeft() + QPointF(rect.width(), 0));
        if (createTransaction) {
            painter->beginTransaction();
        }

        KisDistanceInformation currentDistance;

        while(line.y1() < rectBottom) {
            lineTime.restart();

            KisPaintInformation pi1(line.p1(), 0.0);
            KisPaintInformation pi2(line.p2(), 1.0);
            painter->paintLine(pi1, pi2, &currentDistance);
            painter->device()->setDirty(painter->takeDirtyRegion());

            logStream << "L 1" << i << lineTime.elapsed()
                      << KisTileDataStore::instance()->numTilesInMemory() * 16
                      << KisTileDataStore::instance()->numTiles() * 16
                      << createTransaction << endl;

            line.translate(0, vstep);
        }

        painter->device()->setDirty(painter->takeDirtyRegion());

        if (createTransaction) {
            painter->endTransaction(&undoAdapter);
        }

        // comment/uncomment to emulate user waiting after the stroke
        QTest::qSleep(1000);

        logStream << "C 2" << i << cycleTime.elapsed()
                  << KisTileDataStore::instance()->numTilesInMemory() * 16
                  << KisTileDataStore::instance()->numTiles() * 16
                  << createTransaction
                  << config.memoryHardLimitPercent() / _MiB
                  << config.memorySoftLimitPercent() / _MiB
                  << config.memoryPoolLimitPercent() / _MiB  << endl;
    }

    config.setMemoryHardLimitPercent(oldHardLimit * _MiB);
    config.setMemorySoftLimitPercent(oldSoftLimit * _MiB);
    config.setMemoryPoolLimitPercent(oldPoolLimit * _MiB);

    delete painter;
}

void KisLowMemoryBenchmark::unlimitedMemoryNoHistoryNoPool()
{
    QString presetFileName = "autobrush_300px.kpp";
    // one cycle takes about 48 MiB of memory (total 960 MiB)
    QRectF rect(150,150,4000,4000);
    qreal step = 250;
    int numCycles = 20;

    benchmarkWideArea(presetFileName, rect, step, numCycles, false,
                      3000, 3000, 0, 0);
}

void KisLowMemoryBenchmark::unlimitedMemoryHistoryNoPool()
{
    QString presetFileName = "autobrush_300px.kpp";
    // one cycle takes about 48 MiB of memory (total 960 MiB)
    QRectF rect(150,150,4000,4000);
    qreal step = 250;
    int numCycles = 20;

    benchmarkWideArea(presetFileName, rect, step, numCycles, true,
                      3000, 3000, 0, 0);
}

void KisLowMemoryBenchmark::unlimitedMemoryHistoryPool50()
{
    QString presetFileName = "autobrush_300px.kpp";
    // one cycle takes about 48 MiB of memory (total 960 MiB)
    QRectF rect(150,150,4000,4000);
    qreal step = 250;
    int numCycles = 20;

    benchmarkWideArea(presetFileName, rect, step, numCycles, true,
                      3000, 3000, 50, 0);
}

void KisLowMemoryBenchmark::memory2000History100Pool500HugeBrush()
{
    QString presetFileName = "BIG_TESTING.kpp";
    // one cycle takes about 316 MiB of memory (total 3+ GiB)
    QRectF rect(150,150,7850,7850);
    qreal step = 250;
    int numCycles = 10;

    benchmarkWideArea(presetFileName, rect, step, numCycles, true,
                      2000, 600, 500, 0);
}

QTEST_MAIN(KisLowMemoryBenchmark)
