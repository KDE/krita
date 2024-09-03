/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TransformStrokeStrategyTest.h"

#include <stroke_testing_utils.h>
#include <strokes/inplace_transform_stroke_strategy.h>

#include "kis_selection.h"

#include <testutil.h>
#include "kistest.h"
#include "kis_transform_mask.h"
#include "KisTransformMaskTestingListener.h"

#include "kis_paint_device_debug_utils.h"

#include "kis_transform_mask_params_factory_registry.h"
#include "kis_transform_mask_adapter.h"
#include "KisAnimatedTransformMaskParamsHolder.h"

namespace {

KisAnimatedTransformParamsHolderInterfaceSP createAnimatedParamsHolder(KisDefaultBoundsBaseSP defaultBounds)
{
    return toQShared(new KisAnimatedTransformMaskParamsHolder(defaultBounds));
}

}

void TransformStrokeStrategyTest::initTestCase()
{
    KisTransformMaskParamsFactoryRegistry::instance()->setAnimatedParamsHolderFactory(&createAnimatedParamsHolder);
    KisTransformMaskParamsFactoryRegistry::instance()->addFactory("tooltransformparams", &KisTransformMaskAdapter::fromXML);
    qRegisterMetaType<TransformTransactionProperties>("TransformTransactionProperties");
    qRegisterMetaType<ToolTransformArgs>("ToolTransformArgs");
}

struct TestPlan {
    using Data = KisTransformMaskTestingListener::Data;

    Data initializationPhase;
    Data jobsPhase;
    Data finalizationPhase;
};

class InplaceStrokeTester : public QObject, public utils::StrokeTester
{
    Q_OBJECT
public:

    InplaceStrokeTester(const QString &name)
        : utils::StrokeTester(name, QSize(512, 512))
    {
    }

    QPointF m_offset = QPointF(-2.5, 0);

    QVector<TestPlan> m_testPlan;
    int m_lodLevel = 0;

protected:

    void initImage(KisImageWSP image, KisNodeSP activeNode, int iteration) override
    {
        ENTER_FUNCTION();

        if (iteration == 0) {

            if (m_lodLevel > 0) {
                image->setLodPreferences(KisLodPreferences(KisLodPreferences::LodPreferred |
                                                               KisLodPreferences::LodSupported, 2));
            }

            QTest::qWait(500);

            image->waitForDone();
            m_node = activeNode;
            KisPaintDeviceSP dev = m_node->paintDevice();

            KoColor c(dev->colorSpace());

            c.fromQColor(Qt::green);
            dev->fill(QRect(300,300, 100, 100), c);

            c.fromQColor(Qt::red);
            dev->fill(QRect(310,300, 80, 10), c);

            m_transformMask = new KisTransformMask(image, "tmask");
            m_transformMask->setTestingInterface(new KisTransformMaskTestingListener());
            image->addNode(m_transformMask, m_node);

            m_transformMask->forceUpdateTimedNode();
            image->waitForDone();

            //KIS_DUMP_DEVICE_2(m_node->paintDevice(), QRect(0,0,512,512), "00_initial_pd", "dd");
            //KIS_DUMP_DEVICE_2(m_node->projection(), QRect(0,0,512,512), "01_initial_proj", "dd");

            m_offset = QPointF(-2.5, 0);

        } else if (iteration == 1) {
            m_offset = QPointF(0, -2.5);
        }

        // clear stats
        testingInterface()->clear();

        LEAVE_FUNCTION();
    }

    KisTransformMaskTestingListener* testingInterface() const {
        KisTransformMaskTestingListener *iface = dynamic_cast<KisTransformMaskTestingListener*>(m_transformMask->testingInterface());
        KIS_ASSERT(iface);
        return iface;
    }

    KisTransformMaskTestingListener::Data stats() const {
        return testingInterface()->stats();
    }

    KisTransformMaskTestingListener::Data takeStats() const {
        auto s = testingInterface()->stats();
        testingInterface()->clear();
        return s;
    }

