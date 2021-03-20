/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_image_signal_router_test.h"

#include <simpletest.h>


inline void KisImageSignalRouterTest::checkNotification(KisImageSignalType notification, const char *signal)
{
    QSignalSpy *spy = new QSignalSpy(m_image.data(), signal);
    QCOMPARE(spy->count(), 0);
    m_image->signalRouter()->emitNotification(notification);
    QCOMPARE(spy->count(), 1);
    delete spy;
}

#define checkComplexSignal(method, signal)                      \
    {                                                           \
    QSignalSpy *spy = new QSignalSpy(m_image.data(), signal);   \
    QCOMPARE(spy->count(), 0);                                  \
    m_image->signalRouter()->method;                            \
    QCOMPARE(spy->count(), 1);                                  \
    delete spy;                                                 \
    }

void KisImageSignalRouterTest::init()
{
    initBase();
    constructImage();
}

void KisImageSignalRouterTest::cleanup()
{
    cleanupBase();
}

void KisImageSignalRouterTest::testSignalForwarding()
{

    checkNotification(LayersChangedSignal, SIGNAL(sigLayersChangedAsync()));
    checkNotification(SizeChangedSignal, SIGNAL(sigSizeChanged(QPointF,QPointF)));
    checkNotification(ComplexSizeChangedSignal(), SIGNAL(sigSizeChanged(QPointF,QPointF)));
// These cannot be checked because KoColorProfile and KoColorSpace are not registered metatypes,
// and cannot be registered as metatypes because they are abstract classes.
//    checkNotification(ProfileChangedSignal, SIGNAL(sigProfileChanged(const KoColorProfile*)));
//    checkNotification(ColorSpaceChangedSignal, SIGNAL(sigColorSpaceChanged(const KoColorSpace*)));
    checkNotification(ResolutionChangedSignal, SIGNAL(sigResolutionChanged(double,double)));

    checkComplexSignal(emitNodeChanged(m_layer1.data()), SIGNAL(sigNodeChanged(KisNodeSP)));
    checkComplexSignal(emitNodeHasBeenAdded(m_layer3.data(),0), SIGNAL(sigNodeAddedAsync(KisNodeSP)));
    checkComplexSignal(emitAboutToRemoveANode(m_layer3.data(),0), SIGNAL(sigRemoveNodeAsync(KisNodeSP)));
}

SIMPLE_TEST_MAIN(KisImageSignalRouterTest)
