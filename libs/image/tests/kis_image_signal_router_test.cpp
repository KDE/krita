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

#include <QTest>


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
    checkNotification(ModifiedSignal, SIGNAL(sigImageModified()));
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

QTEST_MAIN(KisImageSignalRouterTest)