    void addPaintingJobs(KisImageWSP image,
                         KisResourcesSnapshotSP resources,
                         int iteration) override
    {
        Q_UNUSED(iteration);
        Q_UNUSED(resources);

        while (m_transaction.transformedNodes().isEmpty()) {
            qDebug() << "Waiting for the transaction to appear...";
            QTest::qWait(5);
        }

        compareStats("INITIALIZATION", m_testPlan[iteration].initializationPhase);

        m_updatesHelper.startUpdateStream(image.data(), strokeId());

        for (int i = 0; i < 100; i++) {
            QTest::qWait(20);

            m_currentArgs.setTransformedCenter(m_currentArgs.transformedCenter() + m_offset);
            m_currentArgs.setAZ(m_currentArgs.aZ() + 1.0 * M_PI / 180.0);
            image->addJob(strokeId(),
                          new InplaceTransformStrokeStrategy::UpdateTransformData(
                              m_currentArgs,
                              InplaceTransformStrokeStrategy::UpdateTransformData::PAINT_DEVICE));
        }


        m_updatesHelper.endUpdateStream();


        /**
         * HINT: we cannot read data from the device at this point when working LoD is
         *       non-zero, because this LoD is actual only at the context of the
         *       jobs actually executing the update request
         */
        ENTER_FUNCTION() << ppVar(image->currentLevelOfDetail());
        QTest::qWait(200);
        //KIS_DUMP_DEVICE_2(m_node->paintDevice(), QRect(0,0,512,512), "80_semi-ended_pd", "dd");
        //KIS_DUMP_DEVICE_2(m_node->projection(), QRect(0,0,512,512), "81_semi-ended_proj", "dd");

        compareStats("JOBS", m_testPlan[iteration].jobsPhase);
    }


    void compareStats(const char *title, KisTransformMaskTestingListener::Data &expected)
    {
        KisTransformMaskTestingListener::Data data = takeStats();

        if (data != expected) {
            qDebug("FAIL: the update stats are not correct as stage \"%s\"", title);
            qDebug() << "   expected:" << expected;
            qDebug() << "   real    :" << data;
            qFatal("compared data is not the same!");
        }
    }

    void iterationEndedCallback(KisImageWSP image, KisNodeSP activeNode, int iteration) override
    {
        qDebug() << "Waiting for the final update...";
        /// the delay in the mask is 3000ms, so we should wait a little bit longer to catch
        /// spurious updates
        QTest::qWait(4000);
        qDebug() << "(done)";

        //KIS_DUMP_DEVICE_2(m_node->paintDevice(), QRect(0,0,512,512), "90_ended_pd", "dd");
        //KIS_DUMP_DEVICE_2(m_node->projection(), QRect(0,0,512,512), "91_ended_proj", "dd");

        compareStats("FINALIZATION", m_testPlan[iteration].finalizationPhase);
    }

    KisStrokeStrategy* createStroke(KisResourcesSnapshotSP resources,
                                    KisImageWSP image) override
    {
        Q_UNUSED(resources);

        const bool forceLodMode = true;

        InplaceTransformStrokeStrategy *strategy =
            new InplaceTransformStrokeStrategy(ToolTransformArgs::FREE_TRANSFORM,
                                               "Bilinear",
                                               false,
                                               {m_transformMask},
                                               nullptr,
                                               nullptr,
                                               image.data(),
                                               image.data(),
                                               image->root(),
                                               forceLodMode);

        m_currentArgs = ToolTransformArgs();
        m_transaction = TransformTransactionProperties(QRectF(), &m_currentArgs, KisNodeList(), {});

        connect(strategy, SIGNAL(sigTransactionGenerated(TransformTransactionProperties, ToolTransformArgs, void*)), SLOT(slotTransactionGenerated(TransformTransactionProperties, ToolTransformArgs, void*)));

        m_strokeStrategyCookie = strategy;
        return strategy;
    }

private Q_SLOTS:
    void slotTransactionGenerated(TransformTransactionProperties transaction, ToolTransformArgs args, void *strokeStrategyCookie)
    {
        KIS_ASSERT(strokeId());
        KIS_ASSERT(strokeStrategyCookie == m_strokeStrategyCookie);

        ENTER_FUNCTION() << ppVar(transaction.originalRect()) << ppVar(transaction.transformedNodes());
        qDebug() << "           " <<  ppVar(args.originalCenter()) << ppVar(args.transformedCenter());

        m_transaction = transaction;
        m_currentArgs = args;
        m_transaction.setCurrentConfigLocation(&m_currentArgs);
    }

private:
    ToolTransformArgs m_currentArgs;
    TransformTransactionProperties m_transaction;

    KisAsynchronousStrokeUpdateHelper m_updatesHelper;
    void *m_strokeStrategyCookie;

