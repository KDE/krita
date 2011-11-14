/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_simple_update_queue_test.h"
#include <qtest_kde.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_adjustment_layer.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_selection.h"

#include "kis_update_job_item.h"
#include "kis_simple_update_queue.h"
#include "scheduler_utils.h"


void KisSimpleUpdateQueueTest::testJobProcessing()
{
    KisTestableUpdaterContext context(2);

    QRect imageRect(0,0,200,200);

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, imageRect.width(), imageRect.height(), cs, "merge test");

    KisPaintLayerSP paintLayer = new KisPaintLayer(image, "test", OPACITY_OPAQUE_U8);

    image->lock();
    image->addNode(paintLayer);
    image->unlock();

    QRect dirtyRect1(0,0,50,100);
    QRect dirtyRect2(0,0,100,100);
    QRect dirtyRect3(50,0,50,100);
    QRect dirtyRect4(150,150,50,50);

    QVector<KisUpdateJobItem*> jobs;
    KisWalkersList walkersList;

    /**
     * Process the queue and look what has been added into
     * the updater context
     */

    KisTestableSimpleUpdateQueue queue;

    queue.addUpdateJob(paintLayer, dirtyRect1, imageRect);
    queue.addUpdateJob(paintLayer, dirtyRect2, imageRect);
    queue.addUpdateJob(paintLayer, dirtyRect3, imageRect);
    queue.addUpdateJob(paintLayer, dirtyRect4, imageRect);

    queue.processQueue(context);

    jobs = context.getJobs();

    QVERIFY(checkWalker(jobs[0]->walker(), dirtyRect2));
    QVERIFY(checkWalker(jobs[1]->walker(), dirtyRect4));
    QCOMPARE(jobs.size(), 2);

    walkersList = queue.getWalkersList();

    QCOMPARE(walkersList.size(), 0);
}

void KisSimpleUpdateQueueTest::testSplitUpdate()
{
    testSplit(false);
}

void KisSimpleUpdateQueueTest::testSplitFullRefresh()
{
    testSplit(true);
}

void KisSimpleUpdateQueueTest::testSplit(bool useFullRefresh)
{
    QRect imageRect(0,0,1024,1024);

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, imageRect.width(), imageRect.height(), cs, "merge test");

    KisPaintLayerSP paintLayer = new KisPaintLayer(image, "test", OPACITY_OPAQUE_U8);

    image->lock();
    image->addNode(paintLayer);
    image->unlock();

    QRect dirtyRect1(0,0,1000,1000);

    KisTestableSimpleUpdateQueue queue;
    KisWalkersList& walkersList = queue.getWalkersList();

    if(!useFullRefresh) {
        queue.addUpdateJob(paintLayer, dirtyRect1, imageRect);
    }
    else {
        queue.addFullRefreshJob(paintLayer, dirtyRect1, imageRect);
    }

    QCOMPARE(walkersList.size(), 4);

    QVERIFY(checkWalker(walkersList[0], QRect(0,0,512,512)));
    QVERIFY(checkWalker(walkersList[1], QRect(512,0,488,512)));
    QVERIFY(checkWalker(walkersList[2], QRect(0,512,512,488)));
    QVERIFY(checkWalker(walkersList[3], QRect(512,512,488,488)));

    queue.optimize();

    //must change nothing

    QCOMPARE(walkersList.size(), 4);
    QVERIFY(checkWalker(walkersList[0], QRect(0,0,512,512)));
    QVERIFY(checkWalker(walkersList[1], QRect(512,0,488,512)));
    QVERIFY(checkWalker(walkersList[2], QRect(0,512,512,488)));
    QVERIFY(checkWalker(walkersList[3], QRect(512,512,488,488)));
}

void KisSimpleUpdateQueueTest::testChecksum()
{
    QRect imageRect(0,0,512,512);
    QRect dirtyRect(100,100,100,100);

    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, imageRect.width(), imageRect.height(), colorSpace, "test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisAdjustmentLayerSP adjustmentLayer = new KisAdjustmentLayer(image, "adj", 0, 0);

    image->lock();
    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(adjustmentLayer, image->rootLayer());
    image->unlock();

    KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
    Q_ASSERT(filter);
    KisFilterConfiguration *configuration = filter->defaultConfiguration(0);


    KisTestableSimpleUpdateQueue queue;
    KisWalkersList& walkersList = queue.getWalkersList();

    queue.addUpdateJob(adjustmentLayer, dirtyRect, imageRect);
    QCOMPARE(walkersList[0]->checksumValid(), true);

    adjustmentLayer->setFilter(configuration);
    QCOMPARE(walkersList[0]->checksumValid(), false);

    QVector<KisUpdateJobItem*> jobs;
    KisTestableUpdaterContext context(2);

    queue.processQueue(context);
    jobs = context.getJobs();

    QCOMPARE(jobs[0]->walker()->checksumValid(), true);

}

void KisSimpleUpdateQueueTest::testMixingTypes()
{
    QRect imageRect(0,0,1024,1024);

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, imageRect.width(), imageRect.height(), cs, "merge test");

    KisPaintLayerSP paintLayer = new KisPaintLayer(image, "test", OPACITY_OPAQUE_U8);

    image->lock();
    image->addNode(paintLayer);
    image->unlock();

    QRect dirtyRect1(0,0,200,200);
    QRect dirtyRect2(0,0,200,200);
    QRect dirtyRect3(20,20,200,200);

    KisTestableSimpleUpdateQueue queue;
    KisWalkersList& walkersList = queue.getWalkersList();

    queue.addUpdateJob(paintLayer, dirtyRect1, imageRect);
    queue.addFullRefreshJob(paintLayer, dirtyRect2, imageRect);
    queue.addFullRefreshJob(paintLayer, dirtyRect3, imageRect);


    QCOMPARE(walkersList.size(), 2);

    QVERIFY(checkWalker(walkersList[0], QRect(0,0,200,200)));
    QVERIFY(checkWalker(walkersList[1], QRect(0,0,220,220)));

    QCOMPARE(walkersList[0]->type(), KisBaseRectsWalker::UPDATE);
    QCOMPARE(walkersList[1]->type(), KisBaseRectsWalker::FULL_REFRESH);
}

QTEST_KDEMAIN(KisSimpleUpdateQueueTest, NoGUI)
#include "kis_simple_update_queue_test.moc"

