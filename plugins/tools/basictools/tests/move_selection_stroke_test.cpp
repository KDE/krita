/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "move_selection_stroke_test.h"

#include <QTest>

#include <KoColor.h>

#include "kis_image.h"
#include "strokes/move_selection_stroke_strategy.h"
#include "stroke_testing_utils.h"
#include "kis_selection.h"
#include "commands/kis_selection_commands.h"
#include "strokes/move_stroke_strategy.h"
#include "kis_paint_layer.h"
#include "kis_image_barrier_locker.h"
#include "kis_paint_device_frames_interface.h"
#include "kis_paint_device_debug_utils.h"

#include <testui.h>

KisPaintDeviceSP lodDevice(KisPaintDeviceSP dev)
{
    KisPaintDeviceSP tmp = new KisPaintDevice(dev->colorSpace());
    dev->tesingFetchLodDevice(tmp);
    return tmp;
}


void MoveSelectionStrokeTest::test()
{
    const QRect imageRect(0,0,800,800);
    KisImageSP image = utils::createImage(0, imageRect.size());
    QScopedPointer<KoCanvasResourceProvider> manager(
        utils::createResourceManager(image));

    image->setLodPreferences(KisLodPreferences(2));
    image->waitForDone();

    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image,
                                 image->root()->firstChild(),
                                 manager.data());

    KisNodeSP currentNode = resources->currentNode();
    KisPaintLayerSP currentPaintLayer = dynamic_cast<KisPaintLayer*>(currentNode.data());
    Q_ASSERT(currentPaintLayer);

    KisPaintDeviceSP device = currentNode->paintDevice();

    {
        KisImageBarrierLocker locker(image);

        device->fill(QRect(0,0,400,400),     KoColor(Qt::red, image->colorSpace()));
        device->fill(QRect(400,0,400,400),   KoColor(Qt::green, image->colorSpace()));
        device->fill(QRect(0,400,400,400),   KoColor(Qt::blue, image->colorSpace()));
        device->fill(QRect(400,400,400,400), KoColor(Qt::yellow, image->colorSpace()));
    }

    {
        KisSelectionSP newSelection = new KisSelection();
        newSelection->pixelSelection()->select(QRect(200,200,400,400), OPACITY_OPAQUE_U8);

        KisSetGlobalSelectionCommand cmd(image, newSelection);
        cmd.redo();
    }

    KIS_DUMP_DEVICE_2(device, imageRect, "00_0_device", "mm");
    KIS_DUMP_DEVICE_2(image->globalSelection()->projection(), imageRect, "01_0_selection", "mm");

    {
        MoveSelectionStrokeStrategy *strategy =
            new MoveSelectionStrokeStrategy(currentPaintLayer,
                                            image->globalSelection(),
                                            image.data(),
                                            image.data());

        KisStrokeId id = image->startStroke(strategy);
        image->addJob(id, new MoveStrokeStrategy::Data(QPoint(100,100)));
        image->endStroke(id);

        image->waitForDone();

        KIS_DUMP_DEVICE_2(device, imageRect, "02_0_device", "mm");
        KIS_DUMP_DEVICE_2(lodDevice(device), imageRect, "02_1_device_lod", "mm");

        KIS_DUMP_DEVICE_2(image->globalSelection()->projection(), imageRect, "03_0_selection", "mm");
        KIS_DUMP_DEVICE_2(lodDevice(image->globalSelection()->projection()), imageRect, "03_1_selection_lod", "mm");

    }

    {
        MoveSelectionStrokeStrategy *strategy =
            new MoveSelectionStrokeStrategy(currentPaintLayer,
                                            image->globalSelection(),
                                            image.data(),
                                            image.data());

        KisStrokeId id = image->startStroke(strategy);
        image->addJob(id, new MoveStrokeStrategy::Data(QPoint(-200,50)));
        image->endStroke(id);

        image->waitForDone();

        KIS_DUMP_DEVICE_2(device, imageRect, "04_0_device", "mm");
        KIS_DUMP_DEVICE_2(lodDevice(device), imageRect, "04_1_device_lod", "mm");

        KIS_DUMP_DEVICE_2(image->globalSelection()->projection(), imageRect, "05_0_selection", "mm");
        KIS_DUMP_DEVICE_2(lodDevice(image->globalSelection()->projection()), imageRect, "05_1_selection_lod", "mm");
    }

}

KISTEST_MAIN(MoveSelectionStrokeTest)
