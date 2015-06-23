/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_image_animation_interface_test.h"

#include <qtest_kde.h>

#include <testutil.h>
#include <KoColor.h>

#include "kis_debug.h"

#include "kis_image_animation_interface.h"
#include "kis_signal_compressor_with_param.h"


void checkFrame(KisImageAnimationInterface *i, int frameId, const QRect &rc)
{
    qDebug() << ppVar(i->currentTime()) << ppVar(i->frameProjection()->exactBounds());

    QCOMPARE(i->currentTime(), frameId);
    QCOMPARE(i->frameProjection()->exactBounds(), rc);
}

void KisImageAnimationInterfaceTest::test()
{
    QRect refRect(QRect(0,0,512,512));
    TestUtil::MaskParent p(refRect);

    KisPaintLayerSP layer2 = new KisPaintLayer(p.image, "paint2", OPACITY_OPAQUE_U8);
    p.image->addNode(layer2);

    const QRect rc1(100,100,100,100);
    const QRect rc2(102,102,102,102);
    const QRect rc3(103,103,103,103);
    const QRect rc4(104,104,104,104);

    KisImageAnimationInterface *i = p.image->animationInterface();
    KisPaintDeviceSP dev1 = p.layer->paintDevice();
    KisPaintDeviceSP dev2 = layer2->paintDevice();

    {
        QCOMPARE(dev1->defaultBounds()->currentTime(), 0);
        QCOMPARE(dev2->defaultBounds()->currentTime(), 0);
        QCOMPARE(i->currentTime(), 0);

        dev1->fill(rc1, KoColor(Qt::red, dev1->colorSpace()));
        QCOMPARE(dev1->exactBounds(), rc1);

        dev2->fill(rc2, KoColor(Qt::green, dev1->colorSpace()));
        QCOMPARE(dev2->exactBounds(), rc2);

        p.image->refreshGraph();
        QCOMPARE(i->frameProjection()->defaultBounds()->currentTime(), 0);
        QCOMPARE(i->frameProjection()->exactBounds(), rc1 | rc2);
    }

    i->switchCurrentTime(10);

    KisKeyframeChannel *channel1 = dev1->keyframeChannel();
    channel1->addKeyframe(10);

    KisKeyframeChannel *channel2 = dev2->keyframeChannel();
    channel2->addKeyframe(10);


    {
        QCOMPARE(dev1->defaultBounds()->currentTime(), 10);
        QCOMPARE(dev2->defaultBounds()->currentTime(), 10);
        QCOMPARE(i->currentTime(), 10);

        QVERIFY(dev1->exactBounds().isEmpty());
        QVERIFY(dev2->exactBounds().isEmpty());

        dev1->fill(rc3, KoColor(Qt::red, dev2->colorSpace()));
        QCOMPARE(dev1->exactBounds(), rc3);

        dev2->fill(rc4, KoColor(Qt::green, dev2->colorSpace()));
        QCOMPARE(dev2->exactBounds(), rc4);

        p.image->refreshGraph();
        QCOMPARE(i->frameProjection()->defaultBounds()->currentTime(), 10);
        QCOMPARE(i->frameProjection()->exactBounds(), rc3 | rc4);
    }

    SignalToFunctionProxy proxy1(boost::bind(checkFrame, i, 0, rc1 | rc2));
    connect(i, SIGNAL(sigFrameReady(bool)), &proxy1, SLOT(start()), Qt::DirectConnection);
    i->requestFrame(0, false);
    QTest::qWait(200);

    i->switchCurrentTime(0);

    {
        QCOMPARE(dev1->defaultBounds()->currentTime(), 0);
        QCOMPARE(dev2->defaultBounds()->currentTime(), 0);
        QCOMPARE(i->currentTime(), 0);

        QCOMPARE(dev1->exactBounds(), rc1);
        QCOMPARE(dev2->exactBounds(), rc2);

        QCOMPARE(i->frameProjection()->defaultBounds()->currentTime(), 0);
        QCOMPARE(i->frameProjection()->exactBounds(), rc1 | rc2);
    }

    SignalToFunctionProxy proxy2(boost::bind(checkFrame, i, 10, rc3 | rc4));
    connect(i, SIGNAL(sigFrameReady(bool)), &proxy2, SLOT(start()), Qt::DirectConnection);
    i->requestFrame(10, false);
    QTest::qWait(200);
}

QTEST_KDEMAIN(KisImageAnimationInterfaceTest, GUI)
