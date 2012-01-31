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

#include "kis_image_signal_router_test.h"

#include <qtest_kde.h>
#include "kis_image_signal_router.h"
#include "kis_image.h"

#define NUMCYCLES 100
#define CYCLE_DELAY 1


class AsyncSignalProducer : public QThread
{
public:
    AsyncSignalProducer(KisImageSignalRouter &router, SignalConsumer &consumer)
        : m_router(router),
          m_consumer(consumer)
    {
    }

    void run() {
        for(int i = 0; i < NUMCYCLES; i++) {
            m_router.emitNotification(LayersChangedSignal);
            QTest::qSleep(CYCLE_DELAY);
            m_router.emitNodeChanged(0);
            QTest::qSleep(CYCLE_DELAY);
            m_router.emitNodeChanged(0);
            QTest::qSleep(CYCLE_DELAY);

            m_router.emitAboutToAddANode(0,i+1);
            Q_ASSERT(m_consumer.getChecker() == i+1);
            QTest::qSleep(CYCLE_DELAY);

            m_router.emitNodeHasBeenAdded(0,i+1);
            QTest::qSleep(CYCLE_DELAY);
            m_router.emitAboutToRemoveANode(0,0);
            QTest::qSleep(CYCLE_DELAY);
            m_router.emitNodeHasBeenRemoved(0,0);
            QTest::qSleep(CYCLE_DELAY);
        }
    }

private:
    KisImageSignalRouter &m_router;
    SignalConsumer &m_consumer;
};

#define CONNECT_FROM_IMAGE(signal)                              \
    connect(image, SIGNAL(signal), SLOT(slotAcceptSignal()))


SignalConsumer::SignalConsumer(KisImageWSP image)
{


    CONNECT_FROM_IMAGE(sigLayersChanged(KisGroupLayerSP));
    CONNECT_FROM_IMAGE(sigNodeChanged(KisNode*));

    connect(image, SIGNAL(sigAboutToAddANode(KisNode*, int)),
            SLOT(slotAcceptSetCheckerSignal(KisNode*, int)));

    CONNECT_FROM_IMAGE(sigNodeHasBeenAdded(KisNode*, int));
    CONNECT_FROM_IMAGE(sigAboutToRemoveANode(KisNode*, int));
    CONNECT_FROM_IMAGE(sigNodeHasBeenRemoved(KisNode*, int));
}

int SignalConsumer::signalsCounter() const {
    return m_signalsCounter;
}

int SignalConsumer::getChecker() const {
    return m_checker;
}

void SignalConsumer::slotAcceptSignal() {
    m_signalsCounter.ref();
    m_intrusionCounter.ref();
    Q_ASSERT(m_intrusionCounter == 1);
    QTest::qSleep(CYCLE_DELAY);
    Q_ASSERT(m_intrusionCounter == 1);
    m_intrusionCounter.deref();
}

void SignalConsumer::slotAcceptSetCheckerSignal(KisNode*, int value) {
    slotAcceptSignal();
    if(value > 0) {
        m_checker = value;
    }
}


void KisImageSignalRouterTest::testSequentiality()
{
    KisImageSP image = new KisImage(0, 100, 100, 0, "testimage");
    KisImageSignalRouter router(image);

    SignalConsumer consumer(image);
    AsyncSignalProducer producer(router, consumer);

    producer.start();


    for(int i = 0; i < NUMCYCLES; i++) {
        router.emitNotification(LayersChangedSignal);
        QTest::qWait(CYCLE_DELAY);
        router.emitNodeChanged(0);
        QTest::qWait(CYCLE_DELAY);
        router.emitNodeChanged(0);
        QTest::qWait(CYCLE_DELAY);
        router.emitAboutToAddANode(0,0);
        QTest::qWait(CYCLE_DELAY);
        router.emitNodeHasBeenAdded(0,0);
        QTest::qWait(CYCLE_DELAY);
        router.emitAboutToRemoveANode(0,0);
        QTest::qWait(CYCLE_DELAY);
        router.emitNodeHasBeenRemoved(0,0);
        QTest::qWait(CYCLE_DELAY);
    }

    while(!producer.wait(CYCLE_DELAY)) qApp->processEvents();

    QCOMPARE(consumer.signalsCounter(), 2 * 9 * NUMCYCLES);
}

QTEST_KDEMAIN(KisImageSignalRouterTest, GUI)
