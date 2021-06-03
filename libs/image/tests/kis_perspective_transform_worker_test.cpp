/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_perspective_transform_worker_test.h"

#include <simpletest.h>

#include <testutil.h>

#define USE_DOCUMENT 0
#include "qimage_based_test.h"

#include "kis_perspectivetransform_worker.h"
#include "kis_transaction.h"


class PerspectiveWorkerTester : public TestUtil::QImageBasedTest
{
public:
    PerspectiveWorkerTester()
        : QImageBasedTest("perspective_worker_test")
    {
        KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
        image = createImage(undoStore);
        image->initialRefreshGraph();

        QVERIFY(checkLayersInitial(image));
    }

    KisPaintDeviceSP paintDevice() {
        return findNode(image->root(), "paint1")->paintDevice();
    }

    void checkLayer(const QString &testName) {
        KisNodeSP node = findNode(image->root(), "paint1");
        QVERIFY(checkOneLayer(image, node, testName, 3));
    }


    KisImageSP image;
};


void KisPerspectiveTransformWorkerTest::testSimpleTransform()
{
    PerspectiveWorkerTester t;
    KisPaintDeviceSP dev = t.paintDevice();

    QPointF dx(326, 214);
    qreal aX = 1.32;
    qreal aY = 0.8;
    qreal z = 1024;

    KisPerspectiveTransformWorker worker(dev, dx, aX, aY, z, true, 0);
    worker.run();

    t.checkLayer("simple_transform");
}

SIMPLE_TEST_MAIN(KisPerspectiveTransformWorkerTest)
