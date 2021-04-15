/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_simple_stroke_strategy_test.h"
#include <simpletest.h>

#include "kis_image.h"
#include "kis_simple_stroke_strategy.h"


class TestingSimpleStrokeStrategy : public KisSimpleStrokeStrategy
{
public:
    TestingSimpleStrokeStrategy()
        : KisSimpleStrokeStrategy(QLatin1String("TestingSimpleStrokeStrategy")),
          m_stageCounter(0)
    {
        enableJob(KisSimpleStrokeStrategy::JOB_INIT);
        enableJob(KisSimpleStrokeStrategy::JOB_FINISH);
        enableJob(KisSimpleStrokeStrategy::JOB_CANCEL);
        enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);
    }

    ~TestingSimpleStrokeStrategy() override {
        QCOMPARE(m_stageCounter, 3);
    }

    void initStrokeCallback() override {
        QCOMPARE(m_stageCounter, 0);
        m_stageCounter++;
    }

    void finishStrokeCallback() override {
        QCOMPARE(m_stageCounter, 2);
        m_stageCounter++;
    }

    void cancelStrokeCallback() override {
        QCOMPARE(m_stageCounter, 2);
        m_stageCounter++;
    }

    void doStrokeCallback(KisStrokeJobData *data) override {
        Q_UNUSED(data);

        QCOMPARE(m_stageCounter, 1);
        m_stageCounter++;
    }

private:
    int m_stageCounter;
};

void KisSimpleStrokeStrategyTest::testFinish()
{
    KisImageSP image = new KisImage(0, 100, 100, 0, "test executor image");

    KisStrokeId id = image->startStroke(new TestingSimpleStrokeStrategy());
    image->addJob(id, 0);
    image->endStroke(id);
    image->waitForDone();
}

void KisSimpleStrokeStrategyTest::testCancel()
{
    KisImageSP image = new KisImage(0, 100, 100, 0, "test executor image");

    KisStrokeId id = image->startStroke(new TestingSimpleStrokeStrategy());
    image->addJob(id, 0);

    /**
     * We add a delay to ensure the job is finished before the cancel
     * is requested, the cancel job will abort it otherwise
     */
    QTest::qSleep(500);

    image->cancelStroke(id);
    image->waitForDone();
}

SIMPLE_TEST_MAIN(KisSimpleStrokeStrategyTest)