    KisNodeSP m_node;
    KisTransformMaskSP m_transformMask;
};

void TransformStrokeStrategyTest::testLod2()
{
    InplaceStrokeTester tester("inplace-lod2");
    tester.setNumIterations(2);
    tester.m_lodLevel = 2;

    {
        KisTransformMaskTestingListener::Data init;
        init.decorateRectTriggeredStaticImageUpdate = 0;
        init.slotDelayedStaticImageUpdate = 0;
        init.forceUpdateTimedNode = 1;
        init.threadSafeForceStaticImageUpdate = 0;
        init.recalculateStaticImage = 0;

        KisTransformMaskTestingListener::Data jobs;
        jobs.decorateRectTriggeredStaticImageUpdate = 0;
        jobs.slotDelayedStaticImageUpdate = 0;
        jobs.forceUpdateTimedNode = 0;
        jobs.threadSafeForceStaticImageUpdate = 0;
        jobs.recalculateStaticImage = 0;

        KisTransformMaskTestingListener::Data end;
        end.decorateRectTriggeredStaticImageUpdate = 0;
        end.slotDelayedStaticImageUpdate = 0;
        end.forceUpdateTimedNode = 0;
        end.threadSafeForceStaticImageUpdate = 1;
        end.recalculateStaticImage = 1;
        tester.m_testPlan << TestPlan{init, jobs, end};
    }

    {
        KisTransformMaskTestingListener::Data init;
        init.decorateRectTriggeredStaticImageUpdate = 0;
        init.slotDelayedStaticImageUpdate = 0;
        init.forceUpdateTimedNode = 1;
        init.threadSafeForceStaticImageUpdate = 1; // we have one more call due to undo() step in the commands
        init.recalculateStaticImage = 2;

        KisTransformMaskTestingListener::Data jobs;
        jobs.decorateRectTriggeredStaticImageUpdate = 0;
        jobs.slotDelayedStaticImageUpdate = 0;
        jobs.forceUpdateTimedNode = 0;
        jobs.threadSafeForceStaticImageUpdate = 0;
        jobs.recalculateStaticImage = 0;

        KisTransformMaskTestingListener::Data end;
        end.decorateRectTriggeredStaticImageUpdate = 0;
        end.slotDelayedStaticImageUpdate = 0;
        end.forceUpdateTimedNode = 0;
        end.threadSafeForceStaticImageUpdate = 1;
        end.recalculateStaticImage = 1;
        tester.m_testPlan << TestPlan{init, jobs, end};
    }

    tester.testSimpleStroke();
}

void TransformStrokeStrategyTest::testLod2Cancelled()
{
    InplaceStrokeTester tester("inplace-lod2-cancelled");
    tester.setNumIterations(1);
    tester.m_lodLevel = 2;

    {
        KisTransformMaskTestingListener::Data init;
        init.decorateRectTriggeredStaticImageUpdate = 0;
        init.slotDelayedStaticImageUpdate = 0;
        init.forceUpdateTimedNode = 1;
        init.threadSafeForceStaticImageUpdate = 0;
        init.recalculateStaticImage = 0;

        KisTransformMaskTestingListener::Data jobs;
        jobs.decorateRectTriggeredStaticImageUpdate = 0;
        jobs.slotDelayedStaticImageUpdate = 0;
        jobs.forceUpdateTimedNode = 0;
        jobs.threadSafeForceStaticImageUpdate = 0;
        jobs.recalculateStaticImage = 0;

        KisTransformMaskTestingListener::Data end;
        end.decorateRectTriggeredStaticImageUpdate = 0;
        end.slotDelayedStaticImageUpdate = 0;
        end.forceUpdateTimedNode = 0;
        end.threadSafeForceStaticImageUpdate = 2;
        end.recalculateStaticImage = 1;
        tester.m_testPlan << TestPlan{init, jobs, end};
    }

    tester.testSimpleStrokeCancelled();
}

