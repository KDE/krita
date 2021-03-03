/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_updater_context_test.h"
#include <simpletest.h>

#include "kistest.h"

#include <QAtomicInt>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_paint_layer.h"

#include "kis_merge_walker.h"
#include "kis_updater_context.h"
#include "kis_image.h"

#include "scheduler_utils.h"

#include "lod_override.h"
#include "config-limit-long-tests.h"

void KisUpdaterContextTest::testJobInterference()
{
    KisTestableUpdaterContext context(3);

    QRect imageRect(0,0,100,100);

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, imageRect.width(), imageRect.height(), cs, "merge test");

    KisPaintLayerSP paintLayer = new KisPaintLayer(image, "test", OPACITY_OPAQUE_U8);

    image->barrierLock();
    image->addNode(paintLayer);
    image->unlock();

    QRect dirtyRect1(0,0,50,100);
    KisBaseRectsWalkerSP walker1 = new KisMergeWalker(imageRect);
    walker1->collectRects(paintLayer, dirtyRect1);

    context.lock();
    context.addMergeJob(walker1);
    context.unlock();

    // overlapping job --- forbidden
    {
        QRect dirtyRect(30,0,100,100);
        KisBaseRectsWalkerSP walker = new KisMergeWalker(imageRect);
        walker->collectRects(paintLayer, dirtyRect);

        context.lock();
        QVERIFY(!context.isJobAllowed(walker));
        context.unlock();
    }

    // not overlapping job --- allowed
    {
        QRect dirtyRect(60,0,100,100);
        KisBaseRectsWalkerSP walker = new KisMergeWalker(imageRect);
        walker->collectRects(paintLayer, dirtyRect);

        context.lock();
        QVERIFY(context.isJobAllowed(walker));
        context.unlock();
    }

    // not overlapping job, conflicting LOD --- forbidden
    {
        TestUtil::LodOverride l(1, image);

        QCOMPARE(paintLayer->paintDevice()->defaultBounds()->currentLevelOfDetail(), 1);

        QRect dirtyRect(60,0,100,100);
        KisBaseRectsWalkerSP walker = new KisMergeWalker(imageRect);
        walker->collectRects(paintLayer, dirtyRect);

        context.lock();
        QVERIFY(!context.isJobAllowed(walker));
        context.unlock();
    }
}

void KisUpdaterContextTest::testSnapshot()
{
    KisTestableUpdaterContext context(3);

    QRect imageRect(0,0,100,100);

    KisBaseRectsWalkerSP walker1 = new KisMergeWalker(imageRect);

    qint32 numMergeJobs = -777;
    qint32 numStrokeJobs = -777;

    context.lock();

    context.getJobsSnapshot(numMergeJobs, numStrokeJobs);
    QCOMPARE(numMergeJobs, 0);
    QCOMPARE(numStrokeJobs, 0);
    QCOMPARE(context.currentLevelOfDetail(), -1);

    context.addMergeJob(walker1);
    context.getJobsSnapshot(numMergeJobs, numStrokeJobs);
    QCOMPARE(numMergeJobs, 1);
    QCOMPARE(numStrokeJobs, 0);
    QCOMPARE(context.currentLevelOfDetail(), 0);


    KisStrokeJobData *data =
        new KisStrokeJobData(KisStrokeJobData::SEQUENTIAL,
                             KisStrokeJobData::NORMAL);

    QScopedPointer<KisStrokeJobStrategy> strategy(
        new KisNoopDabStrategy("test"));

    context.addStrokeJob(new KisStrokeJob(strategy.data(), data, 0, true));
    context.getJobsSnapshot(numMergeJobs, numStrokeJobs);
    QCOMPARE(numMergeJobs, 1);
    QCOMPARE(numStrokeJobs, 1);
    QCOMPARE(context.currentLevelOfDetail(), 0);


    context.addSpontaneousJob(new KisNoopSpontaneousJob());
    context.getJobsSnapshot(numMergeJobs, numStrokeJobs);
    QCOMPARE(numMergeJobs, 2);
    QCOMPARE(numStrokeJobs, 1);
    QCOMPARE(context.currentLevelOfDetail(), 0);

    context.unlock();

    {
        context.lock();
        context.clear();

        context.getJobsSnapshot(numMergeJobs, numStrokeJobs);
        QCOMPARE(numMergeJobs, 0);
        QCOMPARE(numStrokeJobs, 0);
        QCOMPARE(context.currentLevelOfDetail(), -1);

        data =
            new KisStrokeJobData(KisStrokeJobData::SEQUENTIAL,
                                 KisStrokeJobData::NORMAL);

        context.addStrokeJob(new KisStrokeJob(strategy.data(), data, 2, true));
        context.getJobsSnapshot(numMergeJobs, numStrokeJobs);
        QCOMPARE(numMergeJobs, 0);
        QCOMPARE(numStrokeJobs, 1);
        QCOMPARE(context.currentLevelOfDetail(), 2);

        context.unlock();
    }
}

#define NUM_THREADS 10
#ifdef LIMIT_LONG_TESTS
#   define NUM_JOBS 60
#else
#   define NUM_JOBS 6000
#endif
#define EXCLUSIVE_NTH 3
#define NUM_CHECKS 10
#define CHECK_DELAY 3 // ms

class ExclusivenessCheckerStrategy : public KisStrokeJobStrategy
{
public:
    ExclusivenessCheckerStrategy(QAtomicInt &counter,
                                 QAtomicInt &hadConcurrency)
        : m_counter(counter),
          m_hadConcurrency(hadConcurrency)
    {
    }

    void run(KisStrokeJobData *data) override {
        Q_UNUSED(data);

        m_counter.ref();

        for(int i = 0; i < NUM_CHECKS; i++) {
            if(data->isExclusive()) {
                Q_ASSERT(m_counter == 1);
            }
            else if (m_counter > 1) {
                m_hadConcurrency.ref();
            }
            QTest::qSleep(CHECK_DELAY);
        }

        m_counter.deref();
    }

    QString debugId() const override {
        return "ExclusivenessCheckerStrategy";
    }

private:
    QAtomicInt &m_counter;
    QAtomicInt &m_hadConcurrency;
};

void KisUpdaterContextTest::stressTestExclusiveJobs()
{
    KisUpdaterContext context(NUM_THREADS);
    QAtomicInt counter;
    QAtomicInt hadConcurrency;

    for(int i = 0; i < NUM_JOBS; i++) {
        if(context.hasSpareThread()) {
            bool isExclusive = i % EXCLUSIVE_NTH == 0;

            KisStrokeJobData *data =
                new KisStrokeJobData(KisStrokeJobData::SEQUENTIAL,
                                     isExclusive ?
                                     KisStrokeJobData::EXCLUSIVE :
                                     KisStrokeJobData::NORMAL);

            KisStrokeJobStrategy *strategy =
                new ExclusivenessCheckerStrategy(counter, hadConcurrency);

            context.addStrokeJob(new KisStrokeJob(strategy, data, 0, true));
        }
        else {
            QTest::qSleep(CHECK_DELAY);
        }
    }

    context.waitForDone();

    QVERIFY(!counter);
    dbgKrita << "Concurrency observed:" << hadConcurrency
             << "/" << NUM_CHECKS * NUM_JOBS;
}

KISTEST_MAIN(KisUpdaterContextTest)

