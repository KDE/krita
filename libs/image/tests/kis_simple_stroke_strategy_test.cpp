/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_simple_stroke_strategy_test.h"
#include <QTest>

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

QTEST_MAIN(KisSimpleStrokeStrategyTest)