void TransformStrokeStrategyTest::testLod2ContinueAndCancel()
{
    InplaceStrokeTester tester("inplace-lod2-continue-and-cancel");
    tester.setNumIterations(2);
    tester.setCancelOnIteration(1);
    tester.m_lodLevel = 2;

    {
        KisTransformMaskTestingListener::Data init;
        init.decorateRectTriggeredStaticImageUpdate = 0;
        init.slotDelayedStaticImageUpdate = 0;
        init.forceUpdateTimedNode = 1;
        init.threadSafeForceStaticImageUpdate = 0;
        init.recalculateStaticImage = 0;

        KisTransformMaskTestingListener::Data jobs;
        jobs.decorateRectTriggeredStaticImageUpdate = 0;
        jobs.slotDelayedStaticImageUpdate = 0;
        jobs.forceUpdateTimedNode = 0;
        jobs.threadSafeForceStaticImageUpdate = 0;
        jobs.recalculateStaticImage = 0;

        KisTransformMaskTestingListener::Data end;
        end.decorateRectTriggeredStaticImageUpdate = 0;
        end.slotDelayedStaticImageUpdate = 0;
        end.forceUpdateTimedNode = 0;
        end.threadSafeForceStaticImageUpdate = 1;
        end.recalculateStaticImage = 1;
        tester.m_testPlan << TestPlan{init, jobs, end};
    }

    {
        KisTransformMaskTestingListener::Data init;
        init.decorateRectTriggeredStaticImageUpdate = 0;
        init.slotDelayedStaticImageUpdate = 0;
        init.forceUpdateTimedNode = 1;
        init.threadSafeForceStaticImageUpdate = 1; // we have one more call due to undo() step in the commands
        init.recalculateStaticImage = 2;

        KisTransformMaskTestingListener::Data jobs;
        jobs.decorateRectTriggeredStaticImageUpdate = 0;
        jobs.slotDelayedStaticImageUpdate = 0;
        jobs.forceUpdateTimedNode = 0;
        jobs.threadSafeForceStaticImageUpdate = 0;
        jobs.recalculateStaticImage = 0;

        KisTransformMaskTestingListener::Data end;
        end.decorateRectTriggeredStaticImageUpdate = 0;
        end.slotDelayedStaticImageUpdate = 0;
        end.forceUpdateTimedNode = 0;
        end.threadSafeForceStaticImageUpdate = 2;
        end.recalculateStaticImage = 1;
        tester.m_testPlan << TestPlan{init, jobs, end};
    }

    tester.testSimpleStrokeCancelled();
}

void TransformStrokeStrategyTest::testLod0()
{
    InplaceStrokeTester tester("inplace-lod0");
    tester.setNumIterations(2);
    tester.m_lodLevel = 0;

    {
        KisTransformMaskTestingListener::Data init;
        init.decorateRectTriggeredStaticImageUpdate = 0;
        init.slotDelayedStaticImageUpdate = 0;
        init.forceUpdateTimedNode = 1;
        init.threadSafeForceStaticImageUpdate = 0;
        init.recalculateStaticImage = 0;

        KisTransformMaskTestingListener::Data jobs;
        jobs.decorateRectTriggeredStaticImageUpdate = 10000;
        jobs.slotDelayedStaticImageUpdate = 0;
        jobs.forceUpdateTimedNode = 0;
        jobs.threadSafeForceStaticImageUpdate = 0;
        jobs.recalculateStaticImage = 0;

        KisTransformMaskTestingListener::Data end;
        end.decorateRectTriggeredStaticImageUpdate = 0;
        end.slotDelayedStaticImageUpdate = 0;
        end.forceUpdateTimedNode = 0;
        end.threadSafeForceStaticImageUpdate = 0;
        end.recalculateStaticImage = 0;
        tester.m_testPlan << TestPlan{init, jobs, end};
    }

    {
        KisTransformMaskTestingListener::Data init;
        init.decorateRectTriggeredStaticImageUpdate = 0;
        init.slotDelayedStaticImageUpdate = 0;
        init.forceUpdateTimedNode = 1;
        init.threadSafeForceStaticImageUpdate = 1; // we have one more call due to undo() step in the commands
        init.recalculateStaticImage = 2;

        KisTransformMaskTestingListener::Data jobs;
        jobs.decorateRectTriggeredStaticImageUpdate = 10000;
        jobs.slotDelayedStaticImageUpdate = 0;
        jobs.forceUpdateTimedNode = 0;
        jobs.threadSafeForceStaticImageUpdate = 0;
        jobs.recalculateStaticImage = 0;

        KisTransformMaskTestingListener::Data end;
        end.decorateRectTriggeredStaticImageUpdate = 0;
        end.slotDelayedStaticImageUpdate = 0;
        end.forceUpdateTimedNode = 0;
        end.threadSafeForceStaticImageUpdate = 0;
        end.recalculateStaticImage = 0;
        tester.m_testPlan << TestPlan{init, jobs, end};
    }

    tester.testSimpleStroke();
}

