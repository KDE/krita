/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_update_scheduler_test.h"
#include <simpletest.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_adjustment_layer.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_selection.h"

#include "scheduler_utils.h"
#include "kis_update_scheduler.h"
#include "kis_updater_context.h"
#include "kis_update_job_item.h"
#include "kis_simple_update_queue.h"
#include <KisGlobalResourcesInterface.h>

#include "../../sdk/tests/testutil.h"
#include "kistest.h"


KisImageSP KisUpdateSchedulerTest::buildTestingImage()
{
    QImage sourceImage1(QString(FILES_DATA_DIR) + '/' + "hakonepa.png");
    QImage sourceImage2(QString(FILES_DATA_DIR) + '/' + "inverted_hakonepa.png");

    QRect imageRect = QRect(QPoint(0,0), sourceImage1.size());

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, imageRect.width(), imageRect.height(), cs, "merge test");

    KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
    Q_ASSERT(filter);
    KisFilterConfigurationSP configuration = filter->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(configuration);

    KisPaintLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisPaintLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8 / 3);
    KisLayerSP blur1 = new KisAdjustmentLayer(image, "blur1", configuration->cloneWithResourcesSnapshot(), 0);

    paintLayer1->paintDevice()->convertFromQImage(sourceImage1, 0, 0, 0);
    paintLayer2->paintDevice()->convertFromQImage(sourceImage2, 0, 0, 0);

    image->barrierLock();
    image->addNode(paintLayer1);
    image->addNode(paintLayer2);
    image->addNode(blur1);
    image->unlock();

    return image;
}

void KisUpdateSchedulerTest::testMerge()
{
    KisImageSP image = buildTestingImage();
    QRect imageRect = image->bounds();
    KisNodeSP rootLayer = image->rootLayer();
    KisNodeSP paintLayer1 = rootLayer->firstChild();

    QCOMPARE(paintLayer1->name(), QString("paint1"));


    KisUpdateScheduler scheduler(image.data());

    /**
     * Test synchronous Full Refresh
     */

    scheduler.fullRefresh(rootLayer, image->bounds(), image->bounds());
    QCOMPARE(rootLayer->exactBounds(), image->bounds());

    QImage resultFRProjection = rootLayer->projection()->convertToQImage(0);
    resultFRProjection.save(QString(FILES_OUTPUT_DIR) + '/' + "scheduler_fr_merge_result.png");

    /**
     * Test incremental updates
     */

    rootLayer->projection()->clear();

    const qint32 num = 4;
    qint32 width = imageRect.width() / num;
    qint32 lastWidth = imageRect.width() - width;

    QVector<QRect> dirtyRects(num);

    for(qint32 i = 0; i < num-1; i++) {
        dirtyRects[i] = QRect(width*i, 0, width, imageRect.height());
    }
    dirtyRects[num-1] = QRect(width*(num-1), 0, lastWidth, imageRect.height());

    for(qint32 i = 0; i < num; i+=2) {
        scheduler.updateProjection(paintLayer1, dirtyRects[i], image->bounds());
    }

    for(qint32 i = 1; i < num; i+=2) {
        scheduler.updateProjection(paintLayer1, dirtyRects[i], image->bounds());
    }

    scheduler.waitForDone();

    QCOMPARE(rootLayer->exactBounds(), image->bounds());

    QImage resultDirtyProjection = rootLayer->projection()->convertToQImage(0);
    resultDirtyProjection.save(QString(FILES_OUTPUT_DIR) + '/' + "scheduler_dp_merge_result.png");

    QPoint pt;
    QVERIFY(TestUtil::compareQImages(pt, resultFRProjection, resultDirtyProjection));
}

void KisUpdateSchedulerTest::benchmarkOverlappedMerge()
{
    KisImageSP image = buildTestingImage();
    KisNodeSP rootLayer = image->rootLayer();
    KisNodeSP paintLayer1 = rootLayer->firstChild();
    QRect imageRect = image->bounds();

    QCOMPARE(paintLayer1->name(), QString("paint1"));
    QCOMPARE(imageRect, QRect(0,0,640,441));

    KisUpdateScheduler scheduler(image.data());

    const qint32 xShift = 10;
    const qint32 yShift = 0;
    const qint32 numShifts = 64;

    QBENCHMARK{
        QRect dirtyRect(0, 0, 200, imageRect.height());

        for(int i = 0; i < numShifts; i++) {
            // dbgKrita << dirtyRect;
            scheduler.updateProjection(paintLayer1, dirtyRect, image->bounds());
            dirtyRect.translate(xShift, yShift);
        }

        scheduler.waitForDone();
    }
}

