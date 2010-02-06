/*
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_async_merger_test.h"

#include "kis_graph_walker.h"
#include "kis_merge_walkers.h"
#include "kis_async_merger.h"

#include <qtest_kde.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_selection.h"

#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"


    /*
      +-----------+
      |root       |
      | group     |
      |  blur 1   |
      |  paint 2  |
      | paint 1   |
      +-----------+
     */

void KisAsyncMergerTest::testMerger()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageWSP image = new KisImage(0, 640, 448, colorSpace, "merger test");

    QImage sourceImage1(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    QImage sourceImage2(QString(FILES_DATA_DIR) + QDir::separator() + "inverted_hakonepa.png");
    QImage referenceProjection(QString(FILES_DATA_DIR) + QDir::separator() + "merged_hakonepa.png");

    KisPaintDeviceSP device1 = new KisPaintDevice(colorSpace);
    KisPaintDeviceSP device2 = new KisPaintDevice(colorSpace);
    device1->convertFromQImage(sourceImage1, "", 0, 0);
    device2->convertFromQImage(sourceImage2, "", 0, 0);

    KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
    Q_ASSERT(filter);
    KisFilterConfiguration *configuration = filter->defaultConfiguration(0);
    Q_ASSERT(configuration);

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE, device1);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE, device2);
    KisLayerSP groupLayer = new KisGroupLayer(image, "group", 200/*OPACITY_OPAQUE*/);
    KisLayerSP blur1 = new KisAdjustmentLayer(image, "blur1", configuration, 0);

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(groupLayer, image->rootLayer());

    image->addNode(paintLayer2, groupLayer);
    image->addNode(blur1, groupLayer);


    QRect testRect1(0,0,100,441);
    QRect testRect2(100,0,400,441);
    QRect testRect3(500,0,140,441);
    QRect testRect4(580,381,40,40);

    KisMergeWalker walker;
    KisAsyncMerger merger;

    walker.collectRects(paintLayer2, testRect1);
    merger.startMerge(walker);

    walker.collectRects(paintLayer2, testRect2);
    merger.startMerge(walker);

    walker.collectRects(paintLayer2, testRect3);
    merger.startMerge(walker);

    walker.collectRects(paintLayer2, testRect4);
    merger.startMerge(walker);

    // Old style merging: has artefacts at x=100 and x=500
    // And should be turned on inside KisLayer
/*    paintLayer2->setDirty(testRect1);
    QTest::qSleep(3000);
    paintLayer2->setDirty(testRect2);
    QTest::qSleep(3000);
    paintLayer2->setDirty(testRect3);
    QTest::qSleep(3000);
    paintLayer2->setDirty(testRect4);
    QTest::qSleep(3000);
*/

    KisLayerSP rootLayer = image->rootLayer();
    QVERIFY(rootLayer->extent() == image->bounds());

    QImage resultProjection = rootLayer->projection()->convertToQImage(0);
    resultProjection.save(QString(FILES_OUTPUT_DIR) + QDir::separator() + "actual_merge_result.png");
    QVERIFY(resultProjection == referenceProjection);
}


QTEST_KDEMAIN(KisAsyncMergerTest, NoGUI)
#include "kis_async_merger_test.moc"

