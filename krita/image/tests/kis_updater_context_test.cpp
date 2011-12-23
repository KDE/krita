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

#include "kis_updater_context_test.h"
#include <qtest_kde.h>

#include <QAtomicInt>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_paint_layer.h"

#include "kis_merge_walker.h"
#include "kis_updater_context.h"
#include "kis_image.h"

void KisUpdaterContextTest::testJobInterference()
{
    KisTestableUpdaterContext context(3);

    QRect imageRect(0,0,100,100);

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, imageRect.width(), imageRect.height(), cs, "merge test");

    KisPaintLayerSP paintLayer = new KisPaintLayer(image, "test", OPACITY_OPAQUE_U8);

    image->lock();
    image->addNode(paintLayer);

    QRect dirtyRect1(0,0,50,100);
    KisBaseRectsWalkerSP walker1 = new KisMergeWalker(imageRect);
    walker1->collectRects(paintLayer, dirtyRect1);

    QRect dirtyRect2(30,0,100,100);
    KisBaseRectsWalkerSP walker2 = new KisMergeWalker(imageRect);
    walker2->collectRects(paintLayer, dirtyRect2);

    context.lock();
    context.addMergeJob(walker1);

    QVERIFY(!context.isJobAllowed(walker2));

    context.unlock();
}

#define NUM_THREADS 10
#define NUM_JOBS 6000
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

    void run(KisStrokeJobData *data) {
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

            context.addStrokeJob(new KisStrokeJob(strategy, data));
        }
        else {
            QTest::qSleep(CHECK_DELAY);
        }
    }

    context.waitForDone();

    QVERIFY(!counter);
    qDebug() << "Concurrency observed:" << hadConcurrency
             << "/" << NUM_CHECKS * NUM_JOBS;
}

QTEST_KDEMAIN(KisUpdaterContextTest, NoGUI)
#include "kis_updater_context_test.moc"