void KisUpdateSchedulerTest::testLocking()
{
    KisImageSP image = buildTestingImage();
    KisNodeSP rootLayer = image->rootLayer();
    KisNodeSP paintLayer1 = rootLayer->firstChild();
    QRect imageRect = image->bounds();

    QCOMPARE(paintLayer1->name(), QString("paint1"));
    QCOMPARE(imageRect, QRect(0,0,640,441));

    KisTestableUpdateScheduler scheduler(image.data(), 2);
    KisUpdaterContext *context = scheduler.updaterContext();

    QVector<KisUpdateJobItem*> jobs;

    QRect dirtyRect1(0,0,50,100);
    QRect dirtyRect2(0,0,100,100);
    QRect dirtyRect3(50,0,50,100);
    QRect dirtyRect4(150,150,50,50);

    scheduler.updateProjection(paintLayer1, imageRect, imageRect);

    jobs = context->getJobs();
    QCOMPARE(jobs[0]->isRunning(), true);
    QCOMPARE(jobs[1]->isRunning(), false);
    QVERIFY(checkWalker(jobs[0]->walker(), imageRect));

    context->clear();

    scheduler.lock();

    scheduler.updateProjection(paintLayer1, dirtyRect1, imageRect);
    scheduler.updateProjection(paintLayer1, dirtyRect2, imageRect);
    scheduler.updateProjection(paintLayer1, dirtyRect3, imageRect);
    scheduler.updateProjection(paintLayer1, dirtyRect4, imageRect);

    jobs = context->getJobs();
    QCOMPARE(jobs[0]->isRunning(), false);
    QCOMPARE(jobs[1]->isRunning(), false);

    scheduler.unlock();

    jobs = context->getJobs();
    QCOMPARE(jobs[0]->isRunning(), true);
    QCOMPARE(jobs[1]->isRunning(), true);
    QVERIFY(checkWalker(jobs[0]->walker(), dirtyRect2));
    QVERIFY(checkWalker(jobs[1]->walker(), dirtyRect4));
}

void KisUpdateSchedulerTest::testExclusiveStrokes()
{
    KisImageSP image = buildTestingImage();
    KisNodeSP rootLayer = image->rootLayer();
    KisNodeSP paintLayer1 = rootLayer->firstChild();
    QRect imageRect = image->bounds();

    QCOMPARE(paintLayer1->name(), QString("paint1"));
    QCOMPARE(imageRect, QRect(0,0,640,441));

    QRect dirtyRect1(0,0,50,100);

    KisTestableUpdateScheduler scheduler(image.data(), 2);
    KisUpdaterContext *context = scheduler.updaterContext();
    QVector<KisUpdateJobItem*> jobs;

    scheduler.updateProjection(paintLayer1, dirtyRect1, imageRect);

    jobs = context->getJobs();
    QCOMPARE(jobs[0]->isRunning(), true);
    QCOMPARE(jobs[1]->isRunning(), false);
    QVERIFY(checkWalker(jobs[0]->walker(), dirtyRect1));

    KisStrokeId id = scheduler.startStroke(new KisTestingStrokeStrategy(QLatin1String("excl_"), true, false));

    jobs = context->getJobs();
    QCOMPARE(jobs[0]->isRunning(), true);
    QCOMPARE(jobs[1]->isRunning(), false);
    QVERIFY(checkWalker(jobs[0]->walker(), dirtyRect1));

    context->clear();
    scheduler.endStroke(id);

    jobs = context->getJobs();
    QCOMPARE(jobs[0]->isRunning(), true);
    QCOMPARE(jobs[1]->isRunning(), false);
    COMPARE_NAME(jobs[0], "excl_init");

    scheduler.updateProjection(paintLayer1, dirtyRect1, imageRect);

    jobs = context->getJobs();
    QCOMPARE(jobs[0]->isRunning(), true);
    QCOMPARE(jobs[1]->isRunning(), false);
    COMPARE_NAME(jobs[0], "excl_init");

    context->clear();
    scheduler.processQueues();

    jobs = context->getJobs();
    QCOMPARE(jobs[0]->isRunning(), true);
    QCOMPARE(jobs[1]->isRunning(), false);
    COMPARE_NAME(jobs[0], "excl_finish");

    context->clear();
    scheduler.processQueues();

    jobs = context->getJobs();
    QCOMPARE(jobs[0]->isRunning(), true);
    QCOMPARE(jobs[1]->isRunning(), false);
    QVERIFY(checkWalker(jobs[0]->walker(), dirtyRect1));
}

void KisUpdateSchedulerTest::testEmptyStroke()
{
    KisImageSP image = buildTestingImage();

    KisStrokeId id = image->startStroke(new KisStrokeStrategy(QLatin1String()));
    image->addJob(id, 0);
    image->endStroke(id);
    image->waitForDone();
}

#include "kis_lazy_wait_condition.h"