void TransformStrokeStrategyTest::testLod0Cancelled()
{
    InplaceStrokeTester tester("inplace-lod0-cancelled");
    tester.setNumIterations(1);
    tester.m_lodLevel = 0;

    {
        KisTransformMaskTestingListener::Data init;
        init.decorateRectTriggeredStaticImageUpdate = 0;
        init.slotDelayedStaticImageUpdate = 0;
        init.forceUpdateTimedNode = 1;
        init.threadSafeForceStaticImageUpdate = 0;
        init.recalculateStaticImage = 0;

        KisTransformMaskTestingListener::Data jobs;
        jobs.decorateRectTriggeredStaticImageUpdate = 10000;
        jobs.slotDelayedStaticImageUpdate = 0;
        jobs.forceUpdateTimedNode = 0;
        jobs.threadSafeForceStaticImageUpdate = 0;
        jobs.recalculateStaticImage = 0;

        KisTransformMaskTestingListener::Data end;
        end.decorateRectTriggeredStaticImageUpdate = 1;
        end.slotDelayedStaticImageUpdate = 0;
        end.forceUpdateTimedNode = 0;
        end.threadSafeForceStaticImageUpdate = 2; // TODO: check why we get dirty?
        end.recalculateStaticImage = 1;
        tester.m_testPlan << TestPlan{init, jobs, end};
    }

    tester.testSimpleStrokeCancelled();
}

void TransformStrokeStrategyTest::testLod0ContinueAndCancel()
{
    InplaceStrokeTester tester("inplace-lod0-continue-and-cancel");
    tester.setNumIterations(2);
    tester.setCancelOnIteration(1);
    tester.m_lodLevel = 0;

    {
        KisTransformMaskTestingListener::Data init;
        init.decorateRectTriggeredStaticImageUpdate = 0;
        init.slotDelayedStaticImageUpdate = 0;
        init.forceUpdateTimedNode = 1;
        init.threadSafeForceStaticImageUpdate = 0;
        init.recalculateStaticImage = 0;

        KisTransformMaskTestingListener::Data jobs;
        jobs.decorateRectTriggeredStaticImageUpdate = 10000;
        jobs.slotDelayedStaticImageUpdate = 0;
        jobs.forceUpdateTimedNode = 0;
        jobs.threadSafeForceStaticImageUpdate = 0;
        jobs.recalculateStaticImage = 0;

        KisTransformMaskTestingListener::Data end;
        end.decorateRectTriggeredStaticImageUpdate = 0;
        end.slotDelayedStaticImageUpdate = 0;
        end.forceUpdateTimedNode = 0;
        end.threadSafeForceStaticImageUpdate = 0;
        end.recalculateStaticImage = 0;
        tester.m_testPlan << TestPlan{init, jobs, end};
    }

    {
        KisTransformMaskTestingListener::Data init;
        init.decorateRectTriggeredStaticImageUpdate = 0;
        init.slotDelayedStaticImageUpdate = 0;
        init.forceUpdateTimedNode = 1;
        init.threadSafeForceStaticImageUpdate = 1; // we have one more call due to undo() step in the commands
        init.recalculateStaticImage = 2;

        KisTransformMaskTestingListener::Data jobs;
        jobs.decorateRectTriggeredStaticImageUpdate = 10000;
        jobs.slotDelayedStaticImageUpdate = 0;
        jobs.forceUpdateTimedNode = 0;
        jobs.threadSafeForceStaticImageUpdate = 0;
        jobs.recalculateStaticImage = 0;

        KisTransformMaskTestingListener::Data end;
        end.decorateRectTriggeredStaticImageUpdate = 1;
        end.slotDelayedStaticImageUpdate = 0;
        end.forceUpdateTimedNode = 0;
        end.threadSafeForceStaticImageUpdate = 1;
        end.recalculateStaticImage = 1;
        tester.m_testPlan << TestPlan{init, jobs, end};
    }

    tester.testSimpleStrokeCancelled();
}

#define TESTRESOURCES
#define TESTPIGMENT
#define TESTIMAGE
#define TESTBRUSH
#define TESTUI

KISTEST_MAIN(TransformStrokeStrategyTest)

#include <TransformStrokeStrategyTest.moc>