void KisUpdateSchedulerTest::testLazyWaitCondition()
{
    {
        dbgKrita << "Not initialized";
        KisLazyWaitCondition condition;
        QVERIFY(!condition.wait(50));
    }

    {
        dbgKrita << "Initialized, not awake";
        KisLazyWaitCondition condition;
        condition.initWaiting();
        QVERIFY(!condition.wait(50));
        condition.endWaiting();
    }

    {
        dbgKrita << "Initialized, awake";
        KisLazyWaitCondition condition;
        condition.initWaiting();
        condition.wakeAll();
        QVERIFY(condition.wait(50));
        condition.endWaiting();
    }

    {
        dbgKrita << "Initialized, not awake, then awake";
        KisLazyWaitCondition condition;
        condition.initWaiting();
        QVERIFY(!condition.wait(50));
        condition.wakeAll();
        QVERIFY(condition.wait(50));
        condition.endWaiting();
    }

    {
        dbgKrita << "Doublewait";
        KisLazyWaitCondition condition;
        condition.initWaiting();
        condition.initWaiting();
        QVERIFY(!condition.wait(50));
        condition.wakeAll();
        QVERIFY(condition.wait(50));
        QVERIFY(condition.wait(50));
        condition.endWaiting();
    }
}

#define NUM_THREADS 10
#define NUM_CYCLES 500
#define NTH_CHECK 3

class UpdatesBlockTester : public QRunnable
{
public:
    UpdatesBlockTester(KisUpdateScheduler *scheduler, KisNodeSP node)
        : m_scheduler(scheduler), m_node(node)
    {
    }

    void run() override {
        for (int i = 0; i < NUM_CYCLES; i++) {
            if(i % NTH_CHECK == 0) {
                m_scheduler->blockUpdates();
                QTest::qSleep(1); // a bit of salt for crashiness ;)
                Q_ASSERT(!m_scheduler->haveUpdatesRunning());
                m_scheduler->unblockUpdates();
            }
            else {
                QRect updateRect(0,0,100,100);
                updateRect.moveTopLeft(QPoint((i%10)*100, (i%10)*100));
                m_scheduler->updateProjection(m_node, updateRect, QRect(0,0,1100,1100));
            }
        }
    }
private:
    KisUpdateScheduler *m_scheduler;
    KisNodeSP m_node;
};

void KisUpdateSchedulerTest::testBlockUpdates()
{
    KisImageSP image = buildTestingImage();
    KisNodeSP rootLayer = image->rootLayer();
    KisNodeSP paintLayer1 = rootLayer->firstChild();

    KisUpdateScheduler scheduler(image.data());

    QThreadPool threadPool;
    threadPool.setMaxThreadCount(NUM_THREADS);

    for(int i = 0; i< NUM_THREADS; i++) {
        UpdatesBlockTester *tester =
            new UpdatesBlockTester(&scheduler, paintLayer1);

        threadPool.start(tester);
    }

    threadPool.waitForDone();
}

#include "kis_update_time_monitor.h"

void KisUpdateSchedulerTest::testTimeMonitor()
{
    QVector<QRect> dirtyRects;

    KisUpdateTimeMonitor::instance()->startStrokeMeasure();
    KisUpdateTimeMonitor::instance()->reportMouseMove(QPointF(100, 0));

    KisUpdateTimeMonitor::instance()->reportJobStarted((void*) 10);
    QTest::qSleep(300);
    KisUpdateTimeMonitor::instance()->reportJobStarted((void*) 20);
    QTest::qSleep(100);
    dirtyRects << QRect(10,10,10,10);
    KisUpdateTimeMonitor::instance()->reportJobFinished((void*) 10, dirtyRects);
    QTest::qSleep(100);
    dirtyRects.clear();
    dirtyRects << QRect(30,30,10,10);
    KisUpdateTimeMonitor::instance()->reportJobFinished((void*) 20, dirtyRects);
    QTest::qSleep(500);
    KisUpdateTimeMonitor::instance()->reportUpdateFinished(QRect(10,10,10,10));
    QTest::qSleep(300);
    KisUpdateTimeMonitor::instance()->reportUpdateFinished(QRect(30,30,10,10));

    KisUpdateTimeMonitor::instance()->reportMouseMove(QPointF(130, 0));

    KisUpdateTimeMonitor::instance()->endStrokeMeasure();
}

void KisUpdateSchedulerTest::testLodSync()
{
    KisImageSP image = buildTestingImage();
    KisNodeSP rootLayer = image->root();
    KisNodeSP paintLayer1 = rootLayer->firstChild();

    QCOMPARE(paintLayer1->name(), QString("paint1"));

    image->setLodPreferences(KisLodPreferences(2));

    image->explicitRegenerateLevelOfDetail();

    image->waitForDone();
}

KISTEST_MAIN(KisUpdateSchedulerTest)

